Texture2D src_texture0 : register(t0);
Texture2D src_texture1 : register(t1);
SamplerState default_sampler: register(s0);

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
	float4 col0 = src_texture0.Sample(default_sampler, in_tex);
	float4 col1 = src_texture1.Sample(default_sampler, in_tex);
	return lerp(saturate(col0), saturate(col1), col1.a);
}
