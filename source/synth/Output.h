#pragma once

namespace synth
{

class Filter;

struct Output
{
    void filter(Filter& filter);

    Output& operator+=(const Output& rhs);
    Output& operator*=(const float scalar);

    float left{0.0f};
    float right{0.0f};
};

} // namespace synth
