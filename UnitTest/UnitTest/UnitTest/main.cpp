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
std::pair<T, T> Neighbor(T x, T y, T X0, T Y0)
{
    const T G2 = (3 - std::sqrt(3)) / static_cast<T>(6);

    std::ofstream neightbor_ofs("./plotData/neighbor.dat");

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
    
    auto x1 = (x0 + i1) + G2;
    auto y1 = (y0 + j1) + G2;
    auto x2 = (x0 + 1) + 2 * G2;
    auto y2 = (y0 + 1) + 2 * G2;

    neightbor_ofs << x0 << " " << y0 << std::endl;
    neightbor_ofs << x1 << " " << y1 << std::endl;
    neightbor_ofs << x2 << " " << y2 << std::endl;

    return { x, y };
}

TEST_SUITE("Util")
{
    TEST_CASE("Random table")
    {
        std::uint32_t seed = 32;
        std::mt19937 gen(seed);

        constexpr auto N = 256;
        auto table = mlnoise::detail::RandomTable<std::float_t, N>(gen);

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

    TEST_CASE("Skewing")
    {
        std::ofstream skew_ofs("./plotData/skew.dat");
        std::ofstream unskew_ofs("./plotData/unskew.dat");

        for (auto y = 0; y < 32; ++y)
        {
            for (auto x = 0; x < 32; ++x)
            {
                auto skewed = Skew(static_cast<std::float_t>(x), static_cast<std::float_t>(y));
                auto unskewed = Unskew(static_cast<std::float_t>(skewed.first), static_cast<std::float_t>(skewed.second));

                skew_ofs << skewed.first << " " << skewed.second << std::endl;
                unskew_ofs << unskewed.first << " " << unskewed.second << std::endl;

                if (x < 3 && y < 3)
                {
                    Neighbor(static_cast<std::float_t>(x), static_cast<std::float_t>(y), unskewed.first, unskewed.second);
                }
            }
        }

    }
}

TEST_SUITE("Noise")
{
    TEST_CASE("Plot")
    {
        constexpr auto octarve = 1;
        constexpr auto freq    = 1.0f/32;
        constexpr auto amp     = 0.5f;

        Plot<mlnoise::BlockNoise<std::float_t>>  ("./plotData/block.dat", octarve, freq, amp);
        Plot<mlnoise::ValueNoise<std::float_t>>  ("./plotData/value.dat", octarve, freq, amp);
        Plot<mlnoise::PerlinNoise<std::float_t>> ("./plotData/perlin.dat", octarve, freq, amp);
        Plot<mlnoise::SimplexNoise<std::float_t>>("./plotData/simplex.dat", octarve, freq, amp);
    }
}