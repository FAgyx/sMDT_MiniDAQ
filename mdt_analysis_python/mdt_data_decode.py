from matplotlib import pyplot as plt
from miniDAQ_lowlevel import *
import os
import profile

# user input part
filename = 'run00188110_20190723'
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


# user input part end
CHANNEL = 24
datfilename = path + '/data_origin/' + filename + '.dat'
txtfilename = path + '/data_origin/' + filename + '.txt'
wordfilename = path + '/data_gen/' + filename + '_word.txt'
decodefilename = path + '/data_gen/' + filename + '_decode.txt'
figname = path + '/plot/' + filename + '.png'
folders = os.listdir(path)
if 'data_gen' not in folders:
    os.mkdir(path + '/data_gen')
if 'plot' not in folders:
    os.mkdir(path + '/plot')
plotfolders = os.listdir(path+'/plot')
folder_name = filename + '_chnl_plot'
figure_path = path + '/plot/' + folder_name
if folder_name not in plotfolders:
    os.mkdir(figure_path)

# # fileformat_dat_to_txt_chunk(datfilename,txtfilename)
# # print("dat conversion finished")
# # profile.run('fileformat_dat_to_txt_chunk(datfilename,txtfilename)')
# # profile.run('fileformat_dat_to_txt_1(datfilename,txtfilename)')
#
# # lineup_in_wordfile(txtfilename,wordfilename)
# # print("word lineup finished")
#
# # fileformat_dat_to_txt_chunk_and_lineup(datfilename,wordfilename)
# # print("word lineup finished")
# # decode_wordfile_mix(IDtype, wordfilename, decodefilename)
# # print("data decode finished")
#
dat_to_decode(IDtype, datfilename, decodefilename)
print("data decode finished")

# plot_ADC_TDC_perchnl_track(decodefilename, IDtype, filter_num=10, filter_noise=35, TDCID=1, triggerTDC=5,
#                                triggerchnl=23, leftID=0, rightID=2, width_select=1, xlim_left_ADC=-50,
#                                xlim_right_ADC=300, xlim_left_TDC=-200, xlim_right_TDC=500, ylim_top=3000,
#                                path=figure_path)
#



# for tracking plot

TDCID_top = [3,4,7]
TDCID_bottom = [0,1,2]
for i in range(len(TDCID_top)):
    TDCID = TDCID_top[i]
    leftID = TDCID_top[i-1] if i>0 else 99
    rightID = TDCID_top[i+1] if i <len((TDCID_top))-1 else 99
    plot_ADC_TDC_perchnl_track(decodefilename, IDtype, filter_num=10, filter_noise=35, TDCID=TDCID, triggerTDC=5,
                               triggerchnl=23, leftID=leftID, rightID=rightID, width_select=1, xlim_left_ADC=-50,
                               xlim_right_ADC=300, xlim_left_TDC=-200, xlim_right_TDC=500, ylim_top=3000,
                               path=figure_path)
    print("ADC and TDC plots finished for TDC" + str(TDCID).zfill(2))
    plt.close('all')

for i in range(len(TDCID_bottom)):
    TDCID = TDCID_bottom[i]
    leftID = TDCID_bottom[i-1] if i>0 else 99
    rightID = TDCID_bottom[i+1] if i <len((TDCID_bottom))-1 else 99
    plot_ADC_TDC_perchnl_track(decodefilename, IDtype, filter_num=10, filter_noise=35, TDCID=TDCID, triggerTDC=5,
                               triggerchnl=23, leftID=leftID, rightID=rightID, width_select=1, xlim_left_ADC=-50,
                               xlim_right_ADC=300, xlim_left_TDC=-200, xlim_right_TDC=500, ylim_top=3000, path=figure_path)
    print("ADC and TDC plots finished for TDC" + str(TDCID).zfill(2))
    plt.close('all')


# # for none tracking plot
# TDCID = [0,1,2,3,4,7]
# for i in TDCID:
#     #
#     plot_ADC_perchnl(decodefilename, IDtype, filter_num=4, filter_noise=35, TDCID=i, width_select=1, xlim_left=-50, xlim_right=300,
#                      ylim_top=3000, path=figure_path)
#     print("ADC plot finished for TDC" + str(i).zfill(2))
#     plot_TDC_perchnl(decodefilename, IDtype, filter_num=10, filter_noise=35, TDCID=i, triggerTDC=5, triggerchnl=23, xlim_left=-200,
#                      xlim_right=500, path=figure_path)
#     print("TDC plot finished for TDC" + str(i).zfill(2))


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
#

