cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float DE(float3 p)
{
  const float scale = 9;
  const float3 boxfold = float3(1, 1, 1);
  const float spherefold = 0.2;
 
  float4 c0 = float4(p, 1);
  float4 c = c0;
  for (int i = 0; i < 4; ++i)
  {
    c.xyz = clamp(c.xyz, -boxfold, boxfold) * 2 - c.xyz;
    float rr = dot(c.xyz, c.xyz);
    c *= saturate(max(spherefold / rr, spherefold));
    c = c * scale + c0;
  } 
  return ((length(c.xyz) - (scale - 1)) / c.w - pow(scale, -3));
}

float4 ray_marching(float3 ro,  float3 rd)
{
  for (int i = 0; i < 128; ++i)
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
  float3 L = normalize(float3(-1, 1, 2));
  
  float3 C = float3(0.5, 0.8, 0.9);
  float shadow = saturate(DE(p + L * 0.1) - k) / 0.1;
  float ao = 1 - rm.w / 128; ao = ao * ao;
  float A = 0.1;
  float3 col = (A + saturate(dot(L, N)) * shadow) * ao * C;
  
  return float4(col , 1);
}

float4 ps_main(in float2 tc : TEXCOORD) : SV_TARGET
{
  float3 p = float3((tc * 2 - 1), 0);
  p.xy *= float2(view.x * view.w, -1);
 
  float3 param = mpos.xyz * float3(-0.005, 0.005, -0.5) + float3(0.4, 0.3, 40);
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
