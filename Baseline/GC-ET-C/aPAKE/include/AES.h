#ifndef AES_H
#define AES_H
#include "NTL/ZZ.h"
#include "constants.h"
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include "emp-tool/emp-tool.h"


using namespace NTL;
using namespace std;
using namespace emp;


# define SHA256_DIGEST_LENGTH    32


bool not_in(int val, vector<int> vec) {
  for (int i=0; i<vec.size(); i++) {
    if (val == vec[i])
      return false;
  }
  return true;
}


Integer set_diff(vec_ZZ_p A, vec_ZZ_p B, int num_fragments) {

  vector<Integer> SIM_A, SIM_B;
  for (size_t i=0; i<num_fragments; i++) {
    SIM_A.push_back(Integer(32, conv<int>(A[i]), ALICE));
    SIM_B.push_back(Integer(32, conv<int>(B[i]), BOB));
  }

  int counter = 0;
  vector<int> temp;
  int A_length = SIM_A.size();
  int B_length = SIM_B.size();
  for (int i=0; i<A_length; i++) {
    for (int j=0; j<B_length; j++) {
      Bit res = A[i] == B[j];
      if ((res.reveal<bool>()) && (not_in(j,temp))) {
        counter += 1;
        temp.push_back(j);
        break;
      }
    }
  }
  Integer ret = Integer(32, A_length + B_length - 2*counter, PUBLIC);
  // cout << ret << "\n";
  return ret;
}


Integer AES(vector<vector<Bit>> ESP_input, __m128i key, __m128i iv) {
  int size = 0;
  for (int i=0; i<ESP_input.size(); i++) {
    for (int j=0; j<ESP_input[i].size(); j++) {
      size += 1;
    }
  }
  uint8_t input[size];
  uint8_t output_bytes[size];
  int pointer_input = 0;
  for (int i=0; i<ESP_input.size(); i++) {
    for (int j=0; j<ESP_input[i].size(); j++) {
      input[pointer_input] = ESP_input[i][j].reveal<>();
      pointer_input += 1;
    }
  }
  emp::AES_128_CTR_Calculator aes_128_ctr_calculator = emp::AES_128_CTR_Calculator();
  emp::Integer input_integer = emp::Integer(size * 8, input, emp::ALICE);
  emp::Integer output_integer = emp::Integer(size * 8, input, emp::PUBLIC);
  emp::Integer iv_integer = emp::Integer(128, &iv, emp::BOB);
  emp::Integer key_integer = emp::Integer(128, &key, emp::BOB);

  aes_128_ctr_calculator.aes_128_ctr(&(key_integer[0].bit),
                                     &(iv_integer[0].bit),
                                     &(input_integer[0].bit),
                                     &(output_integer[0].bit),
                                     size * 8,
                                     emp::PUBLIC,
                                     77777);

  output_integer.reveal<uint8_t>(output_bytes, PUBLIC);
  return output_integer;
}


Integer Commit(vec_ZZ_p A, vector<vec_ZZ_p> vec_B, int num_fragments, __m128i key, __m128i iv) {
  uint8_t *error;
  for (int i=0; i<vec_B.size(); i++) {

    int diff = set_diff(A, vec_B[i], num_fragments).reveal<int>(PUBLIC);

    if (diff < 0) {
      std::cerr << "File Content Blocked!\n" << std::flush;
      return error;
    }
  }

  vector<vector<Bit>> ret;
  for (int i = 0; i < num_fragments; i++)
  {

    uint64_t high = uint64_t(conv<ulong>((conv<ZZ>(A[i])>>64)));
    uint64_t low = uint64_t(conv<ulong>((conv<ZZ>(A[i]))));

    vector<Bit> result;
    uint64_t mask = 1;
    for (int i = 0; i < 64; ++i) {
        result.push_back(Bit((static_cast<uint64_t>(high) & mask) == 0 ? false : true, PUBLIC));
        mask <<= 1;
    }
    mask = 1;
    for (int i = 0; i < 64; ++i) {
        result.push_back(Bit((static_cast<uint64_t>(low) & mask) == 0 ? false : true, PUBLIC));
        mask <<= 1;
    }

    ret.push_back(result);

  }


  return AES(ret, key, iv);

}


Integer Recommit(ZZ_p A, __m128i key, __m128i iv) {

    vector<Bit> result;

    uint64_t high = uint64_t(conv<ulong>((conv<ZZ>(A)>>64)));
    uint64_t low = uint64_t(conv<ulong>((conv<ZZ>(A))));

    uint64_t mask = 1;
    for (int i = 0; i < 64; ++i) {
        result.push_back(Bit((static_cast<uint64_t>(high) & mask) == 0 ? false : true, PUBLIC));
        mask <<= 1;
    }
    mask = 1;
    for (int i = 0; i < 64; ++i) {
        result.push_back(Bit((static_cast<uint64_t>(low) & mask) == 0 ? false : true, PUBLIC));
        mask <<= 1;
    }

  uint8_t input[result.size()];
  uint8_t output_bytes[result.size()];
  int pointer_input = 0;
  for (int i=0; i<result.size(); i++) {
      input[pointer_input] = result[i].reveal<>();
      pointer_input += 1;
  }
  emp::AES_128_CTR_Calculator aes_128_ctr_calculator = emp::AES_128_CTR_Calculator();
  emp::Integer input_integer = emp::Integer(result.size() * 8, input, emp::ALICE);
  emp::Integer output_integer = emp::Integer(result.size() * 8, input, emp::PUBLIC);
  emp::Integer iv_integer = emp::Integer(128, &iv, emp::BOB);
  emp::Integer key_integer = emp::Integer(128, &key, emp::BOB);

  aes_128_ctr_calculator.aes_128_ctr(&(key_integer[0].bit),
                                     &(iv_integer[0].bit),
                                     &(input_integer[0].bit),
                                     &(output_integer[0].bit),
                                     result.size() * 8,
                                     emp::PUBLIC,
                                     77777);

  output_integer.reveal<uint8_t>(output_bytes, PUBLIC);
  // std::cout << "\nCommitment: ";
  // for (size_t i = 0; i < 32; ++i) {
    // std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)(output_bytes[size/2 + i]) << " ";
  // }
  return output_integer;
}


#endif