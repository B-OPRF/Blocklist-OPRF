#include <emp-tool/emp-tool.h>
#include <emp-ot/ot.h>
#include "../include/constants.h"
#include "../include/Reed-Solomon.h"
#include "../include/Simhash.h"
#include "../include/AES.h"
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


vec_ZZ_p Simhash_gen(std::vector<std::string> file_block) {
  int num_encoding_instances = 10;

  ZZ_p::init(prime);
  ZZ_pContext context;
  context.save();

  vec_ZZ_p P; //= random_vec_ZZ_p(n+secParam);
  P.SetLength(n + secParam);
  for(int i = 0; i < n + secParam; i++)
  {
    P[i] = conv<ZZ_p>(ZZ(67*(2*i + 1)));
  }

  // Defining the set of alphas
  vec_ZZ_p Alpha;
  Alpha.SetLength(k);
  int i = 0;
  for(i; i < k; i++)
  {
    Alpha[i] = P[i];
  } 

  // Defining the set of betas
  vec_ZZ_p Beta;
  Beta.SetLength(n);
  int j = 0;
  for(i,j; j < n; i++, j++)
  {
    Beta[j] = P[i];
  }

  int num_fragments = file_block.size()*num_encoding_instances;
  vec_ZZ_p erasure_coded_fragments[num_fragments];
  for (size_t b = 0; b < file_block.size(); b++) {
    vector<int> ascii_array = strtoASCII(file_block[b]);

    vec_ZZ_p X[num_encoding_instances];
    for(int i = 0, k = 0; i < num_encoding_instances; i++)
    {
      X[i].SetLength(RSMaxLength);
      for(int j = 0; j < RSMaxLength; j++)
      {
        if(k >= RSMaxLength)
          X[i][j] = 1;
        else
          X[i][j] = ascii_array[k++];
      }
      erasure_coded_fragments[b*num_encoding_instances+i] = encode_rs(X[i], RSMaxLength, Alpha, Beta);
    }
  }

  vec_ZZ_p SIM_ret;

  for (int j=0; j<num_fragments; j++) {
      ZZ_p SIM_frag = Simhash(Simhash_helper(erasure_coded_fragments[j], num_fragments));
      SIM_ret.append(SIM_frag);
  }

  return SIM_ret;
}


int main(int argc, char** argv) 
{


  auto start_time = high_resolution_clock::now();

  //std::ifstream lfile("./emp-tool/circuits/sim_files/"+std::to_string(i)+".txt");
  std::ifstream lfile("../sim_files/1.txt");
  std::string line;
  std::string temp;
  std::vector<std::string> file_block;
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
      file_block.push_back(temp);
      temp.clear();
    }
  } 

  vec_ZZ_p SIM_ret = Simhash_gen(file_block);

  int num_encoding_instances = 10;
  int num_fragments = file_block.size()*num_encoding_instances;
  
  auto end_time = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(end_time - start_time);
  std::cout << "Client offline time: " << duration.count() << "ms" << endl; 


  start_time = high_resolution_clock::now();
  vector<vec_ZZ_p> SIM_list;

  for (size_t f=0; f<10; f++) {
    std::ifstream lfile("../sim_files/"+std::to_string(f)+".txt");
    std::string line;
    std::string temp;
    std::vector<std::string> file_block;
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
        file_block.push_back(temp);
        temp.clear();
      }
    } 

    vec_ZZ_p SIM_item = Simhash_gen(file_block);
    
    SIM_list.push_back(SIM_item);
  }

  end_time = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(end_time - start_time);
  std::cout << "Policy Checker offline time: " << duration.count() << "ms" << endl; 

  
  start_time = high_resolution_clock::now();

  
  setup_plain_prot(true, "Registration.txt");

  __m128i key;
  __m128i iv;
  for (size_t i = 0; i < 16; ++i) {
    ((uint8_t *)(&key))[i] = (1337 * i) % 255;
    ((uint8_t *)(&iv))[i] = (31 * i) % 253;
  }

  Integer commitment = Commit(SIM_ret, SIM_list, num_fragments, key, iv);

  int commitment_out = commitment.reveal<int>(PUBLIC);
  std::cout << "Commitment Produced: " << commitment_out << endl; 
    
  finalize_plain_prot();

  end_time = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(end_time - start_time);
  std::cout << "Policy Checker Circuit Generation time: " << duration.count() << "ms" << endl; 
  
  /*
  start_time = high_resolution_clock::now();
  
  parse_party_and_port(argv, &party, &port);

  netio =  new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port, true);
	emp::setup_semi_honest(netio, party, 1024);

  __m128i key;
  __m128i iv;
  for (size_t i = 0; i < 16; ++i) {
    ((uint8_t *)(&key))[i] = (1337 * i) % 255;
    ((uint8_t *)(&iv))[i] = (31 * i) % 253;
  }

  Integer commitment = Commit(SIM_ret, SIM_list, num_fragments, key, iv);

  int commitment_out = commitment.reveal<int>(PUBLIC);
  //std::cout << "Commitment Produced: " << commitment_out << endl; 

  end_time = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(end_time - start_time);
  
  if (party == 1) {
    std::cout << "Client running time: " << duration.count() << "ms" << endl; 
    std::cout << "Client OT cost: " << netio->counter << " bytes" << endl;
  }
  else {
    std::cout << "Policy Checker running time: " << duration.count() << "ms" << endl; 
    std::cout << "Policy Checker OT cost: " << netio->counter << " bytes" << endl;
  }
    
  

  delete netio;
	finalize_semi_honest();
  */
  /*

  start_time = high_resolution_clock::now();

  
  setup_plain_prot(true, "Login.txt");

  __m128i key;
  __m128i iv;
  for (size_t i = 0; i < 16; ++i) {
    ((uint8_t *)(&key))[i] = (1337 * i) % 255;
    ((uint8_t *)(&iv))[i] = (31 * i) % 253;
  }
  
  Integer commitment = Integer(32, 0, BOB);
  Integer login = Recommit(SIM_ret[0], key, iv);

  int commitment_out = commitment.reveal<int>(PUBLIC);
  int login_out = login.reveal<int>(PUBLIC);
  //std::cout << "Commitment Produced: " << commitment_out << endl; 
  
  if (commitment_out == login_out) {
    cout << "login successful";
  }
  else {
    cout << "login successful" << endl;
  }
    
  finalize_plain_prot();
 
  end_time = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(end_time - start_time);
  std::cout << "Policy Checker Circuit Generation time: " << duration.count() << "ms" << endl; 


  start_time = high_resolution_clock::now();
  
  parse_party_and_port(argv, &party, &port);

  netio =  new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port, true);
	emp::setup_semi_honest(netio, party, 1024);

  __m128i key;
  __m128i iv;
  for (size_t i = 0; i < 16; ++i) {
    ((uint8_t *)(&key))[i] = (1337 * i) % 255;
    ((uint8_t *)(&iv))[i] = (31 * i) % 253;
  }
  
  Integer commitment = Integer(32, 0, BOB);
  
  Integer login = Recommit(SIM_ret[0], key, iv);

  int commitment_out = commitment.reveal<int>(PUBLIC);
  int login_out = login.reveal<int>(PUBLIC);
  //std::cout << "Commitment Produced: " << commitment_out << endl; 
  
  if (commitment_out == login_out) {
    cout << "login successful";
  }
  else {
    cout << "login successful" << endl;
  }

  end_time = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(end_time - start_time);
  
  if (party == 1) {
    std::cout << "Storage Server running time: " << duration.count() << "ms" << endl; 
    std::cout << "Storage Server OT cost: " << netio->counter << " bytes" << endl;
  }
  else {
    std::cout << "Policy Checker running time: " << duration.count() << "ms" << endl; 
    std::cout << "Policy Checker OT cost: " << netio->counter << " bytes" << endl;
  }
    
  delete netio;
	finalize_semi_honest();
  */
	return 0;

}