#ifndef UTILS_H
#define UTILS_H
#include "NTL/ZZ.h"
#include "constants.h"
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"


using namespace NTL;


bool getBits(uint64_t val, int index)
{
    
    return ((val >> index) & (uint64_t) 1);

}    
    
ZZ get_zz_from_uint(uint64_t high, uint64_t low)
{
    ZZ ret;


    /* first deal with the low bits, loop and add until we get a zero*/
    uint64_t tmp = low;
    int j = 0;
    int set;
    while(tmp > 0)
    {
        
        if(getBits(low, j) == 1) set = 1;
        else set = 0;

        ret += power(ZZ(2),j) * set;
        j++;
        tmp = tmp >> 1;
        //cout << tmp << endl;
    }


    /* Now deal with the high bits, loop and add until we get a zero*/
    tmp = high;
    int k = 0;
    j = 64;
    while(tmp > 0)
    {
        
        if(getBits(high, k) == 1) set = 1;
        else set = 0;

        ret += power(ZZ(2),j) * set;
        j++;k++;
        tmp = tmp >> 1;
        //cout << tmp << endl;
    }

    return ret;
}

#endif