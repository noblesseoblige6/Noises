#pragma once
#include "mlnoise.h"

namespace mlnoise
{
    template<class T>
    class SimplexNoise : public detail::NoiseBase<SimplexNoise, T>
    {
    private:
        static constexpr std::int32_t TableMaxSize = 256;
        static constexpr std::int32_t TableMask = TableMaxSize - 1;

        static T grad(std::int32_t hash, T x, T y, T z) noexcept
        {
            auto h = hash & 15;

            T u = h < 8 ? x : y;
            T v = h < 4 ? y : h == 12 || h == 14 ? x : z;

            return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
        }

    public:
        SimplexNoise(RandomSeedType seed = 1024)
        {
            RandomEngineType gen(seed);

            m_permutations = detail::PermutationTable<std::uint8_t, TableMaxSize>(gen);
        }
        ~SimplexNoise() = default;

    public:
        std::pair<T, T> Test(T x, T y)
        {
            // Skew the input space to determine which simplex cell we're in
            const T F2 = (3 - std::sqrt(3)) / static_cast<T>(6);
            const T G2 = (std::sqrt(3) - 1) / static_cast<T>(2);

            T s = (x + y) * F2;
            auto i = static_cast<std::int32_t>(std::floor(x - s));
            auto j = static_cast<std::int32_t>(std::floor(y - s));

            // Unskew the cell origin back to (x,y) space
            T t = (i + j) * G2;
            T X0 = i + t;
            T Y0 = j + t;

            // The x,y distances from the cell origin
            T x0 = x - X0;
            T y0 = y - Y0;

            std::int32_t i1, j1;
            // lower triangle
            if (x0 > y0) 
            {
                i1 = 1; j1 = 0;
            }
            // uppper triangle
            else
            {
                i1 = 0; j1 = 1;
            }

            auto x1 = (x0 - i1) + G2;
            auto y1 = (y0 - j1) + G2;
            auto x2 = (x0 - 1) + 2 * G2;
            auto y2 = (y0 - 1) + 2 * G2;

            // Work out the hashed gradient indices of the three simplex corners
            std::int32_t ii = i & 255;
            std::int32_t jj = j & 255;
            std::int32_t gi0 = m_permutations[ii + m_permutations[jj]] % 12;
            std::int32_t gi1 = m_permutations[ii + i1 + m_permutations[jj + j1]] % 12;
            std::int32_t gi2 = m_permutations[ii + 1 + m_permutations[jj + 1]] % 12;

            
            // Calculate the contribution from the three corners
#if 1
            T t0 = 0.5 - x0 * x0 - y0 * y0;
            T n0 = (t0 < 0) ? 0.0 : t0 * t0 * t0 * t0;
            
            T t1 = 0.5 - x1 * x1 - y1 * y1;
            T n1 = (t1 < 0) ? 0.0 : t1 * t1 * t1 * t1;

            T t2 = 0.5 - x2 * x2 - y2 * y2;
            T n2 = (t2 < 0) ? 0.0 : t2 * t2 * t2 * t2;
#else
            T n0, n1, n2;

            T t0 = 0.5 - x0 * x0 - y0 * y0;
            if (t0 < 0) n0 = 0.0;
            else {
                t0 *= t0;
                //n0 = t0 * t0 * grad(gi0, x0, y0, 0); // (x,y) of grad3 used for 2D gradient
                n0 = t0 * t0; // (x,y) of grad3 used for 2D gradient
            }

            T t1 = 0.5 - x1 * x1 - y1 * y1;
            if (t1 < 0) n1 = 0.0;
            else {
                t1 *= t1;
                //n1 = t1 * t1 * grad(gi1, x1, y1, 0);
                n1 = t1 * t1;
            }

            T t2 = 0.5 - x2 * x2 - y2 * y2;
            if (t2 < 0) n2 = 0.0;
            else {
                t2 *= t2;
                //n2 = t2 * t2 * grad(gi2, x2, y2, 0);
                n2 = t2 * t2;
            }
#endif
            // Add contributions from each corner to get the final noise value.
            // The result is scaled to return valu
            //return 70.0 * (n0 + n1 + n2);
            return { 70.0 * (n0 + n1 + n2), 70.0 * (n0 + n1 + n2) };
        }

        T NoiseX(T x, T y)
        {
            return Test(x, y).first;
        }

        T NoiseY(T x, T y)
        {
            return Test(x, y).second;
        }

        T Noise(T x, T y, T z)
        {
            T n0, n1, n2, n3; // Noise contributions from the four corners

            // Skew the input space to determine which simplex cell we're in
            constexpr T F3 = 1.0 / 3.0;
            constexpr T G3 = 1.0 / 6.0; // Very nice and simple unskew factor, too

            T s = (x + y + z) * F3; // Very nice and simple skew factor for 3D
            auto i = static_cast<std::int32_t>(std::floor(x + s));
            auto j = static_cast<std::int32_t>(std::floor(y + s));
            auto k = static_cast<std::int32_t>(std::floor(z + s));

            T t = (i + j + k) * G3;
            T X0 = i - t; // Unskew the cell origin back to (x,y,z) space
            T Y0 = j - t;
            T Z0 = k - t;
            T x0 = x - X0; // The x,y,z distances from the cell origin
            T y0 = y - Y0;
            T z0 = z - Z0;

            // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
            // Determine which simplex we are in.
            std::int32_t i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
            std::int32_t i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
            if (x0 >= y0)
            {
                // X Y Z order
                if (y0 >= z0)
                {
                    i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
                }
                // X Z Y order
                else if (x0 >= z0)
                {
                    i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1;
                }
                // Z X Y order
                else
                {
                    i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1;
                }
            }
            else
            {
                // Z Y X order
                if (y0 < z0)
                {
                    i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1;
                }
                // Y Z X order
                else if (x0 < z0)
                {
                    i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1;
                }
                // Y X Z order
                else
                {
                    i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
                }
            }

            // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
            // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
            // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
            // c = 1/6.
            T x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
            T y1 = y0 - j1 + G3;
            T z1 = z0 - k1 + G3;

            T x2 = x0 - i2 + 2.0 * G3; // Offsets for third corner in (x,y,z) coords
            T y2 = y0 - j2 + 2.0 * G3;
            T z2 = z0 - k2 + 2.0 * G3;

            T x3 = x0 - 1.0 + 3.0 * G3; // Offsets for last corner in (x,y,z) coords
            T y3 = y0 - 1.0 + 3.0 * G3;
            T z3 = z0 - 1.0 + 3.0 * G3;

            // Work out the hashed gradient indices of the four simplex corners
            auto ii = i & 255;
            auto jj = j & 255;
            auto kk = k & 255;

            auto gi0 = m_permutations[ii + m_permutations[jj + m_permutations[kk]]] % 12;
            auto gi1 = m_permutations[ii + i1 + m_permutations[jj + j1 + m_permutations[kk + k1]]] % 12;
            auto gi2 = m_permutations[ii + i2 + m_permutations[jj + j2 + m_permutations[kk + k2]]] % 12;
            auto gi3 = m_permutations[ii + 1 + m_permutations[jj + 1 + m_permutations[kk + 1]]] % 12;

            // Calculate the contribution from the four corners
            T t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
            if (t0 < 0)
            {
                n0 = 0.0;
            }
            else 
            {
                t0 *= t0;
                n0 = t0 * t0 * grad(gi0, x0, y0, z0);
            }

            T t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
            if (t1 < 0)
            {
                n1 = 0.0;
            }
            else 
            {
                t1 *= t1;
                n1 = t1 * t1 * grad(gi1, x1, y1, z1);
            }

            T t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
            if (t2 < 0)
            {
                n2 = 0.0;
            }
            else
            {
                t2 *= t2;
                n2 = t2 * t2 * grad(gi2, x2, y2, z2);
            }

            T t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
            if (t3 < 0)
            {
                n3 = 0.0;
            }
            else
            {
                t3 *= t3;
                n3 = t3 * t3 * grad(gi3, x3, y3, z3);
            }

            // Add contributions from each corner to get the final noise value.
            // The result is scaled to stay just inside [-1,1]
            return 32.0 * (n0 + n1 + n2 + n3);
        }

    private:
        std::array<std::uint8_t, TableMaxSize * 2> m_permutations;
    };
}