#include <math/map.h>

namespace Vultr::Math
{
    double map(double n, double start1, double stop1, double start2, double stop2)
    {
        return ((n - start1) / (stop1 - start1)) * (stop2 - start2) + start2;
    }
} // namespace Vultr::Math
