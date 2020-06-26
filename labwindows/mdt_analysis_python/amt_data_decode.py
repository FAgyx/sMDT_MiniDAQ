from matplotlib import pyplot as plt
from miniDAQ_lowlevel import *

# user input part
filename = 'run00188599_20190624'
path = '../newASD_newTDC/ch2_curr'
ASDTYPE = 'newASD'
TDCTYPE = 'AMT'
TDCID = 0    # mezz ID
xlimleft = 0  # xlim parameter for plot
xlimright = 400
low_limit = 0
high_limit = 400
# user input part end

CHANNEL = 24
datfilename = path + '/data_origin/' + filename + '.dat'
txtfilename = path + '/data_origin/' + filename + '.txt'
wordfilename = path + '/data_gen/' + filename + '_word.txt'
decodefilename = path + '/data_gen/' + filename + '_decode.txt'
figname = path + '/plot/' + filename + '.png'

fileformat_dat_to_txt(datfilename,txtfilename)
print("format conversion finished")
lineup_in_wordfile(txtfilename,wordfilename)
print("word lineup finished")
decode_wordfile(ASDTYPE, TDCTYPE, wordfilename, decodefilename)
print("data decode finished")


file_decode = open(decodefilename, "r")
leading = []
trailing = []
width = []
for i in range(CHANNEL):
    width.append([])
    leading.append(0)
    trailing.append(0)

for line in file_decode.readlines():
    if line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
        chl_ID = int(line[7:9])
        if line[9] == 'W':
            width[chl_ID].append(int(line[11:14],10))
        if line[9] == 'L' or line[9] == 'T':
            width[chl_ID].append(1)


file_decode.close()

for i in range(CHANNEL):
    if len(width[i]):  # channel not empty
        counted = count_elements(width[i])
        time = [k*0.78125*2 for k in sorted(counted.keys())]
        count = [counted[k] for k in sorted(counted.keys())]
        plt.bar(time, count, label=str(i))
        width_in_ns = [data * 0.78125 for data in width[i]]
        hit_width = [data for data in width_in_ns if low_limit < data < high_limit]
        hit_mean = np.mean(hit_width)
        hit_std = np.std(hit_width)
        print('chl'+str(i).zfill(2)+' count ='+str(len(width[i]))+' mean=' + str(hit_mean)[0:8] + ' std=' + str(hit_std)[0:8])

plt.legend()
# plt.xlim(0, max(max(width))+100)
plt.xlim(xlimleft, xlimright)
plt.xlabel('Width (ns)')
plt.ylabel('Frequence')

plt.savefig(figname)
plt.show()
print('finished for '+filename )

