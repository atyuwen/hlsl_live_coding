Texture2D src_texture0 : register(t0);
SamplerState default_sampler: register(s0);

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
	float3 col = src_texture0.Sample(default_sampler, in_tex).rgb;
	return float4(saturate(col), 0.02);
}
