#include <bits/stdc++.h>
#include "SDF.h"
#include "bitmap.h"

using namespace std;
const int RGB = 3;
int main()
{
  char file[256] = "test.bmp";
  Image* inputImg = Read_Bmp(file);
  int w = inputImg->width;
  int h = inputImg->height;
  vector<vector<int> > in(w, vector<int>(h));
  for(int i = 0; i < w;i++){
    for(int j = 0; j < h; j++){
      in[i][j] = inputImg->data[j*w+i].r;
    }
  }
  SDF df(w, h); 
  df.set(in);
  df.generate();
  char out[256] = "distanceField.bmp";
  Image* outImg = Create_Image(w, h);
  for(int i = 0; i < w;i++){
    for(int j = 0; j < h; j++){
      int c = df.get(i, j);
      // int c = in[i][j];
      outImg->data[j*w+i].r = c;
      outImg->data[j*w+i].g = c;
      outImg->data[j*w+i].b = c;
    }
  }
  Write_Bmp(out, outImg);
  Free_Image(outImg); 
  return 0;
}
