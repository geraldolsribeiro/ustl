// This file is part of the ustl library, an STL implementation.
// Copyright (C) 2003 by Mike Sharov <msharov@talentg.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the 
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
// Boston, MA  02111-1307  USA.
//
/// \file uutility.h
///
/// \brief Utility templates.
//

#ifndef UUTILITY_H_6A58BD296269A82A4AAAA4FD19FDB3AC
#define UUTILITY_H_6A58BD296269A82A4AAAA4FD19FDB3AC

#include "utypes.h"
#include <assert.h>
#ifdef HAVE_BYTESWAP_H
    #include <byteswap.h>
#endif

namespace ustl {

#ifdef __GNUC__
    /// Returns the number of elements in a static vector
    #define VectorSize(v)	(sizeof(v) / sizeof(*v))
#else
    // Old compilers will not be able to evaluate *v on an empty vector.
    // The tradeoff here is that VectorSize will not be able to measure arrays of local structs.
    #define VectorSize(v)	(sizeof(v) / ustl::size_of_elements(1, v))
#endif

/// Returns the number of bits in the given type
#define BitsInType(t)	(sizeof(t) * CHAR_BIT)

/// Returns the mask of type \p t with the lowest \p n bits set.
#define BitMask(t,n)	(t(~t(0)) >> ((sizeof(t) * CHAR_BIT) - (n)))

/// Argument that is used only in debug builds (as in an assert)
#ifndef NDEBUG
    #define DebugArg(x)	x
#else
    #define DebugArg(x)
#endif

/// Shorthand for container iteration.
#define foreach(type,i,ctr)	for (type i = ctr.begin(); i != ctr.end(); ++ i)

/// The alignment performed by default.
const size_t c_DefaultAlignment = sizeof(void*);

/// Divides \p n1 by \p n2 and rounds the result up (regular division rounds down).
/// Negative numbers are rounded down because they are an unusual case, supporting
/// which would require a branch. Since this is frequently used in graphics, the
/// speed is important.
///
template <typename T1, typename T2>
inline T1 DivRU (T1 n1, T2 n2)
{
    return (n1 / n2 + (n1 % n2 > 0));
}

/// Rounds \p n up to be divisible by \p grain
template <typename T>
inline T Align (T n, T grain = c_DefaultAlignment)
{
    const T remainder = n % grain;
    return (remainder ? n + (grain - remainder) : n);
}

/// Offsets an iterator
template <typename T>
inline T advance (T i, ssize_t offset)
{
    return (i + offset);
}

/// Offsets a void pointer
template <>
inline const void* advance (const void* p, ssize_t offset)
{
    assert (p || !offset);
    return (reinterpret_cast<const u_char*>(p) + offset);
}

/// Offsets a void pointer
template <>
inline void* advance (void* p, ssize_t offset)
{
    assert (p || !offset);
    return (reinterpret_cast<u_char*>(p) + offset);
}

/// Returns the difference \p p1 - \p p2
template <typename T1, typename T2>
inline ptrdiff_t distance (T1 i1, T2 i2)
{
    return (i2 - i1);
}

#define UNVOID_DISTANCE(T1const,T2const)			\
template <>							\
inline ptrdiff_t distance (T1const void* p1, T2const void* p2)	\
{								\
    return (reinterpret_cast<T2const u_char*>(p2) -		\
	    reinterpret_cast<T1const u_char*>(p1));		\
}
UNVOID_DISTANCE(,)
UNVOID_DISTANCE(const,const)
UNVOID_DISTANCE(,const)
UNVOID_DISTANCE(const,)
#undef UNVOID_DISTANCE

/// \brief Returns the absolute value of \p v
/// Unlike the stdlib functions, this is inline and works with all types.
template <typename T>
inline T absv (T v)
{
    return (v < 0 ? -v : v);
}

/// \brief Returns -1 for negative values, 1 for positive, and 0 for 0
template <typename T>
inline T sign (T v)
{
    return (0 < v ? 1 : (v < 0 ? -1 : 0));
}

/// Returns the absolute value of the distance i1 and i2
template <typename T1, typename T2>
inline size_t abs_distance (T1 i1, T2 i2)
{
    return (absv (distance(i1, i2)));
}

/// Returns the size of \p n elements of size \p T
template <typename T>
inline size_t size_of_elements (size_t n, const T*)
{
    return (n * sizeof(T));
}

#ifdef HAVE_BYTESWAP_H

template <typename T>
inline T bswap (const T& v)
{
    switch (BitsInType(T)) {
	default:	return (v);
	case 16:	return (T (bswap_16 (uint16_t (v))));
	case 32:	return (T (bswap_32 (uint32_t (v))));
#if defined(bswap_64) && defined(HAVE_INT64_T)
	case 64:	return (T (bswap_64 (uint64_t (v))));
#endif
    };
}

#if __BYTE_ORDER == __BIG_ENDIAN
template <typename T> inline T le_to_native (const T& v) { return (bswap (v)); }
template <typename T> inline T be_to_native (const T& v) { return (v); }
template <typename T> inline T native_to_le (const T& v) { return (bswap (v)); }
template <typename T> inline T native_to_be (const T& v) { return (v); }
#elif __BYTE_ORDER == __LITTLE_ENDIAN
template <typename T> inline T le_to_native (const T& v) { return (v); }
template <typename T> inline T be_to_native (const T& v) { return (bswap (v)); }
template <typename T> inline T native_to_le (const T& v) { return (v); }
template <typename T> inline T native_to_be (const T& v) { return (bswap (v)); }
#else
    #error Your system does not define __BYTE_ORDER. Please define it in config.h
#endif // __BYTE_ORDER
#endif // HAVE_BYTESWAP_H

/// Template for for_each to call delete
template <typename T>
inline void Delete (T* p)
{
    delete p;
}

/// Template for for_each to call delete
template <typename T>
inline void DeleteVector (T* p)
{
    delete [] p;
}

/// Returns the minimum of \p a and \p b
template <typename T1, typename T2>
inline const T1 min (const T1& a, const T2& b)
{
    return (a < b ? a : b);
}

/// Returns the maximum of \p a and \p b
template <typename T1, typename T2>
inline const T1 max (const T1& a, const T2& b)
{
    return (b < a ? a : b);
}

/// Template of making != from ! and ==
template <typename T>
inline bool operator!= (const T& x, const T& y)
{
    return (!(x == y));
}

/// Template of making > from <
template <typename T>
inline bool operator> (const T& x, const T& y)
{
    return (y < x);
}

/// Template of making <= from < and ==
template <typename T>
inline bool operator<= (const T& x, const T& y)
{
    return (x == y || x < y);
}

/// Template of making >= from < and ==
template <typename T>
inline bool operator>= (const T& x, const T& y)
{
    return (x == y || y < x);
}

/// Packs \p s multiple times into \p b. Useful for loop unrolling.
template <typename TSmall, typename TBig>
inline void pack_type (TSmall s, TBig& b)
{
    const size_t n = sizeof(TBig) / sizeof(TSmall);
    b = s;
    // Calls to min are here to avoid warnings for shifts bigger than the type. min will be gone when optimized.
    if (n < 2) return;
    b = (b << min (BitsInType(TSmall), BitsInType(TBig))) | b;
    if (n < 4) return;
    b = (b << min (BitsInType(TSmall) * 2, BitsInType(TBig))) | b;
    if (n < 8) return;
    b = (b << min (BitsInType(TSmall) * 4, BitsInType(TBig))) | b;
}

} // namespace ustl

#endif

