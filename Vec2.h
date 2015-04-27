#pragma once
#include <cmath>

class vec2{
  public:
    double x;
    double y;
    double length;
    vec2():x(0.0),y(0.0),length(0.0){};
    ~vec2(){};
    vec2(double x, double y):x(x),y(y),length(sqrt(x*x+y*y)){};
    void norm(){length = sqrt(x*x + y*y);};
    vec2 normalized(){
      norm();
      return vec2(x/length, y/length);
    };
};


