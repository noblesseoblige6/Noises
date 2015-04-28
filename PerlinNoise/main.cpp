#include <bits/stdc++.h>
#include "PerlinNoise2D.h"
#include "bitmap.h"

using namespace std;
int main()
{
  char file[256] = "PerlinNoise.bmp";
  int size = 512;
  Image* colorImg = Create_Image(size, size);
  PerlinNoise2D perlin;
  perlin.setRange(size, size);
  perlin.setPersistence(0.65);
  perlin.setOcterve(8);
  perlin.generate();
  for(int i = 0; i < size;i++){
    for(int j = 0; j < size; j++){
      int val = 256*((perlin.get(i*size+j)+1.0)/2.0);
      colorImg->data[i*size+j].r = val;
      colorImg->data[i*size+j].g = val;
      colorImg->data[i*size+j].b = val;
    }
  }
  Write_Bmp(file, colorImg);
  Free_Image(colorImg); 
  return 0;
}
