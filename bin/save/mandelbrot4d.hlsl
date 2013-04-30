cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float4 mul4d(float4 a, float4 b)
{
  float4 c;
  c.x = a.x * b.x - dot(a.yzw, b.yzw);
  c.yzw = cross(a.yzw, b.yzw) + b.x * a.yzw + a.x * b.yzw;
  return c;
}

float DE(float3 p)
{
  float4 c = 0;
  float4 d = 0;
  [loop] for (int i = 0; i < 16; ++i)
  {
    d = 2 * mul4d(c, d) + 1;
    p.z = -p.z;  // to break the boring symmetry
    c = mul4d(c, c) + float4(p, 0);    
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
    if (d < 0.002) return float4(ro, i);
  }
  return float4(ro, -1);
}

float4 shade(float3 ro, float3 rd)
{
  float4 rm = ray_marching(ro, rd);
  if (rm.w < 0) return float4(0, 0, 0, 0);
  float ao = 1 - rm.w / 64;
  float3 C = lerp(float3(0.8, 0.0, 0.3), float3(0.8, 0.8, 0.9), abs(snoise(rm.xyz)));
  return float4(C * ao, 1);
}

float4 ps_main(in float2 tc : TEXCOORD) : SV_TARGET
{
  float3 p = float3((tc * 2 - 1), 0);
  p.xy *= float2(view.x * view.w, -1);
 
  float3 param = mpos.xyz * float3(-0.002, 0.005, -0.1) + float3(0.2, 1.0, 14);
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