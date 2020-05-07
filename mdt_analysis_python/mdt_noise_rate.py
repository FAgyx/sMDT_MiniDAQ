from matplotlib import pyplot as plt
from miniDAQ_lowlevel import *
import os

# user input part
filename = 'run00188118_20190725'
path = '../data/cosmic_ray'
IDtype = []
IDtype.append([0,'oldASD','HPTDC'])
IDtype.append([1,'oldASD','HPTDC'])
IDtype.append([2,'oldASD','HPTDC'])
IDtype.append([3,'oldASD','HPTDC'])
IDtype.append([4,'oldASD','HPTDC'])
IDtype.append([5,'oldASD','HPTDC'])
IDtype.append([7,'oldASD','HPTDC'])
IDtype.append([8,'oldASD','HPTDC'])
IDtype.append([9,'oldASD','HPTDC'])
IDtype.append([10,'oldASD','HPTDC'])

matchwindow = 1.55/1000000
# TDCID = [8]
# user input part end

CHANNEL = 24
datfilename = path + '/data_origin/' + filename + '.dat'
txtfilename = path + '/data_origin/' + filename + '.txt'
wordfilename = path + '/data_gen/' + filename + '_word.txt'
decodefilename = path + '/data_gen/' + filename + '_decode.txt'

folders = os.listdir(path+'/plot')
folder_name = filename + '_chnl_plot'
figure_path = path + '/plot/' + folder_name
if folder_name not in folders:
    os.mkdir(figure_path)
dat_to_decode(IDtype, datfilename, decodefilename)
print("data decode finished")

TDCID = [0,1,2,3,4,7]    # mezz ID
for TDC in TDCID:
    file_decode = open(decodefilename, "r")
    figname = figure_path + '/TDC'+str(TDC).zfill(2)+'_noise_rate.png'
    trigger_count = 0
    hit_count = CHANNEL*[0]
    for line in file_decode.readlines():
        if line[0:1] == 'e':
            trigger_count = trigger_count + 1
        elif line[0:7] == 'TDC' + str(TDC).zfill(2) + 'CH':
            chl_ID = int(line[7:9])
            if line[9] == 'L' or line[9] == 'W':
                hit_count[chl_ID] = hit_count[chl_ID] + 1
    noise_rate = [data/matchwindow/trigger_count/1000 for data in hit_count]   # kHz
    plt.bar(range(CHANNEL),noise_rate)
    plt.xlabel('CHANNEL #')
    plt.ylabel('Noise Rate(kHz)')
    # plt.ylim([0,3])
    plt.savefig(figname)
    plt.show(block=False)
    plt.clf()

