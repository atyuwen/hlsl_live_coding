cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float box(float3 p)
{
  p = abs(p) - 1;
  return min(max(p.x,max(p.y,p.z)), 0) + length(max(p, 0));
}

float DE(float3 p)
{
  float d = box(p);
  float scale = 1.0;
  for(int i = 0; i < 4; ++i)
  {
    float3 a = fmod(p + 1, 2 * scale) - scale;
    scale /= 3;
    float3 r = abs(a / scale);
    float da = max(r.x, r.y);
    float db = max(r.y, r.z);
    float dc = max(r.z, r.x);
    float c = min(da, min(db, dc)) - 1;
    d = max(d, -c * scale);
  }
  return d;
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
  ao += DE(p + 0.2 * N) * 2.5;
  ao += DE(p + 0.5 * N) * 1.0;
 
  float3 L = normalize(float3(-2, 1, 1));
  float4 S = ray_marching(p + N * 0.001, L);
  float3 C = float3(0.3, 0.2, 0.3) + 0.2 * snoise(p * 100);
  float D = 0.6 * (S.w < 0 ? 1 : 0);
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