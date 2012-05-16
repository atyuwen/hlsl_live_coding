cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 fft;
}

Texture2D tex;
SamplerState samp;

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
  float2 p = (in_tex - 0.5) * 2;
  float a = atan2(p.y, p.x);
  float r = length(p);
  float2 tc = float2(a / 6.28, 0.3 * time.x + 0.1 / r);
  float3 s = tex.Sample(samp, tc);
  s *= 1 + step(0.4, fft.x);
  s *= r * r;
  return float4(s, 1);
}
