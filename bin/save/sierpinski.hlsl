cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float DE(float3 p)
{
  for (int i = 0; i < 16; ++i)
  {
    if (p.x + p.y < 0) p.xy = -p.yx;
    if (p.x + p.z < 0) p.xz = -p.zx;
    if (p.z + p.y < 0) p.zy = -p.yz;
    p = 2 * p - 1;
  }
  return length(p - 1) * pow(2, -16);
}

float4 ray_marching(float3 ro,  float3 rd)
{
  for (int i = 0; i < 64; ++i)
  {
    float d = DE(ro);
    ro += d * rd;
    if (d < 0.003) return float4(ro, i);
  }
  return float4(ro, -1);
}

float4 shade(float3 ro, float3 rd)
{
  float4 rm = ray_marching(ro, rd);
  if (rm.w < 0) return float4(0, 0, 0, 0);

  float aol = 1 - rm.w / 64.0;
  float3 diff = float3(0.1, 0.2, 0.2);
  diff += float3(0.08, 0.04, 0.02) * snoise(rm.xyz * 10);
  return float4(diff * (aol * aol), 1);
}

float4 ps_main(in float2 tc : TEXCOORD) : SV_TARGET
{
  float3 p = float3((tc * 2 - 1), 0);
  p.xy *= float2(view.x * view.w, -1);
 
  float3 gx = normalize(float3( 1, -1, 0));
  float3 gy = normalize(float3( 1,  1, 1)); 
  float3 gz = normalize(float3(-1, -1, 2));

  float3 param = mpos.xyz * float3(-0.005, 0.005, -0.1) + float3(0.2, 0, 8);
  float4 rot;
  sincos(param.x, rot.x, rot.y);
  sincos(param.y, rot.z, rot.w);
  
  float3 rt = float3(0.25, 0.25, 0.25);
  float3 ro = rot.x * rot.w * gx + abs(rot.y) * rot.z * gy + rot.y * gz;
  ro = ro * param.z;
  
  float3 cd = normalize(rt - ro);
  float3 cr = normalize(cross(cd, gy));
  float3 cu = cross(cr, cd);
 
  float3 rd = normalize(p.x * cr + p.y * cu + 5 * cd);
  float4 radiance = shade(ro, rd);
  
  float3 col = float3(0.02, 0.02, 0.02);
  col = lerp(col, radiance.rgb, radiance.a);
  return float4(pow(col, 0.45), 1);
}