#pragma once

#include "Preset.h"

#include <cstddef>
#include <vector>

namespace synple
{

class Presets
{
  public:
    Presets();

    std::size_t size() const { return presets_.size(); }
    const Preset& operator[](std::size_t index) const { return presets_[index]; }

  private:
    void createPrograms();

    std::vector<Preset> presets_;
};

} // namespace synple