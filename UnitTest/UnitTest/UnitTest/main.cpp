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
        
        auto freq = 8.0f / size;

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                //std::cout << perlin.Noise(i, j, 8, 0.65) << std::endl;
                int v = 255 * ((value.Noise(i * freq, j * freq, 8, 0.5) + 1.0) / 2.0);
                colorImg->data[i * size + j].r = v;
                colorImg->data[i * size + j].g = v;
                colorImg->data[i * size + j].b = v;
            }
        }

        Write_Bmp("Noise.bmp", colorImg);
        Free_Image(colorImg);
    }
}