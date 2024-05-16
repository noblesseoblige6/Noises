#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"bitmap.h"

//filename¤ÎBitmap¥Õ¥¡¥¤¥EòÆÉ¤ß¹ş¤ß¡¢¹â¤µ¤ÈÉı¡¢RGB¾ğÊó¤òimg¹½Â¤ÂÎ¤ËÆş¤EE
Image *Read_Bmp(char *filename)
{
    int i, j;
    int real_width;					//¥Ç¡¼¥¿¾å¤Î1¹ÔÊ¬¤Î¥Ğ¥¤¥È¿E
    unsigned int width, height;			//²èÁEÎ²£¤È½Ä¤Î¥Ô¥¯¥»¥EE
    unsigned int color;			//²¿bit¤ÎBitmap¥Õ¥¡¥¤¥EÇ¤¢¤E«
    FILE *fp;
    unsigned char header_buf[HEADERSIZE];	//¥Ø¥Ã¥À¾ğÊó¤ò¼è¤Eş¤E
    unsigned char *bmp_line_data;  //²èÁEÇ¡¼¥¿1¹ÔÊ¬
    Image *img;
    int error;

    if((error = fopen_s(&fp, filename, "rb")) != 0){
        fprintf(stderr, "Error: %s could not read.", filename);
        return NULL;
    }

    fread(header_buf, sizeof(unsigned char), HEADERSIZE, fp); //¥Ø¥Ã¥ÀÉôÊ¬Á´¤Æ¤ò¼è¤Eş¤E

    //ºÇ½é¤Î2¥Ğ¥¤¥È¤¬BM(Bitmap¥Õ¥¡¥¤¥EÎ°E¤Ç¤¢¤E«
    if(strncmp((char*)header_buf, "BM", 2)){
        fprintf(stderr, "Error: %s is not Bitmap file.", filename);
        return NULL;
    }

    memcpy(&width, header_buf + 18, sizeof(width)); //²èÁEÎ¸«¤¿ÌÜ¾å¤ÎÉı¤ò¼èÆÀ
    memcpy(&height, header_buf + 22, sizeof(height)); //²èÁEÎ¹â¤µ¤ò¼èÆÀ
    memcpy(&color, header_buf + 28, sizeof(unsigned int)); //²¿bit¤ÎBitmap¤Ç¤¢¤E«¤ò¼èÆÀ

    //24bit¤ÇÌµ¤±¤EĞ½ªÎ»
    if(color != 24){
        fprintf(stderr, "Error: %s is not 24bit color image", filename);
        return NULL;
    }

    //RGB¾ğÊó¤Ï²èÁEÎ1¹ÔÊ¬¤¬4byte¤ÎÇÜ¿ô¤ÇÌµ¤±¤EĞ¤Ê¤é¤Ê¤¤¤¿¤á¤½¤EË¹ç¤E»¤Æ¤¤¤E
    real_width = width*3 + width%4;

    //²èÁEÎ1¹ÔÊ¬¤ÎRGB¾ğÊó¤ò¼è¤Ã¤Æ¤¯¤E¿¤á¤Î¥Ğ¥Ã¥Õ¥¡¤òÆ°Åª¤Ë¼èÆÀ
    if((bmp_line_data = (unsigned char *)malloc(sizeof(unsigned char)*real_width)) == NULL){
        fprintf(stderr, "Error: Allocation error.\n");
        return NULL;
    }

    //RGB¾ğÊó¤ò¼è¤Eş¤à¤¿¤á¤Î¥Ğ¥Ã¥Õ¥¡¤òÆ°Åª¤Ë¼èÆÀ
    if((img = Create_Image(width, height)) == NULL){
        free(bmp_line_data);
        fclose(fp);
        return NULL;
    }

    //Bitmap¥Õ¥¡¥¤¥EÎRGB¾ğÊó¤Ïº¸²¼¤«¤é±¦¤Ø¡¢²¼¤«¤é¾å¤ËÊÂ¤ó¤Ç¤¤¤E
    for(i=0; i<height; i++){
        fread(bmp_line_data, 1, real_width, fp);
        for(j=0; j<width; j++){
            img->data[(height-i-1)*width + j].b = bmp_line_data[j*3];
            img->data[(height-i-1)*width + j].g = bmp_line_data[j*3 + 1];
            img->data[(height-i-1)*width + j].r = bmp_line_data[j*3 + 2];
        }
    }

    free(bmp_line_data);

    fclose(fp);

    return img;
}

int Write_Bmp(const char *filename, Image *img)
{
    int i, j;
    FILE *fp;
    int real_width;
    unsigned char *bmp_line_data; //²èÁE¹ÔÊ¬¤ÎRGB¾ğÊó¤ò³ÊÇ¼¤¹¤E
    unsigned char header_buf[HEADERSIZE]; //¥Ø¥Ã¥À¤ò³ÊÇ¼¤¹¤E
    unsigned int file_size;
    unsigned int offset_to_data;
    unsigned long info_header_size;
    unsigned int planes;
    unsigned int color;
    unsigned long compress;
    unsigned long data_size;
    long xppm;
    long yppm;
    int error;

    if((error = fopen_s(&fp, filename, "wb")) != 0){
        fprintf(stderr, "Error: %s could not open.", filename);
        return 1;
    }

    real_width = img->width*3 + img->width%4;

    //¤³¤³¤«¤é¥Ø¥Ã¥ÀºûÜ®
    file_size = img->height * real_width + HEADERSIZE;
    offset_to_data = HEADERSIZE;
    info_header_size = INFOHEADERSIZE;
    planes = 1;
    color = 24;
    compress = 0;
    data_size = img->height * real_width;
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
    memcpy(header_buf + 18, &img->width, sizeof(img->width));
    memcpy(header_buf + 22, &img->height, sizeof(img->height));
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

    //¥Ø¥Ã¥À¤Î½ñ¤­¹ş¤ß
    fwrite(header_buf, sizeof(unsigned char), HEADERSIZE, fp);
    
    if((bmp_line_data = (unsigned char *)malloc(sizeof(unsigned char)*real_width)) == NULL){
        fprintf(stderr, "Error: Allocation error.\n");
        fclose(fp);
        return 1;
    }

    //RGB¾ğÊó¤Î½ñ¤­¹ş¤ß
    for(i=0; i<img->height; i++){
        for(j=0; j<img->width; j++){
            bmp_line_data[j*3]			=	img->data[(img->height - i - 1)*img->width + j].b;
            bmp_line_data[j*3 + 1]	=	img->data[(img->height - i - 1)*img->width + j].g;
            bmp_line_data[j*3 + 2]			=	img->data[(img->height - i - 1)*img->width + j].r;
        }
        //RGB¾ğÊó¤E¥Ğ¥¤¥È¤ÎÇÜ¿ô¤Ë¹ç¤E»¤Æ¤¤¤E
        for(j=img->width*3; j<real_width; j++){
            bmp_line_data[j] = 0;
        }
        fwrite(bmp_line_data, sizeof(unsigned char), real_width, fp);
    }

    free(bmp_line_data);

    fclose(fp);

    return 0;
}

Image *Create_Image(int width, int height)
{
    Image *img;

    if((img = (Image *)malloc(sizeof(Image))) == NULL){
        fprintf(stderr, "Allocation error\n");
        return NULL;
    }

    if((img->data = (Rgb*)malloc(sizeof(Rgb)*width*height)) == NULL){
        fprintf(stderr, "Allocation error\n");
        free(img);
        return NULL;
    }

    img->width = width;
    img->height = height;

    return img;
}

//Æ°Åª¤Ë¼èÆÀ¤·¤¿RGB¾ğÊó¤Î³«ÊE
void Free_Image(Image *img)
{
    free(img->data);
    free(img);
}

