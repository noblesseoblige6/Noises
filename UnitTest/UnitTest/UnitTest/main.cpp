#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "bitmap.h"
#include "../../../include/mlnoise/perlin_noise.h"
#include "../../../include/mlnoise/value_noise.h"
#include "../../../include/mlnoise/block_noise.h"

template<class T>
void Noise(const char* file, std::int32_t octarve, std::float_t freq, std::float_t amp)
{
    auto size = 512;
    Image* colorImg = Create_Image(size, size);

    T noise;
    for (auto i = 0; i < size; i++)
    {
        for (auto j = 0; j < size; j++)
        {
            auto res = noise.Noise_01(i * freq, j * freq, octarve, amp);
            unsigned char v = 255 * res;

            colorImg->data[i * size + j].r = v;
            colorImg->data[i * size + j].g = v;
            colorImg->data[i * size + j].b = v;
        }
    }

    Write_Bmp(file, colorImg);
    Free_Image(colorImg);
}


TEST_SUITE("Util")
{
    TEST_CASE("Random table")
    {
        std::uint32_t seed = 32;
        std::mt19937 gen(seed);

        constexpr auto N = 256;
        auto table = mlnoise::random_table<std::float_t, N>(gen);

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
        auto table = mlnoise::permutation_table<N>(gen);

        for (auto itr = table.begin(); itr < table.begin() + N; ++itr)
        {
            CHECK_EQ(*itr, *(itr + N));
        }
    }
}

TEST_SUITE("Noise")
{
    TEST_CASE("Lattice based noise")
    {
        constexpr auto octarve = 4;
        constexpr auto freq    = 1.0/128;
        constexpr auto amp     = 0.5f;

        Noise<mlnoise::BlockNoisef>("./image/block.bmp", octarve, freq, amp);
        Noise<mlnoise::ValueNoisef>("./image/value.bmp", octarve, freq, amp);
        Noise<mlnoise::PerlinNoisef>("./image/perlin.bmp", octarve, freq, amp);
    }
}