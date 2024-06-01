#pragma once
#include "mlnoise.h"
#include <algorithm>

namespace mlnoise
{
    enum class VoronoiOut
    {
        MinDistance = 0,
        MinDistance2,
        AddMinDistance2,
        SubMinDistance2,
        MulMinDistance2,
        DivMinDistance2,
    };

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

            m_values = detail::RandomTable_01<T, TableMaxSize>(gen);
            m_permutations = detail::PermutationTable<std::uint8_t, TableMaxSize>(gen);
        }
        ~VoronoiNoise() = default;
        
    public:
        T Noise(T x, T y, T z)
        {
            auto const xInt = static_cast<std::int32_t>(std::floor(x));
            auto const yInt = static_cast<std::int32_t>(std::floor(y));
            auto const zInt = static_cast<std::int32_t>(std::floor(z));

            auto const xFract = x - xInt;
            auto const yFract = y - yInt;
            auto const zFract = z - zInt;

            auto minDis = (std::numeric_limits<T>::max)();
            auto minDis2 = minDis;
            for (auto ix = -1; ix <= 1; ++ix)
            {
                for (auto iy = -1; iy <= 1; ++iy)
                {
#if 0
                    auto const rx = (xInt + ix) & TableMask;
                    auto const ry = (yInt + iy) & TableMask;

                    auto const dx = m_values[m_permutations[m_permutations[rx]+ry]] * m_jittering;
                    auto const dy = m_values[m_permutations[m_permutations[rx]+ry]] * m_jittering;

                    auto const vecx = (ix + dx) - xFract;
                    auto const vecy = (iy + dy) - yFract;

                    auto const dis = vecx * vecx + vecy * vecy;

                    minDis2 = (std::max)((std::min)(minDis2, dis), minDis);
                    minDis = (std::min)(minDis, dis);
#else
                    for (auto iz = -1; iz <= 1; ++iz)
                    {
                        auto const rx = (xInt + ix) & TableMask;
                        auto const ry = (yInt + iy) & TableMask;
                        auto const rz = (zInt + iz) & TableMask;

                        auto const dx = m_values[m_permutations[m_permutations[m_permutations[rx] + ry] + rz]] * m_jittering;
                        auto const dy = dx;//detail::Remap_01(m_values[m_permutations[m_permutations[m_permutations[rx] + ry] + rz]]);
                        auto const dz = dx;//detail::Remap_01(m_values[m_permutations[m_permutations[m_permutations[rx] + ry] + rz]]);

                        auto const vecx = (ix + dx) - xFract;
                        auto const vecy = (iy + dy) - yFract;
                        auto const vecz = (iz + dz) - zFract;

                        auto const dis = vecx * vecx + vecy * vecy + vecz * vecz;

                        minDis2 = (std::max)((std::min)(minDis2, dis), minDis);
                        minDis = (std::min)(minDis, dis);
                    }
#endif
                }
            }

            return detail::Remap_11(OutValue(minDis, minDis2));
        }

        T GetJittering() const { return m_jittering; }
        void SetJittering(T jittering) { m_jittering = jittering; }

        VoronoiOut GetOutType() const { return m_outType; }
        void SetOutType(VoronoiOut outType) { m_outType = outType; }

        private:
            T OutValue(T min, T min2)
            {
                T res = 0;
                switch (m_outType)
                {
                case mlnoise::VoronoiOut::MinDistance2:
                {
                    res = std::sqrt(min2 / 3);
                }
                break;
                case mlnoise::VoronoiOut::AddMinDistance2:
                {
                    res = std::sqrt((min2 + min) / 3);
                }
                break;
                case mlnoise::VoronoiOut::SubMinDistance2:
                {
                    res = std::sqrt((min2 - min) / 3);

                }
                break;
                case mlnoise::VoronoiOut::MulMinDistance2:
                {
                    res = std::sqrt((min2 * min) / 3);

                }
                break;
                case mlnoise::VoronoiOut::DivMinDistance2:
                {
                    res = std::sqrt((min / min2));

                }
                break;
                case mlnoise::VoronoiOut::MinDistance:
                default:
                {
                    res = std::sqrt(min / 3);
                }
                break;
                }

                return res;
            }

    private:
        std::array<T, TableMaxSize> m_values;
        std::array<std::uint8_t, TableMaxSize * 2> m_permutations;
        T m_jittering{ 1 };
        VoronoiOut m_outType{ VoronoiOut::MinDistance };
    };
}