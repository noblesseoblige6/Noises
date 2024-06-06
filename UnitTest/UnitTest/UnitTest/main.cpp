#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
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
    if (x+s < y+s)
        isInTrianle = false;

    auto x1 = (x0 - i1) + G2;
    auto y1 = (y0 - j1) + G2;
    auto x2 = (x0 - 1) + 2 * G2;
    auto y2 = (y0 - 1) + 2 * G2;

    constexpr T c = 0.5;
    T t0 = c - x0 * x0 - y0 * y0;
    T t1 = c - x1 * x1 - y1 * y1;
    T t2 = c - x2 * x2 - y2 * y2;

    return { x + s, y + s, t1};
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
        std::ofstream interp_ofs("./plotData/simplex_interp.dat");

        for (auto y = 0; y < 32; ++y)
        {
            bool canNewLine = false;
            for (auto x = 0; x < 32; ++x)
            {
                auto skewed = Skew(static_cast<std::float_t>(x/32.f), static_cast<std::float_t>(y/32.f));
                auto unskewed = Unskew(static_cast<std::float_t>(skewed.first), static_cast<std::float_t>(skewed.second));

                skew_ofs << skewed.first << " " << skewed.second << std::endl;
                unskew_ofs << unskewed.first << " " << unskewed.second << std::endl;

                bool isInLowerTriangle = true;
                auto interp = SimplexInterpolate(static_cast<std::float_t>(x/32.f), static_cast<std::float_t>(y/32.f), isInLowerTriangle);
                if (isInLowerTriangle)
                {
                    interp_ofs << std::get<0>(interp) << " " << std::get<1>(interp) << " " << std::get<2>(interp) << std::endl;
                    canNewLine = true;
                }
                else
                {
                    if (canNewLine)
                    {
                        interp_ofs << std::endl;
                        canNewLine = false;
                    }
                }
            }

            if(canNewLine)
                interp_ofs << std::endl;
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