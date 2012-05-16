#ifndef _KEYWORDS_INCLUDED_HPP_
#define _KEYWORDS_INCLUDED_HPP_

#include <string>
#include <boost/preprocessor/repetition.hpp>

#define MAKE_SUFFIX_1D(_, i, s) L#s##L#i,
#define MAKE_SUFFIX_2D_1(_, j, s) L#s##L"1"##L"x"##L#j,
#define MAKE_SUFFIX_2D_2(_, j, s) L#s##L"2"##L"x"##L#j,
#define MAKE_SUFFIX_2D_3(_, j, s) L#s##L"3"##L"x"##L#j,
#define MAKE_SUFFIX_2D_4(_, j, s) L#s##L"4"##L"x"##L#j,

#define DECLARE_TYPE_1D(type) \
	L###type, BOOST_PP_REPEAT_FROM_TO(2, 5, MAKE_SUFFIX_1D, type)

#define DECLARE_TYPE_2D(type) \
	BOOST_PP_REPEAT_FROM_TO(1, 5, MAKE_SUFFIX_2D_1, type) \
	BOOST_PP_REPEAT_FROM_TO(1, 5, MAKE_SUFFIX_2D_2, type) \
	BOOST_PP_REPEAT_FROM_TO(1, 5, MAKE_SUFFIX_2D_3, type) \
	BOOST_PP_REPEAT_FROM_TO(1, 5, MAKE_SUFFIX_2D_4, type)

#define DECLARE_SEMANTIC(semantic, n) \
	L###semantic, BOOST_PP_REPEAT(n, MAKE_SUFFIX_1D, semantic)

static const std::wstring kewords[] =
{
	DECLARE_TYPE_1D(bool)
	DECLARE_TYPE_1D(int)
	DECLARE_TYPE_1D(half)
	DECLARE_TYPE_1D(float)
	DECLARE_TYPE_2D(float)
	L"blendstate", L"break", L"buffer",
	L"cbuffer", L"class", L"compile", L"const", L"continue",
	L"depthstencilstate", L"depthstencilview", L"discard", L"do", L"double",
	L"else", L"extern",
	L"false", L"for",
	L"geometryshader",
	L"if", L"in", L"inline",
	L"inout", L"interface",
	L"matrix",
	L"namespace", L"nointerpolation",
	L"out",
	L"pass", L"pixelshader", L"precise",
	L"rasterizerstate", L"rendertargetview", L"return", L"register",
	L"sampler", L"sampler1D", L"sampler2D", L"sampler3D", L"samplerCUBE", L"SamplerState", L"SamplerComparisonState", L"shared", L"stateblock", L"stateblock_state", L"static", L"string", L"struct", L"switch",
	L"tbuffer", L"technique", L"technique10", L"texture", L"Texture1D", L"Texture1DArray", L"Texture2D", L"Texture2DArray", L"Texture2DMS", L"Texture2DMSArray", L"Texture3D", L"TextureCube", L"TextureCubeArray", L"true", L"typedef",
	L"uniform",
	L"vector", L"vertexshader", L"void", L"volatile", L"while",
};

static const std::wstring semantics[] =
{
	DECLARE_SEMANTIC(binormal, 12)
	DECLARE_SEMANTIC(blendindices, 12)
	DECLARE_SEMANTIC(blendweight, 12)
	DECLARE_SEMANTIC(color, 16)
	DECLARE_SEMANTIC(depth, 16)
	DECLARE_SEMANTIC(normal, 12)
	DECLARE_SEMANTIC(position, 12)
	DECLARE_SEMANTIC(positiont, 0)
	DECLARE_SEMANTIC(psize, 12)
	DECLARE_SEMANTIC(tangent, 12)
	DECLARE_SEMANTIC(texcoord, 16)
	DECLARE_SEMANTIC(color, 16)
	DECLARE_SEMANTIC(fog, 0)
	DECLARE_SEMANTIC(tessfactor, 12)
	DECLARE_SEMANTIC(vface, 0)
	DECLARE_SEMANTIC(vpos, 0)
	DECLARE_SEMANTIC(sv_clipdistance, 8)
	DECLARE_SEMANTIC(sv_culldistance, 8)
	DECLARE_SEMANTIC(sv_coverage, 0)
	DECLARE_SEMANTIC(sv_depth, 0)
	DECLARE_SEMANTIC(sv_dispatchthreadid, 0)
	DECLARE_SEMANTIC(sv_domainlocation, 0)
	DECLARE_SEMANTIC(sv_groupid, 0)
	DECLARE_SEMANTIC(sv_groupindex, 0)
	DECLARE_SEMANTIC(sv_groupthreadid, 0)
	DECLARE_SEMANTIC(sv_gsinstanceid, 0)
	DECLARE_SEMANTIC(sv_insidetessfactor, 0)
	DECLARE_SEMANTIC(sv_isfrontface, 0)
	DECLARE_SEMANTIC(sv_position, 0)
	DECLARE_SEMANTIC(sv_rendertargetarrayindex, 0)
	DECLARE_SEMANTIC(sv_sampleindex, 0)
	DECLARE_SEMANTIC(sv_target, 8)
	DECLARE_SEMANTIC(sv_tessfactor, 0)
	DECLARE_SEMANTIC(sv_viewportarrayindex, 0)
	DECLARE_SEMANTIC(sv_instanceid, 0)
	DECLARE_SEMANTIC(sv_primitiveid, 0)
	DECLARE_SEMANTIC(sv_vertexid, 0)
};

static const std::wstring global_funcs[] =
{
	L"abs",
	L"acos",
	L"all",
	L"AllMemoryBarrier",
	L"AllMemoryBarrierWithGroupSync",
	L"any",
	L"asdouble",
	L"asfloat",
	L"asfloat",
	L"asin",
	L"asint",
	L"asint",
	L"asuint",
	L"asuint",
	L"atan",
	L"atan2",
	L"ceil",
	L"clamp",
	L"clip",
	L"cos",
	L"cosh",
	L"countbits",
	L"cross",
	L"D3DCOLORtoUBYTE4",
	L"ddx",
	L"ddx_coarse",
	L"ddx_fine",
	L"ddy",
	L"ddy_coarse",
	L"ddy_fine",
	L"degrees",
	L"determinant",
	L"DeviceMemoryBarrier",
	L"DeviceMemoryBarrierWithGroupSync",
	L"distance",
	L"dot",
	L"dst",
	L"EvaluateAttributeAtCentroid",
	L"EvaluateAttributeAtSample",
	L"EvaluateAttributeSnapped",
	L"exp",
	L"exp2",
	L"f16tof32",
	L"f32tof16",
	L"faceforward",
	L"firstbithigh",
	L"firstbitlow",
	L"floor",
	L"fmod",
	L"frac",
	L"frexp",
	L"fwidth",
	L"GetRenderTargetSampleCount",
	L"GetRenderTargetSamplePosition",
	L"GroupMemoryBarrier",
	L"GroupMemoryBarrierWithGroupSync",
	L"InterlockedAdd",
	L"InterlockedAnd",
	L"InterlockedCompareExchange",
	L"InterlockedCompareStore",
	L"InterlockedExchange",
	L"InterlockedMax",
	L"InterlockedMin",
	L"InterlockedOr",
	L"InterlockedXor",
	L"isfinite",
	L"isinf",
	L"isnan",
	L"ldexp",
	L"length",
	L"lerp",
	L"lit",
	L"log",
	L"log10",
	L"log2",
	L"mad",
	L"max",
	L"min",
	L"modf",
	L"mul",
	L"noise",
	L"normalize",
	L"pow",
	L"Process2DQuadTessFactorsAvg",
	L"Process2DQuadTessFactorsMax",
	L"Process2DQuadTessFactorsMin",
	L"ProcessIsolineTessFactors",
	L"ProcessQuadTessFactorsAvg",
	L"ProcessQuadTessFactorsMax",
	L"ProcessQuadTessFactorsMin",
	L"ProcessTriTessFactorsAvg",
	L"ProcessTriTessFactorsMax",
	L"ProcessTriTessFactorsMin",
	L"radians",
	L"rcp",
	L"reflect",
	L"refract",
	L"reversebits",
	L"round",
	L"rsqrt",
	L"saturate",
	L"sign",
	L"sin",
	L"sincos",
	L"sinh",
	L"smoothstep",
	L"sqrt",
	L"step",
	L"tan",
	L"tanh",
	L"tex1D",
	L"tex1D",
	L"tex1Dbias",
	L"tex1Dgrad",
	L"tex1Dlod",
	L"tex1Dproj",
	L"tex2D",
	L"tex2D",
	L"tex2Dbias",
	L"tex2Dgrad",
	L"tex2Dlod",
	L"tex2Dproj",
	L"tex3D",
	L"tex3D",
	L"tex3Dbias",
	L"tex3Dgrad",
	L"tex3Dlod",
	L"tex3Dproj",
	L"texCUBE",
	L"texCUBE",
	L"texCUBEbias",
	L"texCUBEgrad",
	L"texCUBElod",
	L"texCUBEproj",
	L"transpose",
	L"trunc",
};

static const std::wstring extend_funcs[] =
{
	L"snoise",
};

static const std::wstring member_funcs[] =
{
	L"Append",
	L"RestartStrip",
	L"CalculateLevelOfDetail",
	L"CalculateLevelOfDetailUnclamped",
	L"Gather",
	L"GetDimensions",
	L"GetSamplePosition",
	L"Load",
	L"Sample",
	L"SampleBias",
	L"SampleCmp",
	L"SampleCmpLevelZero",
	L"SampleGrad",
	L"SampleLevel",
};

#undef MAKE_SUFFIX_1D
#undef MAKE_SUFFIX_2D_1
#undef MAKE_SUFFIX_2D_2
#undef MAKE_SUFFIX_2D_3
#undef MAKE_SUFFIX_2D_4
#undef DECLARE_TYPE_1D
#undef DECLARE_TYPE_2D
#undef DECLARE_SEMANTIC

#endif  // _KEYWORDS_INCLUDED_HPP_