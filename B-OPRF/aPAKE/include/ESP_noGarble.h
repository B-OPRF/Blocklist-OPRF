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

# define SHA256_DIGEST_LENGTH    32
# define bitsize    16
# define maxLen    32


vector<vector<int>> deterTypeOne(vector<int> s) {
  vector<vector<int>> ret;
  int vecSize = s.size();
  int i = 0;
  while (i<vecSize-1) {
    int curr = s[i];
    int j = i+1;
    if (s[j] != curr) {
      i += 1;
      continue;
    }
    while ((s[j] == curr) && (j<vecSize)) {
      j += 1;
    }
    ret.push_back({i,j});
    i = j;
  }
  return ret;
}


vector<vector<int>> deterTypeTwo(vector<int> s) {
  vector<vector<int>> ret;
  int vecSize = s.size();

  int i = 0;
  int min_length = std::max(2, logStar((float) vecSize));

  while (i<vecSize-1) {
    int curr = i;
    int j = i+1;
    while ((s[j] != s[curr]) && (j<vecSize)) {
      curr += 1;
      j += 1;
    }
    if ((j-i) >= min_length) {
      ret.push_back({i,j});
    }
    i = j;
  }
  return ret;
}


int findDif(vector<int> s1, vector<int> s2) {
  int index = s1.size()-1;
  do {
    if (s1[index] != s2[index])
      return s1.size()-(index+1);
    index -= 1;
  } while (index > 0);
  return index;
}


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


vector<int> label(vector<int> s) {
  int vecSize = s.size();
  vector<int> ret;

  for (int i=1; i<vecSize; i++) {
    vector<int> s1 = toBinary(s[i-1]);
    vector<int> s2 = toBinary(s[i]);
    int pos = findDif(s1,s2);
    int bit = s2[s2.size()-pos-1];
    ret.push_back(2*pos+bit);
  }
  return ret;
}


vector<vector<int>> landmark(vector<int> l) {
  if (l.size() == 1) {
    return {{0}};
  }
  vector<vector<int>> ret;
  vector<int> label;
  vector<int> landmark;
  vector<int> queue;
  for (int i=0; i<l.size(); i++) {
    label.push_back(l[i]);
    if (l[i] >= 3) {
      queue.push_back(l[i]);
    }
  }
  sort(queue.begin(), queue.end());
  while (queue.size() != 0) {
    for (int i=0; i<l.size(); i++) {
      int e = l[i];
      if (e == queue[0]) {
        int j=0;
        if (i == 0) {
          while (j==label[i+1]) {
            j += 1;
          }
          assert(j<3);
          label[i] = j;
        }
        else if (i == l.size()-1) {
          while (j==label[i-1]) {
            j += 1;
          }
          assert(j<3);
          label[i] = j;
        }
        else {
          while ((j==label[i+1]) || (j==label[i-1])) {
            j += 1;
          }
          assert(j<3);
          label[i] = j;
        }
      }
    }
    queue.erase(queue.begin());
  }
  int k = 0;
  while (k < label.size()) {
    if ((k == 0) && (label[k] > label[k+1])) {
      landmark.push_back(k);
      k += 1;
    }
    else if ((k == label.size()-1) && (label[k] > label[k-1])) {
      landmark.push_back(k);
    }
    else {
      if ((label[k] > label[k+1]) && (label[k] > label[k-1])) {
        landmark.push_back(k);
        k += 1;
      }
    }
    k += 1;
  }
  int c = 0;
  while (c < label.size()) {
    if (label[c] == 0) {
      if ((c == 0) && (not_in_int(c+1,landmark))) {
        landmark.push_back(c);
      }
      else if ((c == l.size()-1) && (not_in_int(c-1,landmark))) {
        landmark.push_back(c);
      }
      else {
        if ((not_in_int(c+1,landmark)) && (not_in_int(c-1,landmark))) {
          landmark.push_back(c);
        }
      }
    }
    c += 1;
  }
  sort(landmark.begin(), landmark.end());
  ret.push_back(label);
  ret.push_back(landmark);
  return ret;
}


vector<int> alphabetRed(vector<int> s) {
  vector<int> ret = landmark(label(s))[1];
  return ret;
}


bool is_in(vector<int> pair, vector<vector<int>> vec) {

  for (int i=0; i<vec.size(); i++) {
    if ((pair[0] >= vec[i][0]) && (pair[1] <= vec[i][1]))
      return true;
  }
  return false;
}


static bool comp(const vector<int>& vec1, const vector<int>& vec2){
    return vec1[0] < vec2[0];
}


vector<vector<int>> divideType(vector<int> str) {

  int vecSize = str.size();
  vector<vector<int>> ret;
  vector<int> temp;
  if (vecSize <= 3) {
    temp.push_back(3);
    for (int i=0; i<vecSize; i++) {
      temp.push_back(str[i]);
    } 
    ret.push_back(temp);
    return ret;
  }

  vector<vector<int>> t1 = deterTypeOne(str);
  vector<vector<int>> t2 = deterTypeTwo(str);

  vector<vector<int>> res;
  for (int i=0; i<t1.size(); i++) {
    res.push_back(t1[i]);
  }
  for (int j=0; j<t2.size(); j++) {
    res.push_back(t2[j]);
  }

  sort(res.begin(), res.end(), comp);

  for (int s=0; s<res.size()-1; s++) {
    int now = res[s][1];
    int next = res[s+1][0];

    if (now <= next) {
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
  vector<vector<int>> sorted_res;
  while (pointer_res < res.size()) {
    if ((res[pointer_res][0]-curr) == 0) {
      sorted_res.push_back(res[pointer_res]);
    }
    else if ((res[pointer_res][0]-curr) >= 2) {
      sorted_res.push_back({curr,res[pointer_res][0]});
      sorted_res.push_back(res[pointer_res]);
    }
    else {
      sorted_res.push_back({curr,res[pointer_res][1]});
    }
    curr = res[pointer_res][1];
    pointer_res += 1;
  }
 
  if ((vecSize-curr) == 1) {
    int old = sorted_res.back()[0];
    sorted_res.pop_back();
    sorted_res.push_back({old,vecSize});
  }
  else if ((vecSize-curr) >= 2) {
    sorted_res.push_back({curr,vecSize});
  }

  for (int t=0; t<sorted_res.size(); t++) {
    vector<int> type;
    if (sorted_res[t][0] == sorted_res[t][1]) {
      continue;
    }
    else {
      if (is_in(sorted_res[t], t1)) {
        type.push_back(1);
        for (int e=sorted_res[t][0]; e<sorted_res[t][1]; e++) {
          type.push_back(str[e]);
        }
        ret.push_back(type);
      }
      else if (is_in(sorted_res[t], t2)) {
        type.push_back(2);
        for (int e=sorted_res[t][0]; e<sorted_res[t][1]; e++) {
          type.push_back(str[e]);
        }
        ret.push_back(type);
      }
      else {
        type.push_back(3);
        for (int e=sorted_res[t][0]; e<sorted_res[t][1]; e++) {
          type.push_back(str[e]);
        }
        ret.push_back(type);
      }
    }
  }

  vector<vector<int>> final_ret;
  int counter = 0;

  while (counter<ret.size()) {
    if (ret[counter].size() == 2) {
      vector<int> new_type;
      if (counter == 0) {
        new_type.push_back(3);
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
        new_type.push_back(3);
        for (int a=1; a<ret[counter-1].size(); a++) {
          new_type.push_back(ret[counter-1][a]);
        }
        for (int b=1; b<ret[counter].size(); b++) {
          new_type.push_back(ret[counter][b]);
        }
        final_ret.push_back(new_type);
      }
      else {
        if (ret[counter-1][0] == 1) {
          final_ret.pop_back();
          new_type.push_back(3);
          for (int a=1; a<ret[counter-1].size(); a++) {
          new_type.push_back(ret[counter-1][a]);
        }
        for (int b=1; b<ret[counter].size(); b++) {
          new_type.push_back(ret[counter][b]);
        }
          final_ret.push_back(new_type);
        }
        else {
          new_type.push_back(3);
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


vector<int> find_closest(int length, vector<int> landmark) {

  vector<int> ret;
  for (int i=0; i<length; i++) {
    int min_dist = length;
    int min_mark = i;
    for (int l=0; l<landmark.size(); l++) {
      if (abs(landmark[l]-i) <= min_dist) {
        min_dist = abs(landmark[l]-i);
        min_mark = landmark[l];
      }
    }
    ret.push_back(min_mark);
  }
  return ret;
}


vector<vector<int>> divideBlock(vector<vector<int>> pos) {

  vector<vector<int>> ret;

  for (int i=0; i<pos.size(); i++) {
    vector<int> temp;
    for (int j=1; j<pos[i].size(); j++) {
      temp.push_back(pos[i][j]);
    }
    assert(temp.size() > 1);
    
    if (temp.size() <= 3) {
      ret.push_back(temp);
    }
    else {
      if (pos[i][0] == 2) {
        vector<int> landmark = alphabetRed(temp);
        vector<int> close = find_closest(temp.size(), landmark);
        assert(close.size() == temp.size());
        int c = 0;
        while (c<temp.size()-1) {
          vector<int> res;
          int end = c+1;
          while (close[end] == close[c]) {
            end += 1;
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
          // assert(res.size() <= 3);
          ret.push_back(res);
          c = end;
        }
      }
      else {
        for (int i=0; i<temp.size(); i+=2){
          vector<int> res;
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


vector<int> sha3(int *input, int length){
  vector<int> ret;
  uint8_t output_bytes[32];

  emp::sha3_256(output_bytes, input, length);

  for (int i=0; i<32; i++) {
    vector<int> to_insert = toBinary(output_bytes[i]);
    ret.insert(ret.end(), to_insert.begin(), to_insert.end());
  }
  
  return ret;
}


vector<vector<int>> tree(vector<vector<int>> block_array)
{

  vector<vector<int>> ret;

  for (int i=0; i<block_array.size(); i++) {
    vector<int> temp = block_array[i];
    for (int j=0; j<temp.size(); j++) {
      ret.push_back(toBinary(temp[j]));
    }
  }

  vector<vector<int>> node;
  for (int i=0; i<block_array.size(); i++) {
    vector<int> temp = block_array[i];
    int *to_hash = new int[block_array[i].size()*8];
    for (int j=0; j<block_array[i].size(); j++) {
      vector<int> bin_ascii = toBinary(temp[j]);
      for (int d=0; d<8; d++) {
        to_hash[8*j+d] = bin_ascii[d];
      }
    }
    vector<int> hash_val = sha3(to_hash, block_array[i].size()*8);
    node.push_back(hash_val);
    ret.push_back(hash_val);
  }

  if (node.size() == 1) {
    return ret;
  }
  
  vector<vector<int>> layer;
  do{
    layer.clear();
    for (int i=0; i<node.size(); i+=2){
      if (i==node.size()-3) {
        int *to_hash = new int[768];
        for (int j=0; j<3; j++) {
          for (int d=0; d<256; d++) {
            to_hash[256*j+d] = node[j][d];
          }
        }
        vector<int> res = sha3(to_hash, 768);
        layer.push_back(res);
        ret.push_back(res);
        break;
      }
      else{
        int *to_hash = new int[512];
        for (int j=0; j<2; j++) {
          for (int d=0; d<256; d++) {
            to_hash[256*j+d] = node[j][d];
          }
        }
        vector<int> res = sha3(to_hash, 512);
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


vector<vector<int>> ESP_Type(vector<int> emp_array) {
  vector<vector<int>> ret = tree(divideBlock(divideType(emp_array)));
  return ret;
}