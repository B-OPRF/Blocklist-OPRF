import random
import hashlib
import matplotlib.pyplot as plt
from ESP import ESP_Type
# from ESP_old import ESP_res
import csv
import math


cluster = list(range(10000, 110000, 10000))
vecLen = list(range(64, 257, 32))

# plt.figure(figsize=(10, 6))
# plt.xlabel('FAR')
# plt.ylabel('FRR')

# for v in vecLen:
#     with open('table/table_'+str(v)+'.csv', 'r') as f:
#         reader = csv.reader(f)
#         data = list(reader)
#     f.close()
#     curr = data[(40*(cluster//10000-1)):((40*(cluster//10000-1))+40)]
#     FAR = [float(c[1]) for c in curr]
#     FRR = [float(c[2]) for c in curr]
#     plt.plot(FAR, FRR, label=str(v))

# plt.legend()
# plt.show()


# fig, ax1 = plt.subplots()

# color = 'tab:red'
# ax1.set_xlabel('FAR')
# ax1.set_ylabel('FRR', color=color)
# ax1.set_ylim(ymin=-0.05)
# ax1.plot(FAR_128, FRR_BOPRF_128, color=color, marker="o")
# ax1.plot(FAR_128, FRR_H, color=color, linestyle='dashed', marker="o")
# ax1.tick_params(axis='y', labelcolor=color)

# ax2 = ax1.twinx() 

# color = 'tab:blue'
# ax2.set_ylabel('Client Latency (s)', color=color)  
# ax2.set_ylim(ymin=-0.5, ymax = 16)
# ax2.plot(FAR_128, Latency_BOPRF_128, color=color, marker="x")
# ax2.plot(FAR_128, Latency_H, color=color, linestyle='dashed', marker="x")
# ax2.tick_params(axis='y', labelcolor=color)

# fig.tight_layout() 
# plt.title('Precision Recall')
# plt.show()

with open('table/table_224_new.csv', 'r') as f:
    reader = csv.reader(f)
    data_new = list(reader)
f.close()
FAR_T = []
FRR_T = []
for d in data_new[:40]:
    FAR_T.append(float(d[1]))
    FRR_T.append(float(d[2]))
FAR_C = [0.2, 0.17, 0.16, 0.14]
FRR_C = [0.13, 0.14, 0.15, 0.17]
with open('table/table_224.csv', 'r') as f:
    reader = csv.reader(f)
    data = list(reader)
f.close()
for d in data[160:]:
    if (int(d[0]) == 1):
        FAR_C.append(float(d[1]))
        FRR_C.append(float(d[2]))
FAR_V = []
FRR_V = []
for v in vecLen:
    with open('table/table_'+str(v)+'_new.csv', 'r') as f:
        reader = csv.reader(f)
        data = list(reader)
    f.close()
    FAR_V.append(float(data[1][1]))
    FRR_V.append(float(data[1][2]))
FAR_H = [0, 0.05, 0.1, 0.15, 0.2, 0.25]
FRR_H = [0]*len(FAR_H)

# Create subplots
fig, axs = plt.subplots(3, 1, sharex=True, figsize=(8, 12))

# Plot each graph
axs[0].plot(FAR_T, FRR_T, 'r', marker='x', label='varying threshold')
axs[0].plot(FAR_C, FRR_C, 'g', linestyle='dashed', marker='x', label='varying list size')
axs[0].plot(FAR_V, FRR_V, 'b', linestyle='dashdot', marker='x', label='varying vector length')
axs[0].plot(FAR_H, FRR_H, 'y', linestyle='dotted', label='Hashtable')
# axs[0].set_title('Accuracy')

Latency_T = [3.185]*len(FAR_T)
Latency_V = [1.54, 1.869, 2.198, 2.527, 2.856, 3.185, 3.514]
Latency_H = [59.65, 58.03, 56.28, 54.72, 52.72, 50.29]
with open('data_224.csv', 'r') as f:
    reader = csv.reader(f)
    data = list(reader)
f.close()
Latency_C = [float(d)/1000 for d in data[0]]
Comm_C = [float(d) for d in data[1]]

axs[1].plot(FAR_T, [math.log(l) for l in Latency_T], 'r', marker='x')
axs[1].plot(FAR_C, [math.log(l) for l in Latency_C], 'g', linestyle='dashed', marker='x')
axs[1].plot(FAR_V, [math.log(l) for l in Latency_V], 'b', linestyle='dashdot', marker='x')
axs[1].plot(FAR_H, [math.log(l) for l in Latency_H], 'y', linestyle='dotted')
# axs[1].set_title('Client Latency (s)')

Comm_T = [12.072]*len(FAR_T)
Comm_V = [6.862, 7.896, 8.94, 9.984, 11.028, 12.072, 13.116]
Comm_H = [int(33283729*(1-r))*128//(2**20) for r in FAR_H]

axs[2].plot(FAR_T, [math.log(c) for c in Comm_T], 'r', marker='x')
axs[2].plot(FAR_C, [math.log(c) for c in Comm_C], 'g', linestyle='dashed', marker='x')
axs[2].plot(FAR_V, [math.log(c) for c in Comm_V], 'b', linestyle='dashdot', marker='x')
axs[2].plot(FAR_H, [math.log(c) for c in Comm_H], 'y', linestyle='dotted')
# axs[2].set_title('Communication Cost (MB)')

# Label the x-axis on the bottom plot only (shared x-axis)
axs[2].set_xlabel('FAR')

# Set y-axis labels for each subplot
axs[0].set_ylabel('FRR')
axs[1].set_ylabel('Client Latency (s)')
axs[2].set_ylabel('Communication Cost (MB)')

# handles, labels = axs[0].get_legend_handles_labels()
# fig.legend(handles, labels, loc='lower center', ncol=4, bbox_to_anchor=(0.5, -0.05))

# Adjust layout
plt.tight_layout()
plt.subplots_adjust(bottom=0.05)

# Display the plot
axs[0].legend()
# plt.show()

for d in zip(FAR_V, [math.log(l+0.358) for l in Latency_V]):
    print("{0:0.2f}".format(d[0])+' '+"{0:0.2f}".format(d[1]))

