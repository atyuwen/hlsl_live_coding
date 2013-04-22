void vs_main(in float3 in_pos : POSITION,
             in float2 in_tex : TEXCOORD,
			 out float2 out_tex: TEXCOORD,
			 out float4 out_pos: SV_POSITION)
{
	out_pos = float4(in_pos.xy, 0, 1);
	out_tex = in_tex;
}

float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET
{
	return float4(0.3, 0.3, 0.3, 1);
}
