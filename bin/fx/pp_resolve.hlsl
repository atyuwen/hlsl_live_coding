cbuffer Parameters
{
	float2 interleave;
}

Texture2D src_texture0 : register(t0);

float4 resolve(in float4 sc : SV_POSITION) : SV_TARGET
{
	int3 p = int3(sc.xy, 0);
	float3 col = src_texture0.Load(p).rgb;
	return float4(saturate(col), 0.02);
}

float4 halfres_resolve(in float4 sc : SV_POSITION) : SV_TARGET
{
	int2 p = int2(sc.xy);
	clip(-abs(fmod(p, 2) - interleave));
	float3 col = src_texture0.Load(int3(p / 2, 0)).rgb;
	return float4(saturate(col), 0.08);
}

float4 halfres_copy(in float4 sc : SV_POSITION) : SV_TARGET
{
	int3 p = int3(sc.xy / 2, 0);
	return saturate(src_texture0.Load(p));
}
