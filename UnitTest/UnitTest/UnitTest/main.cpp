#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <numeric>

#include "doctest.h"
#include "bitmap.h"

#include "../../../include/mlnoise/PerlinNoise.h"
#include "../../../include/mlnoise/ValueNoise.h"
#include "../../../include/mlnoise/BlockNoise.h"
#include "../../../include/mlnoise/SimplexNoise.h"

template<class Noise>
void Plot(const char* file, std::int32_t octarve, std::float_t freq, std::float_t amp)
{
    std::ofstream ofs(file);

    Noise noise;

    auto size = 512;
    for (auto i = 0; i < size; i++)
    {
        ofs << i << " " << noise.Fractal_01(i * freq, octarve, amp) << std::endl;
    }

    ofs.close();
}

template<class T>
std::pair<T, T> Skew(T x, T y)
{
    const T F2 = (std::sqrt(3) - 1) / static_cast<T>(2);

    T s = (x + y) * F2;
    auto i = std::floor(x + s);
    auto j = std::floor(y + s);

    return { i, j };
}

template<class T>
std::pair<T, T> Unskew(T i, T j)
{
    const T G2 = (3 - std::sqrt(3)) / static_cast<T>(6);

    T s = (i + j) * G2;
    auto x = i - s;
    auto y = j - s;

    return { x, y };
}

template<class T>
std::tuple<T, T, T> SquareInterpolate(T x, T y)
{
    auto X0 = std::floor(x);
    auto Y0 = std::floor(y);
    auto X1 = X0 + 1;
    auto Y1 = Y0 + 1;

    x -= X0;
    y -= Y0;

    T x0 = x - X0;
    T y0 = y - Y0;
    T x1 = x - X1;
    T y1 = y - Y1;

    constexpr T c = 2; // square length of diagonal of the squere
    T t0 = c - x0 * x0 - y0 * y0;
    T t1 = c - x1 * x1 - y0 * y0;
    T t2 = c - x0 * x0 - y1 * y1;
    T t3 = c - x1 * x1 - y1 * y1;

    // (x, y) is in the middle of the square
    T x_mid = 0.5, y_mid = 0.5;
    std::array<T, 4> t_max =
    {
        c - x_mid * x_mid - y_mid * y_mid,
        c - x_mid * x_mid - y_mid * y_mid,
        c - x_mid * x_mid - y_mid * y_mid,
        c - x_mid * x_mid - y_mid * y_mid,
    };

    // (x, y) is at the corner of left bottom
    std::array<T, 4> t_min =
    {
        c - 0 - 0,
        c - 1 - 0,
        c - 0 - 1,
        c - 1 - 1,
    };

    T res = t0 + t1 + t2 + t3;

    // normalize the value in [0, 1] by (v-min)/(max-min)
    T max = std::accumulate(t_max.begin(), t_max.end(), static_cast<T>(0));
    T min = std::accumulate(t_min.begin(), t_min.end(), static_cast<T>(0));

    return { x, y, (res - min) / (max - min) };
}

template<class T>
bool IsLeftSide(std::pair<T, T> a, std::pair<T, T> b)
{
    return (a.first * b.second - a.second * b.first) >= 0;
}

template<class T>
std::tuple<T, T, T> TriangleInterpolate(T x, T y, bool& isInTriangle)
{
    auto X0 = std::floor(x);
    auto Y0 = std::floor(y);
    auto X1 = X0 + 1;
    auto Y1 = Y0;
    auto X2 = X0 + 1/static_cast<T>(2);
    auto Y2 = Y0 + std::sqrt(3)/ static_cast<T>(2);

    auto v0 = std::pair<T, T>(X1 - X0, Y1 - Y0);
    auto v1 = std::pair<T, T>(X2 - X1, Y2 - Y1);
    auto v2 = std::pair<T, T>(X0 - X2, Y0 - Y2);

    // check if the point in the triangle
    if (!IsLeftSide(v0, std::pair<T, T>(x - X0, y - Y0)) ||
        !IsLeftSide(v1, std::pair<T, T>(x - X1, y - Y1)) ||
        !IsLeftSide(v2, std::pair<T, T>(x - X2, y - Y2)))
        isInTriangle = false;

    T x0 = x - X0;
    T y0 = y - Y0;

    T x1 = x - X1;
    T y1 = y - Y1;

    T x2 = x - X2;
    T y2 = y - Y2;

    constexpr T c = 3/static_cast<T>(4);
    T t0 = c - x0 * x0 - y0 * y0;
    T t1 = c - x1 * x1 - y1 * y1;
    T t2 = c - x2 * x2 - y2 * y2;

#if 0
    T n0 = (t0 < 0) ? 0 : (t0 * t0 * t0 * t0) * 2;
    T n1 = (t1 < 0) ? 0 : (t1 * t1 * t1 * t1) * 2;
    T n2 = (t2 < 0) ? 0 : (t2 * t2 * t2 * t2) * 2;

    T r = 1 / std::sqrt(3);
    T max = (c - r * r);
    max = 2 * max * max * max * max;
    max = 1 / (max + max + max);

    auto res = n0 + n1 + n2;
    return { x, y,  res * max };
#else
    T res = t0 + t1 + t2;

    // (x, y) is in the middle of the triangle
    T r = 1 / std::sqrt(3);
    std::array<T, 3> t_max =
    {
        c - r * r,
        c - r * r,
        c - r * r,
    };

    // (x, y) is at the corner of left bottom
    T x_top = 0.5f;
    T y_top = std::sqrt(3) * 0.5;
    std::array<T, 3> t_min =
    {
        c - 0 - 0,
        c - 1 - 0,
        c - x_top * x_top - y_top * y_top,
    };

    // normalize the value in [0, 1] by (v-min)/(max-min)
    T max = std::accumulate(t_max.begin(), t_max.end(), static_cast<T>(0));
    T min = std::accumulate(t_min.begin(), t_min.end(), static_cast<T>(0));

    return { x, y, (res - min ) / (max - min)};
#endif
}

template<class T>
std::tuple<T, T, T> SimplexInterpolate(T x, T y, bool& isInTrianle)
{
    const T F2 = (std::sqrt(3) - 1) / static_cast<T>(2);

    T s = (x + y) * F2;
    auto i = std::floor(x + s);
    auto j = std::floor(y + s);

    const T G2 = (3 - std::sqrt(3)) / static_cast<T>(6);

    T t = (i + j) * G2;
    auto X0 = i - t;
    auto Y0 = j - t;

    T x0 = x - X0;
    T y0 = y - Y0;

    std::int32_t i1 = 0, j1 = 0;
    (x0 > y0) ? i1 = 1 : j1 = 1;

    auto x1 = (x0 - i1) + G2;
    auto y1 = (y0 - j1) + G2;
    auto x2 = (x0 - 1) + 2 * G2;
    auto y2 = (y0 - 1) + 2 * G2;

    constexpr T c = 0.5;
    T t0 = c - x0 * x0 - y0 * y0;
    T t1 = c - x1 * x1 - y1 * y1;
    T t2 = c - x2 * x2 - y2 * y2;

#if 0
    T n0 = (t0 < 0) ? 0 : (t0 * t0 * t0 * t0) * 2;
    T n1 = (t1 < 0) ? 0 : (t1 * t1 * t1 * t1) * 2;
    T n2 = (t2 < 0) ? 0 : (t2 * t2 * t2 * t2) * 2;

    T r = std::sqrt(2) / 3;
    T max = c;
    max = 2 * (max * max * max * max);
    max = 1 / (max + max + max);

    auto res = n0 + n1 + n2;
    return { x, y,  res * max};
#else
    auto res = t0 + t1 + t2;

    // (x, y) is in the middle of the triangle
    T r = std::sqrt(2) / 3;
    std::array<T, 3> t_max =
    {
        c - r * r,
        c - r * r,
        c - r * r,
    };

    // (x, y) is at the corner of left bottom
    T x1_top = 1 - G2;
    T y1_top = -G2;
    T x2_top = 1 - 2 * G2;
    T y2_top = 1 - 2 * G2;
    std::array<T, 3> t_min =
    {
        c - 0 - 0,
        c - x1_top * x1_top - y1_top * y1_top,
        c - x2_top * x2_top - y2_top * y2_top,
    };

    // normalize the value in [0, 1] by (v-min)/(max-min)
    T max = std::accumulate(t_max.begin(), t_max.end(), static_cast<T>(0));
    T min = std::accumulate(t_min.begin(), t_min.end(), static_cast<T>(0));

    return { x, y,  (res - min) / (max - min) };
#endif
}

TEST_SUITE("Util")
{
    TEST_CASE("Random table")
    {
        std::uint32_t seed = 32;
        std::mt19937 gen(seed);

        constexpr auto N = 256;
        auto table = mlnoise::detail::RandomTable_11<std::float_t, N>(gen);

        for (auto itr = table.begin(); itr < table.begin() + N; ++itr)
        {
            CHECK_LE(*itr, 1.0f);
        }
    }

    TEST_CASE("Permutation table")
    {
        std::uint32_t seed = 32;
        std::mt19937 gen(seed);

        constexpr auto N = 256;
        auto table = mlnoise::detail::PermutationTable<std::uint8_t, N>(gen);

        for (auto itr = table.begin(); itr < table.begin() + N; ++itr)
        {
            CHECK_EQ(*itr, *(itr + N));
        }
    }
}

TEST_SUITE("Plot")
{
    TEST_CASE("Noise")
    {
        constexpr auto octarve = 1;
        constexpr auto freq    = 1.0f/32;
        constexpr auto amp     = 0.5f;

        Plot<mlnoise::BlockNoise<std::float_t>>  ("./plotData/block.dat", octarve, freq, amp);
        Plot<mlnoise::ValueNoise<std::float_t>>  ("./plotData/value.dat", octarve, freq, amp);
        Plot<mlnoise::PerlinNoise<std::float_t>> ("./plotData/perlin.dat", octarve, freq, amp);
        Plot<mlnoise::SimplexNoise<std::float_t>>("./plotData/simplex.dat", octarve, freq, amp);
    }


    TEST_CASE("Skew")
    {
        std::ofstream skew_ofs  ("./plotData/skew.dat");
        std::ofstream unskew_ofs("./plotData/unskew.dat");

        auto div = 32;
        for (auto y = 0; y < div; ++y)
        {
            for (auto x = 0; x < div; ++x)
            {
                auto skewed = Skew(static_cast<std::float_t>(x), static_cast<std::float_t>(y));
                auto unskewed = Unskew(static_cast<std::float_t>(skewed.first), static_cast<std::float_t>(skewed.second));

                skew_ofs << skewed.first << " " << skewed.second << std::endl;
                unskew_ofs << unskewed.first << " " << unskewed.second << std::endl;
            }
        }
    }

    TEST_CASE("Square Inteprolation")
    {
        std::ofstream fout("./plotData/square_interp.dat");

        auto div = 100;
        for (auto y = 0; y < div; ++y)
        {
            for (auto x = 0; x < div; ++x)
            {
                auto interp = SquareInterpolate(static_cast<std::double_t>(x) / div, static_cast<std::double_t>(y) / div);
                fout << std::get<0>(interp) << " " << std::get<1>(interp) << " " << std::get<2>(interp) << std::endl;
            }
            fout << std::endl;
        }
    }

    TEST_CASE("Triangle Inteprolation")
    {
        std::ofstream ofs("./plotData/tri_interp.dat");

        auto div = 100;
        for (auto x = 0; x < div; ++x)
        {
            bool canNewLine = false;
            for (auto y = 0; y < div; ++y)
            {
                auto isInTriangle = true;
                auto interp = TriangleInterpolate(static_cast<std::double_t>(x) / div, static_cast<std::double_t>(y) / div, isInTriangle);
                if (isInTriangle)
                {
                    ofs << std::get<0>(interp) << " " << std::get<1>(interp) << " " << std::get<2>(interp) << std::endl;
                    canNewLine = true;
                }
                else
                {
                    if (canNewLine)
                    {
                        ofs << std::endl;
                        canNewLine = false;
                    }
                }
            }

            if (canNewLine)
                ofs << std::endl;
        }
    }

    TEST_CASE("Simplex Inteprolation")
    {
        std::ofstream ofs("./plotData/simplex_interp.dat");

        auto div = 100;
        for (auto y = 0; y < div; ++y)
        {
            bool canNewLine = false;
            for (auto x = 0; x < div; ++x)
            {
                bool isInLowerTriangle = true;
                auto interp = SimplexInterpolate(static_cast<std::double_t>(x) / 32.f, static_cast<std::double_t>(y) / 32.f, isInLowerTriangle);
                if (isInLowerTriangle)
                {
                    ofs << std::get<0>(interp) << " " << std::get<1>(interp) << " " << std::get<2>(interp) << std::endl;
                    canNewLine = true;
                }
                else
                {
                    if (canNewLine)
                    {
                        ofs << std::endl;
                        canNewLine = false;
                    }
                }
            }

            if (canNewLine)
                ofs << std::endl;
        }
    }

    TEST_CASE("2-order equation smoothstep")
    {
        // f(x) = (1-x^2)^2
        std::ofstream ofs   ("./plotData/equation_2_0.dat");
        std::ofstream fd_ofs("./plotData/equation_2_1.dat");
        std::ofstream sd_ofs("./plotData/equation_2_2.dat");

        auto x = -1.f;
        for (auto i = 0; i < 128; ++i)
        {
            ofs << x << " " << std::pow(1 - x * x, 2) << std::endl;
            fd_ofs << x << " " << 4 * std::pow(x, 3) - 4 * x << std::endl;
            sd_ofs << x << " " << 12 * std::pow(x, 2) - 4 << std::endl;
            x += 2.0f / 128;
        }
    }

    TEST_CASE("3-order equation smoothstep")
    {
        // f(x) = (1-x^2)^3
        std::ofstream ofs   ("./plotData/equation_3_0.dat");
        std::ofstream fd_ofs("./plotData/equation_3_1.dat");
        std::ofstream sd_ofs("./plotData/equation_3_2.dat");

        auto x = -1.f;
        for (auto i = 0; i < 128; ++i)
        {
            ofs << x << " " << std::pow(1 - x * x, 3) << std::endl;
            fd_ofs << x << " " << -6 * std::pow(x, 5) + 12 * std::pow(x, 3) - 6 * x << std::endl;
            sd_ofs << x << " " << -30 * std::pow(x, 4) + 36 * std::pow(x, 2) - 6 << std::endl;
            x += 2.0f / 128;
        }
    }

    TEST_CASE("4-order equation smoothstep")
    {
        // f(x) = (1-x^2)^4
        std::ofstream ofs   ("./plotData/equation_4_0.dat");
        std::ofstream fd_ofs("./plotData/equation_4_1.dat");
        std::ofstream sd_ofs("./plotData/equation_4_2.dat");

        auto x = -1.f;
        for (auto i = 0; i < 128; ++i)
        {
            ofs << x << " " << std::pow(1 - x * x, 4) << std::endl;
            fd_ofs << x << " " << 8 * std::pow(x, 7) - 24 * std::pow(x, 5) + 24 * x * x * x - 8 * x << std::endl;
            sd_ofs << x << " " << 56 * std::pow(x, 6) - 120 * std::pow(x, 4) + 72 * x * x - 8 << std::endl;
            x += 2.0f / 128;
        }
    }
}