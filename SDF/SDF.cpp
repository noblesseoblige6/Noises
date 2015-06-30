#include "SDF.h"

SDF::SDF()
{
  width = -1;
  height = -1;
}
SDF::SDF(int w, int h)
{
  width = w;
  height = h;
}
SDF::~SDF(){}

Grid min(Grid a, Grid b)
{
  if(a.distSq() < b.distSq()){return a;}
  return b;
}

float SDF::get(int x, int y)
{
  return distFld[x][y];
}

void SDF::set(vector<vector<int> >& in)
{
  try{
    if(width < 0 || height < 0){
      width = in.size();
      height = in[0].size();
    }
    distFld.resize(width);
    for(int i = 0; i < width; i++){
      distFld[i].resize(height);
      for(int j = 0; j < height; j++){
        distFld[i][j] = in[i][j];
      }
    }
  }
  catch(std::exception& e) {
    std::cerr << "Exception caught: " << e.what() << '\n';
  }
}

Grid SDF::compare(vector<vector<Grid> >& g, int x, int y, int offsetx, int offsety)
{
  Grid center = g[x][y];
  if(x+offsetx >= width || x+offsetx < 0){return center;}
  else if(y+offsety >= height || y+offsety < 0){return center;}

  Grid other = g[x+offsetx][y+offsety];
  other.dx += offsetx;
  other.dy += offsety;
  if(other.distSq() < center.distSq()){return other;}
  return center;
}

void SDF::initialize(vector<vector<Grid> >& g1, vector<vector<Grid> >& g2)
{
  float inf = 9999;
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++){
      if(distFld[j][i] < 127){
        g1[j][i].set(0, 0);
        g2[j][i].set(inf, inf);
      }else{
        g1[j][i].set(inf, inf);
        g2[j][i].set(0, 0);
      }
    }
  }
}

void SDF::propagation(vector<vector<Grid> >& g)
{
  //@comment Scan the image from bottom to top.
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++){
      Grid tmpMin;
      tmpMin = min(compare(g, j, i, -1, 0), compare(g, j, i, -1, -1));
      tmpMin = min(compare(g, j, i, 0, -1), tmpMin);
      tmpMin = min(compare(g, j, i, 1, -1), tmpMin);
      g[j][i] = tmpMin;
    }   
    //@comment visit all pixel in i-th row.
    for(int j = width - 1; j >= 0; j--){
      Grid tmp = g[j][i];
      tmp = min(tmp, compare(g, j, i, 1, 0));
      g[j][i] = tmp;
    }
  }

  //@comment Scan the image from top to bottom.
  for(int i = height-1; i >= 0; i--){
    for(int j = width-1; j >= 0; j--){
      Grid tmpMin;
      tmpMin = min(compare(g, j, i, 1, 0), compare(g, j, i, 1, 1));
      tmpMin = min(compare(g, j, i, 0, 1), tmpMin);
      tmpMin = min(compare(g, j, i, -1, 1), tmpMin);
      g[j][i] = tmpMin;
    }   
    //@comment visit all pixel in i-th row.
    for(int j = 0; j < width; j++){
      Grid tmp = g[j][i];
      tmp = min(tmp, compare(g, j, i, -1, 0));
      g[j][i] = tmp;
    }
  }
}


void SDF::merge(vector<vector<Grid> >& g1, vector<vector<Grid> >& g2)
{
  float dis;
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++){
      dis = (int)(sqrt(g1[j][i].distSq()) - sqrt(g2[j][i].distSq()));
      //@comment clamp the value from 0~255
      // dis = dis * 3 + 128;
      if(dis < 0){dis = 0;}
      else if(dis > 255){dis = 255;}
      distFld[j][i] = dis;
    }
  }
}

void SDF::generate()
{
  vector<vector<Grid> > g1(width, vector<Grid>(height));
  vector<vector<Grid> > g2(width, vector<Grid>(height));
  initialize(g1, g2);
  propagation(g1);
  propagation(g2);
  merge(g1, g2);
}
