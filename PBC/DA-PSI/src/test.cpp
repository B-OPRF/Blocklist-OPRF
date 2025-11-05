#include <emp-tool/emp-tool.h>
#include <emp-ot/ot.h>
#include "../include/OLESender.h"
#include "../include/OLEReceiver.h"
#include <emp-sh2pc/emp-sh2pc.h>
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include "../include/PSISender.h"
#include "../include/PSIReceiver.h"
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
using std::vector;
using namespace chrono;
using namespace NTL;

#define TEST_OLE 0
#define TEST_PSI 1

/*
int secParam = 100;
int n = 4*secParam;
int p = 2*secParam + 1;
int l = n - p;
int inputLength = l/4;
int k = (l-1)/2 + 1;
typedef unsigned long ulong;
*/



int port, party;

int main(int argc, char** argv)
{

    /* Common steps */

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


    int setSize = 1000;
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
    

    /*\\
    vec_ZZ_p x;
    x.SetLength(inputLength);
    for(int i = 0; i < inputLength; i++)
    {
        x[i] = 1; //random_ZZ_p();
    }
    */


    vec_ZZ_p servers_input;
    vec_ZZ_p clients_input_mul;
    vec_ZZ_p clients_input_add;
    int inputLength = 4*setSize*ESPVecLen;       // Number of OLEs we will require
    servers_input.SetLength(inputLength);
    clients_input_mul.SetLength(inputLength);
    clients_input_add.SetLength(inputLength);


    /* These values are only for testing */
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


    /* for testing embed and map*/

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

    auto start_time = high_resolution_clock::now();

    if(party == ALICE)      /* Receiver (server)*/
    {

        if(TEST_PSI)
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



        if(TEST_OLE)
        {
            OLEReceiver server(port, context, prime, num_of_ole_instances);
            server.compute_ole(servers_input, inputLength, Alpha, Beta);
        }
        auto end_time = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end_time - start_time);
        std::cout << "Server PSI time: " << duration.count() << "ms" << endl; 

    }

    else                /* Sender (Cient) */
    {   
        
        if(TEST_PSI)
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

        if(TEST_OLE)
        {    
            OLESender client(port, context, prime, num_of_ole_instances);
            client.compute_ole(clients_input_mul, clients_input_add, inputLength, Alpha, Beta);
        }

        auto end_time = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end_time - start_time);
        std::cout << "Client PSI time: " << duration.count() << "ms" << endl; 

    }
}