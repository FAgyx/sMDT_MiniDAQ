import os
from miniDAQ_lowlevel import *
path = '../data/cosmic_ray'
ASDTYPE = 'newASD'
TDCTYPE = 'AMT'
files = os.listdir(path+'/data_origin')
files_txt = [data[0:-4] for data in files if data[-3:] == 'txt']
for filename in files_txt:
    txtfilename = path + '/data_origin/' + filename + '.txt'
    wordfilename = path + '/data_gen/' + filename + '_word.txt'
    decodefilename = path + '/data_gen/' + filename + '_decode.txt'
    lineup_in_wordfile(txtfilename, wordfilename)
    decode_wordfile(ASDTYPE, TDCTYPE, wordfilename, decodefilename)
    print('decoding finished for ' + filename)