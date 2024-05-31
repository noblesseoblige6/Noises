#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class VoronoiNoise : public detail::NoiseBase<VoronoiNoise, T>
    {
    private:
        static constexpr std::int32_t TableMaxSize = 256;
        static constexpr std::int32_t TableMask = TableMaxSize - 1;

    public:
        VoronoiNoise(RandomSeedType seed = 1024)
        {
            RandomEngineType gen(seed);

            m_values = detail::RandomTable<T, TableMaxSize>(gen);
            m_permutations = detail::PermutationTable<std::uint8_t, TableMaxSize>(gen);
        }
        ~VoronoiNoise() = default;
        
    public:
        T Noise(T x, T y, T z)
        {
            auto const xInt = static_cast<std::int32_t>(std::floor(x));
            auto const yInt = static_cast<std::int32_t>(std::floor(y));
            auto const zInt = static_cast<std::int32_t>(std::floor(z));

            x -= xInt;
            y -= yInt;
            z -= zInt;

            auto minDis = (std::numeric_limits<T>::max)();
            for (auto ix = -1; ix <= 1; ++ix)
            {
                for (auto iy = -1; iy <= 1; ++iy)
                {
                    if (ix == 0 && iy == 0)
                        continue;

                    auto const rx = (xInt + ix) & TableMask;
                    auto const ry = (yInt + iy) & TableMask;

                    auto const vecx = (ix + detail::Remap_01(m_values[m_permutations[m_permutations[rx] + ry]])) - x;
                    auto const vecy = (iy + detail::Remap_01(m_values[m_permutations[m_permutations[rx] + ry]])) - y;

                    auto const dis = vecx * vecx + vecy * vecy;
                    if (dis < minDis)
                        minDis = dis;
                }
            }

            return std::sqrt(minDis);
        }

    private:
        std::array<T, TableMaxSize> m_values;
        std::array<std::uint8_t, TableMaxSize * 2> m_permutations;
    };
}