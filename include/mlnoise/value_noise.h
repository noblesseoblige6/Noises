#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class ValueNoise
    {
    public:
        ValueNoise()
        {
            for (auto i = 0; i < 256; ++i)
            {
                p[i + 256] = p[i] = permutation[i];
            }
        }
        ~ValueNoise() = default;
        
    public:
        T Generate(T x)
        {
            auto xInt = static_cast<std::int32_t>(std::floor(x)) & 255;

            x -= xInt;

            T u = fade(x);

            T v1 = p[xInt];
            T v2 = p[xInt + 1];

            return lerp(v1, v2, u);
        }

        T Generate(T x, T y)
        {
            auto xInt = static_cast<std::int32_t>(std::floor(x)) & 255;
            auto yInt = static_cast<std::int32_t>(std::floor(y)) & 255;

            x -= xInt;
            y -= yInt;

            T u = fade(x);
            T v = fade(y);

            T v1 = p[xInt]     + yInt;
            T v2 = p[xInt + 1] + yInt;
            T v3 = xInt + p[yInt + 1];
            T v4 = xInt + 1 + p[yInt + 1]);

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
                amp = pow(persistence, i); f
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

    private:
        std::int32_t p[512];
    };

}