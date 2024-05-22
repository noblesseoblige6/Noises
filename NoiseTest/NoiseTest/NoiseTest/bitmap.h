#ifndef __BITMAP_H_INCLUDED__
#define __BITMAP_H_INCLUDED__

#define FILEHEADERSIZE 14					//�ե�����Eإå��Υ�����
#define INFOHEADERSIZE 40					//����إå��Υ�����
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

//��������������EХݥ��󥿤��Ԥ���E�NULL���֤�
Image *Read_Bmp(char *filename);

//�񤭹��ߤ���������E�0���Ԥ���E�1���֤�
int Write_Bmp(const char *filename, Image *img);

//Image���ܮ����RGB�����width*heightʬ����ưŪ�˼�������E�
//��������EХݥ��󥿤򡢼��Ԥ���E�NULL���֤�
Image *Create_Image(int width, int height);
//Image���ˡ����E
void Free_Image(Image *img);

#endif /*__BITMAP_H_INCLUDED__*/
