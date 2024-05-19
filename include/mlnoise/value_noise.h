#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class ValueNoise
    {
    private:
        static constexpr std::int32_t TableMaxSize = 256;
        static constexpr std::int32_t TableMask = TableMaxSize - 1;

    public:
        ValueNoise(unsigned seed = 1024)
        {
            std::mt19937 gen(seed);

            // create an array of random values and initialize permutation table
            {
                std::uniform_real_distribution<T> dist;

                for (unsigned k = 0; k < TableMaxSize; ++k)
                {
                    m_r[k] = dist(gen);
                    m_permutationTable[k] = k;
                }
            }

            // shuffle values of the permutation table
            {
                std::uniform_int_distribution<std::uint32_t> dist;
                for (auto k = 0; k < TableMaxSize; ++k)
                {
                    auto i = dist(gen) & TableMask;
                    std::swap(m_permutationTable[k], m_permutationTable[i]);
                    m_permutationTable[k + TableMaxSize] = m_permutationTable[k];
                }
            }
        }
        ~ValueNoise() = default;
        
    public:
        T Generate(T x, T y, T z)
        {
            auto const xInt = static_cast<std::int32_t>(std::floor(x));
            auto const yInt = static_cast<std::int32_t>(std::floor(y));
            auto const zInt = static_cast<std::int32_t>(std::floor(z));

            x -= std::floor(x);
            y -= std::floor(y);
            z -= std::floor(z);

            auto const u = fade(x);
            auto const v = fade(y);
            auto const w = fade(z);

            auto rx0 = xInt & TableMask;
            auto rx1 = (rx0 + 1) & TableMask;
            auto ry0 = yInt & TableMask;
            auto ry1 = (ry0 + 1) & TableMask;
            auto rz0 = zInt & TableMask;
            auto rz1 = (rz0 + 1) & TableMask;

#if 1
            auto const v000 = m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry0] + rz0]];
            auto const v100 = m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx1] + ry0] + rz0]];
            auto const v010 = m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry1] + rz0]];
            auto const v110 = m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx1] + ry1] + rz0]];
            auto const v001 = m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry0] + rz1]];
            auto const v101 = m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx1] + ry0] + rz1]];
            auto const v011 = m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry1] + rz1]];
            auto const v111 = m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx1] + ry1] + rz1]];

            auto const s1 = lerp(v000, v100, u);
            auto const s2 = lerp(v010, v110, u);
            auto const s3 = lerp(v001, v101, u);
            auto const s4 = lerp(v011, v111, u);
            
            auto const t1 = lerp(s1, s2, v);
            auto const t2 = lerp(s3, s4, v);

            return lerp(t1, t2, w);
#else
            auto const v00 = m_r[m_permutationTable[m_permutationTable[rx0] + ry0]];
            auto const v10 = m_r[m_permutationTable[m_permutationTable[rx1] + ry0]];
            auto const v01 = m_r[m_permutationTable[m_permutationTable[rx0] + ry1]];
            auto const v11 = m_r[m_permutationTable[m_permutationTable[rx1] + ry1]];

            auto const s1 = lerp(v00, v10, u);
            auto const s2 = lerp(v01, v11, u);

            return lerp(s1, s2, v);
#endif
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
                total += Generate(x, y, z) * amp;
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
            return remap_11(Noise_01(x, y, octarve, persistence));
        }

    private:
        std::array<T, TableMaxSize> m_r;
        std::array<std::uint32_t, TableMaxSize * 2> m_permutationTable;
    };

}