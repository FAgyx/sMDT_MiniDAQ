from matplotlib import pyplot as plt
from miniDAQ_lowlevel import *
import os

# user input part
filename = 'run00188603_20190707'
path = '../data/cosmic_ray'
ASDTYPE = 'oldASD'
TDCTYPE = 'HPTDC'
TDCID = 9    # mezz ID
xlimleft = 35  # xlim parameter for plot
time_step = 0.78125*2
xlim_right = 300
# user input part end

CHANNEL = 24
datfilename = path + '/data_origin/' + filename + '.dat'
txtfilename = path + '/data_origin/' + filename + '.txt'
wordfilename = path + '/data_gen/' + filename + '_word.txt'
decodefilename = path + '/data_gen/' + filename + '_decode_TDC'+str(TDCID).zfill(2)+'.txt'
figname = path + '/plot/' + filename + '_HPTDC.png'

# fileformat_dat_to_txt(datfilename,txtfilename)
# print("dat conversion finished")
# lineup_in_wordfile(txtfilename,wordfilename)
# print("word lineup finished")
# decode_wordfile(ASDTYPE, TDCTYPE, wordfilename, decodefilename)
# print("data decode finished")


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

if TDCTYPE == 'HPTDC':
    time_step = 0.1953125
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
elif TDCTYPE == 'AMT':
    for line in file_decode.readlines():
        if line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
            chl_ID = int(line[7:9])
            if line[9] == 'W':
                width[chl_ID].append(int(line[11:14], 10))
file_decode.close()



# plot_flag = 0
# line = []
# width[0]=[]
# # width[4]=[]
# for i in range(CHANNEL):
# # i=1
#     if len(width[i]):  # channel not empty
#         plot_flag = 1
#         counted = count_elements(width[i])
#         time = [k * time_step for k in sorted(counted.keys())]
#         count = [counted[k] for k in sorted(counted.keys())]
#         line.append(plt.bar(time, count, label=str(i)))
#         width_in_ns = [data * time_step for data in width[i]]
#         hit_width = [data for data in width_in_ns]
#         # hit_width = [data for data in width_in_ns if offset - 10 < data < offset + 10]
#         hit_mean = np.mean(hit_width)
#         hit_std = np.std(hit_width)
#         print('mean=' + str(hit_mean) + '\nstd=' + str(hit_std))
#
# if plot_flag == 1:
#     if len(line) <= 12:
#         plt.legend(loc='upper right', handles=line)
#     elif len(line) > 12:
#         first_legend = plt.legend(loc='upper left', handles=line[0:12])
#         plt.gca().add_artist(first_legend)
#         plt.legend(loc='upper right', handles=line[12:])
#     # plt.xlim(0, max(max(width))+100)
#     plt.xlim(xlimleft, xlim_right)
#     plt.xlabel('Width (ns)')
#     plt.ylabel('Frequence')
#     # figstr = 'mean=' + str(hit_mean)[0:6] + '\nstd  =' + str(hit_std)[0:6]
#     # plt.text(hit_mean - 150, 0.9 * max(count), figstr, fontsize=15)
#     plt.savefig(figname)
#     plt.show()
#

folders = os.listdir(path+'/plot')
folder_name = filename + '_chnl_plot'
if folder_name not in folders:
    os.mkdir(path + '/plot/' + folder_name)
for i in range(CHANNEL):
    if len(width[i]):  # channel not empty
        plot_flag = 1
        counted = count_elements(width[i])
        time = [k * time_step for k in sorted(counted.keys())]
        count = [counted[k] for k in sorted(counted.keys())]
        plt.bar(time, count, label=str(i))
        width_in_ns = [data * time_step for data in width[i]]
        hit_width = [data for data in width_in_ns]
        # hit_width = [data for data in width_in_ns if offset - 10 < data < offset + 10]
        hit_mean = np.mean(hit_width)
        hit_std = np.std(hit_width)
        print('mean=' + str(hit_mean) + '\nstd=' + str(hit_std))

        plt.legend()
        plt.xlim(xlimleft, xlim_right)
        plt.xlabel('Width (ns)')
        plt.ylabel('Frequence')
        # figstr = 'mean=' + str(hit_mean)[0:6] + '\nstd  =' + str(hit_std)[0:6]
        # plt.text(hit_mean - 150, 0.9 * max(count), figstr, fontsize=15)
        plt.savefig(path + '/plot/' + folder_name + '/TDC'+str(TDCID).zfill(2)+'chnl' + str(i).zfill(2)+ '.png')
        plt.show(block = False)
        plt.figure()
