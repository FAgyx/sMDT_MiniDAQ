import numpy as np
from matplotlib import pyplot as plt
import os

def count_elements(seq):
    hist = {}
    for i in seq:
        hist[i] = hist.get(i, 0) + 1
    return hist

    
def fileformat_dat_to_txt(datfilename, txtfilename):
    # change file format from .dat to .txt for a single file
    with open(datfilename,'rb') as fid:
        data_array = np.fromfile(fid, np.uint8)
    file_txt = open(txtfilename, "w")
    for i in range(len(data_array)):
        dataword1 = format(data_array[i], '02x')
        file_txt.write(dataword1)
        if (i + 1) % 16 == 0:
            file_txt.write('\n')
        elif (i + 1) % 2 == 0:
            file_txt.write(' ')
    file_txt.close()
    return


def fileformat_dat_to_txt_1(datfilename, txtfilename):
    # change file format from .dat to .txt for a single file
    with open(datfilename,'rb') as fid:
        with open(txtfilename, 'w') as file_txt:
            data_num = 0
            for data in np.fromfile(fid, np.uint8):
                dataword1 = format(data, '02x')
                file_txt.write(dataword1)
                data_num = data_num + 1
                if data_num % 16 == 0:
                    file_txt.write('\n')
                elif data_num % 2 == 0:
                    file_txt.write(' ')
    return


def fileformat_dat_to_txt_2(datfilename, txtfilename):
    # change file format from .dat to .txt for a single file
    with open(datfilename,'rb') as fid:
        with open(txtfilename, 'w') as file_txt:
            data_num = 0
            line = ''
            for data in np.fromfile(fid, np.uint8):
                line += format(data, '02x')
                data_num = data_num + 1
                if data_num % 16 == 0:
                    line += '\n'
                    file_txt.write(line)
                    line = ''
                elif data_num % 2 == 0:
                    line += ' '

    return




def fileformat_dat_to_txt_chunk(datfilename, txtfilename):
    # change file format from .dat to .txt for a single file
    # same performance as 1
    with open(datfilename,'rb') as fid:
        with open(txtfilename, 'w') as file_txt:
            for data in iter(lambda: fid.read(4096), b''):
                line = ''
                for i in range(len(data)):
                    line += format(ord(data[i]),'02x')
                    if (i+1) % 16 == 0:
                        line += '\n'
                        file_txt.write(line)
                        line = ''
                    elif (i+1) % 2 == 0:
                        line += ' '
    return


def fileformat_dat_to_txt_chunk_and_lineup(datfilename,wordfilename):
    # change file format from .dat to .txt for a single file
    with open(datfilename,'rb') as fid:
        with open(wordfilename, 'w') as file_word:
            for data in iter(lambda: fid.read(4096), b''):
                li = ''
                for i in range(len(data)):
                    li += format(ord(data[i]),'02x')
                    if (i+1) % 16 == 0:
                        dataword1 = li[6:8] +   li[4:6] +   li[2:4] +   li[0:2] + \
                                    li[14:16] + li[12:14] + li[10:12] + li[8:10] + \
                                    li[22:24] + li[20:22] + li[18:20] + li[16:18] + \
                                    li[30:32] + li[28:30] + li[26:28] + li[24:26]

                        file_word.write(str(dataword1[0:8]) + '\n' + str(dataword1[8:16]) + '\n' + str(dataword1[16:24]) + '\n' + \
                            str(dataword1[24:32]) + '\n')
                        li = ''
    return


def dat_to_decode(IDtype, datfilename, decodefilename):
    # change file format from .dat to .txt for a single file
    type = {}
    for ID in IDtype:
        if ID[2] == 'HPTDC' and ID[1] == 'oldASD':
            type.update({ID[0]: 0})
        elif ID[2] == 'HPTDC' and ID[1] == 'newASD':
            type.update({ID[0]: 1})
        elif ID[2] == 'AMT' and ID[1] == 'oldASD':
            type.update({ID[0]: 2})
        elif ID[2] == 'AMT' and ID[1] == 'newASD':
            type.update({ID[0]: 3})
    with open(datfilename,'rb') as fid:
        with open(decodefilename, 'w') as file_decode:
            for data in iter(lambda: fid.read(4096), b''):
                li = ''
                for i in range(len(data)):
                    li += format(ord(data[i]),'02x')
                    if (i+1) % 16 == 0:
                        dataword1 = li[6:8] +   li[4:6] +   li[2:4] +   li[0:2] + \
                                    li[14:16] + li[12:14] + li[10:12] + li[8:10] + \
                                    li[22:24] + li[20:22] + li[18:20] + li[16:18] + \
                                    li[30:32] + li[28:30] + li[26:28] + li[24:26]
                        file_decode.write(decode_word_mix(type, dataword1[0:8]))
                        file_decode.write(decode_word_mix(type, dataword1[8:16]))
                        file_decode.write(decode_word_mix(type, dataword1[16:24]))
                        file_decode.write(decode_word_mix(type, dataword1[24:32]))

                        li = ''
    return


def lineup_in_wordfile(txtfilename, wordfilename):
    # reorganize raw data file to TDC words
    file_txt = open(txtfilename,"r")
    file_word = open(wordfilename,"w")
    for li in file_txt:
        dataword1 = li[7:9] + li[5:7] + li[2:4] + li[0:2] + li[17:19] + li[15:17] + li[12:14] + \
            li[10:12] + li[27:29] + li[25:27] + li[22:24] + li[20:22] + li[37:39] + li[35:37] + li[32:34] + li[30:32]
        file_word.writelines(str(dataword1[0:8]) + '\n' + str(dataword1[8:16]) + '\n' + str(dataword1[16:24]) + '\n' +\
            str(dataword1[24:32]) + '\n')
    file_word.close()
    file_txt.close()


def decode_wordfile(ASDtype, TDCtype, wordfilename, decodefilename):
    # decode TDC words into event and hit information
    if ASDtype == 'newASD':
        LEADING_FLAG = '5'
        TRAINLING_FLAG = '4'
    elif ASDtype == 'oldASD':
        LEADING_FLAG = '4'
        TRAINLING_FLAG = '5'
    else:
        print('please input correct ASD type in str! (\'newASD\' or \'oldASD\')')
        return
    file_word = open(wordfilename,"r")
    file_decode = open(decodefilename,"w")
    if TDCtype == 'HPTDC':
        for line in file_word:
            if line[0:2] == '00':
                file_decode.write('e'+str(int(line[2:5], 16)).zfill(4) + ' '+str(int(line[5:8], 16))+'\n')
            elif line[0] == LEADING_FLAG:
                line_bin = format(int(line, 16), '032b')
                TDC_id = int(line_bin[4:8], 2)
                Chl_id = int(line_bin[8:13], 2)
                leading = int(line_bin[14:32], 2)
                file_decode.write('TDC'+str(TDC_id).zfill(2)+'CH'+str(Chl_id).zfill(2)+'L\t'+str(leading)+'\n')
            elif line[0] == TRAINLING_FLAG:
                line_bin = format(int(line, 16), '032b')
                TDC_id = int(line_bin[4:8], 2)
                Chl_id = int(line_bin[8:13], 2)
                trailing = int(line_bin[14:32], 2)
                file_decode.write('TDC' + str(TDC_id).zfill(2) + 'CH' + str(Chl_id).zfill(2) + 'T\t' + str(trailing) + '\n')
    elif TDCtype == 'AMT':
        SINGLE_FLAG = '3'
        PAIR_FLAG = '4'
        for line in file_word:
            if line[0:2] == '00':
                file_decode.write('e'+str(int(line[2:5], 16)).zfill(4) + ' '+str(int(line[5:8], 16))+'\n')
            elif line[0] == SINGLE_FLAG:
                line_bin = format(int(line, 16), '032b')
                TDC_id = int(line_bin[4:8], 2)
                Chl_id = int(line_bin[8:13], 2)
                edge_type = 'L' if int(line_bin[13:14], 2) else 'T'
                error = int(line_bin[14:15])
                edge_type = 'E' if error else edge_type
                edge = int(line_bin[15:32], 2)
                file_decode.write('TDC'+str(TDC_id).zfill(2)+'CH'+str(Chl_id).zfill(2)+edge_type+'\t'+str(edge)+'\n')
            elif line[0] == PAIR_FLAG:
                line_bin = format(int(line, 16), '032b')
                TDC_id = int(line_bin[4:8], 2)
                Chl_id = int(line_bin[8:13], 2)
                width = int(line_bin[13:21], 2)
                edge = int(line_bin[21:32], 2)                         
                file_decode.write('TDC'+str(TDC_id).zfill(2)+'CH'+str(Chl_id).zfill(2)+'W\t'+str(width).zfill(3)+'\t'+str(edge)+'\n')
    else:
        print('please input correct TDC type in str! (\'HPTDC\' or \'AMT\')')
        return
    file_word.close()
    file_decode.close()
    return



def decode_wordfile_mix(IDtype, wordfilename, decodefilename):
    # decode TDC words into event and hit information
    type = {}
    for ID in IDtype:
        if ID[2] == 'HPTDC' and ID[1] == 'oldASD':
            type.update({ID[0]:0})
        elif ID[2] == 'HPTDC' and ID[1] == 'newASD':
            type.update({ID[0]:1})
        elif ID[2] == 'AMT' and ID[1] == 'oldASD':
            type.update({ID[0]:2})
        elif ID[2] == 'AMT' and ID[1] == 'newASD':
            type.update({ID[0]:3})
    file_word = open(wordfilename, "r")
    file_decode = open(decodefilename, "w")
    for line in file_word:
        if line[0:2] == '00':
            file_decode.write('e'+str(int(line[2:5], 16)).zfill(4) + ' '+str(int(line[5:8], 16))+'\n')
        elif line[0] in ['3', '4', '5']:
            line_bin = format(int(line, 16), '032b')
            TDC_id = int(line_bin[4:8], 2)
            Chl_id = int(line_bin[8:13], 2)
            edge_type = 'N'
            if type[TDC_id] == 0 or type[TDC_id] == 1:  # oldASD, HPTDC, rising edge first
                edge = int(line_bin[14:32], 2)
                if line[0] == '4':
                    edge_type = 'L' if type[TDC_id] == 0 else 'T'
                elif line[0] == '5':
                    edge_type = 'T' if type[TDC_id] == 0 else 'L'
                file_decode.write('TDC'+str(TDC_id).zfill(2)+'CH'+str(Chl_id).zfill(2)+edge_type+'\t'+str(edge)+'\n')
            elif type[TDC_id] == 2 or type[TDC_id] == 3: # AMT
                if line[0] == '3': # edge mode
                    edge_type = 'L' if int(line_bin[13:14], 2) else 'T'
                    error = int(line_bin[14:15])
                    edge_type = 'E' if error else edge_type
                    edge = int(line_bin[15:32], 2)
                    file_decode.write('TDC'+str(TDC_id).zfill(2)+'CH'+str(Chl_id).zfill(2)+edge_type+'\t'+str(edge)+'\n')
                elif line[0] == '4': # pair mode
                    width = int(line_bin[13:21], 2)
                    edge = int(line_bin[21:32], 2)
                    file_decode.write('TDC'+str(TDC_id).zfill(2)+'CH'+str(Chl_id).zfill(2)+'W\t'+str(width).zfill(3)+'\t'+str(edge)+'\n')
    file_word.close()
    file_decode.close()
    return



def decode_word_mix(type, line):
    # decode TDC words into event and hit information
    if line[0:2] == '00':
        return('e'+str(int(line[2:5], 16)).zfill(4) + ' '+str(int(line[5:8], 16))+'\n')
    elif line[0] in ['3', '4', '5']:
        line_bin = format(int(line, 16), '032b')
        TDC_id = int(line_bin[4:8], 2)
        Chl_id = int(line_bin[8:13], 2)
        edge_type = 'N'
        if type[TDC_id] == 0 or type[TDC_id] == 1:  # oldASD, HPTDC, rising edge first
            edge = int(line_bin[14:32], 2)
            if line[0] == '4':
                edge_type = 'L' if type[TDC_id] == 0 else 'T'
            elif line[0] == '5':
                edge_type = 'T' if type[TDC_id] == 0 else 'L'
            return('TDC'+str(TDC_id).zfill(2)+'CH'+str(Chl_id).zfill(2)+edge_type+'\t'+str(edge)+'\n')
        elif type[TDC_id] == 2 or type[TDC_id] == 3: # AMT
            if line[0] == '3': # edge mode
                edge_type = 'L' if int(line_bin[13:14], 2) else 'T'
                error = int(line_bin[14:15])
                edge_type = 'E' if error else edge_type
                edge = int(line_bin[15:32], 2)
                return('TDC'+str(TDC_id).zfill(2)+'CH'+str(Chl_id).zfill(2)+edge_type+'\t'+str(edge)+'\n')
            elif line[0] == '4': # pair mode
                width = int(line_bin[13:21], 2)
                edge = int(line_bin[21:32], 2)
                return('TDC'+str(TDC_id).zfill(2)+'CH'+str(Chl_id).zfill(2)+'W\t'+str(width).zfill(3)+'\t'+str(edge)+'\n')
    else:
        return('')


def get_width(decodefilename, TDCID, IDtype, filter_num):
    TDCTYPE = [line for line in IDtype if TDCID in line][0][2]
    CHANNEL = 24
    file_decode = open(decodefilename, "r")
    leading = []
    trailing = []
    pair = []
    width = []
    coin = 0
    for i in range(CHANNEL):
        width.append([])
        pair.append(0)
        leading.append(0)
        trailing.append(0)
    if TDCTYPE == 'HPTDC':
        for line in file_decode:
            if line[0:1] == 'e':
                if int(line[6:]) >= filter_num:
                    coin = 1
                    for i in range(CHANNEL):
                        pair[i] = 0
                else:
                    coin = 0
            elif coin == 1 and line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
                chl_ID = int(line[7:9])
                if line[9] == 'L':
                    pair[chl_ID] = 1
                    leading[chl_ID] = int(line[11:], 10)
                elif line[9] == 'T' and pair[chl_ID] == 1:
                    pair[chl_ID] = 0
                    trailing[chl_ID] = int(line[11:], 10)
                    width[chl_ID].append(trailing[chl_ID] - leading[chl_ID])
    elif TDCTYPE == 'AMT':
        for line in file_decode:
            if line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
                chl_ID = int(line[7:9])
                if line[9] == 'W':
                    width[chl_ID].append(int(line[11:14], 10))
    file_decode.close()
    return width


def get_width_hist(decodefilename, TDCID, IDtype, filter_num):
    TDCTYPE = [line for line in IDtype if TDCID in line][0][2]
    CHANNEL = 24
    with open(decodefilename, 'r') as file_decode:
        leading = []
        trailing = []
        pair = []
        width = []
        coin = 0
        for i in range(CHANNEL):
            width.append({})
            pair.append(0)
            leading.append(0)
            trailing.append(0)
        if TDCTYPE == 'HPTDC':
            for line in file_decode:
                if line[0:1] == 'e':
                    if int(line[6:]) >= filter_num:
                        coin = 1
                        for i in range(CHANNEL):
                            pair[i] = 0
                    else:
                        coin = 0
                elif coin == 1 and line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
                    chl_ID = int(line[7:9])
                    if line[9] == 'L':
                        pair[chl_ID] = 1
                        leading[chl_ID] = int(line[11:], 10)
                    elif line[9] == 'T' and pair[chl_ID] == 1:
                        pair[chl_ID] = 0
                        trailing[chl_ID] = int(line[11:], 10)
                        width_tmp = trailing[chl_ID] - leading[chl_ID]
                        width[chl_ID][width_tmp] = width[chl_ID].get(width_tmp, 0) + 1
        elif TDCTYPE == 'AMT':
            for line in file_decode:
                if line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
                    chl_ID = int(line[7:9])
                    if line[9] == 'W':
                        width_tmp = int(line[11:14], 10)
                        width[chl_ID][width_tmp] = width[chl_ID].get(width_tmp, 0) + 1

    return width


def get_width_hist_deletenoise(decodefilename, TDCID, IDtype, filter_num, filter_noise, width_select):
    TDCTYPE = [line for line in IDtype if TDCID in line][0][2]
    CHANNEL = 24
    time_step_HPTDC = 0.1953125
    time_step_AMT = 2 ** width_select * 0.78125
    with open(decodefilename, 'r') as file_decode:
        leading = []
        trailing = []
        pair = []
        width = []
        coin = 0
        for i in range(CHANNEL):
            width.append({})
            pair.append(0)
            leading.append(0)
            trailing.append(0)
        if TDCTYPE == 'HPTDC':
            for line in file_decode:
                if line[0:1] == 'e':
                    if int(line[6:]) >= filter_num:
                        coin = 1
                        for i in range(CHANNEL):
                            pair[i] = 0
                    else:
                        coin = 0
                elif coin == 1 and line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
                    chl_ID = int(line[7:9])
                    if line[9] == 'L':
                        pair[chl_ID] = 1
                        leading[chl_ID] = int(line[11:], 10)
                    elif line[9] == 'T' and pair[chl_ID] == 1:
                        pair[chl_ID] = 0
                        trailing[chl_ID] = int(line[11:], 10)
                        width_tmp = trailing[chl_ID] - leading[chl_ID]
                        if width_tmp*time_step_HPTDC>filter_noise:
                            width[chl_ID][width_tmp] = width[chl_ID].get(width_tmp, 0) + 1
        elif TDCTYPE == 'AMT':
            for line in file_decode:
                if line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
                    chl_ID = int(line[7:9])
                    if line[9] == 'W':
                        width_tmp = int(line[11:14], 10)
                        if width_tmp * time_step_AMT > filter_noise:
                            width[chl_ID][width_tmp] = width[chl_ID].get(width_tmp, 0) + 1

    return width


def chnl_tracking(chnl_orig):
    chnl = [data + 24 if data < 48 else data-48 for data in chnl_orig]
    # check_track_plot(chnl)
    row3 = [data * 4 + 12 for data in range(12)]
    row2 = [data * 4 + 13 for data in range(12)]
    row1 = [data * 4 + 14 for data in range(12)]
    row0 = [data * 4 + 15 for data in range(12)]
    row0_data = [data for data in chnl if data in row0]
    row1_data = [data for data in chnl if data in row1]
    row2_data = [data for data in chnl if data in row2]
    row3_data = [data for data in chnl if data in row3]
    tracked = []
    if not len(row0_data) or not len(row1_data) or not len(row2_data) or not len(row3_data):
        # check_track_plot([])
        return []
    else:
        for data0 in row0_data:
            data1_tracked = []
            data2_tracked = []
            data3_tracked = []
            for data1 in row1_data:
                if data1 in [data0-1, data0-5, data0+3] and data1 not in data1_tracked:
                    data1_tracked.append(data1)
            for data1 in data1_tracked:
                for data2 in row2_data:
                    if data2 in [data1-1, data1-5, data1+3] and data2 not in data2_tracked:
                        data2_tracked.append(data2)
            for data2 in data2_tracked:
                for data3 in row3_data:
                    if data3 in [data2-1, data2-5, data2+3] and data3 not in data3_tracked:
                        data3_tracked.append(data3)
            if len(data3_tracked):
                tracked.append(data0)
                for data in data1_tracked:
                    if data not in tracked:
                        tracked.append(data)
                for data in data2_tracked:
                    if data not in tracked:
                        tracked.append(data)
                for data in data3_tracked:
                    if data not in tracked:
                        tracked.append(data)
                # check_track_plot(tracked)
                tracked_orig = [data - 24 if data >= 24 else data + 48 for data in tracked]
                tracked_target = [data for data in tracked_orig if data < 24]
                return tracked_target
            else:
                return []


def get_width_timediff_hist_track_deletenoise(decodefilename, TDCID, triggerTDC, triggerchnl, leftID, rightID, IDtype, filter_num, filter_noise, width_select):
    TDCTYPE = [line for line in IDtype if TDCID in line][0][2]
    CHANNEL = 24
    time_step = 0.1953125 if TDCTYPE == 'HPTDC' else 2 ** width_select * 0.78125
    filter_noise = filter_noise / time_step
    with open(decodefilename, 'r') as file_decode:
        leading = []
        trailing = []
        pair = []
        width = []
        width_event = []
        coin = 0
        chnl_event = []
        chnl_tracked = []
        time_diff = []
        trigger_found = 0
        trigger_time = 0
        for i in range(CHANNEL):
            width.append({})
            time_diff.append({})
            width_event.append([])
            pair.append(0)
            pair.append(0)
            pair.append(0)
            leading.append(0)
            leading.append(0)
            leading.append(0)
            trailing.append(0)
            trailing.append(0)
            trailing.append(0)
        if TDCTYPE == 'HPTDC':
            for line in file_decode:
                if line[0:1] == 'e':
                    # event valid
                    if len(chnl_event) >= 4:
                        chnl_tracked = chnl_tracking(chnl_event)
                        for chl_ID in chnl_tracked:
                            for width_tmp in width_event[chl_ID]:
                                width[chl_ID][width_tmp] = width[chl_ID].get(width_tmp, 0) + 1
                        if trigger_found:
                            for chl_ID in chnl_tracked:
                                time_diff_tmp = leading[chl_ID] % 8192 * time_step - trigger_time
                                time_diff[chl_ID][time_diff_tmp] = time_diff[chl_ID].get(time_diff_tmp, 0) + 1
                    # event reset
                    trigger_found = 0
                    chnl_event = []
                    for i in range(CHANNEL):
                        width_event[i] = []
                    if int(line[6:]) >= filter_num:
                        coin = 1
                        for i in range(CHANNEL*3):
                            pair[i] = 0
                            leading[i] = 0
                    else:
                        coin = 0
                elif coin == 1 and line[0:5] == 'TDC' + str(TDCID).zfill(2):
                    chl_ID = int(line[7:9])
                    if line[9] == 'L':
                        pair[chl_ID] = 1
                        leading[chl_ID] = int(line[11:], 10)
                    elif line[9] == 'T' and pair[chl_ID] == 1:
                        pair[chl_ID] = 0
                        trailing[chl_ID] = int(line[11:], 10)
                        width_tmp = trailing[chl_ID] - leading[chl_ID]
                        if width_tmp > filter_noise:
                            width_event[chl_ID].append(width_tmp)
                            chnl_event.append(chl_ID)
                elif coin == 1 and line[0:5] == 'TDC' + str(leftID).zfill(2):
                    chl_ID = int(line[7:9])
                    if line[9] == 'L':
                        pair[chl_ID+24] = 1
                        leading[chl_ID+24] = int(line[11:], 10)
                    elif line[9] == 'T' and pair[chl_ID+24] == 1:
                        pair[chl_ID+24] = 0
                        trailing[chl_ID+24] = int(line[11:], 10)
                        width_tmp = trailing[chl_ID+24] - leading[chl_ID+24]
                        if width_tmp > filter_noise:
                            chnl_event.append(chl_ID+24)
                elif coin == 1 and line[0:5] == 'TDC' + str(rightID).zfill(2):
                    chl_ID = int(line[7:9])
                    if line[9] == 'L':
                        pair[chl_ID + 48] = 1
                        leading[chl_ID + 48] = int(line[11:], 10)
                    elif line[9] == 'T' and pair[chl_ID+48] == 1:
                        pair[chl_ID + 48] = 0
                        trailing[chl_ID + 48] = int(line[11:], 10)
                        width_tmp = trailing[chl_ID + 48] - leading[chl_ID + 48]
                        if width_tmp > filter_noise:
                            chnl_event.append(chl_ID + 48)
                elif coin == 1 and line[0:10] == 'TDC' + str(triggerTDC).zfill(2) + 'CH' + str(triggerchnl).zfill(2) + 'L':
                    trigger_time = int(line[11:], 10) % 8192 * time_step
                    trigger_found = 1

        # elif TDCTYPE == 'AMT':
        #     for line in file_decode:
        #         if line[0:7] == 'TDC' + str(TDCID).zfill(2) + 'CH':
        #             chl_ID = int(line[7:9])
        #             if line[9] == 'W':
        #                 width_tmp = int(line[11:14], 10)
        #                 width[chl_ID][width_tmp] = width[chl_ID].get(width_tmp, 0) + 1

    return width, time_diff


def plot_ADC_perchnl(decodefilename, IDtype, filter_num, filter_noise, TDCID, width_select,xlim_left,xlim_right,ylim_top, path):
    width = get_width_hist_deletenoise(decodefilename, TDCID, IDtype, filter_num, filter_noise, width_select)
    CHANNEL = 24
    folders = os.listdir(path)
    if 'TDC'+str(TDCID).zfill(2) not in folders:
        os.mkdir(path + '/TDC'+str(TDCID).zfill(2))

    TDCTYPE = [line for line in IDtype if TDCID in line][0][2]
    time_step = 0.1953125 if TDCTYPE == 'HPTDC' else 2**width_select*0.78125
    for i in range(CHANNEL):
        if len(width[i]):  # channel not empty
            # counted = count_elements(width[i])
            counted = width[i]
            time = [k * time_step for k in sorted(counted.keys())]
            count = [counted[k] for k in sorted(counted.keys())]
            plt.bar(time, count, label=str(i))
            plt.legend()
            plt.minorticks_on()
            plt.tick_params(axis='both', direction='in', which='both')
            plt.xlim(xlim_left, xlim_right)
            # plt.ylim(0, ylim_top)
            plt.xlabel('Width (ns)')
            plt.ylabel('Frequence')
            # figstr = 'mean=' + str(hit_mean)[0:6] + '\nstd  =' + str(hit_std)[0:6]
            # plt.text(hit_mean - 150, 0.9 * max(count), figstr, fontsize=15)
            plt.savefig(path + '/TDC'+str(TDCID).zfill(2)+'/chnl' + str(i).zfill(2) +'_filter'+str(filter_num)+'.png')
            plt.show(block=False)
            plt.clf()
    plot_flag = 0
    line = []
    for i in range(CHANNEL):
        if len(width[i]):  # channel not empty
            plot_flag = 1
            counted = counted = width[i]
            time = [k * time_step for k in sorted(counted.keys())]
            count = [counted[k] for k in sorted(counted.keys())]
            line.append(plt.bar(time, count, label=str(i)))
    if plot_flag == 1:
        if len(line) <= 12:
            plt.legend(loc='upper right', handles=line)
        elif len(line) > 12:
            first_legend = plt.legend(loc='upper left', handles=line[0:12])
            plt.gca().add_artist(first_legend)
            plt.legend(loc='upper right', handles=line[12:])
        # plt.xlim(0, max(max(width))+100)
        plt.xlim(xlim_left, xlim_right)
        # plt.ylim(0, ylim_top)
        plt.xlabel('Width (ns)')
        plt.ylabel('Frequence')
        plt.minorticks_on()
        plt.tick_params(axis='both', direction='in', which='both')
        plt.savefig(path + '/TDC'+str(TDCID).zfill(2) + '.png')
        plt.show(block=False)
        plt.clf()


def check_track_plot(track):
    plt.figure()
    for data in track:
        circle = plt.Circle((17-data/4,data%4),radius=0.1)
        ax = plt.gca()
        ax.add_patch(circle)
    plt.axis('scaled')
    plt.xlim([-1,17])
    plt.ylim([-1, 4])
    plt.show()
    return



def plot_ADC_TDC_perchnl_track(decodefilename, IDtype, filter_num, filter_noise, TDCID, triggerTDC, triggerchnl, leftID,
                    rightID, width_select,xlim_left_ADC,xlim_right_ADC,xlim_left_TDC,xlim_right_TDC,ylim_top, path):
    width, timediff = get_width_timediff_hist_track_deletenoise(decodefilename, TDCID, triggerTDC, triggerchnl, leftID,
                                                                rightID, IDtype, filter_num, filter_noise, width_select)
    CHANNEL = 24
    folders = os.listdir(path)
    if 'TDC'+str(TDCID).zfill(2) not in folders:
        os.mkdir(path + '/TDC'+str(TDCID).zfill(2))

    TDCTYPE = [line for line in IDtype if TDCID in line][0][2]
    time_step = 0.1953125 if TDCTYPE == 'HPTDC' else 2**width_select*0.78125
    for i in range(CHANNEL):
        if len(width[i]):  # channel not empty
            # counted = count_elements(width[i])
            counted = width[i]
            time = [k * time_step for k in sorted(counted.keys())]
            count = [counted[k] for k in sorted(counted.keys())]
            plt.bar(time, count, label=str(i))
            plt.legend()
            plt.minorticks_on()
            plt.tick_params(axis='both', direction='in', which='both')
            plt.xlim(xlim_left_ADC, xlim_right_ADC)
            # plt.ylim(0, ylim_top)
            plt.xlabel('Width (ns)')
            plt.ylabel('Frequence')
            # figstr = 'mean=' + str(hit_mean)[0:6] + '\nstd  =' + str(hit_std)[0:6]
            # plt.text(hit_mean - 150, 0.9 * max(count), figstr, fontsize=15)
            plt.savefig(path + '/TDC'+str(TDCID).zfill(2)+'/chnl' + str(i).zfill(2) +'_filter'+str(filter_num)+'_track.png')
            plt.show(block=False)
            plt.clf()
    plot_flag = 0
    line = []
    for i in range(CHANNEL):
        if len(width[i]):  # channel not empty
            plot_flag = 1
            counted = counted = width[i]
            time = [k * time_step for k in sorted(counted.keys())]
            count = [counted[k] for k in sorted(counted.keys())]
            line.append(plt.bar(time, count, label=str(i)))
    if plot_flag == 1:
        if len(line) <= 12:
            plt.legend(loc='upper right', handles=line)
        elif len(line) > 12:
            first_legend = plt.legend(loc='upper left', handles=line[0:12])
            plt.gca().add_artist(first_legend)
            plt.legend(loc='upper right', handles=line[12:])
        # plt.xlim(0, max(max(width))+100)
        plt.xlim(xlim_left_ADC, xlim_right_ADC)
        # plt.ylim(0, ylim_top)
        plt.xlabel('Width (ns)')
        plt.ylabel('Frequence')
        plt.minorticks_on()
        plt.tick_params(axis='both', direction='in', which='both')
        plt.savefig(path + '/TDC'+str(TDCID).zfill(2) + '_filter' + str(filter_num)+'_track.png')
        plt.show(block=False)
        plt.clf()


    for i in range(CHANNEL):
        if len(timediff[i]):  # channel not empty
            counted = timediff[i]
            time = [k for k in sorted(counted.keys())]
            count = [counted[k] for k in sorted(counted.keys())]
            plt.bar(time, count, label=str(i))
            plt.legend()
            plt.minorticks_on()
            plt.tick_params(axis='both', direction='in', which='both')
            plt.xlim(xlim_left_TDC, xlim_right_TDC)
            plt.xlabel('T0 (ns)')
            plt.ylabel('Count')
            # figstr = 'mean=' + str(hit_mean)[0:6] + '\nstd  =' + str(hit_std)[0:6]
            # plt.text(hit_mean - 150, 0.9 * max(count), figstr, fontsize=15)
            plt.savefig(path + '/TDC'+str(TDCID).zfill(2) + '/T0_chnl' + str(i).zfill(2) +'_filter'+str(filter_num) +'_' +str(filter_noise)+'_track.png')
            plt.show(block=False)
            plt.clf()





def get_timediff(decodefilename, triggerTDC, triggerchnl, TDCID, IDtype, filter_num):
    file_decode = open(decodefilename, "r")
    time_step_HPTDC = 0.1953125
    time_step_AMT = 0.78125
    CHANNEL = 24
    time_diff = []
    TDC_time = []
    leading_found = []
    trigger_found = 0
    trigger_time = 0
    coin = 0
    for i in range(CHANNEL):
        time_diff.append([])
        TDC_time.append(0)
        leading_found.append(0)
    TDCTYPE = [line for line in IDtype if TDCID in line][0][2]
    for line in file_decode:
        if line[0:1] == 'e':
            if int(line[6:]) >= filter_num:
                coin = 1
            else:
                coin = 0
            if trigger_found == 1:
                for i in range(CHANNEL):
                    if leading_found[i] == 1:
                        time_diff[i].append(TDC_time[i]-trigger_time)
                        leading_found[i] = 0
            trigger_found = 0
        elif coin == 1 and line[0:10] == 'TDC'+str(triggerTDC).zfill(2)+'CH'+str(triggerchnl).zfill(2)+'L':
            trigger_time = int(line[11:], 10) % 8192 * time_step_HPTDC
            trigger_found = 1
        elif coin == 1 and line[3:5] == str(TDCID).zfill(2) and line[9] in ['L','W']:
            chl_ID = int(line[7:9])
            if line[9] == 'L':
                TDC_time[chl_ID] = int(line[11:], 10) % 8192 * time_step_HPTDC
            elif line[9] == 'W':
                TDC_time[chl_ID] = int(line[15:], 10) * time_step_AMT
            leading_found[chl_ID] = 1
    file_decode.close()
    return time_diff


def get_timediff_hist(decodefilename, triggerTDC, triggerchnl, TDCID, IDtype, filter_num):
    with open(decodefilename, 'r') as file_decode:
        time_step_HPTDC = 0.1953125
        time_step_AMT = 0.78125
        CHANNEL = 24
        time_diff = []
        TDC_time = []
        leading_found = []
        trigger_found = 0
        trigger_time = 0
        coin = 0
        for i in range(CHANNEL):
            time_diff.append({})
            TDC_time.append(0)
            leading_found.append(0)
        TDCTYPE = [line for line in IDtype if TDCID in line][0][2]
        for line in file_decode:
            if line[0:1] == 'e':
                if int(line[6:]) >= filter_num:
                    coin = 1
                else:
                    coin = 0
                if trigger_found == 1:
                    for i in range(CHANNEL):
                        if leading_found[i] == 1:
                            time_diff_tmp = TDC_time[i]-trigger_time
                            time_diff[i][time_diff_tmp] = time_diff[chl_ID].get(time_diff_tmp, 0) + 1
                            leading_found[i] = 0
                trigger_found = 0
            elif coin == 1 and line[0:10] == 'TDC'+str(triggerTDC).zfill(2)+'CH'+str(triggerchnl).zfill(2)+'L':
                trigger_time = int(line[11:], 10) % 8192 * time_step_HPTDC
                trigger_found = 1
            elif coin == 1 and line[3:5] == str(TDCID).zfill(2) and line[9] in ['L','W']:
                chl_ID = int(line[7:9])
                if line[9] == 'L':
                    TDC_time[chl_ID] = int(line[11:], 10) % 8192 * time_step_HPTDC
                elif line[9] == 'W':
                    TDC_time[chl_ID] = int(line[15:], 10) * time_step_AMT
                leading_found[chl_ID] = 1
    return time_diff



def plot_TDC_perchnl(decodefilename, IDtype, filter_num, filter_noise, TDCID, triggerTDC, triggerchnl,xlim_left,xlim_right,path):
    # width = get_timediff_hist(decodefilename, triggerTDC, triggerchnl, TDCID, IDtype, filter_num)
    width = get_timediff_hist_delete_noise(decodefilename, triggerTDC, triggerchnl, TDCID, IDtype, filter_num, filter_noise)
    CHANNEL = 24
    folders = os.listdir(path)
    if 'TDC'+str(TDCID).zfill(2) not in folders:
        os.mkdir(path + '/TDC'+str(TDCID).zfill(2))
    for i in range(CHANNEL):
        if len(width[i]):  # channel not empty
            counted = width[i]
            time = [k for k in sorted(counted.keys())]
            count = [counted[k] for k in sorted(counted.keys())]
            plt.bar(time, count, label=str(i))
            plt.legend()
            plt.minorticks_on()
            plt.tick_params(axis='both', direction='in', which='both')
            plt.xlim(xlim_left, xlim_right)
            plt.xlabel('T0 (ns)')
            plt.ylabel('Count')
            # figstr = 'mean=' + str(hit_mean)[0:6] + '\nstd  =' + str(hit_std)[0:6]
            # plt.text(hit_mean - 150, 0.9 * max(count), figstr, fontsize=15)
            plt.savefig(path + '/TDC'+str(TDCID).zfill(2) + '/T0_chnl' + str(i).zfill(2) +'_filter'+str(filter_num) +'_' +str(filter_noise)+'.png')
            plt.show(block=False)
            plt.clf()




def get_timediff_hist_delete_noise(decodefilename, triggerTDC, triggerchnl, TDCID, IDtype, filter_num, filter_noise):
    with open(decodefilename, 'r') as file_decode:
        time_step_HPTDC = 0.1953125
        time_step_AMT = 0.78125
        CHANNEL = 24
        time_diff = []
        TDC_time = []
        leading_found = []
        trigger_found = 0
        trigger_time = 0
        coin = 0
        leading = []
        trailing = []
        pair = []
        for i in range(CHANNEL):
            time_diff.append({})
            TDC_time.append(0)
            leading_found.append(0)
            pair.append(0)
            leading.append(0)
            trailing.append(0)
        TDCTYPE = [line for line in IDtype if TDCID in line][0][2]
        for line in file_decode:
            if line[0:1] == 'e':
                if int(line[6:]) >= filter_num:
                    coin = 1
                else:
                    coin = 0
                if trigger_found == 1:
                    for i in range(CHANNEL):
                        if leading_found[i] == 1:
                            time_diff_tmp = TDC_time[i]-trigger_time
                            time_diff[i][time_diff_tmp] = time_diff[i].get(time_diff_tmp, 0) + 1
                            leading_found[i] = 0
                trigger_found = 0
            elif coin == 1 and line[0:10] == 'TDC'+str(triggerTDC).zfill(2)+'CH'+str(triggerchnl).zfill(2)+'L':
                trigger_time = int(line[11:], 10) % 8192 * time_step_HPTDC
                trigger_found = 1
            elif coin == 1 and line[3:5] == str(TDCID).zfill(2) and line[9] in ['L','W','T']:
                chl_ID = int(line[7:9])
                if line[9] == 'L':
                    pair[chl_ID] = 1
                    leading[chl_ID] = int(line[11:], 10)
                    TDC_time[chl_ID] = int(line[11:], 10) % 8192 * time_step_HPTDC
                elif line[9] == 'T' and pair[chl_ID] == 1:
                    pair[chl_ID] = 0
                    trailing[chl_ID] = int(line[11:], 10)
                    width_tmp = trailing[chl_ID] - leading[chl_ID]
                    if (width_tmp * time_step_HPTDC) > filter_noise:
                        leading_found[chl_ID] = 1
                    else:
                        leading_found[chl_ID] = 0
                elif line[9] == 'W':
                    TDC_time[chl_ID] = int(line[15:], 10) * time_step_AMT
                    width_tmp = int(line[11:14], 10)
                    if (width_tmp * time_step_AMT) > filter_noise:
                        leading_found[chl_ID] = 1
                    else:
                        leading_found[chl_ID] = 0
    return time_diff
