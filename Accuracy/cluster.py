import random
import hashlib
import matplotlib.pyplot as plt
from ESP import ESP_Type
# from ESP_old import ESP_res
import csv
import editdistance


def compute_simhash(vec):

    def hashfunc(x):
        return int(hashlib.md5(x.encode('utf-8')).hexdigest(), 16)
    
    ret = 0
    hashbits = 64
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
# file1 = read_file_to_string('edit_1.txt')
file0 = read_file_to_string('common100.txt')
# blocklist = file2 + file1 + file0
# print(len(blocklist))

passwords_path = 'test_5000.txt'
passwords = read_file_to_string(passwords_path)
print(len(passwords))


def count_edit_esp(n):
    outfile = open('cluster_1_32_pw_'+str(n)+'.csv', 'w')
    writer = csv.writer(outfile)
    for i in range(len(passwords)):
        try:
            embed = compute_simhash(ESP_Type(passwords[i]))
        except:
            continue
        dist = 10000
        min_k = 300000
        for k in range(len(blocklist_embedded)):
            if dist > hamming_distance(embed,int(blocklist_embedded[k])):
                dist = hamming_distance(embed,int(blocklist_embedded[k]))
                min_k = k
        min_edit = 10000
        for c in file0:
            edit = editdistance.eval(c, passwords[i])
            if min_edit > edit:
                min_edit = edit
        writer.writerow([passwords[i],dist,min_k,min_edit])


n_vec = [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000]
for n in n_vec:
    blocklist_embedded = []
    counter = 0
    with open('Blocklist_1_32.csv', newline='') as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            if not row:
                continue
            blocklist_embedded.append(row[0])
            counter += 1
            if counter == n:
                break
    print(len(blocklist_embedded))
    count_edit_esp(n)

