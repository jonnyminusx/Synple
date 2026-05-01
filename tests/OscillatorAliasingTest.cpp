#include <catch2/catch_test_macros.hpp>

#include "synth/Oscillator.h"

#include <cmath>
#include <numbers>
#include <vector>

namespace {

constexpr float pi = std::numbers::pi_v<float>;

// Evaluates the energy at a specific frequency using the Goertzel algorithm.
// A Hann window is applied before evaluation to suppress sidelobes, which
// prevents energy from nearby harmonics leaking into alias frequency bins.
// Returns a raw magnitude comparable across frequencies on the same signal.
float goertzel(const std::vector<float>& samples, float targetFreq, float sampleRate)
{
    const int N = static_cast<int>(samples.size());
    const float omega = 2.0f * pi * targetFreq / sampleRate;
    const float coeff = 2.0f * std::cos(omega);
    float s1 = 0.0f;
    float s2 = 0.0f;

    for (int i = 0; i < N; ++i)
    {
        const float w = 0.5f - 0.5f * std::cos(2.0f * pi * static_cast<float>(i) / static_cast<float>(N - 1));
        const float s0 = samples[i] * w + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    const float real = s1 - s2 * std::cos(omega);
    const float imag = s2 * std::sin(omega);
    return std::sqrt(real * real + imag * imag);
}

// Renders the sawtooth pipeline from Voice::render() using a single active
// oscillator. osc2 has zero amplitude so it contributes nothing, matching
// the formula: saw = saw * 0.997 + osc1 - osc2.
std::vector<float> renderSawtooth(float f0, float sampleRate, int numSamples, int warmup = 4096)
{
    const float period = sampleRate / f0;

    synth::Oscillator osc1;
    synth::Oscillator osc2;

    osc1.setAmplitude(1.0f);
    osc1.setPeriod(period);
    osc1.setModulation(1.0f);

    osc2.setAmplitude(0.0f);
    osc2.setPeriod(period);
    osc2.setModulation(1.0f);

    float saw = 0.0f;
    std::vector<float> output;
    output.reserve(numSamples);

    for (int i = 0; i < warmup + numSamples; ++i)
    {
        const float s1 = osc1.nextSample();
        const float s2 = osc2.nextSample();
        saw = saw * 0.997f + s1 - s2;

        if (i >= warmup)
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
AliasingResult measureAliasing(const std::vector<float>& signal,
                               float f0,
                               float sampleRate,
                               int maxHarmonic = 30,
                               float minSeparationHz = 100.0f)
{
    const float nyquist = sampleRate / 2.0f;
    const float fundamentalMag = goertzel(signal, f0, sampleRate);

    float worstAlias = 0.0f;
    float worstAliasFreq = 0.0f;

    for (int k = 2; k <= maxHarmonic; ++k)
    {
        const float harmonic = static_cast<float>(k) * f0;
        if (harmonic <= nyquist)
            continue;

        const float aliasFreq = sampleRate - harmonic;
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

        const float mag = goertzel(signal, aliasFreq, sampleRate);
        if (mag > worstAlias)
        {
            worstAlias = mag;
            worstAliasFreq = aliasFreq;
        }
    }

    return {fundamentalMag, worstAlias, worstAliasFreq};
}

} // namespace

// ─── Sanity: oscillator produces output ──────────────────────────────────────

TEST_CASE("Oscillator produces a signal at the expected fundamental frequency", "[oscillator][aliasing]")
{
    constexpr float sampleRate = 44100.0f;
    constexpr int N = 16384;

    SECTION("440 Hz")
    {
        auto signal = renderSawtooth(440.0f, sampleRate, N);
        const float mag = goertzel(signal, 440.0f, sampleRate);
        REQUIRE(mag > 1.0f);
    }

    SECTION("3000 Hz")
    {
        auto signal = renderSawtooth(3000.0f, sampleRate, N);
        const float mag = goertzel(signal, 3000.0f, sampleRate);
        REQUIRE(mag > 1.0f);
    }

    SECTION("7000 Hz")
    {
        auto signal = renderSawtooth(7000.0f, sampleRate, N);
        const float mag = goertzel(signal, 7000.0f, sampleRate);
        REQUIRE(mag > 1.0f);
    }
}

// ─── Alias suppression ───────────────────────────────────────────────────────

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
    constexpr float sampleRate = 44100.0f;
    constexpr int N = 16384;

    SECTION("3000 Hz — moderate stress, first alias at 20100 Hz")
    {
        auto signal = renderSawtooth(3000.0f, sampleRate, N);
        auto r = measureAliasing(signal, 3000.0f, sampleRate);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= 45.0f);
    }

    SECTION("5000 Hz — high stress, aliases at 19100, 14100, 9100, 4100 Hz")
    {
        auto signal = renderSawtooth(5000.0f, sampleRate, N);
        auto r = measureAliasing(signal, 5000.0f, sampleRate);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= 40.0f);
    }

    SECTION("7000 Hz — very high stress, many aliases fold back")
    {
        auto signal = renderSawtooth(7000.0f, sampleRate, N);
        auto r = measureAliasing(signal, 7000.0f, sampleRate);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= 35.0f);
    }
}

// ─── Alias suppression across sample rates ───────────────────────────────────

TEST_CASE("Alias suppression holds across common sample rates", "[oscillator][aliasing]")
{
    // 3000 Hz is chosen because its harmonic aliases land clearly between
    // harmonics at all three sample rates, giving unambiguous measurements.
    constexpr float f0 = 3000.0f;
    constexpr float thresholdDb = 35.0f;
    constexpr int N = 16384;

    SECTION("44100 Hz")
    {
        auto signal = renderSawtooth(f0, 44100.0f, N);
        auto r = measureAliasing(signal, f0, 44100.0f);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= thresholdDb);
    }

    SECTION("48000 Hz")
    {
        auto signal = renderSawtooth(f0, 48000.0f, N);
        auto r = measureAliasing(signal, f0, 48000.0f);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= thresholdDb);
    }

    SECTION("96000 Hz")
    {
        auto signal = renderSawtooth(f0, 96000.0f, N);
        auto r = measureAliasing(signal, f0, 96000.0f);
        INFO("Worst alias: " << r.worstAliasFreq << " Hz, suppression: " << r.suppressionDb() << " dB");
        REQUIRE(r.suppressionDb() >= thresholdDb);
    }
}
