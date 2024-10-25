import random
import hashlib
import matplotlib.pyplot as plt
from ESP import ESP_Type
# from ESP_old import ESP_res
import csv


vecLen = 32 # TODO: Replace with vector length


def compute_simhash(vec):

    def hashfunc(x):
        return int(hashlib.md5(x.encode('utf-8')).hexdigest(), 16)
    
    ret = 0
    for token in vec:
        hashbits = vecLen
        v = [0] * hashbits
        for c in token:
            h = hashfunc(c)
            for i in range(hashbits):
                bitmask = 1 << i
                if h & bitmask:
                    v[i] += 1
                else:
                    v[i] -= 1
    
        fingerprint = 0
        for i in range(hashbits):
            if v[i] >= 0:
                fingerprint |= 1 << i
        ret |= fingerprint
    return ret


# def compute_simhash(vec):

#     def hashfunc(x):
#         return int(hashlib.md5(x.encode('utf-8')).hexdigest(), 16)
    
#     s = ''.join(vec)
#     hashbits = 64
#     v = [0] * hashbits
#     for token in s:
#         h = hashfunc(token)
#         for i in range(hashbits):
#             bitmask = 1 << i
#             if h & bitmask:
#                 v[i] += 1
#             else:
#                 v[i] -= 1
    
#     fingerprint = 0
#     for i in range(hashbits):
#         if v[i] >= 0:
#             fingerprint |= 1 << i
#     return fingerprint


def hamming_distance(hash1, hash2):
    x = hash1 ^ hash2
    return bin(x).count('1')

def read_file_to_string(file_path):
    ret = []
    try:
        file = open(file_path, 'r')
        for line in file.readlines():
            ret.append(line.replace("\n",""))
        return ret
    except FileNotFoundError:
        print("File not found.")
        return ""


def read_file_to_string_n(file_path, n):
    ret = []
    counter = 0
    try:
        file = open(file_path, 'r')
        for line in file.readlines():
            ret.append(line.replace("\n",""))
            counter += 1
            if counter >= n:
                return ret
        return ret
    except FileNotFoundError:
        print("File not found.")
        return ""


file2 = read_file_to_string('edit_2.txt')
file1 = read_file_to_string('edit_1.txt')
file0 = read_file_to_string('common100.txt')
file = file2 + file1 + file0

SIM = dict()
counter = 0
for string in file:
    counter += 1
    if (len(string) > 1):
        ESP_res = compute_simhash(ESP_Type(string))
        if ESP_res not in SIM:
            SIM[ESP_res] = 1
        else:
            SIM[ESP_res] += 1
    # if (counter % 100000 == 0):
    #     print(counter)

print(len(list(SIM)))


SIM_sorted = sorted(SIM, key=SIM.get, reverse=True)
outfile = open('cluster_'+str(vecLen)+'.csv', 'w')
writer = csv.writer(outfile)
for k in SIM_sorted:
    writer.writerow([k,SIM[k]])


common_path = 'dataset_3500.txt'
common = read_file_to_string(common_path)


def count_edit_esp(data, length):
    outfile = open('cluster_'+str(vecLen)+'_dataset_'+str(length)+'.csv', 'w')
    writer = csv.writer(outfile)
    for i in range(len(common)):
        embed = compute_simhash(ESP_Type(common[i]))
        dist = 10000
        for k in range(length):
            dist = min(dist,hamming_distance(embed,int(data[k][0])))
        flag = 0
        if (common[i] in file):
            flag = 1
        writer.writerow([common[i],dist,flag])
        # if (i%100 == 0):
        #     print(i)


def optimal_PR(length):
    with open('cluster_'+str(vecLen)+'_dataset_'+str(length)+'.csv', 'r') as f:
        reader = csv.reader(f)
        data = list(reader)
    f.close()

    ret = []

    for i in range(40):
        TP = 0
        TN = 0
        FP = 0
        FN = 0
        for d in data:
            if ((int(d[1])) <= i) and ((int(d[2])) == 0):
                FP += 1
            elif ((int(d[1])) <= i) and ((int(d[2])) == 1):
                TP += 1
            elif ((int(d[1])) > i) and ((int(d[2])) == 1):
                FN += 1
            elif ((int(d[1])) > i) and ((int(d[2])) == 0):
                TN += 1
        ret.append((FN/(FN+TP),FP/(FP+TN)))
    return ret
    
    # FAR = min(ret, key=lambda p:p[0])
    # FRR = min(ret, key=lambda p:p[1])
    # return FAR, FRR


with open('cluster_'+str(vecLen)+'.csv', 'r') as f:
    reader = csv.reader(f)
    dataset = list(reader)
f.close()
outfile = open('table_'+str(vecLen)+'.csv', 'w')
writer = csv.writer(outfile)
tests = list(range(10000, len(dataset), 10000))
for l in tests:
    print(l)
    count_edit_esp(dataset, l)
    line = optimal_PR(l)
    for i in range(len(line)):
        writer.writerow([i, line[i][0], line[i][1]])


