#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class BlockNoise : public detail::NoiseBase<BlockNoise, T>
    {
    private:
        static constexpr std::int32_t TableMaxSize = 256;
        static constexpr std::int32_t TableMask = TableMaxSize - 1;

    public:
        BlockNoise(RandomSeedType seed = 1024)
        {
            RandomEngineType gen(seed);

            m_values = detail::RandomTable_11<T, TableMaxSize>(gen);
            m_permutations = detail::PermutationTable<std::uint8_t, TableMaxSize>(gen);
        }
        ~BlockNoise() = default;
        
    public:
        T Noise(T x, T y, T z)
        {
            auto const xInt = static_cast<std::int32_t>(std::floor(x));
            auto const yInt = static_cast<std::int32_t>(std::floor(y));
            auto const zInt = static_cast<std::int32_t>(std::floor(z));

            auto const rx = xInt & TableMask;
            auto const ry = yInt & TableMask;
            auto const rz = zInt & TableMask;

            return m_values[m_permutations[m_permutations[m_permutations[rx] + ry] + rz]];
        }

    private:
        std::array<T, TableMaxSize> m_values;
        std::array<std::uint8_t, TableMaxSize * 2> m_permutations;
    };
}
