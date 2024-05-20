#pragma once
#include <array>
#include <numeric>
#include <random>
#include <functional>

namespace mlnoise
{
    using random_engine_type = std::mt19937;

    template<class T>
    T lerp(T a, T b, T t) noexcept
    {
        return a + t * (b - a);
    }

    template<class T>
    T fade(T t) noexcept
    {
        return  t * t * t * (t * (t * 6 - 15) + 10);
    }

    template<class T>
    T remap_01(T x) noexcept
    {
        return x * 0.5 + 0.5;
    }

    template<class T>
    T remap_11(T x) noexcept
    {
        return  2 * x - 1;
    }

    template<class T>
    T max_amplitude(std::int32_t octarve, T persistence) noexcept
    {
        T res = 0;
        T amp = 1;

        for (auto i = 0; i < octarve; i++)
        {
            res += amp;
            amp *= persistence;
        }

        return res;
    }

    template<class T, std::size_t N, class Engine>
    auto random_table(Engine& gen) noexcept
    {
        std::array<T, N> res;

        auto rand = [dist = std::uniform_real_distribution<T>(0, 1), &gen](T& val) mutable
            {
                val = dist(gen);
            };

        std::for_each(res.begin(), res.end(), rand);

        return res;
    }

    template<std::size_t N, class Engine>
    auto permutation_table(Engine& gen) noexcept
    {
        std::array<std::uint32_t, N> tmp;
        std::iota(tmp.begin(), tmp.end(), 0);
        std::shuffle(tmp.begin(), tmp.end(), gen);

        std::array<std::uint32_t, 2 * N> res;
        std::copy(tmp.begin(), tmp.end(), res.begin());
        std::copy(tmp.begin(), tmp.end(), res.begin() + N);

        return res;
    }
}