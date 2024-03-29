from miniDAQ_lowlevel import *
from matplotlib import pyplot as plt


# user input part
filename = 'run00188496_20190610'
path = './newTDC/run2'
ASDTYPE = 'newASD'
TDCTYPE = 'AMT'
TDCID = 7    # mezz ID
CHNL1 = 2
CHNL2 = 21
offset = 10
xlimleft = offset - 10  # xlim parameter for plot
xlimright = offset + 10

# user input part end

CHANNEL = 24
datfilename = path + '/data_origin/' + filename + '.dat'
txtfilename = path + '/data_origin/' + filename + '.txt'
wordfilename = path + '/data_gen/' + filename + '_word.txt'
decodefilename = path + '/data_gen/' + filename + '_decode.txt'
figname = path + '/plot/' + filename + '_delay.png'

# fileformat_dat_to_txt(datfilename,txtfilename)
# print("dat conversion finished")
# lineup_in_wordfile(txtfilename,wordfilename)
# print("word lineup finished")
# decode_wordfile(ASDTYPE, TDCTYPE, wordfilename, decodefilename)
# print("data decode finished")

file_decode = open(decodefilename, "r")
leading1 = 0
leading2 = 0
pair = 0
width = []
chl1_count = 0
chl2_count = 0

for line in file_decode.readlines():
    if line[0:5] == 'event':
        pair = 0
    elif line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH' and line[9] == 'T':
        chl_ID = line[7:9]
        if line[7:9] == str(CHNL2).zfill(2):
            chl2_count = chl2_count + 1
            pair = 1
            leading2 = int(line[11:], 10)
        elif line[7:9] == str(CHNL1).zfill(2):
            chl1_count = chl1_count + 1
            if pair == 1:
                pair = 0
                leading1 = int(line[11:], 10)
                width.append(leading2 - leading1)
    elif line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH' and line[9] == 'W':
        chl_ID = line[7:9]
        if line[7:9] == str(CHNL2).zfill(2):
            chl2_count = chl2_count + 1
            pair = 1
            leading2 = int(line[15:], 10)
        elif line[7:9] == str(CHNL1).zfill(2):
            chl1_count = chl1_count + 1
            if pair == 1:
                pair = 0
                leading1 = int(line[15:], 10)
                width.append(leading2 - leading1)


file_decode.close()
print('ch'+str(CHNL1) + '_count = ' + str(chl1_count))
print('ch'+str(CHNL2) + '_count = ' + str(chl2_count))

if len(width):  # channel not empty
    counted = count_elements(width)
    time = [k*0.78125 for k in sorted(counted.keys())]
    count = [counted[k] for k in sorted(counted.keys())]
    plt.bar(time, count)

# plt.xlim(0, max(max(width))+100)

width_in_ns = [data*0.78125 for data in width]
# hit_width = [data for data in width_in_ns]
hit_width = [data for data in width_in_ns if offset-10 < data < offset+10]
hit_mean = np.mean(hit_width)
hit_std = np.std(hit_width)
print('mean='+str(hit_mean)+'\nstd='+str(hit_std))

figstr = 'mean='+str(hit_mean)[0:6]+'\nstd  ='+str(hit_std)[0:6]
plt.text(offset+1,0.9*max(count),figstr,fontsize = 15)
plt.xlim(xlimleft, xlimright)
plt.xlabel('Width (ns)')
plt.ylabel('Frequence')

plt.savefig(figname)
plt.show()