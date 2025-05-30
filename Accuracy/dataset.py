import csv           

res = set()

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



def optimal_PR():

    with open('cluster_1_32_pw_5000.csv', 'r') as f:
        reader = csv.reader(f)
        data3 = list(reader)
    f.close()

    threshold = 2
    counter = 0
    ret = []
    for i in range(21,22):
        TP = 0

        TN = 0
        FP = 0
        FN = 0
        """
        for d in data:
            if ((int(d[1])) <= i) and ((int(d[3])) == 0):
                FP += 1
            elif ((int(d[1])) <= i) and ((int(d[3])) == 1):
                TP += 1
            elif ((int(d[1])) > i) and ((int(d[3])) == 1):
                FN += 1
            elif ((int(d[1])) > i) and ((int(d[3])) == 0):
                TN += 1
        """
        for d3 in data3:
            if ((int(d3[1])) <= i) and ((int(d3[3])) > threshold):
                FP += 1
                if counter <= 300000:
                    res.add(d3[0])
                    counter += 1
            elif ((int(d3[1])) <= i) and ((int(d3[3])) <= threshold):
                TP += 1
                # res.add(d3[0])
            elif ((int(d3[1])) > i) and ((int(d3[3])) <= threshold):
                FN += 1   
                if counter <= 300000:
                    res.add(d3[0])
                    counter += 1          
            elif ((int(d3[1])) > i) and ((int(d3[3])) > threshold):
                TN += 1
                # res.add(d3[0])
        ret.append((FN/(FN+TP),FP/(FP+TN)))
    print(ret)

true = read_file_to_string('pw_5000.txt')

for t in true:
    res.add(t)

optimal_PR()
output_file = 'test_5000.txt' 
with open(output_file, 'w', encoding='utf-8') as outfile:
    for r in res:
        outfile.write(r)
        outfile.write('\n')