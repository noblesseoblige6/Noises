#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class PerlinNoise : public detail::NoiseBase<PerlinNoise, T>
    {
    private:
        static constexpr std::int32_t TableMaxSize = 256;
        static constexpr std::int32_t TableMask = TableMaxSize - 1;

        static T grad(std::int32_t hash, T x, T y, T z) noexcept
        {
            auto h = hash & 15;

            T u = h < 8 ? x : y;
            T v = h < 4 ? y : h == 12 || h == 14 ? x : z;

            return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
        }

    public:
        PerlinNoise(RandomSeedType seed = 1024)
        {
            RandomEngineType gen(seed);

            m_permutations = detail::PermutationTable<std::uint8_t, TableMaxSize>(gen);
        }
        ~PerlinNoise() = default;

    public:
        T Noise(T x, T y, T z)
        {
            auto const xInt = static_cast<std::int32_t>(std::floor(x));
            auto const yInt = static_cast<std::int32_t>(std::floor(y));
            auto const zInt = static_cast<std::int32_t>(std::floor(z));

            auto const u = detail::Fade(x - xInt);
            auto const v = detail::Fade(y - yInt);
            auto const w = detail::Fade(z - zInt);

            auto const rx0 = xInt & TableMask;
            auto const rx1 = (xInt + 1) & TableMask;
            auto const ry0 = yInt & TableMask;
            auto const ry1 = (ry0 + 1) & TableMask;
            auto const rz0 = zInt & TableMask;
            auto const rz1 = (rz0 + 1) & TableMask;

            auto const v000 = m_permutations[m_permutations[m_permutations[rx0] + ry0] + rz0];
            auto const v001 = m_permutations[m_permutations[m_permutations[rx0] + ry0] + rz1];
            auto const v010 = m_permutations[m_permutations[m_permutations[rx0] + ry1] + rz0];
            auto const v011 = m_permutations[m_permutations[m_permutations[rx0] + ry1] + rz1];
            auto const v100 = m_permutations[m_permutations[m_permutations[rx1] + ry0] + rz0];
            auto const v101 = m_permutations[m_permutations[m_permutations[rx1] + ry0] + rz1];
            auto const v110 = m_permutations[m_permutations[m_permutations[rx1] + ry1] + rz0];
            auto const v111 = m_permutations[m_permutations[m_permutations[rx1] + ry1] + rz1];

            auto const s1 = detail::Lerp(grad(v000, x, y    , z    ), grad(v100, x - 1, y    , z    ), u);
            auto const s2 = detail::Lerp(grad(v010, x, y - 1, z    ), grad(v110, x - 1, y - 1, z    ), u);
            auto const s3 = detail::Lerp(grad(v001, x, y    , z - 1), grad(v101, x - 1, y    , z - 1), u);
            auto const s4 = detail::Lerp(grad(v011, x, y - 1, z - 1), grad(v111, x - 1, y - 1, z - 1), u);

            auto const t1 = detail::Lerp(s1, s2, v);
            auto const t2 = detail::Lerp(s3, s4, v);

            return detail::Lerp(t1, t2, w);
        }

    private:
        std::array<std::uint8_t, TableMaxSize * 2> m_permutations;
    };
}