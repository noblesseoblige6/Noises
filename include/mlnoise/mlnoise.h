#pragma once
#include <array>
#include <random>
#include <functional>

namespace mlnoise
{
    template<class T>
    T random(int x)
    {
        x = (x << 13) ^ x;
        int xx = ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff);
        return  1.0 - ((T)xx / 1073741824.0);
    }

    template<class T>
    T lerp(T a, T b, T t) noexcept
    {
        return a + t * (b - a);
    }

    template<class T>
    static T fade(T t) noexcept
    {
        return  t * t * t * (t * (t * 6 - 15) + 10);
    }
}