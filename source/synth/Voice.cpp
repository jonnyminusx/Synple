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

} // namespace synth
