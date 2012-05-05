// file: 		common_helper.h
// path:		AywProc\AywProc\core\include
// purpose:		define some common macros
//
// author:		atyuwen
// homepage:	http://code.google.com/p/aywproc/
//
// created:		2010/05/26
// history:	
//
//////////////////////////////////////////////////////////////////////

#ifndef _COMMON_HELPER_H_
#define _COMMON_HELPER_H_

#define DECLARE_CONVERSION_TO_POINTER(Type, addr)		\
	Type * ptr()										\
	{													\
		return (Type *)(addr);							\
	}													\
	const Type * ptr() const							\
	{													\
		return (const Type *)(addr);					\
	}

#define DECLARE_SUBSCRIPT_OPERATOR(Type, addr)			\
	Type & operator [] (size_t i)						\
	{													\
		return (addr)[i];								\
	}													\
	const Type & operator [] (size_t i) const			\
	{													\
		return (addr)[i];								\
	}

#define DECLARE_NAIVE_ITERATOR(Type, addr, n)						\
	typedef Type value_type;										\
	typedef value_type* iterator;									\
	typedef const value_type* const_iterator;						\
	iterator begin() {return iterator(addr);}						\
	iterator end() {return iterator(addr) + (n);}					\
	const_iterator begin() const {return const_iterator(addr);}		\
	const_iterator end() const {return const_iterator(addr) + (n);}

#endif // _COMMON_HELPER_H_