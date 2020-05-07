import numpy as np
from matplotlib import pyplot as plt
CHANNEL = 24
TDCID = 7

filename = 'run00188116_20190509'
decodefilename = './data_gen/' + filename + '_decode.txt'
figname = './plot/' + filename + '.png'
leading = 0
trailing = 0
pair = 0
width = []


def count_elements(seq):
    hist = {}
    for i in seq:
        hist[i] = hist.get(i, 0) + 1
    return hist


for i in range(CHANNEL):
    file_decode = open(decodefilename,"r")
    width.append([])
    for line in file_decode.readlines():
        if line[0:5] == 'event':
            if pair == 2:  # unique matching in one event
                width[i].append(trailing - leading)
            leading = 0  # reset
            trailing = 0
            pair = 0
        elif line[0:9] == 'TDC' + str(TDCID) + 'CH' + str(i) + 'L':
            if pair == 0:  # leading edge found
                leading = int(line[10:], 10)
                pair = 1
            else:
                print('pair warning ' + str(pair) + ' for leading edge')
                pair = 3  # leading edge not unique in one event
        elif line[0:9] == 'TDC' + str(TDCID) + 'CH' + str(i) + 'T':
            if pair == 1:
                trailing = int(line[10:], 10)
                pair = 2   # trailing edge found

            else:
                print('pair warning '+str(pair)+' for trailing edge')
                pair = 4   # trailing edge not paired
    file_decode.close()
    if len(width[i]):  # channel not empty
        counted = count_elements(width[i])
        time = [k*0.1953125 for k in sorted(counted.keys())]
        count = [counted[k] for k in sorted(counted.keys())]
        plt.bar(time, count, label=str(i))

plt.legend()
plt.xlim(0,300)
plt.xlabel('Width (ns)')
plt.ylabel('Frequence')

plt.savefig(figname)
plt.show()
