import numpy as np
from matplotlib import pyplot as plt

import os

path = './oldASD/run4'
files = os.listdir(path + '/data_gen')
files_txt = [data[0:-11] for data in files if data[-10:] == 'decode.txt']
res_file = path + '/result.csv'
file_result = open(res_file, "w")

CHANNEL = 24
TDCID = 7
high_limit = 300
low_limit = 200
chl_ID = 21
for filename in files_txt:
    decodefilename = path + '/data_gen/' + filename + '_decode.txt'
    figname = path + '/plot/' + filename + '.png'
    file_decode = open(decodefilename, "r")
    leading = 0
    trailing = 0
    pair = 0
    width = []

    for line in file_decode.readlines():
        if line[0:5] == 'event':
            pair = 0
        elif line[0:9] == 'TDC' + str(TDCID).zfill(2) + 'CH'+str(chl_ID).zfill(2):
            if line[9] == 'L':
                pair = 1
                leading = int(line[11:], 10)
            elif line[9] == 'T' and pair == 1:
                pair = 0
                trailing = int(line[11:], 10)
                width.append(trailing - leading)
    file_decode.close()
    width_in_ns = [data * 0.1953125 for data in width]
    hit_width = [data for data in width_in_ns if low_limit < data < high_limit]
    hit_mean = np.mean(hit_width)
    hit_std = np.std(hit_width)
    file_result.write(str(hit_mean)+','+str(hit_std)+'\n')
    print('mean=' + str(hit_mean) + '\nstd=' + str(hit_std))
file_result.close()



