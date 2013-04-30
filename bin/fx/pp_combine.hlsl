Texture2D src_texture0 : register(t0);
Texture2D src_texture1 : register(t1);

float4 ps_main(in float4 sc : SV_POSITION) : SV_TARGET
{
	int3 p = int3(sc.xy, 0);
	float4 col0 = src_texture0.Load(p);
	float4 col1 = src_texture1.Load(p);
	return lerp(saturate(col0), saturate(col1), col1.a);
}
