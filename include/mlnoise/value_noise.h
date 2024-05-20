#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class ValueNoise : public detail::BaseNoise<ValueNoise, T>
    {
    private:
        static constexpr std::int32_t TableMaxSize = 256;
        static constexpr std::int32_t TableMask = TableMaxSize - 1;

    public:
        ValueNoise(std::uint32_t seed = 1024)
        {
            std::mt19937 gen(seed);

            m_r = random_table<T, TableMaxSize>(gen);
            m_permutationTable = permutation_table<TableMaxSize>(gen);
        }
        ~ValueNoise() = default;
        
    public:
        T Noise(T x, T y, T z)
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
            auto const rx1 = (rx0 + 1) & TableMask;
            auto const ry0 = yInt & TableMask;
            auto const ry1 = (ry0 + 1) & TableMask;
            auto const rz0 = zInt & TableMask;
            auto const rz1 = (rz0 + 1) & TableMask;

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
        }

    private:
        std::array<T, TableMaxSize> m_r;
        std::array<std::uint32_t, TableMaxSize * 2> m_permutationTable;
    };
}