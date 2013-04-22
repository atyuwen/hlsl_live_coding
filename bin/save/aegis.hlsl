cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 fft;
}

bool rect(int2 p, int l, int r, int b, int t)
{
  return (l <= p.x && p.x <= r && b <= p.y && p.y <= t);
}

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
  int2 p = in_tex * view.xy;
  if (p.x > 640 || p.y > 320) return float4(0.1, 0.1, 0.1, 1);
  p.y = 320 - p.y;
  
  float3 c = 0.15;
  if (rect(p, 14, 625, 30, 308))
    c = float3(0.32, 0.26, 0.3);
  
  float progress = frac(time.x / 300);
  if (rect(p, 14, 625, 15, 16))
  {
    float s = (p.x - 14) / 610.0;
    if (s < progress)
      c = lerp(float3(0.2, 0.3, 0.4), float3(0.8, 0.4, 0.5), s);
    else
      c = 0.1;
  }
 
  float l = progress * 610 + 14;
  if (rect(p, l, l + 6, 10, 21)) c = 0;
  if (rect(p, l, l + 5, 11, 21)) c = 0.4;
  if (rect(p, l + 1, l + 5, 11, 20)) c = 0.3;

  p = int2(p.x - 16, p.y - 32);
  int2 q = floor(p / float2(19, 5));
  int2 r = p - q * float2(19, 5);
  if (q.x >= 0 && q.x < 32 && q.y >= 0 && q.y < 55)
  {
    if (r.x > 17 || r.y > 3)
      return float4(0.15, 0.15, 0.15, 1);

    float v = snoise(float2(q.x, 0.2 * time.x) + fft.xz);
    if (q.y < v * 55) 
    {
      float2 s = q / float2(31, 59);
      c = lerp(float3(0.4, 0.12, 0.1), float3(0.1, 0.4, 0.5), s.x);
      c = lerp(c, c + 0.6, s.y);
      return float4(c, 1);
    }

    return float4(0.16, 0.18, 0.19, 1);
  }
  return float4(c, 1);
}
