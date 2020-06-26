import os
from miniDAQ_lowlevel import *
path = '../newASD_newTDC\ch2_Amp'
files = os.listdir(path+'/data_origin')
files_dat = [data for data in files if data[-3:] == 'dat' and data[0:-3]+'txt' not in files]
for filename in files_dat:
    datname = path + '/data_origin/' + filename
    txtname = path + '/data_origin/' + filename[0:-3]+'txt'
    fileformat_dat_to_txt(datname, txtname)
    print('conversion finished for ' + filename)

    # for i in range(len(data_array)/4):
    #     dataword1 = format(data_array[i*4+3],'02x')+format(data_array[i*4+2],'02x')+format(data_array[i*4+1],'02x')+format(data_array[i*4+0],'02x')
    #     file_word.writelines(dataword1+'\n')


