#include "Voice.h"

namespace synth
{

void Voice::reset()
{
    note_ = std::nullopt;
    velocity_ = 0;
}

void Voice::noteOn(const int note, const int velocity)
{
    note_ = note;
    velocity_ = velocity;
}

void Voice::noteOff(const int note)
{
    if (note_ == note)
    {
        note_ = std::nullopt;
        velocity_ = 0;
    }
}

std::optional<int> Voice::note() const
{
    return note_;
}

int Voice::velocity() const
{
    return velocity_;
}

float Voice::velocityNormalised() const
{
    return static_cast<float>(velocity_) / 127.0f;
}

} // namespace synth
