### Edit Sensitive Parsing ###
import math
import hashlib


def strtoASCII(s):

  ret = []
  
  for i in range(len(s)):
    convert = ord(s[i])
    ret.append(convert)
  return ret


def toBinary(n):

  ret = []
  while (n // 2 != 0):
    ret.append(n % 2)
    n = n // 2
  ret.append(n % 2)
  while (len(ret) < 8):
    ret.append(0)
  ret.reverse()
  return ret


def deterTypeOne(s):
  ret = []
  i = 0
  while (i<len(s)-1):
    curr = s[i]
    j = i+1
    if (s[j] != curr):
      i += 1
      continue
    res = s[j] == curr
    while (res and (j<len(s))):
      j += 1
      if (j<len(s)):
        res = s[j] == curr
    ret.append([i,j])
    i = j
  return ret


def logStar(n):
    curr = math.log(n,2)
    ret = 0
    while curr >= 1:
        curr = math.log(curr,2)
        ret += 1
    return ret


def deterTypeTwo(s):
  ret = []

  i = 0
  min_length = max(2, logStar(len(s)))

  while (i<len(s)-1):
    curr = i
    j = i+1
    res = s[j] != s[curr]
    while (res and (j<len(s))):
      curr += 1
      j += 1
      if (j<len(s)):
        res = s[j] != s[curr]
    if ((j-i) >= min_length):
      ret.append([i,j])
    i = j
  return ret


def index_z(v):
  return v[0]


def is_in(pair, vec):
  for i in range(len(vec)):
    if ((pair[0] >= vec[i][0]) and (pair[1] <= vec[i][1])):
      return True
  return False


def divideType(s):
  ret = []
  temp = []
  if (len(s) <= 3):
    temp.append(3)
    for i in range(len(s)):
      temp.append(s[i])
    ret.append(temp)
    return ret

  t1 = deterTypeOne(s)
  t2 = deterTypeTwo(s)

  res = []
  for i in range(len(t1)):
    res.append(t1[i])
  for j in range(len(t2)):
    res.append(t2[j])

  res.sort(key=index_z)

  for i in range(len(res)-1):
    if (res[i][1] <= res[i+1][0]):
      continue
    else:
      if (is_in(res[i],t1)):
        res[i+1][0] = res[i][1]
      else:
        res[i][1] = res[i+1][0]

  pointer_res = 0
  curr = 0
  sorted_res = []
  while (pointer_res < len(res)):
    now = res[pointer_res][0]
    if ((now-curr) == 0):
      sorted_res.append(res[pointer_res])
    elif ((now-curr) >= 2):
      sorted_res.append((curr,res[pointer_res][0]))
      sorted_res.append(res[pointer_res])
    else:
      sorted_res.append((curr,res[pointer_res][1]))
    curr = res[pointer_res][1]
    pointer_res += 1
 
  if ((len(s)-curr) == 1):
    old = sorted_res[-1][0]
    sorted_res.pop()
    sorted_res.append((old,len(s)))
  elif ((len(s)-curr) >= 2):
    sorted_res.append((curr,len(s)))

  for t in range(len(sorted_res)):
    type_vec = []
    if (sorted_res[t][0] == sorted_res[t][1]):
      continue
    else:
      if (is_in(sorted_res[t], t1)):
        type_vec.append(1)
        for e in range(sorted_res[t][0], sorted_res[t][1]):
          type_vec.append(s[e])
        ret.append(type_vec)
      elif (is_in(sorted_res[t], t2)):
        type_vec.append(2)
        for e in range(sorted_res[t][0], sorted_res[t][1]):
          type_vec.append(s[e])
        ret.append(type_vec)
      else:
        type_vec.append(3)
        for e in range(sorted_res[t][0], sorted_res[t][1]):
          type_vec.append(str[e])
        ret.append(type_vec)
  

  final_ret = []
  counter = 0

  while (counter<len(ret)):
    if (len(ret[counter]) == 2):
      new_type = []
      if (counter == 0):
        new_type.append(3)
        for a in range(1,len(ret[counter])):
          new_type.append(ret[counter][a])
        for b in range(1,len(ret[counter+1])):
          new_type.append(ret[counter+1][b])
        final_ret.append(new_type)
        counter += 1
      elif (counter == (len(ret)-1)):
        final_ret.pop()
        new_type.append(3)
        for a in range(1,len(ret[counter-1])):
          new_type.append(ret[counter-1][a])
        for b in range(1,len(ret[counter])):
          new_type.append(ret[counter][b])
        final_ret.append(new_type)
      else:
        if (ret[counter-1][0] == 1):
          final_ret.pop()
          new_type.append(3)
          for a in range(1,len(ret[counter-1])):
            new_type.append(ret[counter-1][a])
          for b in range(1,len(ret[counter])):
            new_type.append(ret[counter][b])
          final_ret.append(new_type)
        else:
          new_type.append(3)
          for a in range(1,len(ret[counter])):
            new_type.append(ret[counter][a])
          for b in range(1,len(ret[counter+1])):
            new_type.append(ret[counter+1][b])
          final_ret.append(new_type)
          counter += 1
    else:
      final_ret.append(ret[counter])
    counter+=1

  return final_ret
    

def findDif(s1,s2):
  index = len(s1)-1
  while True:
    if (s1[index] != s2[index]):
      return len(s1)-(index+1)
    index -= 1
    if (index <= 0):
      break
  return index


def label(s):
  ret = []

  for i in range(1,len(s)):
    s1 = toBinary(s[i-1])
    s2 = toBinary(s[i])
    pos = findDif(s1,s2)
    ret.append(2*pos+int(s2[len(s2)-pos-1]))
  return ret


def landmark(l):
  if (len(l) == 1):
    return 0
  ret = []
  label = []
  landmark = []
  queue = []
  for i in range(len(l)):
    label.append(l[i])
    if (l[i] >= 3):
      queue.append(l[i])
  queue.sort()
  while (len(queue) != 0):
    for i in range(len(l)):
      e = l[i]
      if (e == queue[0]):
        j = 0
        if (i == 0):
          while (j==label[i+1]):
            j += 1
          assert(j<3)
          label[i] = j
        elif (i == len(l)-1):
          while (j==label[i-1]):
            j += 1
          assert(j<3)
          label[i] = j
        else:
          while ((j==label[i+1]) or (j==label[i-1])):
            j += 1
          assert(j<3)
          label[i] = j
    queue.pop(0)
  k = 0
  while (k < len(label)):
    if (k == 0):
      if (label[k] > label[k+1]):
        landmark.append(k)
        k += 1
    elif (k == len(label)-1):
      if (label[k] > label[k-1]):
        landmark.append(k)
    else:
      if ((label[k] > label[k+1]) and (label[k] > label[k-1])):
        landmark.append(k)
        k += 1
    k += 1
  c = 0
  while (c < len(label)):
    if (label[c] == 0):
      if ((c == 0) and ((c+1) not in landmark)):
        landmark.append(c)
      elif ((c == len(l)-1) and ((c-1) not in landmark)):
        landmark.append(c)
      else:
        if (((c+1) not in landmark) and ((c-1) not in landmark)):
          landmark.append(c)
    c += 1
  landmark.sort()
  ret.append(label)
  ret.append(landmark)
  return ret


def alphabetRed(s):
  ret = [i+1 for i in landmark(label(s))[1]]
  return ret


def find_closest(length, landmark):
  ret = []
  for i in range(length):
    min_dist = length
    min_mark = 0
    for l in range(len(landmark)):
      abs_val = abs(landmark[l]-i)
      if (abs_val <= min_dist):
        min_dist = abs_val
        min_mark = landmark[l]
    ret.append(min_mark)
  return ret


def divideBlock(pos):
  ret = []

  for i in range(len(pos)):
    temp = []
    for j in range(1, len(pos[i])):
      temp.append(pos[i][j])
    assert(len(temp) > 1)

    if (len(temp) <= 3):
      ret.append(temp)
    else:
      if (pos[i][0] == 2):
        landmark = alphabetRed(temp)
        close = find_closest(len(temp), landmark)
        assert(len(close) == len(temp))
        c = 0
        while (c<len(temp)-1):
          res = []
          end = c+1
          move = close[end] == close[c]
          while (move and (end < len(temp))):
            end += 1
            if ((end < len(temp))):
              move = close[end] == close[c]
          for k in range(c, end):
            res.append(temp[k])
          if (len(res) < 2):
            if ((len(temp)-end) < 3):
              for t in range(end, len(temp)):
                res.append(temp[t])
              break
            else:
              res.append(temp[end])
              end += 1

          assert(len(res) <= 4)
          if len(res) == 4:
            ret.append(res[:2])
            ret.append(res[2:])
          else:
            ret.append(res)
          c = end
      else:
        i = 0
        while (i < len(temp)):
          res = []
          if (i==len(temp)-3):
            res.append(temp[i])
            res.append(temp[i+1])
            res.append(temp[i+2])
            i += 3
          else:
            res.append(temp[i])
            res.append(temp[i+1])
            i += 2
          ret.append(res)

  return ret


def tree(block_array):
  ret = []

  for i in range(len(block_array)):
    temp = block_array[i]
    for j in range(len(temp)):
      ret.append(chr(temp[j]))

  node = []
  for i in range(len(block_array)):
    temp = block_array[i]
    to_hash = ""
    for j in range(len(temp)):
      to_hash += chr(temp[j])
    # hash_val = hashlib.sha256(to_hash.encode()).hexdigest()
    node.append(to_hash)
    ret.append(to_hash)

  if (len(node) == 1):
    return ret
  
  layer = []
  while (True):
    layer.clear()
    i = 0
    while (i < len(node)):
      if (i==len(node)-3):
        to_hash = ""
        for j in range(3):
          to_hash += node[i+j]
        # res = hashlib.sha256(to_hash.encode()).hexdigest()
        layer.append(to_hash)
        ret.append(to_hash)
        break
      else:
        to_hash = ""
        for j in range(2):
          to_hash += node[i+j]
        # res = hashlib.sha256(to_hash.encode()).hexdigest()
        layer.append(to_hash)
        ret.append(to_hash)
      i += 2

    node.clear()
    for j in range(len(layer)):
      node.append(layer[j])

    if (len(layer)<=1):
      break

  return ret


def ESP_Type(s):
  array = strtoASCII(s)
  ret = tree(divideBlock(divideType(array)))
  return ret

# print(ESP_Type("xnDknown"))
# print(ESP_Type("password"))
# print(ESP_Type("password123"))