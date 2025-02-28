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

/*
vector<int> strtoASCII(std::string str) {
  // input: a string
  // output: ascii-encoded vector to represent the input string
  vector<int> ret;
  int convert;
  
  for (unsigned int i = 0; i < str.size(); i++) {
    convert = str[i];
    ret.push_back(convert);
  }
  return ret;
}
*/



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
  for (int i=0; i<32; i++) {
    Integer digit = Integer(8, 0, ALICE);
    for (int j=0; j<ESP.size(); j++) {
      if (i < ESP[j].size()) {
        digit = digit + ESP[j][i];
      }    
    }
    ret.push_back(digit);
  }
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


vector<int> Simhash(vector<vector<int>> ESP) {
  vector<int> ret;
  for (int i=0; i<32; i++) {
    int digit = 0;
    for (int j=0; j<ESP.size(); j++) {
      if (i < ESP[j].size()) {
        digit += ESP[j][i];
      }    
    }
    ret.push_back(digit);
  }
  return ret;
}

