#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class BlockNoise
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
        T Generate(T x, T y, T z)
        {
            auto const xInt = static_cast<std::int32_t>(std::floor(x));
            auto const yInt = static_cast<std::int32_t>(std::floor(y));
            auto const zInt = static_cast<std::int32_t>(std::floor(z));

            auto const rx0 = xInt & TableMask;
            auto const ry0 = yInt & TableMask;
            auto const rz0 = zInt & TableMask;

            return m_r[m_permutationTable[m_permutationTable[m_permutationTable[rx0] + ry0] + rz0]];
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

        T Noise_01(T x, std::int32_t octarve, T persistence)
        {
            return Noise(x, 0, octarve, persistence) / max_amplitude(octarve, persistence);
        }

        T Noise_11(T x, std::int32_t octarve, T persistence)
        {
            return remap_11(Noise_01(x, 0, octarve, persistence));
        }

        T Noise_01(T x, T y, std::int32_t octarve, T persistence)
        {
            return Noise(x, y, octarve, persistence) / max_amplitude(octarve, persistence);
        }

        T Noise_11(T x, T y, std::int32_t octarve, T persistence)
        {
            return remap_11(Noise_01(x, y, octarve, persistence));
        }

        T Noise_01(T x, T y, T z, std::int32_t octarve, T persistence)
        {
            return Noise(x, y, z, octarve, persistence) / max_amplitude(octarve, persistence);
        }

        T Noise_11(T x, T y, T z, std::int32_t octarve, T persistence)
        {
            return remap_11(Noise_01(x, y, z, octarve, persistence));
        }

    private:
        std::array<T, TableMaxSize> m_r;
        std::array<std::uint32_t, TableMaxSize * 2> m_permutationTable;
    };

    using BlockNoisef = BlockNoise<std::float_t>;
    using BlockNoised = BlockNoise<std::double_t>;
}