cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 freq;
  float4 mpos;
}

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
  float2 p = (in_tex - 0.5) * 3.0;
  p -= mpos.xy * 0.003;
  p *= 1 / max(0.5, 1 + mpos.z * 0.1);
  float t = time.x - step(0.4, freq.x) * 0.5;
  float2 c = float2(0.3 + sin(t * 0.3) * 0.2, 0.5 + snoise(time.xy) * 0.2);

  float3 trap = 10;
  for (int i; i < 32; ++i)
  {
    p = float2(p.x * p.x - p.y * p.y, 2 * p.x * p.y) + c;
    float l = length(p);
    trap = min(trap, float3(p, l));
    if (l > 1.5) break;
  }

  float3 col = 0.5 * lerp(float3(0.1, 0.3, 0.4), float3(0.2, 0.9, 0.8), trap.x * 0.1);
  col += 0.3 * lerp(float3(0.1, 0.0, 0.4), float3(0.8, 0.2, 0.9), trap.y * 0.1);
  col += 0.2 * lerp(float3(0.8, 0.4, 0.1), float3(0.0, 1.3, 0.5), trap.z);

  return float4(col, 1);
}
