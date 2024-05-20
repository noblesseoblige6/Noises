#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class BlockNoise : public detail::BaseNoise<BlockNoise, T>
    {
    private:
        static constexpr std::int32_t TableMaxSize = 256;
        static constexpr std::int32_t TableMask = TableMaxSize - 1;

    public:
        BlockNoise(std::uint32_t seed = 1024)
        {
            std::mt19937 gen(seed);

            m_r = random_table<T, TableMaxSize>(gen);
            m_permutationTable = permutation_table<TableMaxSize>(gen);
        }
        ~BlockNoise() = default;
        
    public:
        T Noise(T x, T y, T z)
        {
            auto const xInt = static_cast<std::int32_t>(std::floor(x));
            auto const yInt = static_cast<std::int32_t>(std::floor(y));
            auto const zInt = static_cast<std::int32_t>(std::floor(z));

            auto const rx0 = xInt & TableMask;
            auto const ry0 = yInt & TableMask;
            auto const rz0 = zInt & TableMask;

            return m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry0] + rz0]];
        }

    private:
        std::array<T, TableMaxSize> m_r;
        std::array<std::uint32_t, TableMaxSize * 2> m_permutationTable;
    };
}
