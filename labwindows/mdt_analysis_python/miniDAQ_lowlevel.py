import numpy as np

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


def lineup_in_wordfile(txtfilename, wordfilename):
    # reorganize raw data file to TDC words
    file_txt = open(txtfilename,"r")
    file_word = open(wordfilename,"w")
    for li in file_txt.readlines():
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
        for line in file_word.readlines():
            if line[0:2] == '00':
                file_decode.write('event'+str(int(line[2:5], 16))+'\n')
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
        for line in file_word.readlines():
            if line[0:2] == '00':
                file_decode.write('event'+str(int(line[2:5], 16))+'\n')
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





    