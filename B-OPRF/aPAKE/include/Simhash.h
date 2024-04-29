#include "emp-tool/emp-tool.h"
#include "utils.h"
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include<chrono>
#include "NTL/matrix.h"
#include "NTL/vec_vec_ZZ_p.h"
#include "NTL/BasicThreadPool.h"
#include <iostream>
#include <typeinfo>
#include <vector>
#include <new>
#include <time.h>
#include <fstream>
#include <smmintrin.h>
#include <bitset>
using namespace std;
using namespace emp;
using namespace NTL;

# define SHA256_DIGEST_LENGTH    32
# define bitsize    16
# define maxLen    32


// Simhash implemented following the blog post: https://ferd.ca/simhashing-hopefully-made-simple.html

vector<int> ulong_to_bin(ulong val) {
  vector<int> res;
  while (val / 2 != 0) {
    res.push_back(val % 2);
    val = val / 2;
  }
  res.push_back(val % 2);
  while (res.size() < 64) {
    res.push_back(0);
  }
  reverse(res.begin(), res.end());
  assert(res.size()==64);
  return res;
}


vector<Integer> Simhash_helper_emp(vec_ZZ_p file_fragments, int frag_length) {
  vector<Integer> ret;
  vector<vector<int>> res;
  for (int f=0; f<frag_length; f++) {
    res.push_back(ulong_to_bin(conv<ulong>(file_fragments[f])));
  }
  for (int i=0; i<64; i++) {
    int sum = 0;
    for (int j=0; j<res.size(); j++) {
      sum = sum + res[j][i];
    }
    ret.push_back(Integer(32, sum, ALICE));
  }
  return ret;
}


vector<Integer> Simhash_emp(vector<vector<Integer>> ESP) {
  vector<Integer> ret;
  for (int i=0; i<8; i++) {
    Integer digit = Integer(8, 0, ALICE);
    for (int j=0; j<ESP.size(); j++) {
      if (i < ESP[j].size()) {
        Bit b = (ESP[j][i] == Integer(8,0,ALICE));
        if (b.reveal(PUBLIC))
          digit = digit - Integer(8, 1, ALICE);
        else
          digit = digit + Integer(8, 1, ALICE);
      }    
    }
    ret.push_back(digit);
  }
  assert(ret.size()==8);
  return ret;
}


vector<int> Simhash_helper(vec_ZZ_p file_fragments, int frag_length) {
  vector<int> ret;
  vector<vector<int>> res;
  for (int f=0; f<frag_length; f++) {
    res.push_back(ulong_to_bin(conv<ulong>(file_fragments[f])));
  }
  for (int i=0; i<64; i++) {
    int sum = 0;
    for (int j=0; j<res.size(); j++) {
      sum = sum + res[j][i];
    }
    ret.push_back(sum);
  }
  return ret;
}


ZZ_p Simhash(vector<int> vec) {
  vector<int> ret;
  for (int i=0; i<vec.size(); i++) {
    int res = vec[i] >= 0;
    //cout << res.reveal<bool>();
    ret.push_back(res);
  }
  ZZ_p zzpret = bitVec_to_ZZ_p(ret);
  return zzpret;
}
