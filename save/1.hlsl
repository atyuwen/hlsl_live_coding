cbuffer Parameters
{
  float4 time;
  float4 view;
  float4 fft;
}

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
  return float4(0.3, 0.3, 0.3, 1);
}
