#include <iostream>
#include <fstream>
#include"bitmap.h"

namespace bmp
{
    Image* Image::Read(const char* filename)
    {
        std::ifstream fs(filename, std::ios::binary);
        if (!fs)
        {
            std::cerr << "Error: Not found file --> " << filename << std::endl;
            return nullptr;
        }

        char headerBuff[HEADERSIZE] = {};
        fs.read(headerBuff, HEADERSIZE);

        if (strncmp((char*)headerBuff, "BM", 2))
        {
            std::cerr << "Error: Not Bitmap --> " << filename << std::endl;
            return nullptr;
        }

        std::uint32_t width, height, color;
        std::memcpy(&width , headerBuff + 18, sizeof(width));
        std::memcpy(&height, headerBuff + 22, sizeof(height));
        std::memcpy(&color , headerBuff + 28, sizeof(color));

        if (color != 24)
        {
            std::cerr << "Error: Not 24bit color image --> " << filename << std::endl;
            return nullptr;
        }

        Image* pImage = new Image(width, height);
        if (pImage == nullptr)
            return nullptr;

        auto const real_width = width * 3 + width % 4;

        char* strideData = new char[real_width];
        for (auto i = 0; i < height; i++)
        {
            fs.read(strideData, real_width);
            for (auto j = 0; j < width; j++)
            {
                pImage->m_pData[(height - i - 1) * width + j].b = strideData[j * 3];
                pImage->m_pData[(height - i - 1) * width + j].g = strideData[j * 3 + 1];
                pImage->m_pData[(height - i - 1) * width + j].r = strideData[j * 3 + 2];
            }
        }
        delete[] strideData;

        return pImage;
    }

    bool Image::Write(const char* filename, Image* pImage)
    {
        std::ofstream fs(filename, std::ios::binary);
        if (!fs)
        {
            std::cerr << "Error: Failed to open --> " << filename << std::endl;
            return false;
        }

        auto real_width = pImage->m_width * 3 + pImage->m_width % 4;

        std::uint32_t file_size = pImage->m_height * real_width + HEADERSIZE;
        std::uint32_t offset_to_data = HEADERSIZE;
        unsigned long info_header_size = INFOHEADERSIZE;
        std::uint32_t planes = 1;
        std::uint32_t color = 24;
        unsigned long compress = 0;
        unsigned long data_size = pImage->m_height * real_width;
        long xppm = 1;
        long yppm = 1;

        char headerBuff[HEADERSIZE] = {};

        headerBuff[0] = 'B';
        headerBuff[1] = 'M';
        memcpy(headerBuff + 2 , &file_size       , sizeof(file_size));
        memcpy(headerBuff + 10, &offset_to_data  , sizeof(file_size));
        memcpy(headerBuff + 14, &info_header_size, sizeof(info_header_size));
        memcpy(headerBuff + 18, &pImage->m_width , sizeof(pImage->m_width));
        memcpy(headerBuff + 22, &pImage->m_height, sizeof(pImage->m_height));
        memcpy(headerBuff + 26, &planes          , sizeof(planes));
        memcpy(headerBuff + 28, &color           , sizeof(color));
        memcpy(headerBuff + 30, &compress        , sizeof(compress));
        memcpy(headerBuff + 34, &data_size       , sizeof(data_size));
        memcpy(headerBuff + 38, &xppm            , sizeof(xppm));
        memcpy(headerBuff + 42, &yppm            , sizeof(yppm));

        fs.write(headerBuff, HEADERSIZE);

        auto strideData = new char[real_width];
        for (auto i = 0; i < pImage->m_height; i++)
        {
            for (auto j = 0; j < pImage->m_width; j++)
            {
                strideData[j * 3]     = pImage->m_pData[(pImage->m_height - i - 1) * pImage->m_width + j].b;
                strideData[j * 3 + 1] = pImage->m_pData[(pImage->m_height - i - 1) * pImage->m_width + j].g;
                strideData[j * 3 + 2] = pImage->m_pData[(pImage->m_height - i - 1) * pImage->m_width + j].r;
            }

            for (auto j = pImage->m_width * 3; j < real_width; j++)
            {
                strideData[j] = 0;
            }
            fs.write(strideData, real_width);
        }

        delete[] strideData;

        return true;
    }

    Image::Image(std::uint32_t w, std::uint32_t h)
    {
        Create(w, h);
    }

    Image::~Image()
    {
        Free();
    }

    Image::Rgb& Image::Pixel(std::uint32_t x, std::uint32_t y)
    {
        return m_pData[x + y * m_width];
    }

    bool Image::Create(std::uint32_t width, std::uint32_t height)
    {
        m_width = width;
        m_height = height;

        m_pData = new Rgb[width * height];

        return true;
    }

    void Image::Free()
    {
        delete[] m_pData;
    }
}