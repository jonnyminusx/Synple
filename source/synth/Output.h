#pragma once

namespace synth
{

struct Output
{
    Output& operator+=(const Output& rhs)
    {
        left += rhs.left;
        right += rhs.right;
        return *this;
    }

    float left{0.0f};
    float right{0.0f};
};

} // namespace synth
