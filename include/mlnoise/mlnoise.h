#pragma once
#include <array>
#include <numeric>
#include <random>
#include <functional>
#include <limits>

namespace mlnoise
{
    using RandomSeedType = std::uint32_t;
    using RandomEngineType = std::mt19937;
}

namespace mlnoise::detail
{
    template<class T>
    T Lerp(T a, T b, T t) noexcept
    {
        return a + t * (b - a);
    }

    template<class T>
    T Fade(T t) noexcept
    {
        return  t * t * t * (t * (t * 6 - 15) + 10);
    }

    template<class T>
    T Remap_01(T x) noexcept
    {
        return x * 0.5 + 0.5;
    }

    template<class T>
    T Remap_11(T x) noexcept
    {
        return  2 * x - 1;
    }

    template<class T>
    T MaxAmplitude(std::int32_t octave, T persistence) noexcept
    {
        T res = 0;
        T amp = 1;

        for (auto i = 0; i < octave; i++)
        {
            res += amp;
            amp *= persistence;
        }

        return res;
    }

    template<class T, std::size_t N, class Engine>
    auto RandomTable(Engine& gen) noexcept
    {
        std::array<T, N> res;

        auto rand = [dist = std::uniform_real_distribution<T>(-1, 1), &gen](T& val) mutable
            {
                val = dist(gen);
            };

        std::for_each(res.begin(), res.end(), rand);

        return res;
    }

    template<class T, std::size_t N, class Engine>
    auto PermutationTable(Engine& gen) noexcept
    {
        std::array<T, N> tmp;
        std::iota(tmp.begin(), tmp.end(), 0);
        std::shuffle(tmp.begin(), tmp.end(), gen);

        std::array<T, 2 * N> res;
        std::copy(tmp.begin(), tmp.end(), res.begin());
        std::copy(tmp.begin(), tmp.end(), res.begin() + N);

        return res;
    }
}

namespace mlnoise::detail
{
    template<template<class> class Derived, typename T>
    class NoiseBase
    {
    public:
        NoiseBase() = default;
        ~NoiseBase() = default;

    public:
        T Fractal(T x, std::int32_t octave, T persistence)
        {
            return Fractal(x, 0, 0, octave, persistence);
        }

        T Fractal(T x, T y, std::int32_t octave, T persistence)
        {
            return Fractal(x, y, 0, octave, persistence);
        }

        T Fractal(T x, T y, T z, std::int32_t octave, T persistence)
        {
            T total = 0;
            T amp = 1;

            for (auto i = 0; i < octave; i++)
            {
                total += static_cast<Derived<T>&>(*this).Noise(x, y, z) * amp;
                x *= m_lacunarity;
                y *= m_lacunarity;
                z *= m_lacunarity;
                amp *= persistence;
            }
            return total;
        }

        T Fractal_01(T x, std::int32_t octave, T persistence)
        {
            return Remap_01(Fractal_11(x, 0, octave, persistence));
        }

        T Fractal_11(T x, std::int32_t octave, T persistence)
        {
            return Fractal(x, 0, octave, persistence) / MaxAmplitude(octave, persistence);
        }

        T Fractal_01(T x, T y, std::int32_t octave, T persistence)
        {
            return Remap_01(Fractal_11(x, y, octave, persistence));
        }

        T Fractal_11(T x, T y, std::int32_t octave, T persistence)
        {
            return Fractal(x, y, octave, persistence) / MaxAmplitude(octave, persistence);
        }

        T Fractal_01(T x, T y, T z, std::int32_t octave, T persistence)
        {
            return Remap_01(Fractal_11(x, y, z, octave, persistence));
        }

        T Fractal_11(T x, T y, T z, std::int32_t octave, T persistence)
        {
            return Fractal(x, y, z, octave, persistence) / MaxAmplitude(octave, persistence);
        }

        T GetLacunarity() const { return m_lacunarity; }
        void SetLacunarity(T lacunarity) { m_lacunarity = lacunarity; }

    private:
        T m_lacunarity{ 2 };
    };
}