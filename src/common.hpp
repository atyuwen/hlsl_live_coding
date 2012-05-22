#ifndef _COMMON_HPP_INCLUDED_
#define _COMMON_HPP_INCLUDED_

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WindowsX.h>
#include <MMSystem.h>
#include <d3d11.h>
#include <d3d10_1.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <dxgi.h>
#include <d2d1.h>
#include <dwrite.h>
#include <cassert>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d10_1.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#include "ayw/vector.hpp"
using Ayw::float2;
using Ayw::float3;
using Ayw::float4;

#include <string>
#include <boost/lexical_cast.hpp>

#ifdef UNICODE
#define tchar wchar_t
#define tstring std::wstring
#else
#define tchar char
#define tstring std::string
#endif

template <typename DestType, typename SrcType>
DestType lexical_cast_no_exception(const SrcType& arg)
{
	try
	{
		return boost::lexical_cast<DestType>(arg);
	}
	catch (boost::bad_lexical_cast &)
	{
		return DestType();
	}
}

#define to_string(arg) lexical_cast_no_exception<std::string>(arg)
#define to_wstring(arg) lexical_cast_no_exception<std::wstring>(arg)
#define to_tstring(arg) lexical_cast_no_exception<tstring>(arg)

void MessageBoxf(tstring format, ...);

#define SAFE_RELEASE(p) if (p != NULL) {p->Release(); p = NULL;}

#endif  // _COMMON_HPP_INCLUDED_