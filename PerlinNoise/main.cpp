#include <bits/stdc++.h>
#include "PerlinNoise2D.h"
#include "bitmap.h"

using namespace std;
const int RGB = 3;
int main()
{
  char file[256] = "PerlinNoise.bmp";
  int size = 512;
  float octerve = 8;
  float offsets[] = {15.0f, 10.0f, 21.0f};
  Image* colorImg = Create_Image(size, size);
  PerlinNoise2D perlin[RGB];
  for(int i = 0; i < RGB; i++){
    perlin[i].setRange(size, size);
    perlin[i].setPersistence(0.65);
    perlin[i].setOcterve(octerve);
    perlin[i].generate(offsets[i]);
  }
  for(int i = 0; i < size;i++){
    for(int j = 0; j < size; j++){
      int r = 255*((perlin[0].get(i*size+j)+1.0)/2.0);
      int g = 255*((perlin[1].get(i*size+j)+1.0)/2.0);
      int b = 255*((perlin[2].get(i*size+j)+1.0)/2.0);
      colorImg->data[i*size+j].r = r;
      colorImg->data[i*size+j].g = g;
      colorImg->data[i*size+j].b = b;
    }
  }
  Write_Bmp(file, colorImg);
  Free_Image(colorImg); 
  return 0;
}
