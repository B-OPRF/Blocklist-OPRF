#include "emp-tool/emp-tool.h"
#include "emp-sh2pc/emp-sh2pc.h"
#include <iostream>
#include <typeinfo>
#include <vector>
#include <new>
#include <time.h>
#include <fstream>
#include <smmintrin.h>
#include <bitset>
#include <chrono>
using namespace std;
using namespace emp;

#define TEST_Circuit 0
#define TEST_Reg 1

int party, port;
NetIO * netio;

void setup() {
	// usleep(100);
	netio =  new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port, true);
	emp::setup_semi_honest(netio, party, 1024);
}

void done() {
	delete netio;
	finalize_semi_honest();
}


vector<Integer> int_to_emp_A(vector<int> vec) {
  vector<Integer> ret;
  for (int i = 0; i < vec.size(); i++)
  {
    ret.push_back(emp::Integer(8, vec[i], ALICE));
  }
  return ret;
}

vector<Integer> int_to_emp_B(vector<int> vec) {
  vector<Integer> ret;
  for (int i = 0; i < vec.size(); i++)
  {
    ret.push_back(emp::Integer(8, vec[i], BOB));
  }
  return ret;
}


vector<int> strtoASCII(std::string str) {
  vector<int> ret;
  int convert;
  
  for (unsigned int i = 0; i < str.size(); i++) {
    convert = str[i];
    ret.push_back(convert);
  }
  return ret;
}


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
  // assert(ret.size()==8);
  return ret;
}


Bit equality_check(vector<Integer> A, vector<vector<Integer>> B) {
  int B_length = B.size();
  for (int j=0; j<B_length; j++) {
    if (A.size() != B[j].size()) {
      break;
    }
    for (int i=0; i<A.size(); i++) {
      Bit res = A[i] != B[j][i];
      if (res.reveal<bool>()) {
        break;
      }
    }
  }
  return Bit(1, BOB);
}


vector<Integer> AES(Integer *input) {
  vector<Integer> ret;
  __m128i key;
  __m128i iv;
  for (size_t i = 0; i < 16; ++i) {
    ((uint8_t *)(&key))[i] = (1337 * i) % 255;
    ((uint8_t *)(&iv))[i] = (31 * i) % 253;
  }

  uint8_t output_bytes[128];

  emp::AES_128_CTR_Calculator aes_128_ctr_calculator = emp::AES_128_CTR_Calculator();
  emp::Integer input_integer = emp::Integer(128, input, emp::PUBLIC);
  emp::Integer output_integer = emp::Integer(128, input, emp::PUBLIC);
  emp::Integer iv_integer = emp::Integer(128, &iv, emp::PUBLIC);
  emp::Integer key_integer = emp::Integer(128, &key, emp::PUBLIC);

  aes_128_ctr_calculator.aes_128_ctr(&(key_integer[0].bit),
                                     &(iv_integer[0].bit),
                                     &(input_integer[0].bit),
                                     &(output_integer[0].bit),
                                     128,
                                     emp::PUBLIC,
                                     77777);

  output_integer.reveal<uint8_t>(output_bytes, PUBLIC);
  for (int i=0; i<32; i++) {
    vector<Integer> to_insert = toBinary(output_bytes[i]);
    ret.insert(ret.end(), to_insert.begin(), to_insert.end());
  }
  return ret;
}


vector<Integer> Commit(vector<Integer> c_input) {
  Integer *to_aes = new Integer[c_input.size()];
  for (int i=0; i<c_input.size(); i++) {
    to_aes[i] = Integer(8, c_input[i].reveal<int>(PUBLIC), ALICE);
  }
  vector<Integer> ret = AES(to_aes);
  return ret;
}
  

int main(int argc, char** argv) {

  std::ifstream pwfile("./emp-tool/circuits/files/edit_distance_2_results.txt");
  std::string line;
  std::vector<std::string> common;
  if(!pwfile) // test the file open.
  {
    std::cout<<"Error opening output file"<< std::endl;
    system("pause");
    return -1;
  }
  while (std::getline(pwfile, line))
  { 
    common.push_back(line);
  } 


  vector<vector<int>> S_input;

  for (int i=0; i<common.size(); i++) {
    vector<int> ascii = strtoASCII(common[i]);
    S_input.push_back(ascii);
  }

  vector<int> ascii_C = strtoASCII("p1a2s3s4w5o6r7d");

  if (TEST_Circuit) {
    auto start_time = high_resolution_clock::now();
    setup_plain_prot(true,"Base_Ublk.txt");

    vector<Integer> C_input = int_to_emp_A(ascii_C);
  
    vector<vector<Integer>> blocklist;
    for (int i=0; i<S_input.size(); i++) {
      blocklist.push_back(int_to_emp_B(S_input[i]));
    }

    Bit b = equality_check(C_input, blocklist);

    if (b.reveal<bool>()) {
      vector<Integer> commitment = Commit(C_input);
    }
  
	  finalize_plain_prot();
    auto end_time = high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Server circuit time: " << duration.count() << "ms" << endl;
  }

  if (TEST_Reg) {
    auto start_time = high_resolution_clock::now();
    parse_party_and_port(argv, &party, &port);

    setup();

    vector<Integer> C_input = int_to_emp_A(ascii_C);
  
    vector<vector<Integer>> blocklist;
    for (int i=0; i<S_input.size(); i++) {
      blocklist.push_back(int_to_emp_B(S_input[i]));
    }

    Bit b = equality_check(C_input, blocklist);

    if (b.reveal<bool>()) {
      vector<Integer> commitment = Commit(C_input);
    }
  
	  if(party == ALICE) 
    {
      std::cout << "Client OT Cost: " << netio->counter << "bytes" << endl;
    }

    if(party == BOB) 
    {
      std::cout << "Server OT Cost: " << netio->counter << "bytes" << endl;
    }

    done();
    auto end_time = high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    if(party == ALICE) 
    {
      std::cout << "Client Registration time: " << duration.count() << "ms" << endl; 
    }
    if(party == BOB)
    {
      std::cout << "Server Registration time: " << duration.count() << "ms" << endl; 
    }
  }

   
  
	return 0;
}
