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
  persistence = 0.625;
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

double PerlinNoise2D::interpolate(double x, double y)
{
  vec2 d1(x - (int)x, y - (int)y), d2((int)x+1 - x, y - (int)y);
  vec2 d3(x - (int)x, (int)y+1 - y), d4((int)x+1 - x, (int)y+1 - y);
  vec2 c(sCurve(d1.x), sCurve(d1.y));
  vec2 g, w; 
  double s, t, u, v;

  g.x = noise2D(x, y); g.y = noise2D(y, x); 
  g = g.normalized();
  s = dot(g, d1);

  g.x = noise2D(x+1, y); g.y = noise2D(y, x); 
  g = g.normalized(); 
  t = dot(g, d2);

  g.x = noise2D(x, y); g.y = noise2D(y+1, x); 
  g = g.normalized();
  u = dot(g, d3);

  g.x = noise2D(x+1, y); g.y = noise2D(y+1, x); 
  g = g.normalized();
  v = dot(g, d4);

  //@comment Take average to Normalize the value of dot products
   s = (s)/2; t = (t)/2; 
   u = (u)/2; v = (v)/2; 
  

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

double PerlinNoise2D::gradientAt(double x, double y){
  double total = 0.0; 
  double freq = 1.0, amp = 1.0;
  for(int i = 0; i < octarveNum; i++){
    total += interpolate((x/(double)width)*freq, (y/(double)height)*freq)*amp;
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
      cout<<j<<" "<<noises[i*width+j]<<endl;
    }
    cout<<endl;
  }
}
