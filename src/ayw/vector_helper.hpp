// file: 		vector_helper.hpp
// path:		AywProc\AywProc\core\include
// purpose:		define some macros to generate implementation codes for 
//				swizzle operations
// author:		atyuwen
// homepage:	http://code.google.com/p/aywproc/
//
// created:		2010/04/21
// history:	
//
//////////////////////////////////////////////////////////////////////

#ifndef MOUNT_VECTOR_HELPER_MACROS
#define MOUNT_VECTOR_HELPER_MACROS

#include <boost/preprocessor/seq.hpp>

#define SEQ_VECTOR_2T (x)(y)
#define SEQ_VECTOR_3T (x)(y)(z)
#define SEQ_VECTOR_4T (x)(y)(z)(w)

#define IMPL_SWIZZLE_OP_TO_2T(_, seq)						\
	vector_2t<Type> BOOST_PP_SEQ_CAT(seq)() const			\
	{														\
		return vector_2t<Type>(BOOST_PP_SEQ_ENUM(seq));		\
	}                                     

#define IMPL_SWIZZLE_OP_TO_3T(_, seq)						\
	vector_3t<Type> BOOST_PP_SEQ_CAT(seq)() const			\
	{														\
		return vector_3t<Type>(BOOST_PP_SEQ_ENUM(seq));		\
	}  

#define IMPL_SWIZZLE_OP_TO_4T(_, seq)						\
	vector_4t<Type> BOOST_PP_SEQ_CAT(seq)() const			\
	{														\
		return vector_4t<Type>(BOOST_PP_SEQ_ENUM(seq));		\
	}  

#define GEN_SWIZZLE_OP_2T_TO_2T() \
	BOOST_PP_SEQ_FOR_EACH_PRODUCT(IMPL_SWIZZLE_OP_TO_2T, (SEQ_VECTOR_2T)(SEQ_VECTOR_2T)) 

#define GEN_SWIZZLE_OP_2T_TO_3T() \
	BOOST_PP_SEQ_FOR_EACH_PRODUCT(IMPL_SWIZZLE_OP_TO_3T, (SEQ_VECTOR_2T)(SEQ_VECTOR_2T)(SEQ_VECTOR_2T)) 

#define GEN_SWIZZLE_OP_2T_TO_4T() \
	BOOST_PP_SEQ_FOR_EACH_PRODUCT(IMPL_SWIZZLE_OP_TO_4T, (SEQ_VECTOR_2T)(SEQ_VECTOR_2T)(SEQ_VECTOR_2T)(SEQ_VECTOR_2T)) 

#define GEN_SWIZZLE_OP_3T_TO_2T() \
	BOOST_PP_SEQ_FOR_EACH_PRODUCT(IMPL_SWIZZLE_OP_TO_2T, (SEQ_VECTOR_3T)(SEQ_VECTOR_3T))

#define GEN_SWIZZLE_OP_3T_TO_3T() \
	BOOST_PP_SEQ_FOR_EACH_PRODUCT(IMPL_SWIZZLE_OP_TO_3T, (SEQ_VECTOR_3T)(SEQ_VECTOR_3T)(SEQ_VECTOR_3T))

#define GEN_SWIZZLE_OP_3T_TO_4T() \
	BOOST_PP_SEQ_FOR_EACH_PRODUCT(IMPL_SWIZZLE_OP_TO_4T, (SEQ_VECTOR_3T)(SEQ_VECTOR_3T)(SEQ_VECTOR_3T)(SEQ_VECTOR_3T))

#define GEN_SWIZZLE_OP_4T_TO_2T() \
	BOOST_PP_SEQ_FOR_EACH_PRODUCT(IMPL_SWIZZLE_OP_TO_2T, (SEQ_VECTOR_4T)(SEQ_VECTOR_4T))

#define GEN_SWIZZLE_OP_4T_TO_3T() \
	BOOST_PP_SEQ_FOR_EACH_PRODUCT(IMPL_SWIZZLE_OP_TO_3T, (SEQ_VECTOR_4T)(SEQ_VECTOR_4T)(SEQ_VECTOR_4T))

#define GEN_SWIZZLE_OP_4T_TO_4T() \
	BOOST_PP_SEQ_FOR_EACH_PRODUCT(IMPL_SWIZZLE_OP_TO_4T, (SEQ_VECTOR_4T)(SEQ_VECTOR_4T)(SEQ_VECTOR_4T)(SEQ_VECTOR_4T))

#define DECLARE_SWIZZLING_FOR_VECTOR_2T() \
	GEN_SWIZZLE_OP_2T_TO_2T()			  \
	GEN_SWIZZLE_OP_2T_TO_3T()			  \
	GEN_SWIZZLE_OP_2T_TO_4T()

#define DECLARE_SWIZZLING_FOR_VECTOR_3T() \
	GEN_SWIZZLE_OP_3T_TO_2T()			  \
	GEN_SWIZZLE_OP_3T_TO_3T()			  \
	GEN_SWIZZLE_OP_3T_TO_4T()

#define DECLARE_SWIZZLING_FOR_VECTOR_4T() \
	GEN_SWIZZLE_OP_4T_TO_2T()			  \
	GEN_SWIZZLE_OP_4T_TO_3T()			  \
	GEN_SWIZZLE_OP_4T_TO_4T()

#else // unmount these macros

#undef MOUNT_VECTOR_HELPER_MACROS

#undef SEQ_VECTOR_2T
#undef SEQ_VECTOR_3T
#undef SEQ_VECTOR_4T

#undef IMPL_SWIZZLE_OP_TO_2T
#undef IMPL_SWIZZLE_OP_TO_3T
#undef IMPL_SWIZZLE_OP_TO_4T

#undef GEN_SWIZZLE_OP_2T_TO_2T
#undef GEN_SWIZZLE_OP_2T_TO_3T
#undef GEN_SWIZZLE_OP_2T_TO_4T
#undef GEN_SWIZZLE_OP_3T_TO_2T
#undef GEN_SWIZZLE_OP_3T_TO_3T
#undef GEN_SWIZZLE_OP_3T_TO_4T
#undef GEN_SWIZZLE_OP_4T_TO_2T
#undef GEN_SWIZZLE_OP_4T_TO_3T
#undef GEN_SWIZZLE_OP_4T_TO_4T

#undef DECLARE_SWIZZLING_FOR_VECTOR_2T
#undef DECLARE_SWIZZLING_FOR_VECTOR_3T
#undef DECLARE_SWIZZLING_FOR_VECTOR_4T

#endif