import numpy as np
from miniDAQ_lowlevel import *
from matplotlib import pyplot as plt

import os
path = './newTDC/noise_rate_position_compare/newASD_2'
TDCTYPE = 'AMT'
files = os.listdir(path+'/data_gen')
files_txt = [data[0:-11] for data in files if data[-10:] == 'decode.txt']

CHANNEL = 24
TDCID = 8
plot_flag = 0
matchwindow = 1.5/1000000

for filename in files_txt:
    decodefilename = path + '/data_gen/' + filename + '_decode.txt'
    figname = path + '/plot/' + filename + '_noiserate.png'
    file_decode = open(decodefilename, "r")
    trigger_count = 0
    hit_count = CHANNEL * [0]
    for line in file_decode.readlines():
        if line[0:5] == 'event':
            trigger_count = trigger_count + 1
        elif line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
            chl_ID = int(line[7:9])
            if line[9] == 'L' or line[9] == 'W':
                hit_count[chl_ID] = hit_count[chl_ID] + 1
    # hit_count[0] = 0
    noise_rate = [data / matchwindow / trigger_count / 1000 for data in hit_count]  # kHz
    plt.bar(range(CHANNEL), noise_rate)
    plt.xlabel('CHANNEL #')
    plt.ylabel('Noise Rate(kHz)')
    # plt.ylim([0,10])
    plt.savefig(figname)
    # plt.show()
    plt.figure()
    print("plot finished for" + filename)

