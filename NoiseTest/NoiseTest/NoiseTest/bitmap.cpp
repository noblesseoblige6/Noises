#include <iostream>
#include <fstream>
#include"bitmap.h"

namespace bmp
{
    Image* Image::Read(const char* filename)
    {
        char header_buf[HEADERSIZE];

        std::ifstream fs;
        fs.open(filename, std::ios::in | std::ios::binary);

        fs.read(header_buf, HEADERSIZE);

        if (strncmp((char*)header_buf, "BM", 2))
        {
            std::cerr << "Error: Not Bitmap file --> " << filename << std::endl;
            return nullptr;
        }

        std::uint32_t width, height, color;
        std::memcpy(&width , header_buf + 18, sizeof(width));
        std::memcpy(&height, header_buf + 22, sizeof(height));
        std::memcpy(&color , header_buf + 28, sizeof(color));

        if (color != 24)
        {
            std::cerr << "Error: Not 24bit color image --> " << filename << std::endl;
            return nullptr;
        }

        auto real_width = width * 3 + width % 4;
        char* bmp_line_data = new char[real_width];

        Image* pImage = new Image(width, height);
        if (pImage == nullptr)
        {
            delete[] bmp_line_data;
            return nullptr;
        }

        for (auto i = 0; i < height; i++)
        {
            fs.read(bmp_line_data, real_width);
            for (auto j = 0; j < width; j++)
            {
                pImage->m_pData[(height - i - 1) * width + j].b = bmp_line_data[j * 3];
                pImage->m_pData[(height - i - 1) * width + j].g = bmp_line_data[j * 3 + 1];
                pImage->m_pData[(height - i - 1) * width + j].r = bmp_line_data[j * 3 + 2];
            }
        }

        delete[] bmp_line_data;

        return pImage;
#if 0
        int i, j;
        int real_width;					//•«°º•øæÂ§Œ1π‘ ¨§Œ•–•§•»øÅE
        unsigned int width, height;			//≤Ë¡ÅEŒ≤£§»Ωƒ§Œ•‘•Ø•ª•ÅEÅE
        unsigned int color;			//≤øbit§ŒBitmap•’•°•§•ÅE«§¢§ÅE´
        FILE* fp;
        char header_buf[HEADERSIZE];	//•ÿ•√•¿æ Û§ÚºË§ÅE˛§ÅE
        unsigned char* bmp_line_data;  //≤Ë¡ÅE«°º•ø1π‘ ¨
        Image* img;

        fopen_s(&fp, filename, "rb");
        if (fp == nullptr)
        {
            fprintf(stderr, "Error: %s could not read.", filename);
            return nullptr;
        }

        fread(header_buf, sizeof(unsigned char), HEADERSIZE, fp); //•ÿ•√•¿…Ù ¨¡¥§∆§ÚºË§ÅE˛§ÅE


        if (strncmp((char*)header_buf, "BM", 2))
        {
            fprintf(stderr, "Error: %s is not Bitmap file.", filename);
            return nullptr;
        }

        memcpy(&width , header_buf + 18, sizeof(width));
        memcpy(&height, header_buf + 22, sizeof(height));
        memcpy(&color , header_buf + 28, sizeof(unsigned int));

        if (color != 24)
        {
            fprintf(stderr, "Error: %s is not 24bit color image", filename);
            return nullptr;
        }

        //RGBæ Û§œ≤Ë¡ÅEŒ1π‘ ¨§¨4byte§Œ«‹øÙ§«Ãµ§±§ÅE–§ §È§ §§§ø§·§Ω§ÅEÀπÁ§ÅEª§∆§§§ÅE
        real_width = width * 3 + width % 4;

        if ((bmp_line_data = (unsigned char*)malloc(sizeof(unsigned char) * real_width)) == nullptr)
        {
            fprintf(stderr, "Error: Allocation error.\n");
            return nullptr;
        }

        Image* pImage = new Image(width, height);
        if (pImage == nullptr)
        {
            free(bmp_line_data);
            fclose(fp);
            return nullptr;
        }

        //Bitmap•’•°•§•ÅEŒRGBæ Û§œ∫∏≤º§´§È±¶§ÿ°¢≤º§´§ÈæÂ§À ¬§Û§«§§§ÅE
        for (i = 0; i < height; i++)
        {
            fread(bmp_line_data, 1, real_width, fp);
            for (j = 0; j < width; j++)
            {
                pImage->m_pData[(height - i - 1) * width + j].b = bmp_line_data[j * 3];
                pImage->m_pData[(height - i - 1) * width + j].g = bmp_line_data[j * 3 + 1];
                pImage->m_pData[(height - i - 1) * width + j].r = bmp_line_data[j * 3 + 2];
            }
        }

        free(bmp_line_data);

        fclose(fp);

        return pImage;
#endif
    }

    bool Image::Write(const char* filename, Image* pImage)
    {
        std::ofstream fs(filename, std::ios_base::binary);
        if (!fs)
        {
            std::cerr << "Error: Failed to open --> " << filename << std::endl;
            return false;
        }

        char header_buf[HEADERSIZE];

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

        header_buf[0] = 'B';
        header_buf[1] = 'M';
        memcpy(header_buf + 2, &file_size, sizeof(file_size));
        header_buf[6] = 0;
        header_buf[7] = 0;
        header_buf[8] = 0;
        header_buf[9] = 0;
        memcpy(header_buf + 10, &offset_to_data, sizeof(file_size));
        header_buf[11] = 0;
        header_buf[12] = 0;
        header_buf[13] = 0;

        memcpy(header_buf + 14, &info_header_size, sizeof(info_header_size));
        header_buf[15] = 0;
        header_buf[16] = 0;
        header_buf[17] = 0;
        memcpy(header_buf + 18, &pImage->m_width, sizeof(pImage->m_width));
        memcpy(header_buf + 22, &pImage->m_height, sizeof(pImage->m_height));
        memcpy(header_buf + 26, &planes, sizeof(planes));
        memcpy(header_buf + 28, &color, sizeof(color));
        memcpy(header_buf + 30, &compress, sizeof(compress));
        memcpy(header_buf + 34, &data_size, sizeof(data_size));
        memcpy(header_buf + 38, &xppm, sizeof(xppm));
        memcpy(header_buf + 42, &yppm, sizeof(yppm));
        header_buf[46] = 0;
        header_buf[47] = 0;
        header_buf[48] = 0;
        header_buf[49] = 0;
        header_buf[50] = 0;
        header_buf[51] = 0;
        header_buf[52] = 0;
        header_buf[53] = 0;

        fs.write(header_buf, HEADERSIZE);

        char* bmp_line_data = new char[real_width];

        for (auto i = 0; i < pImage->m_height; i++)
        {
            for (auto j = 0; j < pImage->m_width; j++)
            {
                bmp_line_data[j * 3]     = pImage->m_pData[(pImage->m_height - i - 1) * pImage->m_width + j].b;
                bmp_line_data[j * 3 + 1] = pImage->m_pData[(pImage->m_height - i - 1) * pImage->m_width + j].g;
                bmp_line_data[j * 3 + 2] = pImage->m_pData[(pImage->m_height - i - 1) * pImage->m_width + j].r;
            }

            for (auto j = pImage->m_width * 3; j < real_width; j++)
            {
                bmp_line_data[j] = 0;
            }
            fs.write(bmp_line_data, real_width);
        }

        delete[] bmp_line_data;

        return true;
#if 0
        int i, j;
        FILE* fp;
        int real_width;
        unsigned char* bmp_line_data;
        unsigned char header_buf[HEADERSIZE];
        unsigned int file_size;
        unsigned int offset_to_data;
        unsigned long info_header_size;
        unsigned int planes;
        unsigned int color;
        unsigned long compress;
        unsigned long data_size;
        long xppm;
        long yppm;

        if ((fp = fopen(filename, "wb")) == nullptr)
        {
            fprintf(stderr, "Error: %s could not open.", filename);
            return 1;
        }

        real_width = pImage->m_width * 3 + pImage->m_width % 4;

        file_size = pImage->m_height * real_width + HEADERSIZE;
        offset_to_data = HEADERSIZE;
        info_header_size = INFOHEADERSIZE;
        planes = 1;
        color = 24;
        compress = 0;
        data_size = pImage->m_height * real_width;
        xppm = 1;
        yppm = 1;

        header_buf[0] = 'B';
        header_buf[1] = 'M';
        memcpy(header_buf + 2, &file_size, sizeof(file_size));
        header_buf[6] = 0;
        header_buf[7] = 0;
        header_buf[8] = 0;
        header_buf[9] = 0;
        memcpy(header_buf + 10, &offset_to_data, sizeof(file_size));
        header_buf[11] = 0;
        header_buf[12] = 0;
        header_buf[13] = 0;

        memcpy(header_buf + 14, &info_header_size, sizeof(info_header_size));
        header_buf[15] = 0;
        header_buf[16] = 0;
        header_buf[17] = 0;
        memcpy(header_buf + 18, &pImage->m_width , sizeof(pImage->m_width));
        memcpy(header_buf + 22, &pImage->m_height, sizeof(pImage->m_height));
        memcpy(header_buf + 26, &planes, sizeof(planes));
        memcpy(header_buf + 28, &color, sizeof(color));
        memcpy(header_buf + 30, &compress, sizeof(compress));
        memcpy(header_buf + 34, &data_size, sizeof(data_size));
        memcpy(header_buf + 38, &xppm, sizeof(xppm));
        memcpy(header_buf + 42, &yppm, sizeof(yppm));
        header_buf[46] = 0;
        header_buf[47] = 0;
        header_buf[48] = 0;
        header_buf[49] = 0;
        header_buf[50] = 0;
        header_buf[51] = 0;
        header_buf[52] = 0;
        header_buf[53] = 0;

        //•ÿ•√•¿§ŒΩÒ§≠π˛§ﬂ
        fwrite(header_buf, sizeof(unsigned char), HEADERSIZE, fp);

        if ((bmp_line_data = (unsigned char*)malloc(sizeof(unsigned char) * real_width)) == nullptr) {
            fprintf(stderr, "Error: Allocation error.\n");
            fclose(fp);
            return 1;
        }

        for (i = 0; i < pImage->m_height; i++)
        {
            for (j = 0; j < pImage->m_width; j++)
            {
                bmp_line_data[j * 3]     = pImage->m_pData[(pImage->m_height - i - 1) * pImage->m_width + j].b;
                bmp_line_data[j * 3 + 1] = pImage->m_pData[(pImage->m_height - i - 1) * pImage->m_width + j].g;
                bmp_line_data[j * 3 + 2] = pImage->m_pData[(pImage->m_height - i - 1) * pImage->m_width + j].r;
            }

            for (j = pImage->m_width * 3; j < real_width; j++)
            {
                bmp_line_data[j] = 0;
            }
            fwrite(bmp_line_data, sizeof(unsigned char), real_width, fp);
        }

        free(bmp_line_data);

        fclose(fp);

        return 0;
#endif
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