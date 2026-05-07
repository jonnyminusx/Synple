#pragma once

#include <cstddef>
#include <cstring>

namespace synth
{

inline constexpr std::size_t kNumParams = 27;

class Preset
{
  public:
    Preset(const char* name,
           const float p0,
           const float p1,
           const float p2,
           const float p3,
           const float p4,
           const float p5,
           const float p6,
           const float p7,
           const float p8,
           const float p9,
           const float p10,
           const float p11,
           const float p12,
           const float p13,
           const float p14,
           const float p15,
           const float p16,
           const float p17,
           const float p18,
           const float p19,
           const float p20,
           const float p21,
           const float p22,
           const float p23,
           const float p24,
           const float p25,
           const float p26)
    {
        std::strncpy(this->name_, name, sizeof(name_) - 1);
        name_[sizeof(name_) - 1] = '\0';

        parameters_[0] = p0;   // Osc Mix
        parameters_[1] = p1;   // Osc Tune
        parameters_[2] = p2;   // Osc Fine
        parameters_[3] = p3;   // Glide Mode
        parameters_[4] = p4;   // Glide Rate
        parameters_[5] = p5;   // Glide Bend
        parameters_[6] = p6;   // Filter Freq
        parameters_[7] = p7;   // Filter Reso
        parameters_[8] = p8;   // Filter Env
        parameters_[9] = p9;   // Filter LFO
        parameters_[10] = p10; // Velocity
        parameters_[11] = p11; // Filter Attack
        parameters_[12] = p12; // Filter Decay
        parameters_[13] = p13; // Filter Sustain
        parameters_[14] = p14; // Filter Release
        parameters_[15] = p15; // Env Attack
        parameters_[16] = p16; // Env Decay
        parameters_[17] = p17; // Env Sustain
        parameters_[18] = p18; // Env Release
        parameters_[19] = p19; // LFO Rate
        parameters_[20] = p20; // Vibrato
        parameters_[21] = p21; // Noise
        parameters_[22] = p22; // Octave
        parameters_[23] = p23; // Tuning
        parameters_[24] = p24; // Output Level
        parameters_[25] = p25; // Polyphony
        parameters_[26] = p26; // Waveform
    }

    const char* name() const { return name_; }

    const float* parameters() const { return parameters_; }

  private:
    char name_[40];
    float parameters_[kNumParams];
};

} // namespace synth
