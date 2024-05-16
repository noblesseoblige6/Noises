#ifndef __BITMAP_H_INCLUDED__
#define __BITMAP_H_INCLUDED__

#define FILEHEADERSIZE 14					//•’•°•§•ÅEÿ•√•¿§Œ•µ•§•∫
#define INFOHEADERSIZE 40					//æ Û•ÿ•√•¿§Œ•µ•§•∫
#define HEADERSIZE (FILEHEADERSIZE+INFOHEADERSIZE)

typedef struct{
    unsigned char b;
    unsigned char g;
    unsigned char r;
}Rgb;

typedef struct{
    unsigned int height;
    unsigned int width;
    Rgb *data;
}Image;

//ºË∆¿§À¿Æ∏˘§π§ÅE–•›•§•Û•ø§Úº∫«‘§π§ÅE–NULL§Ú ÷§π
Image *Read_Bmp(char *filename);

//ΩÒ§≠π˛§ﬂ§À¿Æ∏˘§π§ÅE–0§Úº∫«‘§π§ÅE–1§Ú ÷§π
int Write_Bmp(const char *filename, Image *img);

//Image§Ú∫˚‹Æ§∑°¢RGBæ Û§‚width*height ¨§¿§±∆∞≈™§ÀºË∆¿§π§ÅE£
//¿Æ∏˘§π§ÅE–•›•§•Û•ø§Ú°¢º∫«‘§π§ÅE–NULL§Ú ÷§π
Image *Create_Image(int width, int height);
//Image§Ú≤ÚÀ°§π§ÅE
void Free_Image(Image *img);

#endif /*__BITMAP_H_INCLUDED__*/
