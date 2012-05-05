Texture2D src_texture : register(t0);
SamplerState default_sampler: register(s0);

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
	return src_texture.Sample(default_sampler, in_tex);
}
