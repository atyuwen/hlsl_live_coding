cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float sphere(float3 p, float r)
{
  return length(p - r) - r;
}

float corner(float3 p)
{
  return min(p.x, min(p.y, p.z));
}

float2 DE(float3 p)
{
  float d1 = sphere(p, 1);
  float d2 = corner(p);
  return d1 < d2 ? float2(d1, 1) : float2(d2, 2);
}

float4 ray_marching(float3 ro,  float3 rd)
{
  for (int i = 0; i < 128; ++i)
  {
    float2 d = DE(ro);
    ro += d.x * rd;
    if (d.x < 0.001) return float4(ro, d.y);
  }
  return float4(ro, -1);
}

float3 brdf(float3 diff, float m, float3 N, float3 L, float3 V)
{
  float3 H = normalize(V + L);
  float3 F = 0.05 + 0.95 * pow(1 - dot(V, H), 5);
  float3 R = F * pow(max(dot(N, H), 0), m);
  return diff + R * (m + 8) / 8;
}

float fbm(float3 p)
{
    float f;
    f  = 0.5000 * qnoise(p); p = p * 2.02;
    f += 0.2500 * qnoise(p); p = p * 2.03;
    f += 0.1250 * qnoise(p); p = p * 2.01;
    f += 0.0625 * qnoise(p);
    return f;
}

float4 shade(inout float3 ro, inout float3 rd)
{
  float4 rm = ray_marching(ro, rd);
  if (rm.w < 0) return 0;
  
  float3 diff, N;
  float m, r;
  if (rm.w < 1.5)
  {
    float s = abs(sin(5.6 * fbm(rm.xyz * 3.4)));
    diff = lerp(float3(0.55, 0.3, 0.25), float3(1, 1, 1), s);
    N = rm.xyz - 1;
    m = 50;
    r = saturate(1 - abs(dot(-rd, N)) + 0.2);
  }
  else
  {
    float s = saturate(length(step(0.01, fmod(rm.xyz, 0.3))) - 1);
    diff = s * float3(0.4, 0.4, 0.4) + float3(0.2, 0.2, 0.2) + 0.1 * qnoise(rm.xyz * 78);
    diff = diff / (1 + dot(rm.xyz, rm.xyz));
    N = normalize(1 - step(0.001, rm.xyz));
    m = 10;
    r = 0;
  }

  float ao = 0.2;
  ao += saturate(DE(rm.xyz + 0.1 * N).x) * 4;
  ao += saturate(DE(rm.xyz + 0.2 * N).x) * 2;
  ao += saturate(DE(rm.xyz + 0.4 * N).x) * 1;

  float3 L = normalize(float3(0.6, 1, 1));
  float shadow = exp((DE(rm.xyz + 0.6 * L).x - 0.6) * 3);
 
  float3 f = brdf(diff, m, N, L, -rd);
  float3 C = saturate(dot(N, L) + 0.3) * f * ao * shadow;
  rd = reflect(rd, N);
  ro = rm.xyz + rd * 0.01;
  return float4(C, r);
}

float4 ps_main(in float2 tc : TEXCOORD) : SV_TARGET
{
  float3 p = float3((tc * 2 - 1), 0);
  p.xy *= float2(view.x * view.w, -1);
 
  float3 param = mpos.xyz * float3(-0.005, 0.005, -0.1) + float3(0.8, 0.7, 12);
  float4 rot;
  sincos(param.x, rot.x, rot.y);
  sincos(param.y, rot.z, rot.w);
  
  float3 rt = float3(1, 1.2, 1);
  float3 ro = float3(rot.x * rot.w, abs(rot.y) * rot.z, rot.y);
  ro = ro * param.z;
  
  float3 cd = normalize(rt - ro);
  float3 cr = normalize(cross(cd, float3(0, 1, 0)));
  float3 cu = cross(cr, cd);

  float3 rd = normalize(p.x * cr + p.y * cu + 5 * cd);
  float4 col = shade(ro, rd);
  if (col.a > 0)
  {
    float4 col2 = shade(ro, rd);
    col = lerp(col, col2, col.a);
  }
  return float4(pow(col.rgb, 0.45), 1);
}
