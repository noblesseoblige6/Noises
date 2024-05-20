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

namespace mlnoise::detail
{
    template<template<class> class Derived, typename T>
    class BaseNoise
    {
    public:
        BaseNoise() = default;
        ~BaseNoise() = default;

    public:
        T Fractal(T x, std::int32_t octarve, T persistence)
        {
            return Fractal(x, 0, 0, octarve, persistence);
        }

        T Fractal(T x, T y, std::int32_t octarve, T persistence)
        {
            return Fractal(x, y, 0, octarve, persistence);
        }

        T Fractal(T x, T y, T z, std::int32_t octarve, T persistence)
        {
            T total = 0;
            T amp = 1;

            for (auto i = 0; i < octarve; i++)
            {
                total += static_cast<Derived<T>&>(*this).Noise(x, y, z) * amp;
                x *= 2.0;
                y *= 2.0;
                z *= 2.0;
                amp *= persistence;
            }
            return total;
        }

        T Fractal_01(T x, std::int32_t octarve, T persistence)
        {
            return remap_01(Fractal_11(x, 0, octarve, persistence));
        }

        T Fractal_11(T x, std::int32_t octarve, T persistence)
        {
            return Fractal(x, 0, octarve, persistence) / max_amplitude(octarve, persistence);
        }

        T Fractal_01(T x, T y, std::int32_t octarve, T persistence)
        {
            return remap_01(Fractal_11(x, y, octarve, persistence));
        }

        T Fractal_11(T x, T y, std::int32_t octarve, T persistence)
        {
            return Fractal(x, y, octarve, persistence) / max_amplitude(octarve, persistence);
        }

        T Fractal_01(T x, T y, T z, std::int32_t octarve, T persistence)
        {
            return remap_01(Fractal_11(x, y, z, octarve, persistence));
        }

        T Fractal_11(T x, T y, T z, std::int32_t octarve, T persistence)
        {
            return Fractal(x, y, z, octarve, persistence) / max_amplitude(octarve, persistence);
        }
    };
}