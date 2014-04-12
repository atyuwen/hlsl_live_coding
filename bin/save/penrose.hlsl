cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float box(float3 p, float3 s)
{
  p = abs(p) - s;
  return min(max(p.x,max(p.y,p.z)), 0) + length(max(p, 0));
}

float3 rotateY(float3 p, float sint, float cost)
{
  float3 np;
  np.y = p.y;
  np.x = dot(p.xz, float2(cost, sint));
  np.z = dot(p.xz, float2(-sint, cost));
  return np;
}

float partA(float3 p)
{
  float k = saturate(sin(time.x * 2) * 0.5 - 0.2);
  p = rotateY(p, sin(k), cos(k));
  
  float a = box(p, float3(1, 6, 1));
  p = rotateY(p, 0, 1);
  p.xy -= float2(2, 5);
  a = min(a, box(p, float3(3, 1, 1))); 
  return a;
}

float partB(float3 p)
{
  float k = saturate(sin(time.x * 2) * 0.5 - 0.2);
  p = rotateY(p, sin(k), cos(k));
 
  float3 op = p;
  p = rotateY(p, -1, 0);
  p.xy -= float2(2, -5);
  return box(p, float3(3, 1, 1));
}

float partC(float3 p)
{
  p.xy -= float2(8, 5);
  p = rotateY(p, 0, 1);
  float c = box(p, float3(3, 1, 1));
  p = rotateY(p, -1, 0);
  p.xz -= float2(-2, 2);
  c = min(c, box(p, float3(3, 1, 1)));
  return c;
}

float DE(float3 p)
{  
  float a = partA(p);
  float b = partB(p);
  float c = partC(p);
  return min(min(a, b), c);
}

float4 ray_marching(float3 ro,  float3 rd)
{
  float4 A = float4(ro, 0);
  int i = 0;
  for (i = 0; i < 64; ++i)
  {
    float d = partA(A.xyz);
    A += float4(d * rd, d);
    if (d < 0.0003) break;
  }
  if (i > 63) A.w = 1000;
  
  float4 B = float4(ro, 0);
  for (i = 0; i < 64; ++i)
  {
    float d = partB(B.xyz);
    B += float4(d * rd, d);
    if (d < 0.0003) break;
  }
  if (i > 63) B.w = 1000;
  
  float4 C = float4(ro, 0);
  for (i = 0; i < 64; ++i)
  {
    float d = partC(C.xyz);
    C += float4(d * rd, d);
    if (d < 0.0003) break;
  }
  if (i > 63) C.w = 1000;
   
  float4 AB = A.w < B.w ? A : B;  
  if (B.w < 100) return AB;
  return AB.w < C.w ? AB : C;
}

float4 shade(float3 ro, float3 rd)
{
  float4 rm = ray_marching(ro, rd);
  if (rm.w > 100) return float4(0, 0, 0, 0);

  float3 p = rm.xyz - rd * 0.001;
  float k = DE(p);
  float gx = DE(p + float3(1e-5, 0, 0)) - k;
  float gy = DE(p + float3(0, 1e-5, 0)) - k;
  float gz = DE(p + float3(0, 0, 1e-5)) - k;
  float3 N = normalize(float3(gx, gy, gz));

  float3 col = float3(0.85, 0.53, 0.64);
  if (abs(N.x) > 0.5) col = float3(0.47, 0.52, 0.68);
  if (abs(N.y) > 0.3) col = float3(0.9, 0.9, 0.8);
  return float4(col, 1);
}

float4 ps_main(in float2 tc : TEXCOORD) : SV_TARGET
{
  float3 p = float3((tc * 2 - 1), 0);
  p.xy *= float2(view.x * view.w, -1);
 
  const float sinphi = sqrt(3.0) / 3.0;
  const float cosphi = sqrt(2.0 / 3.0);
  
  // ortho view
  p.x *= cosphi;
  float3 ro = p * 20 + float3(2, 14, 20);  
  float3 rd = float3(0, -sinphi, -cosphi);
 
  // 45 degree
  const float sc45 = sqrt(2.0) / 2.0;
  ro = rotateY(ro, sc45, sc45);
  rd = rotateY(rd, sc45, sc45);
  
  // shading
  float4 radiance = shade(ro, rd);
  
  float3 A = float3(0.8, 0.7, 0.6);
  if (frac((sin(tc.x * 122.39) + tc.y) * 239.383) < 0.001) A = 1.0;
  float3 B = float3(0.2, 0.8, 0.7);
  float3 col = lerp(A, B, 0.4 * tc.x + 0.7 * tc.y);
  col = lerp(col, radiance.rgb, radiance.a);
  return float4(col, 1);
}