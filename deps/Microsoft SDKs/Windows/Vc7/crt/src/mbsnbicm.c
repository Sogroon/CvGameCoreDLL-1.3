/***
*mbsnbicmp.c - Compare n bytes of strings, ignoring case (MBCS)
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Compare n bytes of strings, ignoring case (MBCS)
*
*******************************************************************************/

#ifdef _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <string.h>
#include <mbstring.h>

/***
* _mbsnbicmp - Compare n bytes of strings, ignoring case (MBCS)
*
*Purpose:
*       Compares up to n bytes of two strings for lexical order.
*       Strings are compared on a character basis, not a byte basis.
*       Case of characters is not considered.
*
*Entry:
*       unsigned char *s1, *s2 = strings to compare
*       size_t n = maximum number of bytes to compare
*
*Exit:
*       returns <0 if s1 < s2
*       returns  0 if s1 == s2
*       returns >0 if s1 > s2
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _mbsnbicmp(
        const unsigned char *s1,
        const unsigned char *s2,
        size_t n
        )
{
        unsigned short c1, c2;
#ifdef _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif  /* _MT */

        if (n==0)
            return(0);

#ifdef _MT
        if ( _ISNOTMBCP_MT(ptmbci) )
#else  /* _MT */
        if ( _ISNOTMBCP )
#endif  /* _MT */
            return _strnicmp(s1, s2, n);

        while (n--) {

            c1 = *s1++;
#ifdef _MT
            if ( __ismbblead_mt(ptmbci, c1) )
#else  /* _MT */
            if ( _ismbblead(c1) )
#endif  /* _MT */
            {
                if (n==0)
                {
                    c1 = 0; /* 'naked' lead - end of string */
#ifdef _MT
                    c2 = __ismbblead_mt(ptmbci, *s2) ? 0 : *s2;
#else  /* _MT */
                    c2 = _ismbblead(*s2) ? 0 : *s2;
#endif  /* _MT */
                    goto test;
                }
                if (*s1 == '\0')
                    c1 = 0;
                else {
                    c1 = ((c1<<8) | *s1++);

#ifdef _MT
                    if ( ((c1 >= _MBUPPERLOW1_MT(ptmbci)) &&
                          (c1 <= _MBUPPERHIGH1_MT(ptmbci))) )
                        c1 += _MBCASEDIFF1_MT(ptmbci);
                    else if ( ((c1 >= _MBUPPERLOW2_MT(ptmbci)) &&
                               (c1 <= _MBUPPERHIGH2_MT(ptmbci))) )
                        c1 += _MBCASEDIFF2_MT(ptmbci);
#else  /* _MT */
                    if ( ((c1 >= _MBUPPERLOW1) && (c1 <= _MBUPPERHIGH1)) )
                        c1 += _MBCASEDIFF1;
                    else if ( ((c1 >= _MBUPPERLOW2) && (c1 <= _MBUPPERHIGH2)) )
                        c1 += _MBCASEDIFF2;
#endif  /* _MT */
                }
            }
            else
#ifdef _MT
                c1 = __mbbtolower_mt(ptmbci, c1);
#else  /* _MT */
                c1 = _mbbtolower(c1);
#endif  /* _MT */

                c2 = *s2++;
#ifdef _MT
                if ( __ismbblead_mt(ptmbci, c2) )
#else  /* _MT */
                if ( _ismbblead(c2) )
#endif  /* _MT */
                {
                    if (n==0)
                    {
                        c2 = 0; /* 'naked' lead - end of string */
                        goto test;
                    }
                    n--;
                    if (*s2 == '\0')
                        c2 = 0;
                    else {
                        c2 = ((c2<<8) | *s2++);

#ifdef _MT
                        if ( ((c2 >= _MBUPPERLOW1_MT(ptmbci)) &&
                              (c2 <= _MBUPPERHIGH1_MT(ptmbci))) )
                            c2 += _MBCASEDIFF1_MT(ptmbci);
                        else if ( ((c2 >= _MBUPPERLOW2_MT(ptmbci)) &&
                                   (c2 <= _MBUPPERHIGH2_MT(ptmbci))) )
                            c2 += _MBCASEDIFF2_MT(ptmbci);
#else  /* _MT */
                        if ( ((c2 >= _MBUPPERLOW1) && (c2 <= _MBUPPERHIGH1)) )
                            c2 += _MBCASEDIFF1;
                        else if ( ((c2 >= _MBUPPERLOW2) && (c2 <= _MBUPPERHIGH2)) )
                            c2 += _MBCASEDIFF2;
#endif  /* _MT */
                    }
                }
                else
#ifdef _MT
                    c2 = __mbbtolower_mt(ptmbci, c2);
#else  /* _MT */
                    c2 = _mbbtolower(c2);
#endif  /* _MT */

test:
            if (c1 != c2)
                return( (c1 > c2) ? 1 : -1);

            if (c1 == 0)
                return(0);
        }

        return(0);
}

#endif  /* _MBCS */
