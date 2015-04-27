#include "PerlinNoise2D.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>

using std::vector;
using std::cout;
using std::endl;
PerlinNoise2D::PerlinNoise2D()
{
  width = height = 256;
  octarveNum = 8; 
  persistence = 0.65;
  interpType = LINEAR;
  noises.clear();
}

PerlinNoise2D::~PerlinNoise2D(){}

double dot(vec2 a, vec2 b)
{
  return a.x*b.x + a.y*b.y;
}

double PerlinNoise2D::noise2D(int x, int y)
{
  int n = x + y * 57;
  n = (n<<13)^n;
  int nn = ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff);
  return  1.0 - ((double)nn / 1073741824.0);   
}

float random(float max)
{
  int r;
  float s;
  r = rand();
  s = (float)(r & 0x7fff)/(float)0x7fff;
  return (s * max);
}

int myfloor(float value)
{
  return (value >= 0 ? (int)value : (int)value-1);
}

double PerlinNoise2D::interpolate(double x, double y, int* permutations, float gradients[8][2])
{
  int xInt = myfloor(x), yInt = myfloor(y);
  double fracX = x - (double)xInt, fracY = y - (double)yInt;
  vec2 d1(fracX, fracY), d2(fracX-1.0, fracY);
  vec2 d3(fracX, fracY-1.0), d4(fracX-1.0, fracY-1.0);
  int grad11 = permutations[(xInt + permutations[yInt & 255]) & 255] & 7;
  int grad12 = permutations[(xInt + 1 + permutations[yInt & 255]) & 255] & 7;
  int grad21 = permutations[(xInt + permutations[(yInt + 1) & 255]) & 255] & 7;
  int grad22 = permutations[(xInt + 1 + permutations[(yInt + 1) & 255]) & 255] & 7;

  vec2 g1(gradients[grad11][0], gradients[grad11][1]);
  vec2 g2(gradients[grad12][0], gradients[grad12][1]);
  vec2 g3(gradients[grad21][0], gradients[grad21][1]);
  vec2 g4(gradients[grad22][0], gradients[grad22][1]);
 
  vec2 c(sCurve(fracX), sCurve(fracY));
  vec2 w; 
  double s, t, u, v;
  //@commnet normalize random vectors
  // g1 = g1.normalized(); g2 = g2.normalized();
  // g3 = g3.normalized(); g4 = g4.normalized();

  s = dot(g1, d1);
  t = dot(g2, d2);
  u = dot(g3, d3);
  v = dot(g4, d4);

  //@comment Take average to Normalize the value of dot products
  // s *= 0.5; t *= 0.5; 
  // u *= 0.5; v *= 0.5; 

  switch(interpType){
    case LINEAR:
      w.x = linearInterpolate(s, t, c.x); 
      w.y = linearInterpolate(u, v, c.x);
      return linearInterpolate(w.x, w.y, c.y);
      break;
    case COSINE:
      w.x = cosineInterpolate(s, t, c.x); 
      w.y = cosineInterpolate(u, v, c.x);
      return cosineInterpolate(w.x, w.y, c.y);
      break;
    default:
      break;
  }
}

double PerlinNoise2D::gradientAt(double x, double y)
{
  double total = 0.0; 
  double freq = 1.0/height, amp = 1.0;

  float gradients[8][2];
  for (int i=0;i<8;++i)
  {
    gradients[i][0] = cos(0.785398163f * (float)i);// 0.785398163 is PI/4.
    gradients[i][1] = sin(0.785398163f * (float)i);
  }

  //set up the random numbers table
  int permutations[256]; //make it as long as the largest dimension
  for(int i=0;i<256;++i){
    permutations[i] = i;//put each number in once
  }
  //randomize the random numbers table
  for(int i=0;i<256;++i){
    int j = (int)random(256);
    int k = permutations[i];
    permutations[i] = permutations[j];
    permutations[j] = k;
  }

  for(int i = 0; i < octarveNum; i++){
    total += interpolate(x*freq, y*freq, permutations, gradients)*amp;
    freq *= 2.0;
    amp *= persistence;
  }
  return total;
}

void PerlinNoise2D::setRange(int w, int h)
{
  width = w;
  height = h;
}

void PerlinNoise2D::generate()
{
  noises.clear();
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      noises.push_back(gradientAt(j, i));
    }
  }
}

void PerlinNoise2D::printData()
{
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      cout<<j<<" "<<noises[j*width+i]<<endl;
    }
    cout<<endl;
  }
}
