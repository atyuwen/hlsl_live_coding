// migrated from iq's GLSL code

float4 __hash(float4 n)
{
  return frac(sin(n) * 43758.5453);
}

float qnoise(float2 x)
{
  float2 p = floor(x);
  float2 f = frac(x);
  f = f * f * (3.0 - 2.0 * f);
  float n = p.x + p.y * 57.0;
  float4 h = __hash(n + float4(113, 114, 170, 171));
  float2 v = lerp(h.xz, h.yw, f.x);
  return lerp(v.x, v.y, f.y);
}

float qnoise(float3 x)
{
  float3 p = floor(x);
  float3 f = frac(x);
  f = f * f * (3.0 - 2.0 * f);

  float n = dot(p, float3(1, 57, 113));
  float4 h0 = __hash(n + float4(0, 57, 113, 170));
  float4 h1 = __hash(n + float4(1, 58, 114, 171));

  float4 u = lerp(h0, h1, f.x);
  float2 v = lerp(u.xz, u.yw, f.y);
  return lerp(v.x, v.y, f.z);
}
