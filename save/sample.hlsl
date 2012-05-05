cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 fft;
}

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
  float2 p = (in_tex - 0.5) * 2.0;
  float t = time.x - step(0.4, fft.x) * 0.3;
  float2 c = float2(0.3 + sin(t * 0.3) * 0.2, 0.5);

  int i = 0;
  for (; i < 32; ++i)
  {
    p = float2(p.x * p.x - p.y * p.y, 2 * p.x * p.y) + c;
    if (length(p) > 1.5) break;
  }
  
  float3 rgb = 0.1 + i / 80.0;
  return float4(rgb, 1);
}
