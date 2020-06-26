from matplotlib import pyplot as plt
from miniDAQ_lowlevel import *

# user input part
filename = 'run00188580_20190613'
path = './newASD/chambernoise'
ASDTYPE = 'newASD'
TDCTYPE = 'HPTDC'
TDCID = 8    # mezz ID
trigger_rate = 1000000/1.5
# user input part end

CHANNEL = 24
datfilename = path + '/data_origin/' + filename + '.dat'
txtfilename = path + '/data_origin/' + filename + '.txt'
wordfilename = path + '/data_gen/' + filename + '_word.txt'
decodefilename = path + '/data_gen/' + filename + '_decode.txt'
figname = path + '/plot/' + filename + '_noise_rate.png'

# fileformat_dat_to_txt(datfilename,txtfilename)a
# print("dat conversion finished")
# lineup_in_wordfile(txtfilename,wordfilename)
# print("word lineup finished")
# decode_wordfile(ASDTYPE, TDCTYPE, wordfilename, decodefilename)
# print("data decode finished")

file_decode = open(decodefilename, "r")
trigger_count = 0
hit_count = CHANNEL*[0]
for line in file_decode.readlines():
    if line[0:5] == 'event':
        trigger_count = trigger_count + 1
    elif line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
        chl_ID = int(line[7:9])
        if line[9] == 'L':
            hit_count[chl_ID] = hit_count[chl_ID] + 1
noise_rate = [data*trigger_rate/trigger_count/1000 for data in hit_count]
plt.bar(range(CHANNEL),noise_rate)
plt.xlabel('CHANNEL #')
plt.ylabel('Noise Rate(kHz)')
# plt.ylim([0,10])
plt.savefig(figname)
plt.show()
print('plot saved')
