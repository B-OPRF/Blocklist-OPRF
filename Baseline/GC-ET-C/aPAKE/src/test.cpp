#include <emp-tool/emp-tool.h>
#include <emp-ot/ot.h>
#include "constants.h"
#include "Reed-Solomon.h"
#include "Simhash.h"
#include "ESP.h"
#include "ESP_noGarble.h"
#include "OLESender.h"
#include "OLEReceiver.h"
#include "PSISender.h"
#include "PSIReceiver.h"
#include <emp-sh2pc/emp-sh2pc.h>
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/ZZ_p.h"
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


int party, port;
NetIO * netio;

#define TEST_Circuit 0
#define TEST_Reg 1
#define TEST_Auth 0


void setup() {
	// usleep(100);
	netio =  new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port, true);
	emp::setup_semi_honest(netio, party, 1024);
}

void done() {
	delete netio;
	finalize_semi_honest();
}


vector<Bit> Simhash_gen_emp(std::string password) {

  vector<int> ascii = strtoASCII(password);
  
  vector<Integer> emp_array = int_to_emp(ascii);
  vector<vector<Integer>> ESP = ESP_Type(emp_array);

  vector<Integer> SIM_res = Simhash_emp(ESP);

  vector<Bit> ret;
  for (int i=0; i<128; i++) {
    if (i<SIM_res.size()) {
      ret.push_back(Bit(SIM_res[i].reveal<int>(PUBLIC), ALICE));
    }
    else {
      ret.push_back(Bit(0, ALICE));
    }
  }
  return ret;
}


vector<int> Simhash_gen(std::string password) {

  vector<int> ascii = strtoASCII(password);
  
  vector<vector<int>> ESP = ESP_Type(ascii);

  vector<int> SIM_res = Simhash(ESP);

  vector<int> ret;
  for (int i=0; i<128; i++) {
    if (i<SIM_res.size()) {
      ret.push_back(SIM_res[i]);
    }
    else {
      ret.push_back(0);
    }
  }
  return ret;
}


bool not_in(int val, vector<int> vec) {
  for (int i=0; i<vec.size(); i++) {
    if (val == vec[i])
      return false;
  }
  return true;
}


bool bitvecEqual(vector<Bit> A, vector<Bit> B) {
  if (A.size() != B.size()) {
    return false;
  }
  for (int i=0; i<A.size(); i++) {
    Bit res = A[i] != B[i];
    if (res.reveal<bool>()) {
      return false;
    }
  }
  return true;
}


vector<vector<Bit>> bitvec_to_emp_B(vector<vector<int>> vec) {
  vector<vector<Bit>> ret;
  for (int i=0; i<vec.size(); i++) {
    vector<Bit> temp;
    for (int j=0; j<vec[i].size(); j++) {
      temp.push_back(Bit(vec[i][j],BOB));
    }
    ret.push_back(temp);
  }
  return ret;
}


Bit set_diff(vector<Bit> A, vector<vector<int>> ESP_B) {
  vector<vector<Bit>> B = bitvec_to_emp_B(ESP_B);

  int counter = 0;
  vector<int> temp;
  int B_length = B.size();
  for (int j=0; j<B_length; j++) {
    if ((bitvecEqual(A,B[j])) && (not_in(j,temp))) {
      counter += 1;
      temp.push_back(j);
      break;
    }
  }
  if (counter > 0) {
    return Bit(0, BOB);
  }
  // cout << ret << "\n";
  return Bit(1, BOB);
}


vector<Integer> Perm(vector<Bit> SIM_ret) {
  Integer *to_aes = new Integer[SIM_ret.size()];
  for (int i=0; i<SIM_ret.size(); i++) {
    to_aes[i] = Integer(8, SIM_ret[i].reveal(PUBLIC), ALICE);
  }
  vector<Integer> AES_ret = AES(to_aes,SIM_ret.size());

  return AES_ret;
}


void recommit(vec_ZZ_p eval_pts, vec_ZZ_p *inSet, int setSize)
{
 
    auto start_time = high_resolution_clock::now();
    
    block b_r[eval_pts.length()];
    vec_ZZ_p commit_elem;
    ZZ_p committed_elem;
    commit_elem.SetLength(eval_pts.length());
    std::string commit_elem_str;
    for(int i = 0; i < eval_pts.length(); i++)
    {
        ZZ_p sum_of_random_elements = ZZ_p(0);
        uint64_t *blk = (uint64_t*) &b_r[i];
        uint64_t high = uint64_t(blk[1]);
        uint64_t low = uint64_t(blk[0]);
        ZZ comb_zz = get_zz_from_uint(high,low); //ZZ(comb);
        commit_elem[i] = conv<ZZ_p>(comb_zz) + sum_of_random_elements;
        NTL::ZZ zzInteger;
        conv(zzInteger, commit_elem[i]);
        while (zzInteger != 0) {
            commit_elem_str += to_string((zzInteger % 10));
            zzInteger /= 10;
        }
            
        if(DEBUG_COMMIT) cout << "R[C1] : " << i << " " << high << " " << low << " " << comb_zz <<  endl;
    }

    reverse(commit_elem_str.begin(),commit_elem_str.end());
    const char* commit_elem_arr = commit_elem_str.c_str();
    committed_elem = sha256(commit_elem_arr);

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    std::cout << "Storage Server Rebuild Commitment Time: " << duration.count() << "ms" << endl; 

}



int main(int argc, char** argv) 
{

  if (TEST_Circuit) {

  vector<vector<int>> SIM_list;

  std::ifstream pwfile("100Common.txt");
  std::string line;
  std::vector<std::string> temp;
  std::vector<std::string> common;
  if(!pwfile) // test the file open.
  {
    std::cout<<"Error opening output file"<< std::endl;
    system("pause");
    return -1;
  }
  while (std::getline(pwfile, line))
  { 
    temp.push_back(line);
  } 

  for (int i=0; i<10000; i++) {
    common.push_back(temp[i%100]);
  }
 
  for (int i=0; i<common.size(); i++) {
    vector<int> SIM_item = Simhash_gen(common[i]);
    SIM_list.push_back(SIM_item);
  }

  auto start_time = high_resolution_clock::now();
  setup_plain_prot(true,"EM_base.txt");

  vector<Bit> SIM_ret = Simhash_gen_emp("password");
  if (set_diff(SIM_ret, SIM_list).reveal<bool>(BOB)) {
    vector<Integer> AES_ret = Perm(SIM_ret);
  }

  finalize_plain_prot();
  auto end_time = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(end_time - start_time);
  std::cout << "Server circuit time: " << duration.count() << "ms" << endl; 

  }

  if (TEST_Reg) {

  auto start_time = high_resolution_clock::now();
  vector<vector<int>> SIM_list;

  std::ifstream pwfile("100Common.txt");
  std::string line;
  std::vector<std::string> temp;
  std::vector<std::string> common;
  if(!pwfile) // test the file open.
  {
    std::cout<<"Error opening output file"<< std::endl;
    system("pause");
    return -1;
  }
  while (std::getline(pwfile, line))
  { 
    temp.push_back(line);
  } 

  for (int i=0; i<10000; i++) {
    common.push_back(temp[i%100]);
  }
 
  for (int i=0; i<common.size(); i++) {
    vector<int> SIM_item = Simhash_gen(common[i]);
    SIM_list.push_back(SIM_item);
  }

  auto end_time = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(end_time - start_time);
  std::cout << "Server Offline Simhash time: " << duration.count() << "ms" << endl; 


  start_time = high_resolution_clock::now();
  parse_party_and_port(argv, &party, &port);
  setup();

  vector<Bit> SIM_ret = Simhash_gen_emp("password");
  Bit b = set_diff(SIM_ret, SIM_list);
  vector<Integer> AES_ret = Perm(SIM_ret);

  if(party == ALICE) 
  {
    std::cout << "Client OT Cost: " << netio->counter << "bytes" << endl;
  }

  if(party == BOB) 
  {
    std::cout << "Server OT Cost: " << netio->counter << "bytes" << endl;
  }

  done();
  end_time = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(end_time - start_time);
  if(party == ALICE) 
  {
    std::cout << "Client Registration time: " << duration.count() << "ms" << endl; 
  }
  if(party == BOB)
  {
    std::cout << "Server Registration time: " << duration.count() << "ms" << endl; 
  }


  }

  if (TEST_Auth) {

    // auto start_time = high_resolution_clock::now();
    // setup_plain_prot(true,"Auth_base.txt");

    // vector<Bit> SIM_ret= Simhash_gen_emp("password");
    // vector<Integer> AES_ret = Perm(SIM_ret);

    // finalize_plain_prot();
    // auto end_time = high_resolution_clock::now();
    // auto duration = duration_cast<milliseconds>(end_time - start_time);
    // std::cout << "Server circuit time: " << duration.count() << "ms" << endl;
    auto start_time = high_resolution_clock::now();
    parse_party_and_port(argv, &party, &port);

    setup();

    vector<Bit> SIM_ret = Simhash_gen_emp("password");
    vector<Integer> commitment = Perm(SIM_ret);

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
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    if(party == ALICE) 
    {
      std::cout << "Client Authentication time: " << duration.count() << "ms" << endl; 
    }
    if(party == BOB)
    {
      std::cout << "Server Authentication time: " << duration.count() << "ms" << endl; 
    }

  }

	return 0;

}