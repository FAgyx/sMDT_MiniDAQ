# ASDtype = 'newASD'

# if ASDtype == 'newASD':
#     path = '../CSMMiniDAQ006_newASD_newTDC/'
# elif ASDtype == 'oldASD':
#     path = '../CSMMiniDAQ006_newASD_HPTDC/'
path = '../CSMMiniDAQ007_mix_setup/'
# path = '../CSMMiniDAQ006_newASD_HPTDC/'

filename = path + 'ASDsetup.txt'
file = open(filename, "w")

ASD_main_thr = 51
int_gate = 4
rundown_current = 2

ASD_main_thr_code = 128-(ASD_main_thr+1)/2
if ASD_main_thr_code < 0:
    ASD_main_thr_code = 0
# new ASD setup JTAG chain
ASD0_CHANNEL_0_MODE = [format(0b01,'b').zfill(2)]
ASD0_CHANNEL_1_MODE = [format(0b01,'b').zfill(2)]
ASD0_CHANNEL_2_MODE = [format(0b01,'b').zfill(2)]
ASD0_CHANNEL_3_MODE = [format(0b01,'b').zfill(2)]
ASD0_CHANNEL_4_MODE = [format(0b01,'b').zfill(2)]
ASD0_CHANNEL_5_MODE = [format(0b01,'b').zfill(2)]
ASD0_CHANNEL_6_MODE = [format(0b01,'b').zfill(2)]
ASD0_CHANNEL_7_MODE = [format(0b01,'b').zfill(2)]
ASD0_CHIP_MODE = [format(0,'b').zfill(1)]
ASD0_DEADTIME = [format(7,'b').zfill(3)]
ASD0_INT_GATE = [format(int_gate,'b').zfill(4)]
ASD0_RUND_CURR = [format(rundown_current,'b').zfill(3)]
ASD0_HYST_DAC = [''.join(reversed(format(7,'b').zfill(4)))]
ASD0_WILK_THR_DAC = [format(2,'b').zfill(3)]
ASD0_MAIN_THR_DAC = [format(ASD_main_thr_code,'b').zfill(8)]
ASD0_CAP_SELECT = [format(0,'b').zfill(3)]
ASD0_CH_MASK_TST_CTRL = [format(0,'b').zfill(8)]
ASD0_VMON = [format(0,'b').zfill(2)]
ASD0_setup = [ASD0_CHANNEL_0_MODE, ASD0_CHANNEL_1_MODE, ASD0_CHANNEL_2_MODE, ASD0_CHANNEL_3_MODE,
           ASD0_CHANNEL_4_MODE, ASD0_CHANNEL_5_MODE, ASD0_CHANNEL_6_MODE, ASD0_CHANNEL_7_MODE,
           ASD0_CHIP_MODE, ASD0_DEADTIME, ASD0_INT_GATE, ASD0_RUND_CURR, ASD0_HYST_DAC, ASD0_WILK_THR_DAC,
           ASD0_MAIN_THR_DAC, ASD0_CAP_SELECT, ASD0_CH_MASK_TST_CTRL, ASD0_VMON]
asd_bin_str = ''
for s in ASD0_setup:
    asd_bin_str = asd_bin_str + ''.join(s)
asd_setup_str = 3 * asd_bin_str
# asd_setup_str = '0' + asd_setup_str
file.write('ASDsetup ' + str(len(asd_setup_str)) + '\n')
file.write(asd_setup_str + '\n')





# bit0 is the last bit to enter the shift register chain
# old ASD setup JTAG chain
ASD1_CHANNEL_0MSB_7LSB_MASK = [format(0b00000000, 'b').zfill(8)[-8:]]
ASD1_CAP_SELECT = [format(0, 'b').zfill(3)]
ASD1_MAIN_THR_DAC = [format(ASD_main_thr_code, 'b').zfill(8)]
ASD1_WILK_THR_DAC = [format(2, 'b').zfill(3)]
ASD1_HYST_DAC = [format(7, 'b').zfill(4)]
ASD1_INT_GATE = [format(5, 'b').zfill(4)]
ASD1_RUND_CURR = [format(4, 'b').zfill(3)]
ASD1_DEADTIME = [format(7, 'b').zfill(3)]
ASD1_CHANNEL_0_MODE = [format(0b01, 'b').zfill(2)]  # 00,01=ON 10=LO 11=HI
ASD1_CHANNEL_1_MODE = [format(0b01, 'b').zfill(2)]
ASD1_CHANNEL_2_MODE = [format(0b01, 'b').zfill(2)]
ASD1_CHANNEL_3_MODE = [format(0b01, 'b').zfill(2)]
ASD1_CHANNEL_4_MODE = [format(0b01, 'b').zfill(2)]
ASD1_CHANNEL_5_MODE = [format(0b01, 'b').zfill(2)]
ASD1_CHANNEL_6_MODE = [format(0b01, 'b').zfill(2)]
ASD1_CHANNEL_7_MODE = [format(0b01, 'b').zfill(2)]
ASD1_CHIP_MODE = [format(0, 'b').zfill(1)]
ASD1_setup = [ASD1_CHANNEL_0MSB_7LSB_MASK, ASD1_CAP_SELECT, ASD1_MAIN_THR_DAC, ASD1_WILK_THR_DAC,
              ASD1_HYST_DAC, ASD1_INT_GATE, ASD1_RUND_CURR, ASD1_DEADTIME, ASD1_CHANNEL_0_MODE,
              ASD1_CHANNEL_1_MODE, ASD1_CHANNEL_2_MODE, ASD1_CHANNEL_3_MODE, ASD1_CHANNEL_4_MODE,
              ASD1_CHANNEL_5_MODE, ASD1_CHANNEL_6_MODE, ASD1_CHANNEL_7_MODE, ASD1_CHIP_MODE]

asd_bin_str = ''
for s in ASD1_setup:
    asd_bin_str = asd_bin_str + ''.join(s)
asd_setup_str = 3 * asd_bin_str
# asd_setup_str = '0' + asd_setup_str
file.write('oldASDset ' + str(len(asd_setup_str)) + '\n')
file.write(asd_setup_str + '\n')



# setup_0
# TTC_setup
enable_new_ttc = ['0']
enable_master_reset_code = ['1']
enable_direct_bunch_reset = ['0']
enable_direct_event_reset = ['0']
enable_direct_trigger = ['0']
# bcr_distribute
auto_roll_over = ['0']
bypass_bcr_distribution = ['0']
# tdc_mode
enable_trigger = ['1']
channel_data_debug = ['0']
enable_leading = ['0']
enable_pair = ['1']
enbale_fake_hit = ['1']
rising_is_leading = [format(0x000000,'b').zfill(24)]
channel_enable_r = [format(0xFFFFFF,'b').zfill(24)]
channel_enable_f = [format(0xFFFFFF,'b').zfill(24)]
# readout
TDC_ID = format(0x7AAAA,'b').zfill(19)
enable_trigger_timeout = ['0']
enable_high_speed = ['0']
enable_legacy = ['1']
full_width_res = ['0']
width_select = ['001']
enable_8b10b = ['0']
enable_insert = ['0']
enable_error_packet = ['0']
enable_TDC_ID = ['0']
enable_error_notify = ['0']
# setup_0
TTC_setup = [enable_new_ttc, enable_master_reset_code, enable_direct_bunch_reset, enable_direct_event_reset,
             enable_direct_trigger]
bcr_distribute = [auto_roll_over, bypass_bcr_distribution]
tdc_mode = [enable_trigger, channel_data_debug, enable_leading, enable_pair, enbale_fake_hit, rising_is_leading,
            channel_enable_r, channel_enable_f]
TDC_ID_l = [TDC_ID]
readout = [enable_trigger_timeout, enable_high_speed, enable_legacy, full_width_res, width_select, enable_8b10b,
           enable_insert, enable_error_packet, enable_TDC_ID, enable_error_notify]
setup_0 = TTC_setup + bcr_distribute + tdc_mode + TDC_ID_l + readout

tdc_setup_0_bin_str = ''
for s in setup_0:
    tdc_setup_0_bin_str = tdc_setup_0_bin_str + ''.join(s)
file.write('TDCsetup0 '+str(len(tdc_setup_0_bin_str)) + '\n')
file.write(tdc_setup_0_bin_str + '\n')


# setup_1
combine_time_out_config = [format(100,'b').zfill(10)]
fake_hit_time_interval = [format(256,'b').zfill(12)]
syn_packet_number = [format(0xFFF,'b').zfill(12)]
# coarse_out
roll_over = [format(0xFFF,'b').zfill(12)]
coarse_count_offset = [format(0,'b').zfill(12)]
bunch_offset = [format(0x000,'b').zfill(12)]   # F9C
event_offset = [format(0,'b').zfill(12)]
match_window = [format(61,'b').zfill(12)]
# setup_1
timer_out = [combine_time_out_config, fake_hit_time_interval, syn_packet_number]
coarse_out = [roll_over, coarse_count_offset, bunch_offset, event_offset, match_window]
setup_1 = timer_out + coarse_out

tdc_setup_1_bin_str = ''
for s in setup_1:
    tdc_setup_1_bin_str = tdc_setup_1_bin_str + ''.join(s)
file.write('TDCsetup1 '+str(len(tdc_setup_1_bin_str)) + '\n')
file.write(tdc_setup_1_bin_str + '\n')


# setup_2
fine_sel = ['0011']
lut = [format(0x19CA01CC,'b').zfill(32)]
setup_2 = [fine_sel,lut]
tdc_setup_2_bin_str = ''
for s in setup_2:
    tdc_setup_2_bin_str = tdc_setup_2_bin_str + ''.join(s)
file.write('TDCsetup2 '+str(len(tdc_setup_2_bin_str)) + '\n')
file.write(tdc_setup_2_bin_str + '\n')

# control_0
rst_ePLL = ['0']
reset_jtag_in = ['0']
event_teset_jtag_in = ['0']
chnl_fifo_overflow_clear = ['0']
debug_port_select =['0000']
reset_control = [rst_ePLL, reset_jtag_in, event_teset_jtag_in, chnl_fifo_overflow_clear, debug_port_select]
control_0 = reset_control
tdc_control_0_bin_str = ''
for s in control_0:
    tdc_control_0_bin_str = tdc_control_0_bin_str + ''.join(s)
file.write('TDCcontrol0 '+str(len(tdc_control_0_bin_str)) + '\n')
file.write(tdc_control_0_bin_str + '\n')



# control_1
phase_clk160 = ['01000']
phase_clk320_0 = ['0100']
phase_clk320_1 = ['0000']
phase_clk320_2 = ['0010']
ePllRes = ['0010']
ePllIcp = ['0100']
ePllCap = ['10']
ePLL_control = [phase_clk160, phase_clk320_0, phase_clk320_1, phase_clk320_2, ePllRes, ePllIcp, ePllCap, ePllRes, ePllIcp, ePllCap, ePllRes, ePllIcp, ePllCap]
control_1 = ePLL_control
tdc_control_1_bin_str = ''
for s in control_1:
    tdc_control_1_bin_str = tdc_control_1_bin_str + ''.join(s)
file.write('TDCcontrol1 '+str(len(tdc_control_1_bin_str)) + '\n')
file.write(tdc_control_1_bin_str + '\n')
print("update for " + "mainthr = " + str(ASD_main_thr) + "mv")
