#include "emp-tool/emp-tool.h"
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

# define SHA256_DIGEST_LENGTH    32
# define bitsize    16
# define maxLen    32


vector<int> strtoASCII(std::string str) {

  vector<int> ret;
  int convert;
  
  for (unsigned int i = 0; i < str.size(); i++) {
    convert = str[i];
    ret.push_back(convert);
  }
  return ret;
}


vector<Integer> int_to_emp(vector<int> vec) {
  vector<Integer> ret;
  for (int i = 0; i < vec.size(); i++)
  {
    ret.push_back(emp::Integer(bitsize, vec[i], ALICE));
  }
  return ret;
}


vector<Integer> toBinary_(int n)
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


vector<Integer> toBinary_emp(Integer val)
{
  int n = val.reveal<int>(PUBLIC);
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


vector<Integer> sha3(Integer *input, int length){
  vector<Integer> ret;
  uint8_t output_bytes[32];

  emp::Integer output = Integer(10, 32, ALICE);

  SHA3_256_Calculator sha3_256_calculator = SHA3_256_Calculator();
  sha3_256_calculator.sha3_256(&output, input, length);

  output.reveal<uint8_t>(output_bytes, PUBLIC);

  for (int i=0; i<32; i++) {
    vector<Integer> to_insert = toBinary_(output_bytes[i]);
    ret.insert(ret.end(), to_insert.begin(), to_insert.end());
  }
  return ret;
}


vector<Integer> AES(Integer *input, int size) {
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
    vector<Integer> to_insert = toBinary_(output_bytes[i]);
    ret.insert(ret.end(), to_insert.begin(), to_insert.end());
  }
  return ret;
}


bool not_in(Integer val, vector<Integer> vec) {
  for (int i=0; i<vec.size(); i++) {
    Bit res = val == vec[i];
    if (res.reveal<bool>())
      return false;
  }
  return true;
}


bool not_in_int(int val, vector<int> vec) {
  for (int i=0; i<vec.size(); i++) {
    if (val == vec[i])
      return false;
  }
  return true;
}


vector<vector<Integer>> deterTypeOne(vector<Integer> s) {
  vector<vector<Integer>> ret;
  int vecSize = s.size();
  int i = 0;
  while (i<vecSize-1) {
    Integer curr = s[i];
    int j = i+1;
    Bit res = s[j] != curr;
    if (res.reveal<bool>()) {
      i += 1;
      continue;
    }
    res = s[j] == curr;
    while ((res.reveal<bool>()) && (j<vecSize)) {
      j += 1;
      if (j<vecSize) {
        res = s[j] == curr;
      } 
    }
    emp::Integer start = Integer(32, i, ALICE);
    emp::Integer end = Integer(32, j, ALICE);
    ret.push_back({start,end});
    i = j;
  }
  return ret;
}


int logStar(float n) {
  int ret = 0;
  float curr = log2(n);
  while (curr >= 1) {
    curr = log2(curr);
    ret += 1;
  }
  return ret;
}


vector<vector<Integer>> deterTypeTwo(vector<Integer> s) {
  vector<vector<Integer>> ret;
  int vecSize = s.size();

  int i = 0;
  int min_length = std::max(2, logStar((float) vecSize));

  while (i<vecSize-1) {
    int curr = i;
    int j = i+1;
    Bit res = s[j] != s[curr];
    while ((res.reveal<bool>()) && (j<vecSize)) {
      curr += 1;
      j += 1;
      if (j<vecSize) {
        res = s[j] != s[curr];
      }
    }
    if ((j-i) >= min_length) {
      emp::Integer start = Integer(32, i, ALICE);
      emp::Integer end = Integer(32, j, ALICE);
      ret.push_back({start,end});
    }
    i = j;
  }
  return ret;
}


static bool comp1(const vector<Integer>& vec1, const vector<Integer>& vec2){
  Bit res = vec1[0] < vec2[0];
  bool ret = res.reveal<bool>();
  return ret;
}


bool is_in(vector<Integer> pair, vector<vector<Integer>> vec) {
  for (int i=0; i<vec.size(); i++) {
    Bit res1 = pair[0] >= vec[i][0];
    Bit res2 = pair[1] <= vec[i][1];
    if ((res1.reveal<bool>()) && (res2.reveal<bool>()))
      return true;
  }
  return false;
}


vector<vector<Integer>> divideType(vector<Integer> str) {
  int vecSize = str.size();
  vector<vector<Integer>> ret;
  vector<Integer> temp;
  if (vecSize <= 3) {
    temp.push_back(Integer(32, 3, ALICE));
    for (int i=0; i<vecSize; i++) {
      temp.push_back(str[i]);
    } 
    ret.push_back(temp);
    return ret;
  }

  vector<vector<Integer>> t1 = deterTypeOne(str);
  vector<vector<Integer>> t2 = deterTypeTwo(str);

  vector<vector<Integer>> res;
  for (int i=0; i<t1.size(); i++) {
    res.push_back(t1[i]);
  }
  for (int j=0; j<t2.size(); j++) {
    res.push_back(t2[j]);
  }

  sort(res.begin(), res.end(), comp1);

  for (int s=0; s<res.size()-1; s++) {
    Integer now = res[s][1];
    Integer next = res[s+1][0];

    Bit nn = now <= next;
    if (nn.reveal<bool>()) {
      continue;
    }
    else {
      if (is_in(res[s],t1)) {
        res[s+1][0] = now;
      }
      else {
        res[s][1] = next;
      }
    }
  }

  int curr = 0;
  int pointer_res = 0;
  vector<vector<Integer>> sorted_res;
  while (pointer_res < res.size()) {
    int now = res[pointer_res][0].reveal<int>(PUBLIC);
    if ((now-curr) == 0) {
      sorted_res.push_back(res[pointer_res]);
    }
    else if ((now-curr) >= 2) {
      sorted_res.push_back({Integer(32, curr, ALICE),res[pointer_res][0]});
      sorted_res.push_back(res[pointer_res]);
    }
    else {
      sorted_res.push_back({Integer(32, curr, ALICE),res[pointer_res][1]});
    }
    curr = res[pointer_res][1].reveal<int>(PUBLIC);
    pointer_res += 1;
  }
 
  if ((vecSize-curr) == 1) {
    Integer old = sorted_res.back()[0];
    sorted_res.pop_back();
    sorted_res.push_back({old,Integer(32, vecSize, ALICE)});
  }
  else if ((vecSize-curr) >= 2) {
    sorted_res.push_back({curr,Integer(32, vecSize, ALICE)});
  }

  for (int t=0; t<sorted_res.size(); t++) {
    vector<Integer> type;
    Bit sort_comp = sorted_res[t][0] == sorted_res[t][1];
    if (sort_comp.reveal<bool>()) {
      continue;
    }
    else {
      if (is_in(sorted_res[t], t1)) {
        type.push_back(Integer(32, 1, ALICE));
        for (int e=sorted_res[t][0].reveal<int>(PUBLIC); e<sorted_res[t][1].reveal<int>(PUBLIC); e++) {
          type.push_back(str[e]);
        }
        ret.push_back(type);
      }
      else if (is_in(sorted_res[t], t2)) {
        type.push_back(Integer(32, 2, ALICE));
        for (int e=sorted_res[t][0].reveal<int>(PUBLIC); e<sorted_res[t][1].reveal<int>(PUBLIC); e++) {
          type.push_back(str[e]);
        }
        ret.push_back(type);
      }
      else {
        type.push_back(Integer(32, 3, ALICE));
        for (int e=sorted_res[t][0].reveal<int>(PUBLIC); e<sorted_res[t][1].reveal<int>(PUBLIC); e++) {
          type.push_back(str[e]);
        }
        ret.push_back(type);
      }
    }
  }

  vector<vector<Integer>> final_ret;
  int counter = 0;

  while (counter<ret.size()) {
    if (ret[counter].size() == 2) {
      vector<Integer> new_type;
      if (counter == 0) {
        new_type.push_back(Integer(32, 3, ALICE));
        for (int a=1; a<ret[counter].size(); a++) {
          new_type.push_back(ret[counter][a]);
        }
        for (int b=1; b<ret[counter+1].size(); b++) {
          new_type.push_back(ret[counter+1][b]);
        }
        final_ret.push_back(new_type);
        counter += 1;
      }
      else if (counter == (ret.size()-1)) {
        final_ret.pop_back();
        new_type.push_back(Integer(32, 3, ALICE));
        for (int a=1; a<ret[counter-1].size(); a++) {
          new_type.push_back(ret[counter-1][a]);
        }
        for (int b=1; b<ret[counter].size(); b++) {
          new_type.push_back(ret[counter][b]);
        }
        final_ret.push_back(new_type);
      }
      else {
        if (ret[counter-1][0].reveal<int>(PUBLIC) == 1) {
          final_ret.pop_back();
          new_type.push_back(Integer(32, 3, ALICE));
          for (int a=1; a<ret[counter-1].size(); a++) {
          new_type.push_back(ret[counter-1][a]);
        }
        for (int b=1; b<ret[counter].size(); b++) {
          new_type.push_back(ret[counter][b]);
        }
          final_ret.push_back(new_type);
        }
        else {
          new_type.push_back(Integer(32, 3, ALICE));
          for (int a=1; a<ret[counter].size(); a++) {
          new_type.push_back(ret[counter][a]);
        }
        for (int b=1; b<ret[counter+1].size(); b++) {
          new_type.push_back(ret[counter+1][b]);
        }
          final_ret.push_back(new_type);
          counter += 1;
        }
      }
    }
    else {
      final_ret.push_back(ret[counter]);
    }
    counter+=1;
  }

  return final_ret;
}


int findDif(vector<Integer> s1, vector<Integer> s2) {
  int index = s1.size()-1;
  do {
    Bit res = s1[index] != s2[index];
    if (res.reveal<bool>())
      return s1.size()-(index+1);
    index -= 1;
  } while (index > 0);
  return index;
}


vector<Integer> label(vector<Integer> s) {
  int vecSize = s.size();
  vector<Integer> ret;

  for (int i=1; i<vecSize; i++) {
    vector<Integer> s1 = toBinary_emp(s[i-1]);
    vector<Integer> s2 = toBinary_emp(s[i]);
    int pos = findDif(s1,s2);
    int bit = s2[s2.size()-pos-1].reveal<int>(PUBLIC);
    ret.push_back(Integer(32,2*pos+bit,ALICE));
  }
  return ret;
}


static bool comp2(const Integer val1, const Integer val2){
  Bit res = val1 < val2;
  bool ret = res.reveal<bool>();
  return ret;
}


vector<vector<Integer>> landmark(vector<Integer> l) {
  if (l.size() == 1) {
    return {{Integer(32, 0, ALICE)}};
  }
  vector<vector<Integer>> ret;
  vector<Integer> label;
  vector<Integer> landmark;
  vector<Integer> queue;
  for (int i=0; i<l.size(); i++) {
    label.push_back(l[i]);
    if (l[i].reveal<int>(PUBLIC) >= 3) {
      queue.push_back(l[i]);
    }
  }
  sort(queue.begin(), queue.end(), comp2);
  while (queue.size() != 0) {
    for (int i=0; i<l.size(); i++) {
      Integer e = l[i];
      Bit res = e == queue[0];
      if (res.reveal<bool>()) {
        int j=0;
        if (i == 0) {
          while (j==label[i+1].reveal<int>(PUBLIC)) {
            j += 1;
          }
          assert(j<3);
          label[i] = Integer(32, j, ALICE);
        }
        else if (i == l.size()-1) {
          while (j==label[i-1].reveal<int>(PUBLIC)) {
            j += 1;
          }
          assert(j<3);
          label[i] = Integer(32, j, ALICE);
        }
        else {
          while ((j==label[i+1].reveal<int>(PUBLIC)) || (j==label[i-1].reveal<int>(PUBLIC))) {
            j += 1;
          }
          assert(j<3);
          label[i] = Integer(32, j, ALICE);
        }
      }
    }
    queue.erase(queue.begin());
  }
  int k = 0;
  while (k < label.size()) {  
    if (k == 0) {
      Bit res1 = label[k] > label[k+1];
      if (res1.reveal<bool>()) {
        landmark.push_back(Integer(32, k, ALICE));
        k += 1;
      }
    }
    else if (k == label.size()-1) {
      Bit res2 = label[k] > label[k-1];
      if (res2.reveal<bool>()) {
        landmark.push_back(Integer(32, k, ALICE));
      }
    }
    else {
      Bit res1 = label[k] > label[k+1];
      Bit res2 = label[k] > label[k-1];
      if ((res1.reveal<bool>()) && (res2.reveal<bool>())) {
        landmark.push_back(Integer(32, k, ALICE));
        k += 1;
      }
    }
    k += 1;
  }
  int c = 0;
  while (c < label.size()) {
    if (label[c].reveal<int>(PUBLIC) == 0) {
      if ((c == 0) && (not_in(Integer(32, c+1, ALICE),landmark))) {
        landmark.push_back(Integer(32, c, ALICE));
      }
      else if ((c == l.size()-1) && (not_in(Integer(32, c-1, ALICE),landmark))) {
        landmark.push_back(Integer(32, c, ALICE));
      }
      else {
        if ((not_in(Integer(32, c+1, ALICE),landmark)) && (not_in(Integer(32, c-1, ALICE),landmark))) {
          landmark.push_back(Integer(32, c, ALICE));
        }
      }
    }
    c += 1;
  }
  sort(landmark.begin(), landmark.end(), comp2);
  ret.push_back(label);
  ret.push_back(landmark);
  return ret;
}


vector<Integer> alphabetRed(vector<Integer> s) {
  vector<Integer> ret = landmark(label(s))[1];
  return ret;
}


vector<Integer> find_closest(int length, vector<Integer> landmark) {
  vector<Integer> ret;
  for (int i=0; i<length; i++) {
    int min_dist = length;
    Integer min_mark = Integer(32, 0, ALICE);
    for (int l=0; l<landmark.size(); l++) {
      int abs_val = abs(landmark[l].reveal<int>(PUBLIC)-i);
      if (abs_val <= min_dist) {
        min_dist = abs_val;
        min_mark = landmark[l];
      }
    }
    ret.push_back(min_mark);
  }
  return ret;
}


vector<vector<Integer>> divideBlock(vector<vector<Integer>> pos) {

  vector<vector<Integer>> ret;

  for (int i=0; i<pos.size(); i++) {
    vector<Integer> temp;
    for (int j=1; j<pos[i].size(); j++) {
      temp.push_back(pos[i][j]);
    }
    assert(temp.size() > 1);

    if (temp.size() <= 3) {
      ret.push_back(temp);
    }
    else {
      if (pos[i][0].reveal<int>(PUBLIC) == 2) {
        vector<Integer> landmark = alphabetRed(temp);
        vector<Integer> close = find_closest(temp.size(), landmark);
        assert(close.size() == temp.size());
        int c = 0;
        while (c<temp.size()-1) {
          vector<Integer> res;
          int end = c+1;
          Bit move = close[end] == close[c];
          while ((move.reveal<bool>()) && (end < temp.size())) {
            end += 1;
            if ((end < temp.size())) {
              move = close[end] == close[c];
            }
          }
          for (int k=c; k<end; k++) {
            res.push_back(temp[k]);
          }
          if (res.size() < 2) {
            if ((temp.size()-end) < 3) {
              for (int t=end; t<temp.size(); t++) {
                res.push_back(temp[t]);
              }
              break;
            }
            else {
              res.push_back(temp[end]);
              end += 1;
            }
          }
          assert(res.size() <= 3);
          ret.push_back(res);
          c = end;
        }
      }
      else {
        for (int i=0; i<temp.size(); i+=2){
          vector<Integer> res;
          if (i==temp.size()-3) {
            res.push_back(temp[i]);
            res.push_back(temp[i+1]);
            res.push_back(temp[i+2]);
          }
          else{
            res.push_back(temp[i]);
            res.push_back(temp[i+1]);
          }
          ret.push_back(res);
        }
      }
    }
  }
  return ret;
}


vector<vector<Integer>> tree(vector<vector<Integer>> block_array)
{
  vector<vector<Integer>> ret;

  for (int i=0; i<block_array.size(); i++) {
    vector<Integer> temp = block_array[i];
    for (int j=0; j<temp.size(); j++) {
      ret.push_back(toBinary_emp(temp[j]));
    }
  }

  vector<vector<Integer>> node;
  for (int i=0; i<block_array.size(); i++) {
    vector<Integer> temp = block_array[i];
    Integer *to_hash = new Integer[block_array[i].size()*8];
    for (int j=0; j<block_array[i].size(); j++) {
      vector<Integer> bin_ascii = toBinary_emp(temp[j]);
      for (int d=0; d<8; d++) {
        to_hash[8*j+d] = bin_ascii[d];
      }
    }
    vector<Integer> hash_val = AES(to_hash,block_array[i].size()*8);
    node.push_back(hash_val);
    ret.push_back(hash_val);
  }

  if (node.size() == 1) {
    return ret;
  }
  
  vector<vector<Integer>> layer;
  do{
    layer.clear();
    for (int i=0; i<node.size(); i+=2){
      if (i==node.size()-3) {
        Integer *to_hash = new Integer[768];
        for (int j=0; j<3; j++) {
          for (int d=0; d<256; d++) {
            to_hash[256*j+d] = node[j][d];
          }
        }
        vector<Integer> res = AES(to_hash,768);
        layer.push_back(res);
        ret.push_back(res);
        break;
      }
      else{
        Integer *to_hash = new Integer[512];
        for (int j=0; j<2; j++) {
          for (int d=0; d<256; d++) {
            to_hash[256*j+d] = node[j][d];
          }
        }
        vector<Integer> res = AES(to_hash,512);
        layer.push_back(res);
        ret.push_back(res);
      }
    }

    node.clear();
    for (int j=0; j<layer.size(); j++) {
      node.push_back(layer[j]);
    }      
  } while (layer.size()>1);

  return ret;
}


vector<vector<Integer>> ESP_Type(vector<Integer> emp_array) {
  vector<vector<Integer>> ret = tree(divideBlock(divideType(emp_array)));
  // vector<vector<Bit>> final_ret;
  // for (int i=0; i<ret.size(); i++) {
  //   vector<Bit> temp;
  //   for (int j=0; j<ret[i].size(); j++) {
  //     temp.push_back(Bit(ret[i][j].reveal<int>(PUBLIC), ALICE));
  //   }
  //   final_ret.push_back(temp);
  // }
  return ret;
}



