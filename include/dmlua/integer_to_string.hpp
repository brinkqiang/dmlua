

#ifndef __INTEGER_TO_STRING_H_INCLUDE__
#define __INTEGER_TO_STRING_H_INCLUDE__

#include <stdio.h>

template <typename C>
inline
C const*
get_digit_character()
{
    static C const  s_characters[19] =
    {
        '9'
            ,   '8'
            ,   '7'
            ,   '6'
            ,   '5'
            ,   '4'
            ,   '3'
            ,   '2'
            ,   '1'
            ,   '0'
            ,   '1'
            ,   '2'
            ,   '3'
            ,   '4'
            ,   '5'
            ,   '6'
            ,   '7'
            ,   '8'
            ,   '9'
    };
    return s_characters + 9;
}

template<   typename C
,   typename I
>
inline
C const*
unsigned_integer_to_string(C* buf, size_t cchBuf, I i)
{
    C* psz = buf + cchBuf - 1;  // Set pointer to last character.

    *psz = 0;   // Set the terminating null character.

    do
    {
        typedef I           rem_t;

        rem_t lsd = static_cast<rem_t>(i % 10);   // Determine the least significant digit.

        i = static_cast<I>(i / 10);                 // Deal with next most significant.

        --psz;                                      // Move back.

        *psz = get_digit_character<C>()[lsd];

    } while(i != 0);

    return psz;
}

template<   typename C
,   typename I
>
inline
C const*
signed_integer_to_string(C* buf, size_t cchBuf, I i)
{
    typedef I           rem_t;

    C* psz = buf + cchBuf - 1;  // Set pointer to last character.

    *psz = 0;   // Set the terminating null character.

    if(i < 0)
    {
        do
        {
            rem_t lsd = static_cast<rem_t>(i % 10);   // Determine the least significant digit.

            i = static_cast<I>(i / 10);                 // Deal with next most significant.

            --psz;                                      // Move back.

            *psz = get_digit_character<C>()[lsd];

        } while(i != 0);

        *(--psz) = C('-');              // Prepend the minus sign.
    }
    else
    {
        do
        {
            rem_t   lsd = static_cast<rem_t>(i % 10);   // Determine the least significant digit.

            i = static_cast<I>(i / 10);                 // Deal with next most significant.

            --psz;                                      // Move back.

            *psz = get_digit_character<C>()[lsd];

        } while(i != 0);
    }

    return psz;
}

template <typename C>
inline
C const*
integer_to_string(C* buf, size_t cchBuf, char i)
{
    return signed_integer_to_string(buf, cchBuf, i);
}

template <typename C>
inline
C const*
integer_to_string(C* buf, size_t cchBuf, unsigned char i)
{
    return unsigned_integer_to_string(buf, cchBuf, i);
}

template <typename C>
inline
C const*
integer_to_string(C* buf, size_t cchBuf, short i)
{
    return signed_integer_to_string(buf, cchBuf, i);
}

template <typename C>
inline
C const*
integer_to_string(C* buf, size_t cchBuf, unsigned short i)
{
    return unsigned_integer_to_string(buf, cchBuf, i);
}

template <typename C>
inline
C const*
integer_to_string(C* buf, size_t cchBuf, int i)
{
    return signed_integer_to_string(buf, cchBuf, i);
}

template <typename C>
inline
C const*
integer_to_string(C* buf, size_t cchBuf, unsigned int i)
{
    return unsigned_integer_to_string(buf, cchBuf, i);
}

template <typename C>
inline
C const*
integer_to_string(C* buf, size_t cchBuf, long long const& i)
{
    return signed_integer_to_string(buf, cchBuf, i);
}

template <typename C>
inline
C const*
integer_to_string(C* buf, size_t cchBuf, unsigned long long const& i)
{
    return unsigned_integer_to_string(buf, cchBuf, i);
}

template< typename C
, size_t           N
>
inline
C const*
integer_to_string(C (&buf)[N], char i)
{

    return signed_integer_to_string(buf, N, i);
}

template< typename C
, size_t           N
>
inline
C const*
integer_to_string(C (&buf)[N], unsigned char i)
{
    return unsigned_integer_to_string(buf, N, i);
}

template< typename C
, size_t           N
>
inline
C const*
integer_to_string(C (&buf)[N], short i)
{
    return signed_integer_to_string(buf, N, i);
}

template< typename C
, size_t           N
>
inline
C const*
integer_to_string(C (&buf)[N], unsigned short i)
{
    return unsigned_integer_to_string(buf, N, i);
}

template< typename C
, size_t           N
>
inline
C const*
integer_to_string(C (&buf)[N], int i)
{
    return signed_integer_to_string(buf, N, i);
}

template< typename C
, size_t           N
>
inline
C const*
integer_to_string(C (&buf)[N], unsigned int i)
{
    return unsigned_integer_to_string(buf, N, i);
}

template< typename C
, size_t           N
>
inline
C const*
integer_to_string(C (&buf)[N], long long const& i)
{
    return signed_integer_to_string(buf, N, i);
}

template< typename C
, size_t           N
>
inline
C const*
integer_to_string(C (&buf)[N], unsigned long long const& i)
{
    return unsigned_integer_to_string(buf, N, i);
}

#endif // __INTEGER_TO_STRING_H_INCLUDE__
