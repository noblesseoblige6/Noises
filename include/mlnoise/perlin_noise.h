#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class PerlinNoise
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
        PerlinNoise(std::uint32_t seed = 1024)
        {
            std::mt19937 gen(seed);

            m_permutationTable = permutation_table<TableMaxSize>(gen);
        }
        ~PerlinNoise() = default;

    public:
        T Generate(T x, T y, T z)
        {
            auto const xInt = static_cast<std::int32_t>(std::floor(x));
            auto const yInt = static_cast<std::int32_t>(std::floor(y));
            auto const zInt = static_cast<std::int32_t>(std::floor(z));

            x -= xInt;
            y -= yInt;
            z -= zInt;

            auto const u = fade(x);
            auto const v = fade(y);
            auto const w = fade(z);

            auto const rx0 = xInt & TableMask;
            auto const rx1 = (xInt + 1) & TableMask;
            auto const ry0 = yInt & TableMask;
            auto const ry1 = (ry0 + 1) & TableMask;
            auto const rz0 = zInt & TableMask;
            auto const rz1 = (rz0 + 1) & TableMask;

            auto const v000 = m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry0] + rz0];
            auto const v001 = m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry0] + rz1];
            auto const v010 = m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry1] + rz0];
            auto const v011 = m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry1] + rz1];
            auto const v100 = m_permutationTable[m_permutationTable[m_permutationTable[rx1] + ry0] + rz0];
            auto const v101 = m_permutationTable[m_permutationTable[m_permutationTable[rx1] + ry0] + rz1];
            auto const v110 = m_permutationTable[m_permutationTable[m_permutationTable[rx1] + ry1] + rz0];
            auto const v111 = m_permutationTable[m_permutationTable[m_permutationTable[rx1] + ry1] + rz1];

            auto s1 = lerp(grad(v000, x, y    , z    ), grad(v100, x - 1, y    , z    ), u);
            auto s2 = lerp(grad(v010, x, y - 1, z    ), grad(v110, x - 1, y - 1, z    ), u);
            auto s3 = lerp(grad(v001, x, y    , z - 1), grad(v101, x - 1, y    , z - 1), u);
            auto s4 = lerp(grad(v011, x, y - 1, z - 1), grad(v111, x - 1, y - 1, z - 1), u);

            auto t1 = lerp(s1, s2, v);
            auto t2 = lerp(s3, s4, v);

            return lerp(t1, t2, w);
        }


        T Noise(T x, std::int32_t octarve, T persistence)
        {
            return Noise(x, 0, 0, octarve, persistence);
        }

        T Noise(T x, T y, std::int32_t octarve, T persistence)
        {
            return Noise(x, y, 0, octarve, persistence);
        }

        T Noise(T x, T y, T z, std::int32_t octarve, T persistence)
        {
            T total = 0;
            T amp = 1;

            for (auto i = 0; i < octarve; i++)
            {
                total += remap_01(Generate(x, y, z)) * amp;
                x *= 2.0;
                y *= 2.0;
                z *= 2.0;
                amp *= persistence;
            }
            return total;
        }

        T Noise_01(T x, T y, std::int32_t octarve, T persistence)
        {
            return Noise(x, y, octarve, persistence) / max_amplitude(octarve, persistence);
        }

        T Noise_11(T x, T y, std::int32_t octarve, T persistence)
        {
            return remap_11(Noise(x, y, octarve, persistence));
        }

    private:
        std::array<std::uint32_t, TableMaxSize * 2> m_permutationTable;
    };

    using PerlinNoisef = PerlinNoise<std::float_t>;
    using PerlinNoised = PerlinNoise<std::double_t>;
}