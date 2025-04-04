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

  private:
    std::optional<int> note_;
    int velocity_{0};
};

} // namespace synth
