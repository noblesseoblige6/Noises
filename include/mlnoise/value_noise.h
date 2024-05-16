#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class ValueNoise
    {
    private:
        static constexpr std::int32_t TableMaxSize = 256;
        static constexpr std::int32_t TableMask = TableMaxSize-1;

    public:
        ValueNoise(unsigned seed=2016)
        {
            std::mt19937 gen(seed);
            std::uniform_real_distribution<float> distrFloat;
            auto randFloat = std::bind(distrFloat, gen);

            // create an array of random values and initialize permutation table
            for (unsigned k = 0; k < TableMaxSize; ++k)
            {
                //r[k] = randFloat();
                m_permutationTable[k] = k;
            }

            // shuffle values of the permutation table
            std::uniform_int_distribution<unsigned> distrUInt;
            auto randUInt = std::bind(distrUInt, gen);
            for (unsigned k = 0; k < TableMaxSize; ++k)
            {
                unsigned i = randUInt() & TableMaxSize - 1;
                std::swap(m_permutationTable[k], m_permutationTable[i]);
                m_permutationTable[k + TableMaxSize] = m_permutationTable[k];
            }
        }
        ~ValueNoise() = default;
        
    public:
        T Generate(T x)
        {
            auto xInt = static_cast<std::int32_t>(std::floor(x)) & 255;

            x -= xInt;

            T u = fade(x);

            T v1 = m_permutationTable[xInt];
            T v2 = m_permutationTable[xInt + 1];

            return lerp(v1, v2, u);
        }

        T Generate(T x, T y)
        {
            auto xInt = static_cast<std::int32_t>(std::floor(x)) & TableMask;
            auto yInt = static_cast<std::int32_t>(std::floor(y)) & TableMask;

            x -= std::floor(x);
            y -= std::floor(y);

            T u = fade(x);
            T v = fade(y);

            auto rx0 = xInt & TableMask;
            auto rx1 = (rx0 + 1) & TableMask;
            auto ry0 = yInt & TableMask;
            auto ry1 = (ry0 + 1) & TableMask;

            T v1 = m_permutationTable[ry0 * TableMask + rx0];
            T v2 = m_permutationTable[ry0 * TableMask + rx1];
            T v3 = m_permutationTable[ry1 * TableMask + rx0];
            T v4 = m_permutationTable[ry1 * TableMask + rx1];

            T w1 = lerp(v1, v2, u);
            T w2 = lerp(v3, v4, u);

            return lerp(w1, w2, v);
        }

        T Noise(T x, std::int32_t octarve, T persistence)
        {
            T total = 0.0;
            T amp = 1.0;

            for (auto i = 0; i < octarve; i++)
            {
                total += Generate(x) * amp;
                x *= 2;
                amp = pow(persistence, i);
            }
            return total;
        }

        T Noise(T x, T y, std::int32_t octarve, T persistence)
        {
            T total = 0.0;
            T amp = 1.0;

            for (auto i = 0; i < octarve; i++)
            {
                total += Generate(x, y) * amp;
                x *= 2.0;
                y *= 2.0;
                amp *= persistence;
            }
            return total;
        }

        //T Noise(T x, T y, T z, std::int32_t octarve, T persistence)
        //{
        //    T total = 0;
        //    T amp = 1;

        //    for (auto i = 0; i < octarve; i++)
        //    {
        //        total += Generate(x, y, z) * amp;
        //        x *= 2.0;
        //        y *= 2.0;
        //        z *= 2.0;
        //        amp *= persistence;
        //    }
        //    return total;
        //}

    private:
        std::array<std::uint32_t, TableMaxSize * 2> m_permutationTable;
    };

}