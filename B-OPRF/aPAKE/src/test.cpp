#include <emp-tool/emp-tool.h>
#include <emp-ot/ot.h>
#include "constants.h"
#include "Reed-Solomon.h"
#include "Simhash.h"
#include "ESP.h"
#include "ESP_noGarble.h"
#include "interpolate.h"
#include "OLESender.h"
#include "OLEReceiver.h"
#include "PSISender.h"
#include "PSIReceiver.h"
#include "Authentication.h"
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
  for (int i=0; i<8; i++) {
    ret.push_back(Bit(SIM_res[i].reveal<int>(PUBLIC), ALICE));
  }
  return ret;
}


vec_ZZ_p Simhash_gen(std::string password) {

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

  vector<int> ascii = strtoASCII(password);
  vector<vector<int>> ESP = ESP_Type(ascii);

  vector<int> ESP_res;

  for (int i=0; i<32; i++) {
    int digit = 0;
    for (int j=0; j<ESP.size(); j++) {
      if (i < ESP[j].size()) {
        digit = digit + ESP[j][i];
      }    
    }
    ESP_res.push_back(digit);
  }

  ZZ_p SIM_res = Simhash(ESP_res);

  vec_ZZ_p SIM_ret;
  for (int i=0; i<RSMaxLength; i++) {
    SIM_ret.append(SIM_res);
  }

  vec_ZZ_p ret = encode_rs(SIM_ret, RSMaxLength, Alpha, Beta);

  return ret;
}


vector<vector<Bit>> Mask(vector<Bit> SIM_ret) {
  vector<vector<Bit>> ret;
  PRG prg;
  bool rand_block[8];
  for (int i=0; i<maxLen; i++) {
    prg.random_bool(rand_block, 8);
    vector<Bit> temp;
    for (int j=0; j<8; j++) {
      temp.push_back(SIM_ret[j]&Bit(rand_block[j],ALICE));
    }
    ret.push_back(temp);
  }
  return ret;
}


vector<Integer> Perm(vector<Bit> SIM_ret) {
  // vector<vector<Integer>> SIM_ret_bin;
  // for (int i=0; i<SIM_ret.size(); i++) {
  //   SIM_ret_bin.push_back(toBinary_emp(SIM_ret[i]));
  // }
  vector<vector<Bit>> S_w = Mask(SIM_ret);
  vector<vector<Bit>> embedding = f(S_w);

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

  /*
  Embed-and-Map circuit size: 2.6 MB
  Client EM OT cost: 1.83 MB
  Server EM OT cost: 0.34 MB

  Client EmbedMap time: 1374 ms
  Server circuit time: 358 ms
  Server EmbedMap time: 1373 ms

  */

  if (TEST_Circuit) {

  auto start_time = high_resolution_clock::now();
  setup_plain_prot(true,"EM.txt");

  vector<Bit> SIM_ret = Simhash_gen_emp("password");
  vector<Integer> AES_ret = Perm(SIM_ret);

  finalize_plain_prot();
  auto end_time = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(end_time - start_time);
  std::cout << "Server circuit time: " << duration.count() << "ms" << endl; 

  }

  if (TEST_Reg) {

  auto start_time = high_resolution_clock::now();
  parse_party_and_port(argv, &party, &port);
  setup();

  vector<Bit> SIM_ret = Simhash_gen_emp("password");
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
  auto end_time = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(end_time - start_time);
  if(party == ALICE)
  {
    std::cout << "Client EmbedMap time: " << duration.count() << "ms" << endl; 
  }
  if(party == BOB)
  {
    std::cout << "Server EmbedMap time: " << duration.count() << "ms" << endl; 
  }


  // start_time = high_resolution_clock::now();
  vector<vec_ZZ_p> SIM_list;

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

  int blocklist_size = 5000;

  for (int i=0; i<blocklist_size; i++) {
    common.push_back(temp[i%100]);
  }
 
  for (int i=0; i<common.size(); i++) {
    vec_ZZ_p SIM_item = Simhash_gen(common[i]);
    SIM_list.push_back(SIM_item);
  }
  
  // end_time = high_resolution_clock::now();
  // duration = duration_cast<milliseconds>(end_time - start_time);
  // if(party == ALICE)      // Server
  // {
  // std::cout << "Server Offline Simhash time: " << duration.count() << "ms" << endl;
  // } 


    ZZ_p::init(prime);
    //ZZ_p::init(ZZ(pow(2,57) - 1));
    ZZ_pContext context;
    context.save();

    vec_ZZ_p P; //= random_vec_ZZ_p(n+secParam);
    P.SetLength(n + secParam);
    for(int i = 0; i < n + secParam; i++)
    {
        P[i] = conv<ZZ_p>(ZZ(67*(2*i + 1)));
    }


    int setSize = blocklist_size;
    int ESPVecLen = 32;

    vec_ZZ_p eval_points;
    eval_points.SetLength(2*ESPVecLen+1);
    for(int i = 0; i < eval_points.length(); i++)
    {
        eval_points[i] = conv<ZZ_p>(ZZ(57*(2*i + 1)));
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
    

    vec_ZZ_p servers_input;
    vec_ZZ_p clients_input_mul;
    vec_ZZ_p clients_input_add;
    int inputLength = 4*setSize*ESPVecLen;       // Number of OLEs we will require
    servers_input.SetLength(inputLength);
    clients_input_mul.SetLength(inputLength);
    clients_input_add.SetLength(inputLength);


    for(int i = 0; i < inputLength; i++)
    {
        servers_input[i] = 1; //random_ZZ_p();            
        clients_input_mul[i] = i;
        clients_input_add[i] = i;
    }



    parse_party_and_port(argv, &party, &port);
    //NetIO* ios[2];
    //for(int i = 0; i < 2; ++i) {ios[i] = new NetIO(party == ALICE?nullptr:"127.0.0.1",port+i);}         // We need two channels, one for communicating in the plain, the other for the OT
    
    int num_of_oles = inputLength;          // a number that is a multiple of 50
    int num_of_ole_instances;
    if(num_of_oles <= RSMaxLength)
        num_of_ole_instances = 1;
    else
        num_of_ole_instances = num_of_oles/RSMaxLength + 1;



    ZZ_pX embed_map_key_poly_one; //= random_ZZ_pX(ESPVecLen);
    ZZ_pX embed_map_key_poly_two;  //= random_ZZ_pX(2*ESPVecLen);
    vec_ZZ_p embed_map_key_poly_one_pts; 
    embed_map_key_poly_one_pts.SetLength(ESPVecLen + 1);
    vec_ZZ_p embed_map_key_poly_two_pts;
    embed_map_key_poly_two_pts.SetLength(2*ESPVecLen + 1);
    vec_ZZ_p set_of_X_for_key_poly_one;
    set_of_X_for_key_poly_one.SetLength(ESPVecLen + 1);
    vec_ZZ_p set_of_X_for_key_poly_two;
    set_of_X_for_key_poly_two.SetLength(2*ESPVecLen + 1);


    for(int i = 0; i < ESPVecLen + 1; i++)
    {
        embed_map_key_poly_one_pts[i] = 103*i + 5;
        set_of_X_for_key_poly_one[i] = eval_points[i];
    }
    embed_map_key_poly_one = interpolate(set_of_X_for_key_poly_one, embed_map_key_poly_one_pts);


        for(int i = 0; i < 2*ESPVecLen + 1; i++)
    {
        embed_map_key_poly_two_pts[i] = 83*i + 3;
        set_of_X_for_key_poly_two[i] = eval_points[i];
    }
    embed_map_key_poly_two = interpolate(set_of_X_for_key_poly_two, embed_map_key_poly_two_pts);


    if(party == ALICE)      // Receiver (Server)
    {
        vec_ZZ_p inSet[setSize];
        for(int i = 0; i < setSize; i++)
        {
            inSet[i].SetLength(ESPVecLen);
            for(int j = 0; j < ESPVecLen; j++)
            {
                inSet[i][j] = j+1; //random_ZZ_p();
            }
        }
      
        PSIReceiver psi_receiver = PSIReceiver(port, setSize, context, prime, ESPVecLen, Alpha, Beta);    
        //psi_receiver.get_intersection(eval_points, inSet);
        psi_receiver.test_and_commit(eval_points, inSet, embed_map_key_poly_one, embed_map_key_poly_two);

    }

    else                // Sender (Cient)
    {   
        vec_ZZ_p inSet[setSize];
        for(int i = 0; i < setSize; i++)
        {
            inSet[i].SetLength(ESPVecLen);
            for(int j = 0; j < ESPVecLen; j++)
            {
                inSet[i][j] = 2*j + 1; //random_ZZ_p();
            }
        }
        
        PSISender psi_sender = PSISender(port, setSize, context, prime, ESPVecLen, Alpha, Beta);    
        //psi_sender.get_intersection(eval_points, inSet);
        psi_sender.test_and_commit(eval_points, inSet, embed_map_key_poly_one, embed_map_key_poly_two);

    }

    }

    if (TEST_Auth) {

    ZZ_p::init(prime);
    //ZZ_p::init(ZZ(pow(2,57) - 1));
    ZZ_pContext context;
    context.save();

    auto start_time = high_resolution_clock::now();

    vector<int> ascii = strtoASCII("password");
    vector<vector<int>> ESP = ESP_Type(ascii);

    vector<int> ESP_res;

    for (int i=0; i<64; i++) {
      int digit = 0;
      for (int j=0; j<ESP.size(); j++) {
        if (i < ESP[j].size()) {
          if (ESP[j][i] == 0)
            digit = digit -1;
          else
            digit = digit + 1;
        }    
      }
      ESP_res.push_back(digit);
    }

    ZZ_p SIM_ret = Simhash(ESP_res);
    vec_ZZ_p y_C_vec = secret_share(SIM_ret, maxLen);
  
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    std::cout << "Client Authentication Offline time: " << duration.count() << "ms" << endl; 

    start_time = high_resolution_clock::now();

    vec_ZZ_p r = random_vec_ZZ_p(maxLen);
    vector<int> SIM;
    for (int i=0; i<ESP_res.size(); i++) {
      int res = ESP_res[i] >= 0;
      SIM.push_back(res);
    }
    setup_plain_prot(false, "");

	  vec_ZZ_p ss = secret_share(AES(SIM), maxLen);

	  
    vec_ZZ_p y_S_vec = secret_share(SIM_ret, maxLen);
    vec_ZZ_p sub;
    for (int i=0; i<64; i++) {
      sub.append(ss[i]-r[i]*y_S_vec[i]);
    }

    end_time = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end_time - start_time);
    std::cout << "Server Authentication Offline time: " << duration.count() << "ms" << endl;
    finalize_plain_prot();
    vec_ZZ_p P;
    P.SetLength(maxLen);
    for(int i = 0; i < n + secParam; i++)
    {
        P[i] = conv<ZZ_p>(ZZ(67*(2*i + 1)));
    }

    vec_ZZ_p Alpha;
    Alpha.SetLength(k); 
    int i = 0;
    for(i; i < k; i++)
    {
        Alpha[i] = P[i];
    } 

    vec_ZZ_p Beta;
    Beta.SetLength(n);
    int j = 0;
    for(i,j; j < n; i++, j++)
    {
        Beta[j] = P[i];
    } 

    int num_of_oles = 4*32;   
    int num_of_ole_instances;
    if(num_of_oles <= RSMaxLength)
        num_of_ole_instances = 1;
    else
        num_of_ole_instances = num_of_oles/RSMaxLength + 1;

    parse_party_and_port(argv, &party, &port);

    NetIO* io;
    io = new NetIO(party == ALICE?nullptr:"127.0.0.1",port+2); 
    if(party == ALICE)      // Client
    {
      OLEReceiver ole_receiver(port, context, prime, num_of_ole_instances);
      vec_ZZ_p ss_C = ole_receiver.compute_ole(y_C_vec, num_of_oles, Alpha, Beta);
    }

    else                // Server
    {   
      OLESender ole_sender(port, context, prime, num_of_ole_instances);
      ole_sender.compute_ole(r, sub, num_of_oles, Alpha, Beta);
    }


    }

	return 0;

}