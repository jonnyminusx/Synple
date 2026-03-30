#pragma once

#include "Preset.h"
#include <vector>

class Presets
{
  public:
    Presets();

    size_t size() const
    {
        return presets_.size();
    }
    const Preset& operator[](size_t index) const
    {
        return presets_[index];
    }

  private:
    void createPrograms();

    std::vector<Preset> presets_;
};
