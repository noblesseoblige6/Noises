#include "PerlinNoise2D.h"
#include <iostream>
#include <cmath>

using std::vector;
using std::cout;
using std::endl;
PerlinNoise2D::PerlinNoise2D()
{
  square = 64;
  width = height = 128;
  octarveNum = 8;
  persistence = 0.625;
  interpType = LINEAR;
  noises.clear();
}

PerlinNoise2D::~PerlinNoise2D(){}

double PerlinNoise2D::noise2D(int x, int y)
{
  int n = x + y * 57;
  n = (n<<13)^n;
  int nn = ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff);
  return  1.0 - ((double)nn / 1073741824.0);   
}

double PerlinNoise2D::interpolate(double x, double y)
{
  vec2 d1(x - (int)x, y - (int)y);
  vec2 d2(x - (int)x+1, y - (int)y+1);
  vec2 c(d1.x, d1.y), g, w; 
  double s, t, u, v;
  
  g.x = noise((int)x); g.y = noise((int)y); 
  g = g.normalized();
  s = g.x*d1.x + g.y*d1.y;
  
  g.x = noise((int)x+1); g.y = noise((int)y); 
  g = g.normalized(); 
  t = g.x*d2.x + g.y*d1.y;

  g.x = noise((int)x); g.y = noise((int)y+1); 
  g = g.normalized();
  u = g.x*d1.x + g.y*d2.y;

  g.x = noise((int)x+1); g.y = noise((int)y+1); 
  g = g.normalized();
  v = g.x*d2.x + g.y*d2.y;

  switch(interpType){
    case LINEAR:
      w.x = linearInterpolate(s, t, c.x); w.y = linearInterpolate(u, v, c.x);
      return linearInterpolate(w.x, w.y, c.y);
      break;
    case COSINE:
      w.x = cosineInterpolate(s, t, c.x); w.y = cosineInterpolate(u, v, c.x);
      return cosineInterpolate(w.x, w.y, c.y);
      break;
    default:
      break;
  }
}

double PerlinNoise2D::gradientAt(double x, double y){
  double total = 0.0; 
  double freq = 1.0, amp = 1.0;
  for(int i = 0; i < octarveNum; i++){
    total += interpolate(x/(double)width*freq, y/(double)height*freq) * amp;
    // double xoffset = (noise((int)x)+1)/2, yoffset = (noise((int)y)+1)/2;
    // total += interpolate((x+xoffset)*freq, (y+yoffset)*freq) * amp;
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
      noises.push_back(gradientAt(i, j));
    }
  }
}

void PerlinNoise2D::printData()
{
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      cout<<i<<" "<<j<<" "<<noises[i*width+j]<<endl;
    }
  }
}
