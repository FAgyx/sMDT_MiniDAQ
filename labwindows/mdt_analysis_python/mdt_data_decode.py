from matplotlib import pyplot as plt
from miniDAQ_lowlevel import *

# user input part
filename = 'run00188580_20190613'
path = './newASD/chambernoise'
ASDTYPE = 'newASD'
TDCTYPE = 'HPTDC'
TDCID = 8    # mezz ID
xlimleft = 0  # xlim parameter for plot
xlimright = 300
# user input part end

CHANNEL = 24
datfilename = path + '/data_origin/' + filename + '.dat'
txtfilename = path + '/data_origin/' + filename + '.txt'
wordfilename = path + '/data_gen/' + filename + '_word.txt'
decodefilename = path + '/data_gen/' + filename + '_decode.txt'
figname = path + '/plot/' + filename + '.png'

fileformat_dat_to_txt(datfilename,txtfilename)
print("dat conversion finished")
lineup_in_wordfile(txtfilename,wordfilename)
print("word lineup finished")
decode_wordfile(ASDTYPE, TDCTYPE, wordfilename, decodefilename)
print("data decode finished")


file_decode = open(decodefilename, "r")
leading = []
trailing = []
pair = []
width = []
for i in range(CHANNEL):
    width.append([])
    pair.append(0)
    leading.append(0)
    trailing.append(0)

for line in file_decode.readlines():
    if line[0:5] == 'event':
        for i in range(CHANNEL):
            pair[i] = 0
    elif line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
        chl_ID = int(line[7:9])
        if line[9] == 'L':
            pair[chl_ID] = 1
            leading[chl_ID] = int(line[11:], 10)
        elif line[9] == 'T' and pair[chl_ID] == 1:
            pair[chl_ID] = 0
            trailing[chl_ID] = int(line[11:], 10)
            width[chl_ID].append(trailing[chl_ID] - leading[chl_ID])

file_decode.close()
line = []
for i in range(CHANNEL):
    if len(width[i]):  # channel not empty
        counted = count_elements(width[i])
        time = [k*0.1953125 for k in sorted(counted.keys())]
        count = [counted[k] for k in sorted(counted.keys())]
        line.append(plt.bar(time, count, label=str(i)))
if len(line) <= 12:
    plt.legend(loc='upper right', handles=line)
elif len(line) > 12:
    first_legend = plt.legend(loc='upper left', handles=line[0:12])
    plt.gca().add_artist(first_legend)
    plt.legend(loc='upper right', handles=line[12:])

plt.xlim(xlimleft, xlimright)
plt.xlabel('Width (ns)')
plt.ylabel('Frequence')

plt.savefig(figname)
plt.show()
print('plot saved')
