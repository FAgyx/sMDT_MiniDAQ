from miniDAQ_lowlevel import *
from matplotlib import pyplot as plt


# user input part
filename = 'run00188475_20190610'
path = './newTDC/run1'
ASDTYPE = 'newASD'
TDCTYPE = 'AMT'
TDCID = 7    # mezz ID
CHNL1 = 2
CHNL2 = 21
offset = 5
xlimleft = offset - 20  # xlim parameter for plot
xlimright = offset + 20

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
leading1 = []
leading2 = []
pair = 0
width = []
chl1_count = 0
chl2_count = 0

for line in file_decode.readlines():
    if line[0:5] == 'event':
        pair = 0
    elif line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH' and line[9] == 'T':
        chl_ID = line[7:9]
        if line[7:9] == str(CHNL1).zfill(2):
            leading1.append(int(line[11:], 10))
        elif line[7:9] == str(CHNL2).zfill(2):
            leading2.append(int(line[11:], 10))
leading1_not4 = [k for k in leading1 if k % 4 != 0]
leading2_not4 = [k for k in leading2 if k % 4 != 0]
print('leading1 not 4 length ='+str(len(leading1_not4)))
print(leading1_not4)
print('leading2 not 4 length ='+str(len(leading2_not4)))
print(leading2_not4)