#pragma once

#ifndef FASSERT_H
#define FASSERT_H

// Only compile in FAssert's if FASSERT_ENABLE is defined.  By default, however, let's key off of
// _DEBUG.  Sometimes, however, it's useful to enable asserts in release builds, and you can do that
// simply by changing the following lines to define FASSERT_ENABLE or using project settings to override
#ifdef _DEBUG
#define FASSERT_ENABLE
#endif 

#ifdef FASSERT_ENABLE
/*  f1rpo: Inlining functions with assertions could be a bad choice.
	Let's at least not force-inline. Could in particular be important for ScaledNum.h. */
#define __forceinline __inline

#ifdef WIN32

bool FAssertDlg( const char*, const char*, const char*, unsigned int, bool& );

#define FAssert( expr )	\
{ \
	static bool bIgnoreAlways = false; \
	if( !bIgnoreAlways && !(expr) ) \
{ \
	if( FAssertDlg( #expr, 0, __FILE__, __LINE__, bIgnoreAlways ) ) \
{ _asm int 3 } \
} \
}

#define FAssertMsg( expr, msg ) \
{ \
	static bool bIgnoreAlways = false; \
	if( !bIgnoreAlways && !(expr) ) \
{ \
	if( FAssertDlg( #expr, msg, __FILE__, __LINE__, bIgnoreAlways ) ) \
{ _asm int 3 } \
} \
}

#else
// Non Win32 platforms--just use built-in FAssert
#define FAssert( expr )	FAssert( expr )
#define FAssertMsg( expr, msg )	FAssert( expr )

#endif

// <f1rpo> Moved from CvInitCore.h. Merged some changes from K-Mod and AdvCiv.
#define FASSERT_BOUNDS(lower,upper,index,fnString)\
	if (static_cast<int>(index) < static_cast<int>(lower))\
	{\
		char acOut[256];\
		sprintf(acOut, "Index in %s expected to be >= %d. (value: %d)", fnString,\
				static_cast<int>(lower), static_cast<int>(index));\
		FAssertMsg(static_cast<int>(index) >= static_cast<int>(lower), acOut);\
	}\
	else if (static_cast<int>(index) >= static_cast<int>(upper))\
	{\
		char acOut[256];\
		sprintf(acOut, "Index in %s expected to be < %d. (value: %d)", fnString,\
				static_cast<int>(upper), static_cast<int>(index));\
		FAssertMsg(static_cast<int>(index) < static_cast<int>(upper), acOut);\
	}
// </f1rpo>

#else
// FASSERT_ENABLE not defined
#define FAssert( expr ) void(0) // f1rpo (forces semicolon)
#define FAssertMsg( expr, msg ) void(0) // f1rpo
#define FASSERT_BOUNDS(lower,upper,index,fnString) void(0) // f1rpo
#endif

/*	f1rpo: The fnString is pretty unnecessary; we get __FILE__ and __LINE__ anyway.
	Could also get __FUNCTION__. */
#define FAssertBounds(lower,upper,index) FASSERT_BOUNDS(lower,upper,index, "array accessor");

#endif // FASSERT_H
