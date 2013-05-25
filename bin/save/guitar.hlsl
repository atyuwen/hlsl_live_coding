cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float body(float3 p)
{
  p.x *= 0.6;
  float d1 = length(p.xy) - 0.4;
  p.y -= 0.5;
  float d2 = length(p.xy) - 0.18;
  float d = lerp(d1, d2, saturate((p.y + 0.9) * 0.8));
  d = max(d, abs(p.z) - 0.1);  
  return d;
}

float neck(float3 p, out float id)
{ 
  id = 1;
  p.y -= 1;
  float t = min(p.y + 0.7, 1.21);
  float h = saturate(p.y + 0.7 - 1.21);
  p.z -= 0.114 - saturate(t) * 0.02 - h * 0.15;
  if (p.z > -0.012) id = 2;
  p.x *= (1 + 0.3 * t) - min(12 * h, 0.4 + h);
  p.y *= (1 + h * abs(p.x));
  float s = clamp((t - 0.32) * 10, 0.3, 1);
  float d = length(p.xz * float2(1, s)) - 0.05;
  p.z += 0.3;
  s = fmod(pow(t, 0.48) + 2.005, 0.036) - 0.002;
  s = saturate(0.004 - s * s * 800 * t);
  if (s > 0 && p.z > 0.3) id = 3;
  p.z -= s;
  d = max(d, length(p.xz) - 0.3);
  d = max(d, abs(p.y) - 0.74);  
  return d * 0.7;
}

float inner(float3 p)
{
  p.y -= 0.32;
  float d1 = max(length(p.xy) - 0.1, -p.z);
  float d2 = max(length(p.xy) - 0.2, abs(p.z) - 0.09);
  return min(d1, d2);  
}

float bridge(float3 p, out float id)
{
  id = 1;
  p.y += 0.06;
  p.z += clamp(abs(p.x) * 0.12, 0.01, 0.02) - 0.11;
  float d = length(max(abs(p) - float3(0.15, 0.02, 0.02), 0)) * 0.9;
  float d2 = length(max(abs(p - float3(0, 0.012, 0)) - float3(0.07, 0.003, 0.027), 0));
  if (d2 < d) {d = d2; id = 2;}
  if (abs(p.x) > 0.06) return d;
  p.x = fmod(p.x + 2, 0.02) - 0.01;
  p.z -= 0.025;
  float d3 = length(p.xyz) - 0.005;
  if (d3 < d) {d = d3; id = 3;}  
  return d;
}

float screws(float3 p)
{
  float t = min(p.y - 1.53, 0.18);
  p.z -= 0.075 - 0.15 * t;
  p.y = fmod(t - p.z * 0.2, 0.06) - 0.03;
  p.x = abs(p.x) - 0.033;
  float r = p.z < 0 ? 0.01 : 0.005;
  float d = max(length(p.xy) - r, abs(p.z) - 0.035);
  d = min(d, max(length(p.xy) - 0.009, abs(p.z) - 0.02));
  p.z += 0.024;
  d = min(d, max(length(p.yz) - 0.004, abs(p.x) - 0.03));

  // handle
  p.x -= 0.02 + 0.05 * t;
  float h = length(p.xyz) - 0.014;
  p.x -= 0.01;
  h = max(h, length(max(abs(p.xyz) - float3(0.01, 0.01, 0.004), 0)));
  d = min(d, h);
  return d * 0.9;
}

float strings(float3 p)
{
  p.y -= 0.72;
  float t = min(p.y + 0.785, 1.57);
  p.x *= (1 + 0.4 * t);
  if (abs(p.x) > 0.06) return 1e3;
  
  float f = saturate(p.y - 0.78);
  if (p.y > 0) p.y *= 0.65 + 3.6 * abs(p.x);
  p.x += clamp(-sign(p.x) * 0.2 * f, -abs(p.x), abs(p.x));
  p.z += 0.15 * f;
  
  float r = 0.0006 - p.x * 0.006;
  p.z -= 0.125 - 0.015 * t;
  p.x = fmod(p.x + 2, 0.02) - 0.01;
  float d = length(p.xz) - r;
  d = max(d, abs(p.y) - 0.785);
  return d;
}

float2 DE(float3 p)
{
  // bounding box
  float3 bb = saturate(abs(p.xyz) - float3(0.5, 2, 0.3));
  if (any(bb)) return float2(length(bb) + 0.01, -1);
 
  float d = body(p);
  float id = 1;
  float sid = 0;
  float t = neck(p, sid);
  if (t < d) {d = t; id = 2 + sid * 0.1;}
  t = -inner(p);
  if (t > d) {d = t; id = 3;}
  t = bridge(p, sid);
  if (t < d) {d = t; id = 4 + sid * 0.1;}
  t = screws(p);
  if (t < d) {d = t; id = 5;}
  t = strings(p);
  if (t < d) {d = t; id = 6;}  
  return float2(d, id);
}

float4 ray_marching(float3 ro,  float3 rd)
{
  float3 p = ro;
  for (int i = 0; i < 64; ++i)
  {
    float2 d = DE(p);
    p += d.x * rd;
    if (d.x < 0.0001) return float4(p, d.y);
  }
 
  float t = (-0.4 - ro.y) / rd.y;
  float3 floorp = ro + t * rd;
  if (t > 0) return float4(floorp, 0);
  return float4(ro, -1);
}

float3 brdf(float3 diff, float m, float3 N, float3 L, float3 V)
{
  float3 H = normalize(V + L);
  float3 F = 0.05 + 0.95 * pow(1 - dot(V, H), 5);
  float3 R = F * pow(max(dot(N, H), 0), m);
  return diff + R * (m + 8) / 8;
}

float4 shade(float3 ro, float3 rd)
{
  float4 rm = ray_marching(ro, rd);
  if (rm.w < 0) return float4(0, 0, 0, 0);
    
  float3 p = rm.xyz;
  float k = DE(p).x;
  float gx = DE(p + float3(1e-5, 0, 0)).x - k;
  float gy = DE(p + float3(0, 1e-5, 0)).x - k;
  float gz = DE(p + float3(0, 0, 1e-5)).x - k;
  float3 N = normalize(float3(gx, gy, gz));
 
  float ao = 0;
  ao += DE(p + 0.01 * N).x * 50;
  ao += DE(p + 0.02 * N).x * 10;

  float3 L = normalize(float3(-0.1, 1, 1));
  float sr = ray_marching(p + 0.001 * L, L).w;
  float shadow = sr > 0 ? 0 : 1;
 
  float3 diff = 0.8;
  float m = 10;
  if (rm.w < 0.9) // floor
  {
    shadow = saturate(0.4 + 0.6 * shadow + 0.3 * length(p.xz));
    return float4(float3(0.02, 0.02, 0.02) * shadow, 1);
  }
  if (rm.w < 1.9) // body
  {
    float3 C = float3(0.8, 0.6, 0.2);
    float s = length(p.xy - float2(0, 0.32));
    if (abs(s - 0.12) < 0.008) C = float3(0.01, 0.004, 0);
    
    diff = lerp(float3(0.02, 0.008, 0.001), C, saturate(N.z));
    float r = qnoise(200 * p.xzz + 2 * qnoise(5 * p.yyz));
    diff *= (r * 0.3 + 0.7);
    if (abs(abs(p.z) - 0.08) < 0.005) diff = 0.8;
  }
  else if (rm.w < 2.25) // neck
  {
    diff = float3(0.3, 0.18, 0.1) * (0.7 + qnoise(300 * p) * 0.3);
    if (rm.w > 2.15) diff *= 0.4;
  }
  else if (rm.w < 2.9)
  {
    diff = float3(0.8, 0.6, 0.4);
    m = 80;
  }
  else if (rm.w < 3.9) // inner
  {
    diff = float3(0.25, 0.2, 0.15) * (0.5 + 0.5 * qnoise(400 * p.xzz));
  }
  else if (rm.w < 4.15) // bridge
  {
    diff = 0;
  }
  else if (rm.w < 4.25)
  {
    diff = 0.7;
  }
  else if (rm.w < 4.35)
  {
    diff = 0.04; m = 80;
  }
  else if (rm.w < 5.9) // screws
  {
    m = 50;
  }
  else // strings
  {
    m = 50;
  }
  float3 f = brdf(diff, m, N, L, -rd);

  float3 A = 0.2;
  float3 C = (saturate(dot(N, L)) * f * shadow + A * diff) * ao;
  return float4(C, 1);
}

float4 ps_main(in float2 tc : TEXCOORD) : SV_TARGET
{
  float3 p = float3((tc * 2 - 1), 0);
  p.xy *= float2(view.x * view.w, -1);
 
  float3 param = mpos.xyz * float3(-0.005, 0.005, -0.1) + float3(0.4, 0.3, 7);
  float4 rot;
  sincos(param.x, rot.x, rot.y);
  sincos(param.y, rot.z, rot.w);
  
  float3 rt = float3(0, 0.6, 0);
  float3 ro = float3(rot.x * rot.w, abs(rot.y) * rot.z, rot.y);
  ro = ro * param.z;
  
  float3 cd = normalize(rt - ro);
  float3 cr = normalize(cross(cd, float3(0, 1, 0)));
  float3 cu = cross(cr, cd);
 
  float3 rd = normalize(p.x * cr + p.y * cu + 5 * cd);
  float4 radiance = shade(ro, rd);
  
  float3 col = float3(0.02, 0.02, 0.02);
  col = lerp(col, radiance.rgb, radiance.a);
  return float4(pow(col, 0.45), 1);
}
