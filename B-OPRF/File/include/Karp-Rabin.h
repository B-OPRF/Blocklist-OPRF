#ifndef KRFingerprint_H
#define KRFingerprint_H
#include "NTL/ZZ.h"
#include "constants.h"
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include "emp-tool/emp-tool.h"


using namespace NTL;
using namespace std;
using namespace emp;


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

/*
vector<Integer> toBinary(int n)
{
  vector<Integer> ret;
  while (n / 2 != 0) {
    ret.push_back(Integer(8, n % 2, ALICE));
    n = n / 2;
  }
  ret.push_back(Integer(8, n % 2, ALICE));
  while (ret.size() < 8) {
    ret.push_back(Integer(8, 0, ALICE));
  }
  reverse(ret.begin(), ret.end());
  assert(ret.size()==8);
  return ret;
}
*/
vector<int> toBinary(int n)
{
  vector<int> ret;
  while (n / 2 != 0) {
    ret.push_back(n % 2);
    n = n / 2;
  }
  ret.push_back(n % 2);
  while (ret.size() < 8) {
    ret.push_back(0);
  }
  reverse(ret.begin(), ret.end());
  assert(ret.size()==8);
  return ret;
}

vector<vector<Integer>> bitvec_to_emp_A(vector<vector<int>> vec) {
  // input: a vector of bit vectors
  // output: turn the input vector into Alice's Garbled circuit input
  vector<vector<Integer>> ret;
  for (int i=0; i<vec.size(); i++) {
    vector<Integer> temp;
    for (int j=0; j<vec[i].size(); j++) {
      temp.push_back(Integer(8,vec[i][j],ALICE));
    }
    ret.push_back(temp);
  }
  return ret;
}

/*
vector<vector<Integer>> file_to_bitvec(std::string str) {
    vector<vector<Integer>> ret;
    vector<int> ascii = strtoASCII(str);
    for (size_t i=0; i < ascii.size(); i++) {
        ret.push_back(toBinary(ascii[i]));
    }
    return ret;
}
*/



/*
vector<vector<Integer>> to_bitvec_calc(vector<vector<Bit>> bitvec) {

    vector<vector<Integer>> ret;

    for (size_t i=0; i < bitvec.size(); i++) {
        vector<Integer> temp;
        for (size_t j=0; j < bitvec[i].size; j++) {
            int res = bitvec[i][j].reveal<int>();
            temp.push_back(Integer(8, res, ALICE));
        }
        ret.push_back(temp);
    }

    return ret;
}
*/


// Karp-Rabin Fingerprinting Algorithm
vector<vector<Bit>> fingerprint_kp(vector<vector<Integer>> input) {
    
    vector<vector<Bit>> ret;
 
    for (size_t i=0; i < input.size(); i++) {
        ZZ hash = conv<ZZ>(0);

        for (size_t j=0; j < input[i].size(); j++) {
            int c = input[i][j].reveal<int>(PUBLIC);
            hash = (hash * conv<ZZ>(alphabet_size) + conv<ZZ>(c)) % prime;
        }

           
        uint64_t high = uint64_t(conv<ulong>((hash>>64)));
        uint64_t low = uint64_t(conv<ulong>(hash));

        vector<Bit> result;
        uint64_t mask = 1;
        for (int i = 0; i < 64; ++i) {
            result.push_back(Bit((static_cast<uint64_t>(high) & mask) == 0 ? false : true, ALICE));
            mask <<= 1;
        }
        mask = 1;
        for (int i = 0; i < 64; ++i) {
            result.push_back(Bit((static_cast<uint64_t>(low) & mask) == 0 ? false : true, ALICE));
            mask <<= 1;
        }

        /*
        block final = makeBlock(high,low);
        int* data = (int*)&final;
        vector<Bit> result;

        for (size_t d=0; d<2; d++) {
            for (size_t b=63; b>=0; b--) {
                result.push_back(Bit((data[d] >> b) & 1, ALICE));
            }
        }
        */
        ret.push_back(result);
    
        
        
    }
    
    return ret;
}


/*
int main()
{
    char txt[] = "GEEKS FOR GEEKS";
    char pat[] = "GEEK";
 
    // we mod to avoid overflowing of value but we should
    // take as big q as possible to avoid the collison
    int q = INT_MAX;
 
    // Function Call
    search(pat, txt, q);
    return 0;
}


#include <emp-tool/emp-tool.h>
#include <emp-ot/ot.h>
#include "../include/constants.h"
#include "../include/Karp-Rabin.h"
#include <emp-sh2pc/emp-sh2pc.h>
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include<chrono>
#include "NTL/matrix.h"
#include "NTL/vec_vec_ZZ_p.h"
#include "NTL/BasicThreadPool.h"
#include<vector>
#include<numeric>
#include<algorithm>
#include <thread>
#include<future>
#include<iostream>
#include <cstdlib>
#include<string>
#include <fstream>
#if defined(__linux__)
        #include <sys/time.h>
        #include <sys/resource.h>
#elif defined(__APPLE__)
        #include <unistd.h>
        #include <sys/resource.h>
        #include <mach/mach.h>
#endif

using namespace std;
using namespace emp;
using namespace chrono;
using namespace NTL;


int secParam = 100;
int n = 4*secParam;
int p = 2*secParam + 1;
int l = n - p;
int inputLength = l/4;
int k = (l-1)/2 + 1;
typedef unsigned long ulong;



int main(int argc, char** argv) 
{
    // 4KB
    // Simhash
    // circuit generation time: 4.71 s
    // circuit size: 72.3 MB
    // OT cost: 15 MB, 0.26 MB
    // running time: 48 ms

    // Karp - Rabin
    // circuit generation time: 865 ms
    // circuit size: 7.2 MB
    // OT cost: 0.26 MB, 0.52 MB
    // running time: 410 ms

    // Karp - Rabin (without GC)
    // running time: 26 ms

      clock_t t;

      //std::ifstream lfile("./emp-tool/circuits/sim_files/"+std::to_string(i)+".txt");
      std::ifstream lfile("../sim_files/1.txt");
      std::string line;
      std::string temp;
      std::vector<std::string> ret;
      if(!lfile) // test the file open.
      {
        std::cout<<"Error opening output file"<< std::endl;
        system("pause");
        return -1;
      }
    
      while (std::getline(lfile, line))
      { 
        if (temp.length() + line.length() < 4000) {
            temp.append(line);
        }
        else {
          temp.append(line.substr(0,4000-temp.length()));
          ret.push_back(temp);
          temp.clear();
        }
      } 

      while (std::getline(lfile, line))
      { 
        if (temp.length() + line.length() <= 4000) {
            temp.append(line);
        }
        else {
          break;
        }
      } 



      vector<int> ascii_array = strtoASCII(temp); 

      cout << "ascii generated";

      vector<vector<int>> Fragments;
      for (int j=0; j < ascii_array.size(); j++) {
        vector<int> ascii_bit = toBinary(ascii_array[j]);
        
        for (int h=0; h<ascii_bit.size(); h++) {
          cout << ascii_bit[h];
        }
        
        Fragments.push_back(ascii_bit);
      }
      t = clock();

      vector<vector<int>> res;
      
      for (size_t i=0; i < Fragments.size(); i++) {
        ZZ hash = conv<ZZ>(0);

        for (size_t j=0; j < Fragments[i].size(); j++) {
            int c = Fragments[i][j];
            hash = (hash * conv<ZZ>(alphabet_size) + conv<ZZ>(c)) % prime;
        }

           
        uint64_t high = uint64_t(conv<ulong>((hash>>64)));
        uint64_t low = uint64_t(conv<ulong>(hash));

        vector<int> result;
        uint64_t mask = 1;
        for (int i = 0; i < 64; ++i) {
            result.push_back((static_cast<uint64_t>(high) & mask) == 0 ? false : true);
            mask <<= 1;
        }
        mask = 1;
        for (int i = 0; i < 64; ++i) {
            result.push_back((static_cast<uint64_t>(low) & mask) == 0 ? false : true);
            mask <<= 1;
        }

        res.push_back(result);
      }
      
      
      //int port, party;

	  //parse_party_and_port(argv, &party, &port);
	  //NetIO* io = new NetIO(party==ALICE?nullptr:"127.0.0.1", port);

	  //setup_semi_honest(io, party);

    
      setup_plain_prot(true, "karp-rabin.txt");

      vector<vector<Integer>> Fragments_emp = bitvec_to_emp_A(Fragments);

      vector<vector<Bit>> Fingerprints = fingerprint_kp(Fragments_emp);

      finalize_plain_prot();  
      //finalize_semi_honest();
      //cout << party << ":";
      //cout << io->counter << endl;
	  //delete io;
      
      

      t = clock() - t;
  //}
  
    cout << ((float)t)/(CLOCKS_PER_SEC);

	return 0;

}
*/

#endif