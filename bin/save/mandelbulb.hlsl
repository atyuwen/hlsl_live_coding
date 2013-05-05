cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float DE(float3 p)
{
  float3 c = p;
  float r = length(c);
  float dr = 1;
  for (int i = 0; i < 4 && r < 3; ++i)
  {
    float xr = pow(r, 7);
    dr = 6 * xr * dr + 1;
  
    float theta = atan2(c.y, c.x) * 8;
    float phi = asin(c.z / r) * 8;
    r = xr * r;
    c = r * float3(cos(phi) * cos(theta), cos(phi) * sin(theta), sin(phi));
   
    c += p;
    r = length(c);
  }
  return 0.35 * log(r) * r / dr;
}

float4 ray_marching(float3 ro,  float3 rd)
{
  for (int i = 0; i < 64; ++i)
  {
    float d = DE(ro);
    ro += d * rd;
    if (d < 0.001) return float4(ro, i);
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
  ao += DE(p + 0.1 * N) * 2.5;
  ao += DE(p + 0.2 * N) * 1.0;

  float3 L = normalize(float3(-1, 1, 2));
  float4 S = ray_marching(p + N * 0.01, L);
  float3 C = lerp(float3(0.6, 0.8, 0.6), float3(1.0, 0.0, 0.0), rm.w / 64);
  float D = 0.7 * (S.w < 0 ? 1 : 0);
  
  float A = 0.1;
  float3 col = (A + D * saturate(dot(L, N))) * ao * C;
  return float4(col , 1);
}

float4 ps_main(in float2 tc : TEXCOORD) : SV_TARGET
{
  float3 p = float3((tc * 2 - 1), 0);
  p.xy *= float2(view.x * view.w, -1);
 
  float3 param = mpos.xyz * float3(-0.005, 0.005, -0.1) + float3(0.4, 0.3, 4.5);
  float4 rot;
  sincos(param.x, rot.x, rot.y);
  sincos(param.y, rot.z, rot.w);
  
  float3 rt = float3(0, 0, 0);
  float3 ro = float3(rot.x * rot.w, abs(rot.y) * rot.z, rot.y);
  ro = ro * param.z;
  
  float3 cd = normalize(rt - ro);
  float3 cr = normalize(cross(cd, float3(0, 1, 0)));
  float3 cu = cross(cr, cd);
 
  float3 rd = normalize(p.x * cr + p.y * cu + 3 * cd);
  float4 radiance = shade(ro, rd);
  
  float3 col = float3(0.02, 0.02, 0.02);
  col = lerp(col, radiance.rgb, radiance.a);
  return float4(pow(col, 0.45), 1);
}