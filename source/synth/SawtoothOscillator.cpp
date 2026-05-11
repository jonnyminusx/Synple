#include "SawtoothOscillator.h"

namespace synth
{

void SawtoothOscillator::reset()
{
    primary_.reset();
    secondary_.reset();
    saw_ = 0.0f;
}

void SawtoothOscillator::setAmplitude(const float amplitude)
{
    primary_.setAmplitude(amplitude);
}

void SawtoothOscillator::setPeriod(const float period)
{
    primary_.setPeriod(period);
    secondary_.setPeriod(period);
}

void SawtoothOscillator::setModulation(const float modulation)
{
    primary_.setModulation(modulation);
    secondary_.setModulation(modulation);
}

void SawtoothOscillator::noteOn(const float period)
{
    primary_.setPeriod(period);
    secondary_.setPeriod(period);
}

void SawtoothOscillator::setSquareWave(const float secondaryAmplitude, const float period)
{
    secondary_.setAmplitude(secondaryAmplitude);
    if (secondaryAmplitude > 0.0f)
        secondary_.squareWave(primary_, period);
}

float SawtoothOscillator::nextSample()
{
    saw_ = saw_ * 0.997f + primary_.nextSample() - secondary_.nextSample();
    return saw_;
}

} // namespace synth
