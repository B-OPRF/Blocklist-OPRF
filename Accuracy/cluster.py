import random
import hashlib
import matplotlib.pyplot as plt
from ESP import ESP_Type
# from ESP_old import ESP_res
import csv


def compute_simhash(vec):

    def hashfunc(x):
        return int(hashlib.sha256(x.encode('utf-8')).hexdigest(), 16)
    
    ret = 0
    for token in vec:
        hashbits = 224
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
blocklist = file2 + file1 + file0
print(len(blocklist))

passwords_path = 'pw.txt'
passwords = read_file_to_string(passwords_path)
print(len(passwords))

blocklist_embedded = read_file_to_string('cluster.txt')
print(len(blocklist_embedded))


def count_edit_esp():
    outfile = open('cluster_224_dataset.csv', 'w')
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
        flag = 0
        if (passwords[i] in blocklist):
            flag = 1
        writer.writerow([passwords[i],dist,min_k,flag])

count_edit_esp()

