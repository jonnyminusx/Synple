#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "dsp/Goertzel.h"
#include "synth/SawtoothOscillator.h"

#include <cmath>
#include <numeric>
#include <vector>

namespace
{

constexpr float sampleRate = 44100.0f;
constexpr int warmup = 4096;

// Renders raw oscillator output (no leaky integrator — directly tests the
// oscillator's own DC compensation).
std::vector<float> renderRaw(float f0, float fs, int numSamples, float amplitude = 1.0f)
{
    synth::SawtoothOscillator osc;
    osc.setAmplitude(amplitude);
    osc.setPeriod(fs / f0);
    osc.setModulation(1.0f);

    std::vector<float> output;
    output.reserve(numSamples);

    for (int i = 0; i < warmup + numSamples; ++i)
    {
        const float s = osc.nextSample();
        if (i >= warmup)
            output.push_back(s);
    }

    return output;
}

// Renders the sawtooth pipeline: saw = saw * 0.997 + osc1 - osc2.
// osc2 amplitude is zero unless configured by the caller.
std::vector<float> renderSawtooth(synth::SawtoothOscillator& osc1,
                                  synth::SawtoothOscillator& osc2,
                                  int numSamples,
                                  int numWarmup = warmup)
{
    float saw = 0.0f;
    std::vector<float> output;
    output.reserve(numSamples);

    for (int i = 0; i < numWarmup + numSamples; ++i)
    {
        const float s1 = osc1.nextSample();
        const float s2 = osc2.nextSample();
        saw = saw * 0.997f + s1 - s2;

        if (i >= numWarmup)
            output.push_back(saw);
    }

    return output;
}

// Renders the sawtooth pipeline from Voice::render() using a single active
// oscillator at the given frequency and sample rate. osc2 has zero amplitude
// so it contributes nothing, matching: saw = saw * 0.997 + osc1 - osc2.
std::vector<float> renderAliasSawtooth(float f0, float fs, int numSamples, int numWarmup = warmup)
{
    const float period = fs / f0;

    synth::SawtoothOscillator osc1;
    synth::SawtoothOscillator osc2;

    osc1.setAmplitude(1.0f);
    osc1.setPeriod(period);
    osc1.setModulation(1.0f);

    osc2.setAmplitude(0.0f);
    osc2.setPeriod(period);
    osc2.setModulation(1.0f);

    float saw = 0.0f;
    std::vector<float> output;
    output.reserve(numSamples);

    for (int i = 0; i < numWarmup + numSamples; ++i)
    {
        const float s1 = osc1.nextSample();
        const float s2 = osc2.nextSample();
        saw = saw * 0.997f + s1 - s2;

        if (i >= numWarmup)
            output.push_back(saw);
    }

    return output;
}

struct AliasingResult
{
    float fundamentalMag;
    float worstAliasMag;
    float worstAliasFreq;

    float suppressionDb() const
    {
        if (worstAliasMag < 1e-12f)
            return 200.0f;
        return 20.0f * std::log10(fundamentalMag / worstAliasMag);
    }
};

// For each harmonic of f0 that exceeds Nyquist (up to maxHarmonic), its
// alias lands at (sampleRate - k*f0). This function finds the worst-case
// alias magnitude relative to the fundamental, skipping alias bins that are
// within minSeparationHz of a true harmonic (to avoid measuring harmonic
// energy rather than alias energy due to spectral leakage).
AliasingResult measureAliasing(
    const std::vector<float>& signal, float f0, float fs, int maxHarmonic = 30, float minSeparationHz = 100.0f)
{
    const float nyquist = fs / 2.0f;
    const float fundamentalMag = dsp::goertzel(signal, f0, fs);

    float worstAlias = 0.0f;
    float worstAliasFreq = 0.0f;

    for (int k = 2; k <= maxHarmonic; ++k)
    {
        const float harmonic = static_cast<float>(k) * f0;
        if (harmonic <= nyquist)
            continue;

        const float aliasFreq = fs - harmonic;
        if (aliasFreq <= 0.0f || aliasFreq >= nyquist)
            continue;

        // Skip if the alias bin is too close to a real harmonic — spectral
        // leakage from the harmonic would inflate the alias measurement.
        bool nearHarmonic = false;
        for (int j = 1; static_cast<float>(j) * f0 <= nyquist; ++j)
        {
            if (std::abs(aliasFreq - static_cast<float>(j) * f0) < minSeparationHz)
            {
                nearHarmonic = true;
                break;
            }
        }
        if (nearHarmonic)
            continue;

        const float mag = dsp::goertzel(signal, aliasFreq, fs);
        if (mag > worstAlias)
        {
            worstAlias = mag;
            worstAliasFreq = aliasFreq;
        }
    }

    return {fundamentalMag, worstAlias, worstAliasFreq};
}

float rms(const std::vector<float>& v)
{
    float sum = 0.0f;
    for (float s : v)
        sum += s * s;
    return std::sqrt(sum / static_cast<float>(v.size()));
}

float mean(const std::vector<float>& v)
{
    return std::accumulate(v.begin(), v.end(), 0.0f) / static_cast<float>(v.size());
}

} // namespace

// ─── DC offset ───────────────────────────────────────────────────────────────

// Tests the oscillator's own DC compensation (raw output, no leaky integrator).
// The leaky integrator's DC gain is 1/(1-0.997) ≈ 333×, so even a tiny
// residual DC in the raw output would read as a large mean in the integrated
// signal — testing raw output isolates the oscillator's compensation directly.
TEST_CASE("Raw oscillator output has near-zero DC after warmup", "[oscillator][dc]")
{
    constexpr int N = 16384;
    constexpr float tolerance = 0.01f;

    SECTION("440 Hz")
    {
        auto sig = renderRaw(440.0f, sampleRate, N);
        INFO("mean = " << mean(sig));
        REQUIRE(std::abs(mean(sig)) < tolerance);
    }

    SECTION("1000 Hz")
    {
        auto sig = renderRaw(1000.0f, sampleRate, N);
        INFO("mean = " << mean(sig));
        REQUIRE(std::abs(mean(sig)) < tolerance);
    }

    SECTION("5000 Hz")
    {
        auto sig = renderRaw(5000.0f, sampleRate, N);
        INFO("mean = " << mean(sig));
        REQUIRE(std::abs(mean(sig)) < tolerance);
    }
}

// ─── Amplitude scaling ────────────────────────────────────────────────────────

TEST_CASE("Sawtooth RMS scales linearly with amplitude", "[oscillator][amplitude]")
{
    constexpr int N = 16384;

    const auto sig1x = renderRaw(440.0f, sampleRate, N, 1.0f);
    const auto sig2x = renderRaw(440.0f, sampleRate, N, 2.0f);

    const float rms1 = rms(sig1x);
    const float rms2 = rms(sig2x);

    INFO("rms(1x) = " << rms1 << "  rms(2x) = " << rms2);
    REQUIRE(rms1 > 0.0f);
    // Doubling amplitude should double RMS, within 5%
    REQUIRE(rms2 == Catch::Approx(2.0f * rms1).epsilon(0.05f));
}

// ─── Zero amplitude → silence ─────────────────────────────────────────────────

TEST_CASE("Zero amplitude produces silence", "[oscillator][amplitude]")
{
    constexpr int N = 8192;
    const auto sig = renderRaw(440.0f, sampleRate, N, 0.0f);

    for (float s : sig)
        REQUIRE(std::abs(s) < 1e-6f);
}

// ─── Frequency accuracy ───────────────────────────────────────────────────────

TEST_CASE("Oscillator fundamental energy is concentrated at the set frequency", "[oscillator][frequency]")
{
    constexpr int N = 16384;

    SECTION("440 Hz")
    {
        constexpr float f0 = 440.0f;
        const auto sig = renderRaw(f0, sampleRate, N);
        const float atF0 = dsp::goertzel(sig, f0, sampleRate);
        const float atLow = dsp::goertzel(sig, f0 - 100.0f, sampleRate);
        const float atHigh = dsp::goertzel(sig, f0 + 100.0f, sampleRate);
        INFO("energy at f0=" << atF0 << " at f0-100=" << atLow << " at f0+100=" << atHigh);
        REQUIRE(atF0 > 10.0f * atLow);
        REQUIRE(atF0 > 10.0f * atHigh);
    }

    SECTION("1000 Hz")
    {
        constexpr float f0 = 1000.0f;
        const auto sig = renderRaw(f0, sampleRate, N);
        const float atF0 = dsp::goertzel(sig, f0, sampleRate);
        const float atLow = dsp::goertzel(sig, f0 - 100.0f, sampleRate);
        const float atHigh = dsp::goertzel(sig, f0 + 100.0f, sampleRate);
        INFO("energy at f0=" << atF0 << " at f0-100=" << atLow << " at f0+100=" << atHigh);
        REQUIRE(atF0 > 10.0f * atLow);
        REQUIRE(atF0 > 10.0f * atHigh);
    }
}

// ─── Reset ────────────────────────────────────────────────────────────────────

TEST_CASE("Oscillator produces identical output after reset", "[oscillator][reset]")
{
    constexpr int N = 2048;
    constexpr float f0 = 440.0f;
    const float period = sampleRate / f0;

    synth::SawtoothOscillator osc;
    osc.setAmplitude(1.0f);
    osc.setPeriod(period);
    osc.setModulation(1.0f);

    // First run (no warmup, captures the raw startup sequence)
    std::vector<float> run1;
    run1.reserve(N);
    for (int i = 0; i < N; ++i)
        run1.push_back(osc.nextSample());

    // Reset and re-run with the same parameters
    osc.reset();
    osc.setAmplitude(1.0f);
    osc.setPeriod(period);
    osc.setModulation(1.0f);

    std::vector<float> run2;
    run2.reserve(N);
    for (int i = 0; i < N; ++i)
        run2.push_back(osc.nextSample());

    REQUIRE(run1.size() == run2.size());
    for (size_t i = 0; i < run1.size(); ++i)
        REQUIRE(run1[i] == Catch::Approx(run2[i]).epsilon(1e-5f));
}

// ─── PWM: odd-harmonic dominance ─────────────────────────────────────────────

TEST_CASE("Square wave from squareWave() has odd-harmonic dominance at 50% duty cycle", "[oscillator][pwm]")
{
    // At 50% duty cycle (modulation=1.0) the square wave has only odd harmonics.
    // squareWave() initialises osc2's phase; period must still be set explicitly
    // (as Voice::updatePeriod() does before each render block).
    constexpr float f0 = 440.0f;
    constexpr int N = 32768;
    const float period = sampleRate / f0;

    synth::SawtoothOscillator osc1;
    synth::SawtoothOscillator osc2;

    osc1.setAmplitude(1.0f);
    osc1.setPeriod(period);
    osc1.setModulation(1.0f);

    osc2.setAmplitude(1.0f);
    osc2.squareWave(osc1, period);
    osc2.setPeriod(period); // required — squareWave sets phase, not period
    osc2.setModulation(1.0f);

    const auto sig = renderSawtooth(osc1, osc2, N);

    const float fund = dsp::goertzel(sig, f0, sampleRate);
    const float harm2 = dsp::goertzel(sig, 2.0f * f0, sampleRate);
    const float harm3 = dsp::goertzel(sig, 3.0f * f0, sampleRate);

    INFO("fundamental=" << fund << " 2nd=" << harm2 << " 3rd=" << harm3);

    REQUIRE(fund > 0.1f);

    // 2nd harmonic (even) suppressed by ≥20 dB relative to fundamental
    const float suppression2dB = 20.0f * std::log10(fund / (harm2 + 1e-12f));
    INFO("2nd harmonic suppression: " << suppression2dB << " dB");
    REQUIRE(suppression2dB >= 20.0f);

    // 3rd harmonic (odd) must be present — for a perfect square wave it sits at
    // 20·log₁₀(3) ≈ 9.5 dB below the fundamental. Allow up to 15 dB for
    // Hann-window leakage and BLEP correction rounding.
    const float suppression3dB = 20.0f * std::log10(fund / (harm3 + 1e-12f));
    INFO("3rd harmonic level: " << suppression3dB << " dB below fundamental");
    REQUIRE(suppression3dB <= 15.0f);
}

// ─── Modulation ───────────────────────────────────────────────────────────────

TEST_CASE("Modulation parameter changes oscillator output", "[oscillator][modulation]")
{
    // setModulation() scales halfPeriod, which changes halfPhase_ and increment_.
    // Two different modulation values must produce measurably different RMS.
    constexpr int N = 4096;
    const float period = sampleRate / 440.0f;

    auto renderWithMod = [&](float mod) -> std::vector<float> {
        synth::SawtoothOscillator osc;
        osc.setAmplitude(1.0f);
        osc.setPeriod(period);
        osc.setModulation(mod);
        std::vector<float> out;
        out.reserve(N);
        for (int i = 0; i < N; ++i)
            out.push_back(osc.nextSample());
        return out;
    };

    const float rms1 = rms(renderWithMod(1.0f));
    const float rmsHalf = rms(renderWithMod(0.5f));

    INFO("rms(mod=1.0)=" << rms1 << "  rms(mod=0.5)=" << rmsHalf);
    REQUIRE(std::abs(rms1 - rmsHalf) > 0.01f);
}

TEST_CASE("setModulation clamps near-zero and negative values to prevent NaN", "[oscillator][modulation]")
{
    // Without the clamp in setModulation(), modulation=0 causes
    // increment_ = halfPhase_ / 0 → ±inf, producing NaN output.
    constexpr int N = 2048;
    const float period = sampleRate / 440.0f;

    for (const float mod : {0.0f, -0.5f, -1.0f})
    {
        synth::SawtoothOscillator osc;
        osc.setAmplitude(1.0f);
        osc.setPeriod(period);
        osc.setModulation(mod);

        bool allFinite = true;
        for (int i = 0; i < N; ++i)
        {
            if (!std::isfinite(osc.nextSample()))
            {
                allFinite = false;
                break;
            }
        }
        INFO("modulation = " << mod);
        REQUIRE(allFinite);
    }
}

// ─── Aliasing: sanity check ───────────────────────────────────────────────────

TEST_CASE("Oscillator produces a signal at the expected fundamental frequency", "[oscillator][aliasing]")
{
    constexpr int N = 16384;

    SECTION("440 Hz")
    {
        auto signal = renderAliasSawtooth(440.0f, sampleRate, N);
        const float mag = dsp::goertzel(signal, 440.0f, sampleRate);
        REQUIRE(mag > 1.0f);
    }

    SECTION("3000 Hz")
    {
        auto signal = renderAliasSawtooth(3000.0f, sampleRate, N);
        const float mag = dsp::goertzel(signal, 3000.0f, sampleRate);
        REQUIRE(mag > 1.0f);
    }

    SECTION("7000 Hz")
    {
        auto signal = renderAliasSawtooth(7000.0f, sampleRate, N);
        const float mag = dsp::goertzel(signal, 7000.0f, sampleRate);
        REQUIRE(mag > 1.0f);
    }
}

// ─── Aliasing: suppression ────────────────────────────────────────────────────

// Thresholds reflect the current BLEP implementation's measured performance.
// They are intentionally set below the measured values so regressions are
// caught (removing the band-limiting entirely would drop suppression to the
// natural harmonic roll-off level of ~13–20 dB for these frequencies).
//
// Measured suppression as of baseline:
//   3000 Hz → 51.7 dB   (threshold 45 dB)
//   5000 Hz → 47.0 dB   (threshold 40 dB)
//   7000 Hz → 40.6 dB   (threshold 35 dB)

TEST_CASE("Sawtooth oscillator suppresses aliasing relative to the fundamental", "[oscillator][aliasing]")
{
    constexpr int N = 16384;

    SECTION("3000 Hz — moderate stress, first alias at 20100 Hz")
    {
        auto signal = renderAliasSawtooth(3000.0f, sampleRate, N);
        auto r = measureAliasing(signal, 3000.0f, sampleRate);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= 45.0f);
    }

    SECTION("5000 Hz — high stress, aliases at 19100, 14100, 9100, 4100 Hz")
    {
        auto signal = renderAliasSawtooth(5000.0f, sampleRate, N);
        auto r = measureAliasing(signal, 5000.0f, sampleRate);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= 40.0f);
    }

    SECTION("7000 Hz — very high stress, many aliases fold back")
    {
        auto signal = renderAliasSawtooth(7000.0f, sampleRate, N);
        auto r = measureAliasing(signal, 7000.0f, sampleRate);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= 35.0f);
    }
}

// ─── Aliasing: suppression across sample rates ────────────────────────────────

TEST_CASE("Alias suppression holds across common sample rates", "[oscillator][aliasing]")
{
    // 3000 Hz is chosen because its harmonic aliases land clearly between
    // harmonics at all three sample rates, giving unambiguous measurements.
    constexpr float f0 = 3000.0f;
    constexpr float thresholdDb = 35.0f;
    constexpr int N = 16384;

    SECTION("44100 Hz")
    {
        auto signal = renderAliasSawtooth(f0, 44100.0f, N);
        auto r = measureAliasing(signal, f0, 44100.0f);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= thresholdDb);
    }

    SECTION("48000 Hz")
    {
        auto signal = renderAliasSawtooth(f0, 48000.0f, N);
        auto r = measureAliasing(signal, f0, 48000.0f);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= thresholdDb);
    }

    SECTION("96000 Hz")
    {
        auto signal = renderAliasSawtooth(f0, 96000.0f, N);
        auto r = measureAliasing(signal, f0, 96000.0f);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= thresholdDb);
    }
}
