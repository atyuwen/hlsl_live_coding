cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 fft;
}

float tetrahedron(float3 p, float r)
{
  float a = dot(p, float3(0.81649655, -0.3333333, 0.47140452));
  float b = dot(p, float3(0.00000000, -0.3333333, -0.94280905));
  float c = dot(p, float3(-0.81649655, -0.3333333, 0.47140452));
  float d = dot(p, float3(0, 1, 0));
  float f = max(a, max(b, max(c, d)));
  return max(f - r, 0);
}

float sierpinski_0(float3 p, float r)
{
  p = p * 2;
  float a = tetrahedron(p - float3(0, -3, 0) * r, r) / 2;
  float b = tetrahedron(p - float3(-2.4494898, 1, -1.4142135) * r, r) / 2;
  float c = tetrahedron(p - float3(2.4494898, 1, -1.4142135) * r, r) / 2;
  float d = tetrahedron(p - float3(0, 1, 2.8284271) * r, r) / 2;
  return min(a, min(b, min(c, d)));
}

float sierpinski_1(float3 p, float r)
{
  p = p * 2;
  float a = sierpinski_0(p - float3(0, -3, 0) * r, r) / 2;
  float b = sierpinski_0(p - float3(-2.4494898, 1, -1.4142135) * r, r) / 2;
  float c = sierpinski_0(p - float3(2.4494898, 1, -1.4142135) * r, r) / 2;
  float d = sierpinski_0(p - float3(0, 1, 2.8284271) * r, r) / 2;
  return min(a, min(b, min(c, d)));
}

float sierpinski(float3 p, float r)
{
  float3 q = p;
  p.x = dot(q.xz, float2(cos(time.x), sin(time.x)));
  p.z = dot(q.xz, float2(-sin(time.x), cos(time.x)));

  p = p * 2;
  float a = sierpinski_1(p - float3(0, -3, 0) * r, r) / 2;
  float b = sierpinski_1(p - float3(-2.4494898, 1, -1.4142135) * r, r) / 2;
  float c = sierpinski_1(p - float3(2.4494898, 1, -1.4142135) * r, r) / 2;
  float d = sierpinski_1(p - float3(0, 1, 2.8284271) * r, r) / 2;
  return min(a, min(b, min(c, d)));
}

float4 intersect(float3 ro,  float3 rd, float r)
{
  for (int i = 0; i < 32; ++i)
  {
    float d = sierpinski(ro, r);
    ro += d * rd;
    if (d < 0.5) return float4(ro, i);
  }
  return float4(ro, -1);
}

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
  float3 c = float3(950, 440, 0);
  float2 Rr = float2(100, 30);

  float3 p = float3(in_tex * view.xy, 0) - c;
  float3 eye = float3(0, sin(time.x * 0.4) * 300, -2000);

  // early cut by bounding sphere
  if (length(p) > Rr.x) 
    return float4(0.3, 0.3, 0.3, 1);

  float3 rd = normalize(p - eye);
  float4 rm = intersect(eye, rd, Rr.y);
  if (rm.w < 0)
    return float4(0.3, 0.3, 0.3, 1);

  // fake ao and lighting
  float aol = (33 - rm.w) / 38;
  aol = aol * aol;
  float3 col = lerp(float3(0.11, 0.13, 0.17), float3(1.28, 1.32, 1.12), aol);
  col += step(0.5, fft.x) * 0.2;
  return float4(col, 1);
}
