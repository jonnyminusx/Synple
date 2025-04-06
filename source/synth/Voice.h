#pragma once

#include <optional>

namespace synth
{

class Voice
{
  public:
    void reset();
    void noteOn(const int note, const int velocity);
    void noteOff(const int note);

    std::optional<int> note() const;
    int velocity() const;
    float velocityNormalised() const;

  private:
    std::optional<int> note_;
    int velocity_{0};
};

} // namespace synth
