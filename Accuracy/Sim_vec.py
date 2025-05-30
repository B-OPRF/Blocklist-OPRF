import random
import hashlib
import matplotlib.pyplot as plt
from ESP import ESP_Type
# from ESP_old import ESP_res
import csv


def compute_simhash(vec, n):

    def hashfunc(x):
        return int(hashlib.md5(x.encode('utf-8')).hexdigest(), 16)
    
    ret = 0
    hashbits = n
    v = [0] * hashbits
    for token in vec:
        
        h = hashfunc(token)
        
        for i in range(hashbits):
            bitmask = 1 << i
            if h & bitmask:
                v[i] += 1
            else:
                v[i] -= 1

    for i in range(hashbits):
        if v[i] >= 0:
            ret |= 1 << i

    return ret


# def compute_simhash(vec):

#     def hashfunc(x):
#         return int(hashlib.md5(x.encode('utf-8')).hexdigest(), 16)
    
#     ret = 0
#     for token in vec:
#         hashbits = 224
#         v = [0] * hashbits
#         for c in token:
#             h = hashfunc(c)
#             print(h)
#             for i in range(hashbits):
#                 bitmask = 1 << i
#                 if h & bitmask:
#                     v[i] += 1
#                 else:
#                     v[i] -= 1
    
#         fingerprint = 0
#         for i in range(hashbits):
#             if v[i] >= 0:
#                 fingerprint |= 1 << i
#         ret |= fingerprint
#     return ret


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


# file2 = read_file_to_string('edit_2.txt')
file1 = read_file_to_string('edit_1.txt')
file0 = read_file_to_string('common100.txt')
# file = file2 + file1 + file0
file = file1 + file0

print(len(file))

n_vec = [24, 40, 48, 56, 64]

for n in n_vec:

    SIM = dict()
    counter = 0
    for string in file:
        counter += 1
        if (len(string) > 1):
            ESP_res = compute_simhash(ESP_Type(string), n)
            if ESP_res in SIM:
                SIM[ESP_res] += 1
            else:
                SIM[ESP_res] = 1
    # if (counter % 10000 == 0):
    #     print(len(list(SIM)))

    sorted_SIM = dict(sorted(SIM.items(), key=lambda item: item[1], reverse=True))

# with open("Blocklist_32.txt", "w") as blocklist: 
#     for s in SIM:
#         blocklist.write(str(s) + "\n")

    outfile = open('Blocklist_1_'+str(n)+'.csv', 'w')
    writer = csv.writer(outfile)
    for x in sorted_SIM:
        writer.writerow([x,sorted_SIM[x]])

# with open("only_SIM_res.txt", "w") as ESP_file: 
#     for s in SIM:
#         ESP_file.write(str(s) + "\n")

# common_path = '10k.txt'
# common = read_file_to_string(common_path)

# blocklist = read_file_to_string('only_SIM_res.txt')

# def ham_sim_file():
    
#     outfile = open('edit2_ham_sim.csv', 'w')
#     writer = csv.writer(outfile)
#     for i in range(len(file)):
#         for j in range(len(common)):
#             (e1, e2) = ESP_res(file[i],common[j])
#             dist = hamming_distance(compute_simhash(e1),compute_simhash(e2))
#             writer.writerow([file[i],common[j],dist])

#     outfile.close()


def count_edit_esp():
    outfile = open('edit_new_sim.csv', 'w')
    writer = csv.writer(outfile)
    for i in range(len(common)):
        embed = compute_simhash(ESP_Type(common[i]))
        dist = 10000
        for k in range(len(blocklist)):
            dist = min(dist,hamming_distance(embed,int(blocklist[k])))
        flag = 0
        if (common[i] in file):
            flag = 1
        writer.writerow([common[i],dist,flag])
        if (i%50 == 0):
            print(i)


def optimal_PR():
    with open('edit_new_sim.csv', 'r') as f:
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
                print(d[0])
                FN += 1
            elif ((int(d[1])) > i) and ((int(d[2])) == 0):
                TN += 1
        ret.append((FN/(FN+TP),FP/(FP+TN)))
    FAR = min(ret, key=lambda p:p[0])
    FRR = min(ret, key=lambda p:p[1])
    return FAR, FRR



# count_edit_esp()
# print(optimal_PR())

# FAR = [0.8,0.8,0.8,0.76,0.233,0]
# FRR = [0,0,0,0.0005,0.0111,0.0175]
# L = [100, 1000, 10000, 1000000, 2967214, 30316515]
# x_h = [0, 1]
# y_h = [0, 0]

# plt.figure(figsize=(10, 6))
# plt.plot(FAR, FRR, label='B-OPRF')
# plt.plot(x_h, y_h, label='Hashtable')
# plt.xlabel('FAR')
# plt.ylabel('FRR')
# # plt1.title('Precision Recall')
# plt.legend()
# plt.show()

# plt.figure(figsize=(10, 6))
# plt.plot(L, FAR, label='FAR')
# plt.plot(L, FRR, label='FRR')
# plt.xlabel('|L|')
# plt.ylabel('Percentage')
# plt.legend()
# plt.show()
