#pragma once

namespace synth
{

class Filter
{
  public:
    void updateCoefficients(const float cutoff, const float Q);
    void reset();
    void setSampleRate(const float sampleRate);
    float render(const float x);

  private:
    float sampleRate_ = 0.0f;

    float k_ = 0.0f;
    float g_ = 0.0f;
    float a1_ = 0.0f;
    float a2_ = 0.0f;
    float a3_ = 0.0f;

    float ic1eq_ = 0.0f;
    float ic2eq_ = 0.0f;
};

} // namespace synth
