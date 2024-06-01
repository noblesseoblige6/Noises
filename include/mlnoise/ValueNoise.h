#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class ValueNoise : public detail::NoiseBase<ValueNoise, T>
    {
    private:
        static constexpr std::int32_t TableMaxSize = 256;
        static constexpr std::int32_t TableMask = TableMaxSize - 1;

    public:
        ValueNoise(RandomSeedType seed = 1024)
        {
            RandomEngineType gen(seed);

            m_values = detail::RandomTable_11<T, TableMaxSize>(gen);
            m_permutations = detail::PermutationTable<std::uint8_t, TableMaxSize>(gen);
        }
        ~ValueNoise() = default;
        
    public:
        T Noise(T x, T y, T z)
        {
            auto const xInt = static_cast<std::int32_t>(std::floor(x));
            auto const yInt = static_cast<std::int32_t>(std::floor(y));
            auto const zInt = static_cast<std::int32_t>(std::floor(z));

            auto const u = this->SmoothStep(x - xInt);
            auto const v = this->SmoothStep(y - yInt);
            auto const w = this->SmoothStep(z - zInt);

            auto const rx0 = xInt & TableMask;
            auto const rx1 = (rx0 + 1) & TableMask;
            auto const ry0 = yInt & TableMask;
            auto const ry1 = (ry0 + 1) & TableMask;
            auto const rz0 = zInt & TableMask;
            auto const rz1 = (rz0 + 1) & TableMask;

            auto const v000 = m_values[m_permutations[m_permutations[m_permutations[rx0] + ry0] + rz0]];
            auto const v100 = m_values[m_permutations[m_permutations[m_permutations[rx1] + ry0] + rz0]];
            auto const v010 = m_values[m_permutations[m_permutations[m_permutations[rx0] + ry1] + rz0]];
            auto const v110 = m_values[m_permutations[m_permutations[m_permutations[rx1] + ry1] + rz0]];
            auto const v001 = m_values[m_permutations[m_permutations[m_permutations[rx0] + ry0] + rz1]];
            auto const v101 = m_values[m_permutations[m_permutations[m_permutations[rx1] + ry0] + rz1]];
            auto const v011 = m_values[m_permutations[m_permutations[m_permutations[rx0] + ry1] + rz1]];
            auto const v111 = m_values[m_permutations[m_permutations[m_permutations[rx1] + ry1] + rz1]];

            auto const s1 = detail::Lerp(v000, v100, u);
            auto const s2 = detail::Lerp(v010, v110, u);
            auto const s3 = detail::Lerp(v001, v101, u);
            auto const s4 = detail::Lerp(v011, v111, u);
            
            auto const t1 = detail::Lerp(s1, s2, v);
            auto const t2 = detail::Lerp(s3, s4, v);

            return detail::Lerp(t1, t2, w);
        }

    private:
        std::array<T, TableMaxSize> m_values;
        std::array<std::uint8_t, TableMaxSize * 2> m_permutations;
    };
}