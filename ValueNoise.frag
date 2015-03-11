precision mediump float;
uniform float time;
uniform vec2  mouse;
uniform vec2  resolution;

const int   oct  = 8;
const float per  = 0.5;
const float PI   = 3.1415926;
const float cCorners = 1.0 / 16.0;
const float cSides   = 1.0 / 8.0;
const float cCenter  = 1.0 / 4.0;

// •âŠÔŠÖ”
float interpolate(float a, float b, float x){
  float f = (1.0 - cos(x * PI)) * 0.5;
  return a * (1.0 - f) + b * f;
}

// —”¶¬
float rnd(vec2 p){
  return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}

float smoothNoise(vec2 p){
  float center = rnd(vec2(p.x, p.y))*cCenter;
  float side = (rnd(vec2(p.x+1.0, p.y))+rnd(vec2(p.x-1.0, p.y))+rnd(vec2(p.x, p.y+1.0))+rnd(vec2(p.x, p.y-1.0)))*cSides;
  float corner = (rnd(vec2(p.x+1.0, p.y+1.0))+rnd(vec2(p.x+1.0, p.y-1.0))+rnd(vec2(p.x-1.0, p.y+1.0))+rnd(vec2(p.x-1.0, p.y-1.0)))*cCorners;
  return corner+side+center;
}

// •âŠÔ—”
float irnd(vec2 p){
  vec2 i = floor(p);
  vec2 f = fract(p);
  vec4 v = vec4(smoothNoise(vec2(i.x,       i.y      )),
      smoothNoise(vec2(i.x + 1.0, i.y      )),
      smoothNoise(vec2(i.x,       i.y + 1.0)),
      smoothNoise(vec2(i.x + 1.0, i.y + 1.0)));

  return interpolate(interpolate(v.x, v.y, f.x), interpolate(v.z, v.w, f.x), f.y);
}

// ƒmƒCƒY¶¬
float noise(vec2 p){
  float t = 0.0;
  for(int i = 0; i < oct; i++){
    float freq = pow(2.0, float(i));
    float amp  = pow(per, float(oct-i));
    t += irnd(vec2(p.x / freq, p.y / freq)) * amp;
  }
  return t;
}

void main(void){
  // noise
  vec2 t = gl_FragCoord.xy;
  float n = noise(t);

  gl_FragColor = vec4(vec3(n), 1.0);
}
