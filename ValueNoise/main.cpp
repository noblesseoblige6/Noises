#include <bits/stdc++.h>
#include "ValueNoise2D.h"
#include "bitmap.h"

using namespace std;
int main()
{
  char file[256] = "ValueNoise.bmp";
  int size = 512;
  Image* colorImg = Create_Image(size, size);
  ValueNoise2D noise;
  noise.setRange(size, size);
  noise.setPersistence(0.65);
  noise.setOcterve(8);
  noise.generate();
  for(int i = 0; i < size;i++){
    for(int j = 0; j < size; j++){
      int val = 256*((noise.get(i*size+j)+1.0)/2.0);
      colorImg->data[i*size+j].r = val;
      colorImg->data[i*size+j].g = val;
      colorImg->data[i*size+j].b = val;
    }
  }
  Write_Bmp(file, colorImg);
  Free_Image(colorImg); 
  return 0;
}
