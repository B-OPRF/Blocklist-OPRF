#include "NTL/ZZ.h"
#include "constants.h"
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include "emp-tool/emp-tool.h"


using namespace NTL;
using namespace std;


vec_ZZ_p secret_share(ZZ_p y_C, int theta) {
    vec_ZZ_p shares;
    ZZ_pX poly;
    vec_ZZ_p points;

    SetCoeff(poly, 0, y_C);
    for (long i = 1; i < theta; ++i) {
        ZZ_p coeff; 
        random(coeff);
        SetCoeff(poly, i, coeff);
    }

    for (long i = 0; i < theta; ++i) {
        ZZ_p point; 
        random(point);
        points.append(point);
    }

    for (long i = 0; i < theta; ++i) {
        ZZ_p share = eval(poly, points[i]);
        shares.append(share);
    }

    return shares;
}

ZZ_p AES(vector<int> SIM) {
  vector<int> ret;
  uint8_t input[128];
  uint8_t output_bytes[128];
  for (size_t i = 0; i < 128; ++i) {
    if (i < SIM.size())
        input[i] = SIM[i];
    else
        input[i] = 0;
  }
  __m128i key;
  __m128i iv;
  for (size_t i = 0; i < 16; ++i) {
    ((uint8_t *)(&key))[i] = (1337 * i) % 255;
    ((uint8_t *)(&iv))[i] = (31 * i) % 253;
  }


  emp::aes_128_ctr(key, iv, input, output_bytes, 128, 77777);

  for (int i=0; i<32; i++) {
    vector<int> to_insert = toBinary(output_bytes[i]);
    ret.insert(ret.end(), to_insert.begin(), to_insert.end());
  }

  ZZ_p zzpret = bitVec_to_ZZ_p(ret);
  return zzpret;
}