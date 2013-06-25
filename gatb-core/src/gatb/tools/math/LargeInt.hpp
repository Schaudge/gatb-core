/*****************************************************************************
 *   GATB : Genome Assembly Tool Box                                         *
 *   Authors: [R.Chikhi, G.Rizk, E.Drezen]                                   *
 *   Based on Minia, Authors: [R.Chikhi, G.Rizk], CeCILL license             *
 *   Copyright (c) INRIA, CeCILL license, 2013                               *
 *****************************************************************************/

/** \file LargeInt.hpp
 *  \date 01/03/2013
 *  \author edrezen
 *  \brief Class that manages large integers
 *
 * arbitrary-precision integer library
 * very limited: only does what minia needs (but not what minia deserves)
 * This file holds interfaces related to the Design Pattern Observer.
 */

#ifndef _GATB_CORE_TOOLS_MATH_LARGEINT_HPP_
#define _GATB_CORE_TOOLS_MATH_LARGEINT_HPP_

/********************************************************************************/

#include <stdint.h>
#include <algorithm>
#include <iostream>

#include <gatb/system/api/Exception.hpp>
#include <gatb/tools/math/NativeInt64.hpp>

#ifndef ASSERTS
#define NDEBUG // disable asserts; those asserts make sure that with PRECISION == [1 or 2], all is correct
#endif

#include <assert.h>

// some 64-bit assert macros
#if defined(_LP64)
#define assert128(x) assert(precision != 2 || (x));
#else
#define assert128(x) ;
#endif

extern const unsigned char revcomp_4NT[];
extern const unsigned char comp_NT    [];

/********************************************************************************/
namespace gatb  {
namespace core  {
namespace tools {
/** \brief Math package */
namespace math  {
/********************************************************************************/

/** \brief Large integer class
 */
template<int precision>  class LargeInt
{
public:

    static const char* getName ()
    {
        static char buffer[256];
        static bool first = true;
        if (first)  {  first = false;  snprintf (buffer, sizeof(buffer), "LargeInt<%d>", precision);  }
        return buffer;
    }

    static const size_t getSize ()  { return 8*sizeof(uint64_t)*precision; }

    /********************************************************************************/
    /** Constructor.
     * \param[in] val : initial value of the large integer. */
    LargeInt(const uint64_t& val = 0)
    {
        array[0] = val;   for (int i = 1; i < precision; i++)  {  array[i] = 0;  }
    }

    /********************************************************************************/
    LargeInt operator+ (const LargeInt& other) const
    {
        LargeInt result;
        int carry = 0;
        for (int i = 0 ; i < precision ; i++)
        {
            result.array[i] = array[i] + other.array[i] + carry;
            carry = (result.array[i] < array[i]) ? 1 : 0;
        }

        assert    (precision != 1 || (result == other.array[0] + array[0]));
        assert128 (result.toInt128() == other.toInt128() + toInt128());
        return result;
    }

    /********************************************************************************/
    LargeInt operator- (const LargeInt& other) const
    {
        LargeInt result;
        int carry = 0;
        for (int i = 0 ; i < precision ; i++)
        {
            result.array[i] = array[i] - other.array[i] - carry;
            carry = (result.array[i] > array[i]) ? 1 : 0;
        }

        assert(precision != 1 || (result == array[0] - other.array[0]));
        assert128(result.toInt128() == toInt128() - other.toInt128());
        return result;
    }

    
    
    /********************************************************************************/
    LargeInt operator*(const int& coeff) const
    {
        LargeInt result (*this);
        // minia doesn't have that many multiplications cases

        if (coeff == 2 || coeff == 4)
        {
            result = result << (coeff / 2);
        }
        else
        {
            if (coeff == 21)
            {
                result = (result << 4) + (result << 2) + result;
            }
            else
            {
                printf("unsupported LargeInt multiplication: %d\n",coeff);
                exit(1);
            }
        }

        assert(precision != 1 || (result == array[0] * coeff));
        assert128(result.toInt128() == toInt128() * coeff);
        return result;
    }

    /********************************************************************************/
    LargeInt operator/(const uint32_t& divisor) const
    {
        LargeInt result;
        std::fill( result.array, result.array + precision, 0 );

        // inspired by Divide32() from http://subversion.assembla.com/svn/pxcode/RakNet/Source/BigInt.cpp

        uint64_t r = 0;
        uint32_t mask32bits = ~0;
        for (int i = precision-1; i >= 0; --i)
        {
            for (int j = 1; j >= 0; --j) // [j=1: high-32 bits, j=0: low-32 bits] of array[i]
            {
                uint64_t n = (r << 32) | ((array[i] >> (32*j)) & mask32bits );
                result.array[i] = result.array[i] | (((n / divisor) & mask32bits) << (32*j));
                r = n % divisor;
            }
        }
        assert(precision != 1 || (result == array[0] / divisor));
        assert128(result.toInt128() == toInt128() / divisor);
        return result;
    }

    /********************************************************************************/
    uint32_t operator%(const uint32_t& divisor) const
    {
        uint64_t r = 0;
        uint32_t mask32bits = ~0;
        for (int i = precision-1; i >= 0; --i)
        {
            for (int j = 1; j >= 0; --j) // [j=1: high-32 bits, j=0: low-32 bits] of array[i]
            {
                uint64_t n = (r << 32) | ((array[i] >> (32*j)) & mask32bits );
                r = n % divisor;
            }
        }

        assert(precision != 1 || (r == array[0] % divisor));
        assert128(r == toInt128() % divisor);
        return (uint32_t)r;
    }

    /********************************************************************************/
    LargeInt operator^(const LargeInt& other) const
    {
        LargeInt result;
        for (int i=0 ; i < precision ; i++)
            result.array[i] = array[i] ^ other.array[i];

        assert(precision != 1 || (result == (array[0] ^ other.array[0])));
        assert128(result.toInt128() == (toInt128() ^ other.toInt128()));
        return result;
    }

    /********************************************************************************/
    LargeInt operator|(const LargeInt& other) const
    {
        LargeInt result;
        for (int i=0 ; i < precision ; i++)
            result.array[i] = array[i] | other.array[i];
        
        assert(precision != 1 || (result == (array[0] | other.array[0])));
        assert128(result.toInt128() == (toInt128() | other.toInt128()));
        return result;
    }
    
    /********************************************************************************/
    LargeInt operator&(const LargeInt& other) const
    {
        LargeInt result;
        for (int i=0 ; i < precision ; i++)
            result.array[i] = array[i] & other.array[i];

        assert(precision != 1 || (result == (array[0] & other.array[0])));
        assert128(result.toInt128() == (toInt128() & other.toInt128()));
        return result;
    }

    /********************************************************************************/
    LargeInt operator&(const char& other) const
    {
        LargeInt result;
        result.array[0] = array[0] & other;
        return result;
    }

    /********************************************************************************/
    LargeInt operator~() const
                    {
        LargeInt result;
        for (int i=0 ; i < precision ; i++)
            result.array[i] = ~array[i];

        assert(precision != 1 || (result == ~array[0]));
        assert128(result.toInt128() == ~toInt128());
        return result;
                    }

    /********************************************************************************/
    LargeInt operator<<(const int& coeff) const
    {
        LargeInt result (0);

        int large_shift = coeff / 64;
        int small_shift = coeff % 64;

        for (int i = large_shift ; i < precision-1; i++)
        {
            result.array[i] = result.array[i] | (array[i-large_shift] << small_shift);

            if (small_shift == 0) // gcc "bug".. uint64_t x; x>>64 == 1<<63, x<<64 == 1
            {
                result.array[i+1] = 0;
            }
            else
            {
                result.array[i+1] = array[i-large_shift] >> (64 - small_shift);
            }

        }
        result.array[precision-1] = result.array[precision-1] | (array[precision-1-large_shift] << small_shift);

        assert(precision != 1 || (result == (array[0] << coeff)));
        assert128(result.toInt128() == (toInt128() << coeff));
        return result;
    }

    /********************************************************************************/
    LargeInt operator>>(const int& coeff) const
    {
        LargeInt result (0);

        int large_shift = coeff / 64;
        int small_shift = coeff % 64;

        result.array[0] = (array[large_shift] >> small_shift);

        for (int i = 1 ; i < precision - large_shift ; i++)
        {
            result.array[i] = (array[i+large_shift] >> small_shift);
            if (small_shift == 0 && large_shift > 0) // gcc "bug".. uint64_t x; x>>64 == 1<<63, x<<64 == 1
            {
                result.array[i-1] =  result.array[i-1];
            }
            else
            {
                result.array[i-1] =  result.array[i-1] | (array[i+large_shift] << (64 - small_shift));
            }
        }

        assert(precision != 1 || ( small_shift == 0 || (result == array[0] >> coeff)));
        assert128(small_shift == 0 || (result.toInt128() == (toInt128() >> coeff)));
        return result;
    }

    /********************************************************************************/
    bool     operator!=(const LargeInt& c) const
                    {
        for (int i = 0 ; i < precision ; i++)
            if( array[i] != c.array[i] )
                return true;
        return false;
                    }

    /********************************************************************************/
    bool operator==(const LargeInt& c) const
    {
        for (int i = 0 ; i < precision ; i++)
            if( array[i] != c.array[i] )
                return false;
        return true;
    }

    /********************************************************************************/
    bool operator<(const LargeInt& c) const
    {
        for (int i = precision-1 ; i>=0 ; --i)
            if( array[i] != c.array[i] )
                return array[i] < c.array[i];

        return false;
    }

    /********************************************************************************/
    bool operator<=(const LargeInt& c) const
    {
        return operator==(c) || operator<(c);
    }

    /********************************************************************************/
    LargeInt& operator+=  (const LargeInt& other)
    {
        // NOT so easy to optimize because of the carry
        *this = *this + other;
        return *this;
    }

    /********************************************************************************/
    LargeInt& operator^=  (const LargeInt& other)
    {
        for (int i=0 ; i < precision ; i++)  {  array[i] ^= other.array[i];  }
        return *this;
    }

    /********************************************************************************/
    friend std::ostream & operator<<(std::ostream & s, const LargeInt<precision> & l)
    {
        int i=0;

        /** We want to display the number in hexa (easier to do...) */
        s << std::hex;

        /** We skip leading 0. */
        for (i=precision-1; i>=0 && l.array[i]==0; i--)  {}

        /** We dump the different parts of the large integer. */
        for (  ; i>=0 ; i--)  { s << l.array[i];   if (i>=1) { s << ".";  }  }

        /** We go back to decimal format. */
        s << std::dec;

        /** We return the output stream. */
        return s;
    }

private:
    uint64_t array[precision];

    template<int T>  friend LargeInt<T> revcomp (const LargeInt<T>& i, size_t sizeKmer);
    template<int T>  friend u_int64_t   hash    (const LargeInt<T>& key, u_int64_t  seed);
    template<int T>  friend u_int64_t   simplehash16    (const LargeInt<T>& key, int  shift);

    // c++ fun fact:
    // "const" will ban the function from being anything which can attempt to alter any member variables in the object.
};

/********************************************************************************/
template<int precision>  inline LargeInt<precision> revcomp (const LargeInt<precision>& x, size_t sizeKmer)
{
    const LargeInt<precision> res = x;

    unsigned char* kmerrev  = (unsigned char *) (&(res.array[0]));
    unsigned char* kmer     = (unsigned char *) (&(x.array[0]));

    for (size_t i=0; i<8*precision; ++i)
    {
        kmerrev[8*precision-1-i] = revcomp_4NT [kmer[i]];
    }

    return (res >> (2*( 32*precision - sizeKmer))  ) ;
}

/********************************************************************************/
template<int precision>  inline u_int64_t hash (const LargeInt<precision>& elem, u_int64_t seed=0)
{
    // hash = XOR_of_series[hash(i-th chunk iof 64 bits)]
    u_int64_t result = 0, chunk, mask = ~0;

    LargeInt<precision> intermediate = elem;
    for (size_t i=0;i<precision;i++)
    {
        chunk = (intermediate & mask).array[0];
        intermediate = intermediate >> 64;

        result ^= NativeInt64::hash64 (chunk,seed);
    }
    return result;
}

/********************************************************************************/
template<int precision>  inline u_int64_t simplehash16 (const LargeInt<precision>& elem, int  shift)
{
    u_int64_t result = 0, chunk, mask = ~0;
    LargeInt<precision> intermediate = elem;
    
    chunk = (intermediate & mask).array[0];
    result ^= NativeInt64::simplehash16_64 (chunk,shift);

    return result;
}

    
/********************************************************************************/
} } } } /* end of namespaces. */
/********************************************************************************/

#endif /* _GATB_CORE_TOOLS_MATH_LARGEINT_HPP_ */
