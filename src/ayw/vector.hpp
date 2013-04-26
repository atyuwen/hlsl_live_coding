// file: 		vector.hpp
// path:		AywProc\AywProc\core\include
// purpose:		vector template class
//
// author:		atyuwen
// homepage:	http://code.google.com/p/aywproc/
//
// created:		2010/04/20
// history:	
//
//////////////////////////////////////////////////////////////////////

#ifndef _VECTOR_HPP_
#define _VECTOR_HPP_

#include <cmath>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>

#include "common_helper.h"
#include "vector_helper.hpp"

namespace Ayw
{
	template<typename Type> struct vector_2t;
	template<typename Type> struct vector_3t;
	template<typename Type> struct vector_4t;

	typedef vector_2t<float> float2;
	typedef vector_3t<float> float3;
	typedef vector_4t<float> float4;

	template<typename Type>
	struct vector_2t
	{
	public:
		vector_2t(Type nx = Type(), Type ny = Type())
			: x(nx)
			, y(ny)
		{
		}

	public:
		DECLARE_CONVERSION_TO_POINTER(Type, &x)
		DECLARE_SUBSCRIPT_OPERATOR(Type, &x)
		DECLARE_NATIVE_ITERATOR(Type, &x, 2)
		DECLARE_SWIZZLING_FOR_VECTOR_2T();

	public:
		Type length_sqr() const
		{
			return x * x + y * y;
		}

		Type length() const
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			return std::sqrt(length_sqr());
		}

		vector_2t& normalize()
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			return (*this) /= length();
		}

	public:
		vector_2t& operator += (const vector_2t &rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		vector_2t& operator -= (const vector_2t &rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		vector_2t& operator *= (Type scale)
		{
			x *= scale;
			y *= scale;
			return *this;
		}

		vector_2t& operator /= (Type scale)
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			Type scale_r = 1 / scale;
			return (*this) *= scale_r;
		}

		vector_2t& operator *= (const vector_2t &rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			return *this;
		}

		vector_2t& operator /= (const vector_2t &rhs)
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			x /= rhs.x;
			y /= rhs.y;
			return *this;
		}

	public:
		Type x;
		Type y;
	};

	template<typename Type>
	struct vector_3t
	{
	public:
		vector_3t(Type nx = Type(), Type ny = Type(),Type nz = Type())
			: x(nx)
			, y(ny)
			, z(nz)
		{
		}

	public:
		DECLARE_CONVERSION_TO_POINTER(Type, &x)
		DECLARE_SUBSCRIPT_OPERATOR(Type, &x)
		DECLARE_NATIVE_ITERATOR(Type, &x, 3)
		DECLARE_SWIZZLING_FOR_VECTOR_3T();

	public:
		Type length_sqr() const
		{
			return x * x + y * y + z * z;
		}

		Type length() const
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			return std::sqrt(length_sqr());
		}

		vector_3t& normalize()
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			return (*this) /= length();
		}

	public:
		vector_3t& operator += (const vector_3t &rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		vector_3t& operator -= (const vector_3t &rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		vector_3t& operator *= (Type scale)
		{
			x *= scale;
			y *= scale;
			z *= scale;
			return *this;
		}

		vector_3t& operator /= (Type scale)
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			Type scale_r = 1 / scale;
			return (*this) *= scale_r;
		}

		vector_3t& operator *= (const vector_3t &rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			return *this;
		}

		vector_3t& operator /= (const vector_3t &rhs)
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			return *this;
		}

	public:
		Type x;
		Type y;
		Type z;
	};

	template<typename Type>
	struct vector_4t
	{
	public:
		vector_4t(Type nx = Type(), Type ny = Type(), Type nz = Type(), Type nw = Type())
			: x(nx)
			, y(ny)
			, z(nz)
			, w(nw)
		{
		}

	public:
		DECLARE_CONVERSION_TO_POINTER(Type, &x)
		DECLARE_SUBSCRIPT_OPERATOR(Type, &x)
		DECLARE_NATIVE_ITERATOR(Type, &x, 4)
		DECLARE_SWIZZLING_FOR_VECTOR_4T();

	public:
		Type length_sqr() const
		{
			return x * x + y * y + z * z + w * w;
		}

		Type length() const
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			return std::sqrt(length_sqr());
		}

		vector_4t& normalize()
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			return (*this) /= length();
		}

	public:
		vector_4t& operator += (const vector_4t &rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		vector_4t& operator -= (const vector_4t &rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			z -= rhs.w;
			return *this;
		}

		vector_4t& operator *= (Type scale)
		{
			x *= scale;
			y *= scale;
			z *= scale;
			w *= scale;
			return *this;
		}

		vector_4t& operator /= (Type scale)
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			Type scale_r = 1 / scale;
			return (*this) *= scale_r;
		}

		vector_4t& operator *= (const vector_4t &rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			w *= rhs.w;
			return *this;
		}

		vector_4t& operator /= (const vector_4t &rhs)
		{
			BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			w /= rhs.w;
			return *this;
		}

	public:
		Type x;
		Type y;
		Type z;
		Type w;
	};

	//////////////////////////////////////////////////////////////////////////
	// negation
	template<typename Type>
	vector_2t<Type> operator - (const vector_2t<Type> &operand)
	{
		return vector_2t<Type>(-operand.x, -operand.y);
	}

	template<typename Type>
	vector_3t<Type> operator - (const vector_3t<Type> &operand)
	{
		return vector_3t<Type>(-operand.x, -operand.y, -operand.z);
	}

	template<typename Type>
	vector_4t<Type> operator - (const vector_4t<Type> &operand)
	{
		return vector_4t<Type>(-operand.x, -operand.y, -operand.z, -operand.w);
	}

	//////////////////////////////////////////////////////////////////////////
	// addition
	template<typename Type>
	vector_2t<Type> operator + (const vector_2t<Type> &lhs, const vector_2t<Type> &rhs)
	{
		return vector_2t<Type> (lhs.x + rhs.x, lhs.y + rhs.y);
	}

	template<typename Type>
	vector_3t<Type> operator + (const vector_3t<Type> &lhs, const vector_3t<Type> &rhs)
	{
		return vector_3t<Type> (lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
	}

	template<typename Type>
	vector_4t<Type> operator + (const vector_4t<Type> &lhs, const vector_4t<Type> &rhs)
	{
		return vector_4t<Type> (lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
	}

	//////////////////////////////////////////////////////////////////////////
	// subtraction
	template<typename Type>
	vector_2t<Type> operator - (const vector_2t<Type> &lhs, const vector_2t<Type> &rhs)
	{
		return vector_2t<Type> (lhs.x - rhs.x, lhs.y - rhs.y);
	}

	template<typename Type>
	vector_3t<Type> operator - (const vector_3t<Type> &lhs, const vector_3t<Type> &rhs)
	{
		return vector_3t<Type> (lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
	}

	template<typename Type>
	vector_4t<Type> operator - (const vector_4t<Type> &lhs, const vector_4t<Type> &rhs)
	{
		return vector_4t<Type> (lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
	}

	//////////////////////////////////////////////////////////////////////////
	// multiplication : vector * scalar
	template<typename Type, typename S>
	vector_2t<Type> operator * (const vector_2t<Type> &lhs, const S& scale)
	{
		return vector_2t<Type> (lhs.x * scale, lhs.y * scale);
	}

	template<typename Type, typename S>
	vector_3t<Type> operator * (const vector_3t<Type> &lhs, const S& scale)
	{
		return vector_3t<Type> (lhs.x * scale, lhs.y * scale, lhs.z * scale);
	}

	template<typename Type, typename S>
	vector_4t<Type> operator * (const vector_4t<Type> &lhs, const S& scale)
	{
		return vector_4t<Type> (lhs.x * scale, lhs.y * scale, lhs.z * scale, lhs.w * scale);
	}

	//////////////////////////////////////////////////////////////////////////
	// multiplication : scalar * vector
	template<typename Type, typename S>
	vector_2t<Type> operator * (const S& scale , const vector_2t<Type> &rhs)
	{
		return vector_2t<Type> (rhs.x * scale, rhs.y * scale);
	}

	template<typename Type, typename S>
	vector_3t<Type> operator * (const S& scale , const vector_3t<Type> &rhs)
	{
		return vector_3t<Type> (rhs.x * scale, rhs.y * scale, rhs.z * scale);
	}

	template<typename Type, typename S>
	vector_4t<Type> operator * (const S& scale , const vector_4t<Type> &rhs)
	{
		return vector_4t<Type> (rhs.x * scale, rhs.y * scale, rhs.z * scale, rhs.w * scale);
	}

	//////////////////////////////////////////////////////////////////////////
	// division : vector / scalar
	template<typename Type, typename S>
	vector_2t<Type> operator / (const vector_2t<Type> &lhs, const S& scale)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
		Type scale_r = static_cast<Type>(1) / scale;
		return lhs * scale_r;
	}

	template<typename Type, typename S>
	vector_3t<Type> operator / (const vector_3t<Type> &lhs, const S& scale)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
		Type scale_r = static_cast<Type>(1) / scale;
		return lhs * scale_r;
	}

	template<typename Type, typename S>
	vector_4t<Type> operator / (const vector_4t<Type> &lhs, const S& scale)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
		Type scale_r = static_cast<Type>(1) / scale;
		return lhs * scale_r;
	}

	//////////////////////////////////////////////////////////////////////////
	// piecewise multiplication : vector * vector
	template<typename Type>
	vector_2t<Type> operator * (const vector_2t<Type> &lhs, const vector_2t<Type> &rhs)
	{
		return vector_2t<Type> (lhs.x * rhs.x, lhs.y * rhs.y);
	}

	template<typename Type>
	vector_3t<Type> operator * (const vector_3t<Type> &lhs, const vector_3t<Type> &rhs)
	{
		return vector_3t<Type> (lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
	}

	template<typename Type>
	vector_4t<Type> operator * (const vector_4t<Type> &lhs, const vector_4t<Type> &rhs)
	{
		return vector_4t<Type> (lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
	}

	//////////////////////////////////////////////////////////////////////////
	// piecewise division : vector / vector
	template<typename Type>
	vector_2t<Type> operator / (const vector_2t<Type> &lhs, const vector_2t<Type> &rhs)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
		return vector_2t<Type> (lhs.x / rhs.x, lhs.y / rhs.y);
	}

	template<typename Type>
	vector_3t<Type> operator / (const vector_3t<Type> &lhs, const vector_3t<Type> &rhs)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
		return vector_3t<Type> (lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
	}

	template<typename Type>
	vector_4t<Type> operator / (const vector_4t<Type> &lhs, const vector_4t<Type> &rhs)
	{
		BOOST_STATIC_ASSERT(boost::is_floating_point<Type>::value);
		return vector_4t<Type> (lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
	}

	//////////////////////////////////////////////////////////////////////////
	// normalization
	template<typename Type>
	vector_2t<Type> normalize(const vector_2t<Type> &operand)
	{
		vector_2t<Type> vec(operand);
		return vec.normalize();
	}

	template<typename Type>
	vector_3t<Type> normalize(const vector_3t<Type> &operand)
	{
		vector_3t<Type> vec(operand);
		return vec.normalize();
	}

	template<typename Type>
	vector_4t<Type> normalize(const vector_4t<Type> &operand)
	{
		vector_4t<Type> vec(operand);
		return vec.normalize();
	}

	//////////////////////////////////////////////////////////////////////////
	// dot product
	template<typename Type>
	Type dot(const vector_2t<Type> &lhs, const vector_2t<Type> &rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}

	template<typename Type>
	Type dot(const vector_3t<Type> &lhs, const vector_3t<Type> &rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	template<typename Type>
	Type dot(const vector_4t<Type> &lhs, const vector_4t<Type> &rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	}

	//////////////////////////////////////////////////////////////////////////
	// cross product
	template<typename Type>
	Type cross(const vector_2t<Type> &lhs, const vector_2t<Type> &rhs)
	{
		return lhs.x * rhs.y - lhs.y * rhs.x;
	}

	template<typename Type>
	vector_3t<Type> cross(const vector_3t<Type> &lhs, const vector_3t<Type> &rhs)
	{
		return vector_3t<Type> (
			lhs.y * rhs.z - lhs.z * rhs.y,
			rhs.x * lhs.z - lhs.x * rhs.z,
			lhs.x * rhs.y - lhs.y * rhs.x);
	}
}

#include "vector_helper.hpp"

#endif // _VECTOR_HPP_