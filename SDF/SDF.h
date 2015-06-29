#pragma once
#include<iostream>
#include <exception>
#include<vector>
#include<iterator>
#include<cmath>
using std::vector;
using std::iterator;
using std::exception;
struct Grid
{
  float dx;
  float dy;
  float distSq()
  {
    return dx*dx+dy*dy;
  }
  void set(float _x, float _y)
  {
    dx = _x;
    dy = _y;
  }
};

class SDF
{
  private:
    int width;
    int height;
    vector<vector<int> > distFld;
    Grid compare(vector<vector<Grid> >&, int, int, int, int);
    void initialize(vector<vector<Grid> >&, vector<vector<Grid> >&);
    void propagation(vector<vector<Grid> >&);
    void merge(vector<vector<Grid> >&, vector<vector<Grid> >&);

  public:
    SDF();
    ~SDF();
    SDF(int w, int h);
    void generate();
    float get(int x, int y);
    void set(vector<vector<int> >& in);
};
