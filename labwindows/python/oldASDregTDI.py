path = '../CSMMiniDAQ006_newASD_HPTDC/'
filename = path + 'ASDsetup.txt'
file = open(filename, "w")

ASD0_CHANNEL_0MSB_7LSB_MASK = [format(0,'b').zfill(8)]
ASD0_CAP_SELECT = [format(0,'b').zfill(3)]
ASD0_MAIN_THR_DAC = [format(106,'b').zfill(8)]
ASD0_WILK_THR_DAC = [format(2,'b').zfill(3)]
ASD0_HYST_DAC = [format(7,'b').zfill(4)]
ASD0_INT_GATE = [format(4,'b').zfill(4)]
ASD0_RUND_CURR = [format(5,'b').zfill(3)]
ASD0_DEADTIME = [format(7,'b').zfill(3)]
ASD0_CHANNEL_0_MODE = [format(0,'b').zfill(2)]
ASD0_CHANNEL_1_MODE = [format(0,'b').zfill(2)]
ASD0_CHANNEL_2_MODE = [format(0,'b').zfill(2)]
ASD0_CHANNEL_3_MODE = [format(0,'b').zfill(2)]
ASD0_CHANNEL_4_MODE = [format(0,'b').zfill(2)]
ASD0_CHANNEL_5_MODE = [format(0,'b').zfill(2)]
ASD0_CHANNEL_6_MODE = [format(0,'b').zfill(2)]
ASD0_CHANNEL_7_MODE = [format(0,'b').zfill(2)]
ASD0_CHIP_MODE = [format(0,'b').zfill(1)]

ASD0_setup = [ASD0_CHANNEL_0MSB_7LSB_MASK,ASD0_CAP_SELECT,ASD0_MAIN_THR_DAC,ASD0_WILK_THR_DAC,
            ASD0_HYST_DAC,ASD0_INT_GATE,ASD0_RUND_CURR,ASD0_DEADTIME,ASD0_CHANNEL_0_MODE,ASD0_CHANNEL_1_MODE,
            ASD0_CHANNEL_2_MODE,ASD0_CHANNEL_3_MODE,ASD0_CHANNEL_4_MODE,ASD0_CHANNEL_5_MODE,ASD0_CHANNEL_6_MODE,
            ASD0_CHANNEL_7_MODE,ASD0_CHIP_MODE]
asd_bin_str = ''
for s in ASD0_setup:
    asd_bin_str = asd_bin_str + ''.join(s)
asd_setup_str = 3*asd_bin_str
file.write('ASDsetup '+str(len(asd_setup_str))+'\n')
file.write(asd_setup_str)