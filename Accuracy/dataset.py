import csv

def optimal_PR():
    with open('cluster_224_dataset.csv', 'r') as f:
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
            if ((int(d[1])) <= i) and ((int(d[3])) == 0):
                FP += 1
            elif ((int(d[1])) <= i) and ((int(d[3])) == 1):
                TP += 1
            elif ((int(d[1])) > i) and ((int(d[3])) == 1):
                FN += 1             
            elif ((int(d[1])) > i) and ((int(d[3])) == 0):
                TN += 1
        ret.append((FN/(FN+TP),FP/(FP+TN)))
    print(ret)

optimal_PR()