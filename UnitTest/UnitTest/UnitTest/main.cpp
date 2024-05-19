#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "bitmap.h"
#include "../../../include/mlnoise/perlin_noise.h"
#include "../../../include/mlnoise/value_noise.h"
TEST_SUITE("")
{
    TEST_CASE("")
    {
        int size = 512;

        Image* colorImg = Create_Image(size, size);
        mlnoise::PerlinNoise<std::float_t> perlin;
        mlnoise::ValueNoise<std::float_t> value;
        
        auto freq = 0.05f;
        auto amp = 0.5f;
        auto octarve = 8;

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                //std::cout << perlin.Noise(i, j, 8, 0.65) << std::endl;
                auto res = value.Noise_01(i * freq, j * freq, octarve, amp);
                unsigned char v = 255 * res;
                if (res > 1.0f)
                    std::cout << res << std::endl;
                colorImg->data[i * size + j].r = v;
                colorImg->data[i * size + j].g = v;
                colorImg->data[i * size + j].b = v;
            }
        }

        Write_Bmp("Noise.bmp", colorImg);
        Free_Image(colorImg);
    }
}