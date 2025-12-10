#pragma once

#include <numbers>

namespace constants
{

constexpr float pi{std::numbers::pi_v<float>};
constexpr float quarterPi{pi / 4.0f};
constexpr float tau{2.0f * pi};

} // namespace constants
