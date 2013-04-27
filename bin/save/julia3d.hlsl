cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float3 mul3d(float3 a, float3 b)
{
  float3 c;
  c.x = dot(a.xy, b.xz) - dot(a.yz, b.yz);
  c.y = dot(a.xy, b.yx);
  c.z = dot(a.xz, b.zx);
  return c;
}

float DE(float3 p)
{
  float3 b = float3(0.24, 0.38, 0.83);
  float3 c = p;
  float3 d = 1;
  [loop] for (int i = 0; i < 16; ++i)
  {
    d = 2 * mul3d(c, d);
    c = mul3d(c, c) + b;
    if (length(c) > 3.0) break;
  }
  float r = length(c);
  float dr = length(d);
  return 0.5 * log(r) * r / dr;
}

float4 ray_marching(float3 ro,  float3 rd)
{
  for (int i = 0; i < 64; ++i)
  {
    float d = DE(ro);
    ro += d * rd;
    if (d < 0.01) return float4(ro, i);
  }
  return float4(ro, -1);
}

float4 shade(float3 ro, float3 rd)
{
  float4 rm = ray_marching(ro, rd);
  if (rm.w < 0) return float4(0, 0, 0, 0);
   
  float3 p = rm.xyz;
  float k = DE(p);
  float gx = DE(p + float3(1e-5, 0, 0)) - k;
  float gy = DE(p + float3(0, 1e-5, 0)) - k;
  float gz = DE(p + float3(0, 0, 1e-5)) - k;
  float3 N = normalize(float3(gx, gy, gz));
  
  float ao = 0;
  ao += DE(p + 0.2 * N) * 2.5;
  ao += DE(p + 0.5 * N) * 1.0;

  float3 L = normalize(float3(-2, 1, 0.5));
  float3 C = float3(0.3, 0.2, 0.8);
  float D = 0.7;
  float A = 0.1;
  float3 col = (A + D * saturate(dot(L, N))) * ao * C;
  return float4(col , 1);
}

float4 ps_main(in float2 tc : TEXCOORD) : SV_TARGET
{
  float3 p = float3((tc * 2 - 1), 0);
  p.xy *= float2(view.x * view.w, -1);
 
  float3 param = mpos.xyz * float3(-0.005, 0.005, -0.1) + float3(0.4, 0.3, 18);
  float4 rot;
  sincos(param.x, rot.x, rot.y);
  sincos(param.y, rot.z, rot.w);
  
  float3 rt = float3(0, 0, 0);
  float3 ro = float3(rot.x * rot.w, abs(rot.y) * rot.z, rot.y);
  ro = ro * param.z;
  
  float3 cd = normalize(rt - ro);
  float3 cr = normalize(cross(cd, float3(0, 1, 0)));
  float3 cu = cross(cr, cd);
 
  float3 rd = normalize(p.x * cr + p.y * cu + 10 * cd);
  float4 radiance = shade(ro, rd);
  
  float3 col = float3(0.02, 0.02, 0.02);
  col = lerp(col, radiance.rgb, radiance.a);
  return float4(pow(col, 0.45), 1);
}