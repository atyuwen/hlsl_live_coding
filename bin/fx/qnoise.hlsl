// migrated from iq's GLSL code

float __hash(float n)
{
  return frac(sin(n) * 43758.5453);
}

float qnoise(in float2 x)
{
  float2 p = floor(x);
  float2 f = frac(x);
  f = f * f * (3.0 - 2.0 * f);
  float n = p.x + p.y * 57.0;
  float res = lerp(lerp(__hash(n+113.0), __hash(n+114.0),f.x),
                   lerp(__hash(n+170.0), __hash(n+171.0),f.x),f.y);
  return res;
}

float qnoise(in float3 x)
{
  float3 p = floor(x);
  float3 f = frac(x);
  f = f * f * (3.0 - 2.0 * f);
  float n = p.x + p.y * 57.0 + 113.0 * p.z;
  float res = lerp(lerp(lerp(__hash(n+  0.0), __hash(n+  1.0),f.x),
                        lerp(__hash(n+ 57.0), __hash(n+ 58.0),f.x),f.y),
                   lerp(lerp(__hash(n+113.0), __hash(n+114.0),f.x),
                        lerp(__hash(n+170.0), __hash(n+171.0),f.x),f.y),f.z);
  return res;
}
