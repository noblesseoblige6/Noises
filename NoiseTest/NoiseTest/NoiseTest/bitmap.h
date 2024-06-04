#pragma once

namespace bmp
{
    class Image
    {
    public:
        static constexpr auto FILEHEADERSIZE = 14;
        static constexpr auto INFOHEADERSIZE = 40;
        static constexpr auto HEADERSIZE     = (FILEHEADERSIZE+INFOHEADERSIZE);

        static Image* Read(const char* filename);
        static bool Write(const char* filename, Image* pImage);

        struct Rgb
        {
            unsigned char b;
            unsigned char g;
            unsigned char r;
        };

    public:
        Image() = delete;
        Image(std::uint32_t w, std::uint32_t h);
        ~Image();

        Rgb& Pixel(std::uint32_t x, std::uint32_t y);

    private:
        bool Create(std::uint32_t width, std::uint32_t height);
        void Free();

    private:
        std::uint32_t m_width;
        std::uint32_t m_height;

        Rgb* m_pData;
    };
}