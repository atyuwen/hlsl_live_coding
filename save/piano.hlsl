cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 fft;
}

float white_keys(float3 p)
{
  float ox = p.x;
  p.x = fmod(abs(ox + 0.03), 0.06) - 0.03;
  float d1 = length(max(abs(p) - float3(0.023, 0.002, 0.2), 0)) - 0.005;
  p.y += 0.04;
  float d2 = length(max(abs(p) - float3(0.023, 0.04, 0.18), 0)) - 0.002;

  int r = fmod(floor(ox / 0.06) + 70, 7);
  if (r == 0 || r == 4) return min(d1, d2);

  p.x = fmod(abs(ox), 0.06) - 0.03;
  p.z -= 0.1;
  float3 t = abs(p) - float3(0.02, 0.1, 0.15);
  float d3 = min(max(t.x, max(t.y, t.z)), 0) + length(max(t, 0));
  return max(min(d1, d2), -d3);
}

float black_keys(float3 p)
{
  int r = fmod(floor(p.x / 0.06) + 70, 7);
  if (r == 0 || r == 4) return 1000;  
  p.x = fmod(abs(p.x), 0.06) - 0.03;
  p.x *= (1 + 10 * p.y);
  p.y += 0.5 * p.z * p.z;
  p.z -= 0.08;
  p.y -= 0.014;
  return length(max(abs(p) - float3(0.01, 0.03, 0.122), 0)) - 0.003;
}

float2 piano(float3 p)
{
  float d1 = white_keys(p);
  float d2 = black_keys(p);
  return d1 < d2 ? float2(d1, 1) : float2(d2, 2);
}

float4 ray_marching(float3 ro, float3 rd)
{
  for (int i = 0; i < 32; ++i)
  {
    float2 dm = piano(ro);
    if (dm.x < 0.001) return float4(ro, dm.y);
    ro += rd * dm.x;
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

float3 lit_white_keys(float3 p, float3 N)
{
  float3 V = normalize(float3(0, 3, -5));
  float3 L = normalize(float3(3, 3, -1));
  float3 C = 2 * brdf(0.9, 50, N, L, V) * max(dot(N, L), 0.1);
  float3 R = reflect(V, N);
  float4 RT = ray_marching(p, R);
  if (RT.w > 1) C = 0;
  
  float ao = 0;
  ao += 0.5 * (0.02 - piano(p + 0.02 * N).x);
  ao += 0.25 * (0.04 - piano(p + 0.04 * N).x);
  ao += 0.125 * (0.06 - piano(p + 0.06 * N).x);
  ao = max(1 - 50 * ao, 0.2);
  return C * ao;
}

float3 lit_black_keys(float3 p, float3 N)
{
  float3 V = normalize(float3(0, 3, -5));
  float3 L = normalize(float3(0, 3, 2));
  return 2 * brdf(0, 50, N, L, V);
}

float4 do_lighting(float3 ro, float3 rd)
{
  float4 rm = ray_marching(ro, rd);
  if (rm.w < 0) return float4(0, 0, 0, 0);

  float gx = (piano(rm.xyz + float3(0.0001, 0, 0)) - piano(rm.xyz - float3(0.0001, 0, 0))).x;
  float gy = (piano(rm.xyz + float3(0, 0.0001, 0)) - piano(rm.xyz - float3(0, 0.0001, 0))).x;
  float gz = (piano(rm.xyz + float3(0, 0, 0.0001)) - piano(rm.xyz - float3(0, 0, 0.0001))).x;
  float3 N = normalize(float3(gx, gy, gz));
  
  if (rm.w < 1.5) return float4(lit_white_keys(rm.xyz, N), 1);
  return float4(lit_black_keys(rm.xyz, N), 1);
}

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
  float2 p = (in_tex - 0.5) * 2 * float2(1, -view.y / view.x);
  if (abs(p.y) > 256.0 / 1024.0) return 0;

  float3 ro = float3(0, 3, -5);
  float3 rt = float3(0, 0, 0);
  float3 cd = normalize(rt - ro);
  float3 cr = normalize(cross(cd, float3(0, 1, 0)));
  float3 cu = cross(cr, cd);
  float3 rd = normalize(p.x * cr + p.y * cu + 5 * cd);

  float3 col = 0.2 - 0.1 * length(p - float2(0.2, 0.3));
  float4 luma = do_lighting(ro, rd);
  col = lerp(col, luma.rgb, luma.a);
  return float4(pow(col, 0.45), 1);
}
