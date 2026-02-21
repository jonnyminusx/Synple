#include "Output.h"
#include "Filter.h"

namespace synth
{

Output& Output::operator+=(const Output& rhs)
{
    left += rhs.left;
    right += rhs.right;
    return *this;
}

Output& Output::operator*=(const float scalar)
{
    left *= scalar;
    right *= scalar;
    return *this;
}

void Output::filter(Filter& filter)
{
    left = filter.render(left);
    right = filter.render(right);
}

} // namespace synth
