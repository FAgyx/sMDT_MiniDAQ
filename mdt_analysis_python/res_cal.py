import numpy as np
from matplotlib import pyplot as plt


filename = 'run00188338_20190514'
path = './oldASD/run4'
high_limit = 250
low_limit = 150

CHANNEL = 24
TDCID = 7
i = 21
decodefilename = path + '/data_gen/' + filename + '_decode.txt'


def count_elements(seq):
    hist = {}
    for i in seq:
        hist[i] = hist.get(i, 0) + 1
    return hist


leading = 0
trailing = 0
pair = 0
width = []
file_decode = open(decodefilename,"r")

for line in file_decode.readlines():
    if line[0:5] == 'event':
        leading = 0  # reset
        trailing = 0
        pair = 0
    elif line[0:9] == 'TDC' + str(TDCID) + 'CH' + str(i) + 'L':
        if pair == 0:  # leading edge found
            leading = int(line[10:], 10)
            pair = 1
        else:
            # print('pair warning ' + str(pair) + ' for leading edge')
            pair = 1  # leading edge not unique in one event
    elif line[0:9] == 'TDC' + str(TDCID) + 'CH' + str(i) + 'T':
        if pair == 1:
            trailing = int(line[10:], 10)
            width.append(trailing - leading)
            pair = 0   # trailing edge found

        else:
            # print('pair warning '+str(pair)+' for trailing edge')
            pair = 0   # trailing edge not paired
file_decode.close()
counted = count_elements(width)
time = [k*0.1953125 for k in sorted(counted.keys())]
count = [counted[k] for k in sorted(counted.keys())]
plt.bar(time, count, label=str(i))
plt.legend()
# plt.xlim(0, max(max(width))+100)
plt.xlim(170,220)
plt.ylim(0,14000)
plt.xlabel('Width (ns)')
plt.ylabel('Frequence')

width_in_ns = [data*0.1953125 for data in width]
hit_width = [data for data in width_in_ns if low_limit < data < high_limit]
hit_mean = np.mean(hit_width)
hit_std = np.std(hit_width)
print('mean='+str(hit_mean)+'\nstd='+str(hit_std))
figstr = 'mean='+str(hit_mean)[0:6]+'\nstd='+str(hit_std)[0:6]

plt.text(hit_mean+5,10000,figstr,fontsize = 15)
plt.show()