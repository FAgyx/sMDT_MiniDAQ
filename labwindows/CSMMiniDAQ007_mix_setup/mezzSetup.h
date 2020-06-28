#ifndef mezzSetup_H
#define mezzSetup_H
//
// Filename: mezzSetup.h
// Author: T.S. Dai
//
// Description:
// This file contains prototypes for all of the Mezzanine Card JTAG Setup functions.
//
#define MAXNUMBERASD                  3
#define MAXNUMBERMEZZANINE           18

// AMT ID code
#define AMT1ID               0x18B83131 // AMT1 IDCODE
#define AMT2ID               0x18B85031 // AMT2 IDCODE
#define AMT3ID               0x38B85031 // AMT3 IDCODE
#define MEZZINITTYPEMASK            0xF
#define MEZZNOINIT                    0 // Mezzanine Card is not initialized
#define MEZZINITBYSETUPALL            1 // Mezzanine Card initialized by SetupAll
#define MEZZINITBYSETUPIND            2 // Mezzanine Card initialized individually
#define MEZZINITERROR              0xF0 // Four bits used to indicate init error
#define MEZZAMTINITERROR           0x10 // Bit 4 set == AMT init error
#define MEZZASDINITERROR           0x20 // Bit 5 set == ASD init error

#define AMTSETUP_LENGTH             180 // length of binary AMT setup array needed for 1 mezz card

#define AMTS_SETUP                     MAXNUMBERASD*ASD_SETUP_LENGTH  // Offset of AMT setup register
#define SETUP_LENGTH                   AMTS_SETUP+AMTSETUP_LENGTH     // length of binary setup array needed for 1 mezz card
#define AMTSTATUSLENGTH              72 // Length of the status register in bits

#define AMTK_course_err              0
#define AMTK_chan_sel_err            1
#define AMTK_L1_buf_parity_err       2
#define AMTK_trig_fifo_err           3
#define AMTK_trig_match_err          4
#define AMTK_ro_parity_err           5
#define AMTK_ro_state_err            6
#define AMTK_setup_parity_err        7
#define AMTK_jtag_instr_err          8
#define AMTK_setup_read_parity       9
#define AMTK_ro_fifo_full           10
#define AMTK_ro_fifo_empty          11   
#define AMTK_L1_write_address       12
#define AMTK_l1_overflow            20   
#define AMTK_l1_overflow_rcvr       21     
#define AMTK_l1_nearly_full         22
#define AMTK_l1_empty               23
#define AMTK_L1_read_address        24
#define AMTK_l1_running             32
#define AMTK_trig_fifo_full         33
#define AMTK_trig_fifo_nearly_full  34  
#define AMTK_trig_fifo_empty        35 
#define AMTK_start_address          36
#define AMTK_trig_fifo_occy         44
#define AMTK_course_count           47
#define AMTK_ro_fifo_occy           60
#define AMTK_zeros                  66
#define AMTK_general_in             68




#define AMTS_PLL_MULT                0
#define AMTS_CLK_MODE                2  
#define AMTS_DIS_RING                4
#define AMTS_DIR_CNTL                5
#define AMTS_TST_INV                 6
#define AMTS_TST_MOD                 7
#define AMTS_ERST_BCECRST            8
#define AMTS_DIS_ENCODE              9
#define AMTS_GL_RST                 11
#define AMTS_MSK_WDW                12
#define AMTS_SRCH_WDW               24
#define AMTS_MTCH_WDW               36
#define AMTS_RC_OFFSET              48
#define AMTS_EC_OFFSET              60
#define AMTS_BC_OFFSET              72
#define AMTS_COARSE_OFFSET          84
#define AMTS_CNT_RO                 96
#define AMTS_TDC_ID                108
#define AMTS_ERRORFLAGTEST         112
#define AMTS_SEL_WID               113
#define AMTS_RD_SPEED              116
#define AMTS_STROBE_SEL            118  
#define AMTS_ENB_LEAD              120
#define AMTS_ENB_TRL               121
#define AMTS_ENB_PAIR              122
#define AMTS_ENABLE_REJECTED       123
#define AMTS_TRAILER               124
#define AMTS_HEADER                125
#define AMTS_ENB_SER               126
#define AMTS_RELATIVE              127
#define AMTS_MASK                  128
#define AMTS_MATCH                 129
#define AMTS_L1_OCC                130
#define AMTS_AUTO_REJ              131
#define AMTS_SET_ON_BR             132
#define AMTS_MR_ON_ER              133
#define AMTS_RCH_BUF               134
#define AMTS_MR_RST                135
#define AMTS_BUF_OVR               136
#define AMTS_MRK_OVR               137
#define AMTS_EMRK_REJ              138
#define AMTS_INCLKBOOST            139  
#define AMTS_MARK                  140
#define AMTS_TRG_FULL              141
#define AMTS_L1_REJ                142
#define AMTS_FUL_REJ               143
#define AMTS_COARSE                144
#define AMTS_CH_SEL                145
#define AMTS_L1_PAR                146
#define AMTS_FIFO_PAR              147
#define AMTS_TRG_MATCH             148
#define AMTS_RO_FIFO               149
#define AMTS_RO_STATE              150
#define AMTS_SETUP_PAR             151
#define AMTS_JTAG_INS              152
#define AMTS_SEP_ON_ER             153
#define AMTS_SEP_ON_BR             154
#define AMTS_SEP_RO                155
#define AMTS_CH0                   156
#define AMTS_CH1                   157
#define AMTS_CH2                   158
#define AMTS_CH3                   159
#define AMTS_CH4                   160
#define AMTS_CH5                   161
#define AMTS_CH6                   162
#define AMTS_CH7                   163
#define AMTS_CH8                   164
#define AMTS_CH9                   165
#define AMTS_CH10                  166
#define AMTS_CH11                  167
#define AMTS_CH12                  168
#define AMTS_CH13                  169
#define AMTS_CH14                  170
#define AMTS_CH15                  171
#define AMTS_CH16                  172
#define AMTS_CH17                  173
#define AMTS_CH18                  174
#define AMTS_CH19                  175
#define AMTS_CH20                  176
#define AMTS_CH21                  177
#define AMTS_CH22                  178
#define AMTS_CH23                  179

// define AMT clock mode
#define AMTUNDEFINEDCLKMODE         -1
#define AMTDSSTROBE                  0
#define AMTDSSTROBECONTINUE          1
#define AMTLEADEGDE                  2     // For AMT1 and AMT2
#define AMTCLOCKOFF                  2	   // For AMT3
#define AMTEDGECONTINE               3




//
//setting

/*

//for old ASD

#define ASD_SETUP_LENGTH             53
#define ASD_CH0                      0
#define ASD_CH1                      1 
#define ASD_CH2                      2 
#define ASD_CH3                      3 
#define ASD_CH4                      4 
#define ASD_CH5                      5 
#define ASD_CH6                      6 
#define ASD_CH7                      7 
#define ASD_CAL_CAP                  8
#define ASD_MAIN_THRESH             11
#define ASD_WIL_THRESH              19
#define ASD_HYSTERESIS              22
#define ASD_WIL_INT                 26
#define ASD_WIL_CUR                 30
#define ASD_DEADTIME                33
#define ASD_CHANNEL_MODE_0          36
#define ASD_CHANNEL_MODE_1          38 
#define ASD_CHANNEL_MODE_2          40
#define ASD_CHANNEL_MODE_3          42
#define ASD_CHANNEL_MODE_4          44
#define ASD_CHANNEL_MODE_5          46
#define ASD_CHANNEL_MODE_6          48
#define ASD_CHANNEL_MODE_7          50
#define ASD_CHIP_MODE               52

#define A3P250SETUP_LENGTH					159
//offset
#define ASD0_CH0                      0
#define ASD0_CH1                      1 
#define ASD0_CH2                      2 
#define ASD0_CH3                      3 
#define ASD0_CH4                      4 
#define ASD0_CH5                      5 
#define ASD0_CH6                      6 
#define ASD0_CH7                      7 
#define ASD0_CAL_CAP                  8
#define ASD0_MAIN_THRESH             11
#define ASD0_WIL_THRESH              19
#define ASD0_HYSTERESIS              22
#define ASD0_WIL_INT                 26
#define ASD0_WIL_CUR                 30
#define ASD0_DEADTIME                33
#define ASD0_CHANNEL_MODE_0          36
#define ASD0_CHANNEL_MODE_1          38 
#define ASD0_CHANNEL_MODE_2          40
#define ASD0_CHANNEL_MODE_3          42
#define ASD0_CHANNEL_MODE_4          44
#define ASD0_CHANNEL_MODE_5          46
#define ASD0_CHANNEL_MODE_6          48
#define ASD0_CHANNEL_MODE_7          50
#define ASD0_CHIP_MODE               52

#define ASD1_CH0                     53
#define ASD1_CH1                     54 
#define ASD1_CH2                     55 
#define ASD1_CH3                     56 
#define ASD1_CH4                     57 
#define ASD1_CH5                     58 
#define ASD1_CH6                     59 
#define ASD1_CH7                     60 
#define ASD1_CAL_CAP                 61
#define ASD1_MAIN_THRESH             64
#define ASD1_WIL_THRESH              72
#define ASD1_HYSTERESIS              75
#define ASD1_WIL_INT                 79
#define ASD1_WIL_CUR                 83
#define ASD1_DEADTIME                86
#define ASD1_CHANNEL_MODE_0          89
#define ASD1_CHANNEL_MODE_1          91 
#define ASD1_CHANNEL_MODE_2          93
#define ASD1_CHANNEL_MODE_3          95
#define ASD1_CHANNEL_MODE_4          97
#define ASD1_CHANNEL_MODE_5          99
#define ASD1_CHANNEL_MODE_6          101
#define ASD1_CHANNEL_MODE_7          103
#define ASD1_CHIP_MODE               105

#define ASD2_CH0                     106
#define ASD2_CH1                     107 
#define ASD2_CH2                     108 
#define ASD2_CH3                     109 
#define ASD2_CH4                     110 
#define ASD2_CH5                     111
#define ASD2_CH6                     112 
#define ASD2_CH7                     113
#define ASD2_CAL_CAP                 114
#define ASD2_MAIN_THRESH             117
#define ASD2_WIL_THRESH              125
#define ASD2_HYSTERESIS              128
#define ASD2_WIL_INT                 132
#define ASD2_WIL_CUR                 136
#define ASD2_DEADTIME                139
#define ASD2_CHANNEL_MODE_0          142
#define ASD2_CHANNEL_MODE_1          144 
#define ASD2_CHANNEL_MODE_2          146
#define ASD2_CHANNEL_MODE_3          148
#define ASD2_CHANNEL_MODE_4          150
#define ASD2_CHANNEL_MODE_5          152
#define ASD2_CHANNEL_MODE_6          154
#define ASD2_CHANNEL_MODE_7          156
#define ASD2_CHIP_MODE               158

//value
#define asd0_ch0						 	0
#define asd0_ch1						 	0	
#define asd0_ch2						 	0
#define asd0_ch3						 	0
#define asd0_ch4						 	0
#define asd0_ch5						 	0
#define asd0_ch6						 	0
#define asd0_ch7						 	0
#define asd0_cal_cap					 	0
#define asd0_main_thresh					100
#define asd0_wil_thresh				 		2
#define asd0_hysteresis				 		7
#define asd0_wil_int					 	4
#define asd0_wil_cur                  		5
#define asd0_deadtime				 		7
#define asd0_chmod_0					 	0					   
#define asd0_chmod_1					 	0
#define asd0_chmod_2					 	0
#define asd0_chmod_3					 	0
#define asd0_chmod_4					 	0
#define asd0_chmod_5					 	0
#define asd0_chmod_6					 	0
#define asd0_chmod_7					 	0
#define asd0_chip_mode				 		0

#define asd1_ch0						 	0
#define asd1_ch1						 	0	
#define asd1_ch2						 	0
#define asd1_ch3						 	0
#define asd1_ch4						 	0
#define asd1_ch5						 	0
#define asd1_ch6						 	0
#define asd1_ch7						 	0
#define asd1_cal_cap					 	0
#define asd1_main_thresh					100
#define asd1_wil_thresh				 		2
#define asd1_hysteresis				 		7
#define asd1_wil_int					 	4
#define asd1_wil_cur                  		5
#define asd1_deadtime				 		7
#define asd1_chmod_0					 	0
#define asd1_chmod_1					 	0
#define asd1_chmod_2					 	0
#define asd1_chmod_3					 	0
#define asd1_chmod_4					 	0
#define asd1_chmod_5					 	0
#define asd1_chmod_6					 	0
#define asd1_chmod_7					 	0
#define asd1_chip_mode				 		0

#define asd2_ch0						 	0
#define asd2_ch1						 	0	
#define asd2_ch2						 	0
#define asd2_ch3						 	0
#define asd2_ch4						 	0
#define asd2_ch5						 	0
#define asd2_ch6						 	0
#define asd2_ch7						 	0
#define asd2_cal_cap					 	0
#define asd2_main_thresh					100
#define asd2_wil_thresh				 		2
#define asd2_hysteresis				 		7
#define asd2_wil_int					 	4
#define asd2_wil_cur                  		5
#define asd2_deadtime				 		7
#define asd2_chmod_0					 	0  // has noise under vth 80
#define asd2_chmod_1					 	0   //no noise under vth 80 
#define asd2_chmod_2					 	0  //very low noise
#define asd2_chmod_3					 	0  // has noise under vth 80
#define asd2_chmod_4					 	0  //2 atcive   1 no signal
#define asd2_chmod_5					 	0  //no noise under vth 80
#define asd2_chmod_6					 	0  //no noise under vth 80
#define asd2_chmod_7					 	0   //no noise under vth 80
#define asd2_chip_mode				 		0

//#define asd2_main_thresh					100
//#define asd2_wil_thresh				 	2
//#define asd2_hysteresis				 	7
//#define asd2_wil_int					 	4
//#define asd2_wil_cur                  	5
//#define asd2_deadtime				 		7				
 
*/


 
  
//for new ASD according to mannual (not used bits first shifted in)

#define ASD_SETUP_LENGTH             	55

#define  ASD_CHANNEL_MODE_0				0
#define  ASD_CHANNEL_MODE_1       		2
#define  ASD_CHANNEL_MODE_2       		4
#define  ASD_CHANNEL_MODE_3       		6
#define  ASD_CHANNEL_MODE_4       		8
#define  ASD_CHANNEL_MODE_5       		10
#define  ASD_CHANNEL_MODE_6       		12
#define  ASD_CHANNEL_MODE_7				14
#define  ASD_CHIP_MODE                  16
#define  ASD_DEADTIME	               	17
#define  ASD_WIL_INT	               	20
#define  ASD_WIL_CUR					24
#define  ASD_HYSTERESIS                 27
#define  ASD_WIL_THRESH                 31
#define  ASD_MAIN_THRESH                34
#define  ASD_CAL_CAP				    42
#define  ASD_CH0                      	45
#define  ASD_CH1                      	46 
#define  ASD_CH2                      	47 
#define  ASD_CH3                      	48 
#define  ASD_CH4                      	49 
#define  ASD_CH5                      	50 
#define  ASD_CH6                      	51 
#define  ASD_CH7                      	52
#define  ASD_VMON					    53

int ASD_length;
#define A3P250SETUP_LENGTH							165
//#define A3P250SETUP_LENGTH							159
//offset

#define  ASD0_CHANNEL_0_MODE       					0
#define  ASD0_CHANNEL_1_MODE       					2
#define  ASD0_CHANNEL_2_MODE       					4
#define  ASD0_CHANNEL_3_MODE       					6
#define  ASD0_CHANNEL_4_MODE       					8
#define  ASD0_CHANNEL_5_MODE       					10
#define  ASD0_CHANNEL_6_MODE       					12
#define  ASD0_CHANNEL_7_MODE				        14
#define  ASD0_CHIP_MODE                             16
#define  ASD0_DEADTIME	                			17
#define  ASD0_INT_GATE	                			20
#define  ASD0_RUND_CURR					            24
#define  ASD0_HYST_DAC                              27
#define  ASD0_WILK_THR_DAC                          31
#define  ASD0_MAIN_THR_DAC                          34
#define  ASD0_CAP_SELECT				            42
#define  ASD0_CH_MASK_TST_CTRL				        45
#define  ASD0_VMON					                53 

#define  ASD1_CHANNEL_0_MODE       					55
#define  ASD1_CHANNEL_1_MODE       					57
#define  ASD1_CHANNEL_2_MODE       					59
#define  ASD1_CHANNEL_3_MODE       					61
#define  ASD1_CHANNEL_4_MODE       					63
#define  ASD1_CHANNEL_5_MODE       					65
#define  ASD1_CHANNEL_6_MODE       					67
#define  ASD1_CHANNEL_7_MODE				        69
#define  ASD1_CHIP_MODE                             71
#define  ASD1_DEADTIME	                			72
#define  ASD1_INT_GATE	                			75
#define  ASD1_RUND_CURR					            79
#define  ASD1_HYST_DAC                              82
#define  ASD1_WILK_THR_DAC                          86
#define  ASD1_MAIN_THR_DAC                          89
#define  ASD1_CAP_SELECT				            97
#define  ASD1_CH_MASK_TST_CTRL				        100
#define  ASD1_VMON					                108

#define  ASD2_CHANNEL_0_MODE       					110
#define  ASD2_CHANNEL_1_MODE       					112
#define  ASD2_CHANNEL_2_MODE       					114
#define  ASD2_CHANNEL_3_MODE       					116
#define  ASD2_CHANNEL_4_MODE       					118
#define  ASD2_CHANNEL_5_MODE       					120
#define  ASD2_CHANNEL_6_MODE       					122
#define  ASD2_CHANNEL_7_MODE				        124
#define  ASD2_CHIP_MODE                             126
#define  ASD2_DEADTIME	                			127
#define  ASD2_INT_GATE	                			130
#define  ASD2_RUND_CURR					            134
#define  ASD2_HYST_DAC                              137
#define  ASD2_WILK_THR_DAC                          141
#define  ASD2_MAIN_THR_DAC                          144
#define  ASD2_CAP_SELECT				            152
#define  ASD2_CH_MASK_TST_CTRL				        155
#define  ASD2_VMON					                163
     
 
//value
#define  asd0_vmon           						0  
#define  asd0_ch_mask_tst_ctrl        				0    
#define  asd0_cap_select           		    		0         
#define  asd0_main_thr_dac                  		0   
#define  asd0_wilk_thr_dac         					2   
#define  asd0_hyst_dac             					7   //reversed LSB
#define  asd0_rund_curr            					3   
#define  asd0_int_gate             					4   
#define  asd0_deadtime             					7   
#define  asd0_chip_mode            					0   
#define  asd0_channel_7_mode       					0   //00 on 10 on 01 LO 11 HI 
#define  asd0_channel_6_mode       					0   
#define  asd0_channel_5_mode       					0   
#define  asd0_channel_4_mode       					0   
#define  asd0_channel_3_mode       					0   
#define  asd0_channel_2_mode       					0   
#define  asd0_channel_1_mode       					0   
#define  asd0_channel_0_mode       					0   

#define  asd1_vmon                 					0     	
#define  asd1_ch_mask_tst_ctrl     					0   
#define  asd1_cap_select           					0   
#define  asd1_main_thr_dac         					0 
#define  asd1_wilk_thr_dac         					2   
#define  asd1_hyst_dac             					7   
#define  asd1_rund_curr            					3   
#define  asd1_int_gate             					4   
#define  asd1_deadtime             					7   
#define  asd1_chip_mode            					0   
#define  asd1_channel_7_mode       					0   
#define  asd1_channel_6_mode       					0   
#define  asd1_channel_5_mode       					0   
#define  asd1_channel_4_mode       					0   
#define  asd1_channel_3_mode       					0   
#define  asd1_channel_2_mode       					0   
#define  asd1_channel_1_mode       					0   
#define  asd1_channel_0_mode       					0

#define  asd2_vmon                 					0   
#define  asd2_ch_mask_tst_ctrl     					0   
#define  asd2_cap_select           					0   
#define  asd2_main_thr_dac         					0 
#define  asd2_wilk_thr_dac         					2   
#define  asd2_hyst_dac             					7   
#define  asd2_rund_curr            					3   
#define  asd2_int_gate             					4   
#define  asd2_deadtime             					7   
#define  asd2_chip_mode            					0   
#define  asd2_channel_7_mode       					0   
#define  asd2_channel_6_mode       					0   
#define  asd2_channel_5_mode       					0   
#define  asd2_channel_4_mode       					0   
#define  asd2_channel_3_mode       					0   
#define  asd2_channel_2_mode       					0   
#define  asd2_channel_1_mode       					0   
#define  asd2_channel_0_mode       					0   




  


//setup register offsets
 


#define HPTDCSETUP_LENGTH				                647

#define TEST_SELECT 									0
#define ENABLE_ERROR_MARK 								4
#define ENABLE_ERROR_BYPASS 							5
#define ENABLE_ERROR									6
#define READOUT_SINGLE_CYCLE_SPEED						17
#define SERIAL_DELAY									20
#define STROBE_SELECT									24
#define READOUT_SPEED_SELECT							26
#define TOKEN_DELAY										27
#define ENABLE_LOCAL_TRAILER							31
#define ENABLE_LOCAL_HEADER								32
#define ENABLE_GLOBAL_TRAILER							33
#define ENABLE_GLOBAL_HEADER							34
#define KEEP_TOKEN										35
#define MASTER											36
#define ENABLE_BYTEWISE									37
#define ENABLE_SERIAL									38
#define ENABLE_JTAG_READOUT								39
#define TDC_ID_INDEX									40
#define SELECT_BYPASS_INPUTS							44
#define READOUT_FIFO_SIZE								45
#define REJECT_COUNT_OFFSET								48
#define SEARCH_WINDOW									60
#define MATCH_WINDOW									72
#define LEADING_RESOLUTION								84
#define FIXED_PATTERN									87
#define ENABLE_FIXED_PATTERN							115
#define MAX_EVENT_SIZE									116
#define REJECT_READOUT_FIFO_FULL						120
#define ENABLE_READOUT_OCCUPANCY						121
#define ENABLE_READOUT_SEPARATOR						122
#define ENABLE_OVERFLOW_DETECT							123
#define ENABLE_RELATIVE									124
#define ENABLE_AUTOMATIC_REJECT							125
#define EVENT_COUNT_OFFSET								126
#define TRIGGER_COUNT_OFFSET							138
#define ENABLE_SET_COUNTERS_ON_BUNCH_RESET				150
#define ENABLE_MASTER_RESET_CODE						151
#define ENABLE_MASTER_RESET_ON_EVENT_RESET				152
#define ENABLE_RESET_CHANNEL_BUFFER_WHENSEPARATOR		153
#define ENABLE_SEPARATOR_ON_EVENT_RESET					154
#define ENABLE_SEPARATOR_ON_BUNCH_RESET					155
#define ENABLE_DIRECT_EVENT_RESET						156
#define ENABLE_DIRECT_BUNCH_RESET						157
#define ENABLE_DIRECT_TRIGGER							158
#define OFFSET31										159
#define OFFSET30										168
#define OFFSET29										177
#define OFFSET28										186
#define OFFSET27										195
#define OFFSET26										204
#define OFFSET25										213
#define OFFSET24										222
#define OFFSET23										231
#define OFFSET22										240
#define OFFSET21										249
#define OFFSET20										258
#define OFFSET19										267
#define OFFSET18										276
#define OFFSET17										285
#define OFFSET16										294
#define OFFSET15										303
#define OFFSET14										312
#define OFFSET13										321
#define OFFSET12										330
#define OFFSET11										339
#define OFFSET10										348
#define OFFSET9											357
#define OFFSET8											366
#define OFFSET7											375
#define OFFSET6											384
#define OFFSET5											393
#define OFFSET4											402
#define OFFSET3											411
#define OFFSET2											420
#define OFFSET1											429
#define OFFSET0											438
#define	COARSE_COUNT_OFFSET								447
#define DLL_TAP_ADJUST									459
#define	RC_ADJUST										555
#define NOT_USED_567									567
#define LOW_POWER_MODE									570
#define WIDTH_SELECT									571
#define	VERNIER_OFFSET									575
#define	DLL_CONTROL										580
#define	DEAD_TIME										584
#define TEST_INVERT										586
#define TEST_MODE										587
#define ENABLE_TRAILING									588
#define ENABLE_LEADING									589
#define MODE_RC_COMPRESSION								590
#define MODE_RC											591
#define DLL_MODE										592
#define PLL_CONTROL										594
#define SERIAL_CLOCK_DELAY								602
#define IO_CLOCK_DELAY									606
#define CORE_CLOCK_DELAY								610
#define DLL_CLOCK_DELAY									614
#define SERIAL_CLOCK_SOURCE								618
#define IO_CLOCK_SOURCE									620
#define CORE_CLOCK_SOURCE								622
#define DLL_CLOCK_SOURCE								624
#define ROLL_OVER										627
#define ENABLE_MATCHING									639
#define ENABLE_PAIR										640
#define ENABLE_TTL_SERIAL								641
#define ENABLE_TTL_CONTROL								642
#define ENABLE_TTL_RESET								643
#define ENABLE_TTL_CLOCK								644
#define ENABLE_TTL_HIT									645
#define SETUP_PARITY									646


//Control register offsets

#define HPTDCCONTROL_LENGTH								40

#define ENABLE_PATTERN									0
#define GLOBLE_RESET									4
#define ENABLE_CHANNEL									5
#define DLL_RESET										37
#define PLL_RESET										38
#define CONTROL_PARITY									39



//Control registers
# define          hptdc_enable_pattern          		5
# define          hptdc_global_reset          			0
# define          hptdc_enable_channel   				16777215
//# define          hptdc_enable_channel   				8388609
# define              hptdc_dll_reset          			0
# define               hptdc_pll_reset          		0
# define          hptdc_control_parity          		0

//Setup registers

/*

# define             hptdc_test_select         			14
# define       hptdc_enable_error_mark          		1
# define     hptdc_enable_error_bypass          		0
# define            hptdc_enable_error       2047
# define      hptdc_readout_sincyspeed          0
# define            hptdc_serial_delay          0
# define           hptdc_strobe_select          0
# define    hptdc_readout_speed_select          1
# define             hptdc_token_delay          0
# define    hptdc_enable_local_trailer          1
# define     hptdc_enable_local_header          1
# define   hptdc_enable_global_trailer          0
# define    hptdc_enable_global_header          0
# define              hptdc_keep_token          1
# define                  hptdc_master          1
# define         hptdc_enable_bytewise          0
# define           hptdc_enable_serial          1
# define     hptdc_enable_jtag_readout          0
# define                  hptdc_tdc_id          0
# define    hptdc_select_bypass_inputs          0
# define       hptdc_readout_fifo_size          7
# define     hptdc_reject_count_offset          0
# define           hptdc_search_window         61
# define            hptdc_match_window         51
# define      hptdc_leading_resolution          1
# define          hptdc_fixed_pattern           0
# define   hptdc_enable_fixed_pattern           0
# define         hptdc_max_event_size           9
# define hptdc_reject_readout_fifo_full         1
# define hptdc_enable_readout_occupancy         0
# define hptdc_enable_readout_separator         0
# define hptdc_enable_overflow_detect           1
# define        hptdc_enable_relative           1
# define hptdc_enable_automatic_reject          1
# define     hptdc_event_count_offset           0
# define   hptdc_trigger_count_offset        3552
# define  hptdc_enable_set_counbunres           1
# define hptdc_enable_master_reset_code         1
# define hptdc_enable_mast_res_evnt_res          1
# define hptdc_enable_res_chn_buf_sepa          0
# define hptdc_enable_sepa_on_evnt_res          0
# define hptdc_enable_sepa_on_bnch_res          0
# define hptdc_enable_direct_evnt_res          0
# define hptdc_enable_direct_bnch_res          0
# define  hptdc_enable_direct_trigger          0
# define               hptdc_offset31          0
# define               hptdc_offset30          0
# define               hptdc_offset29          0
# define               hptdc_offset28          0
# define               hptdc_offset27          0
# define               hptdc_offset26          0
# define               hptdc_offset25          0
# define               hptdc_offset24          0
# define               hptdc_offset23          0
# define               hptdc_offset22          0
# define               hptdc_offset21          0
# define               hptdc_offset20          0
# define               hptdc_offset19          0
# define               hptdc_offset18          0
# define               hptdc_offset17          0
# define               hptdc_offset16          0
# define               hptdc_offset15          0
# define               hptdc_offset14          0
# define               hptdc_offset13          0
# define               hptdc_offset12          0
# define               hptdc_offset11          0
# define               hptdc_offset10          0
# define                hptdc_offset9          0
# define                hptdc_offset8          0
# define                hptdc_offset7          0
# define                hptdc_offset6          0
# define                hptdc_offset5          0
# define                hptdc_offset4          0
# define                hptdc_offset3          0
# define                hptdc_offset2          0
# define                hptdc_offset1          0
# define                hptdc_offset0          0
# define    hptdc_coarse_count_offset        118
# define         hptdc_dll_tap_adjust          0
# define              hptdc_rc_adjust          0
# define           hptdc_not_used_567          0
# define         hptdc_low_power_mode          0
# define           hptdc_width_select          1
# define         hptdc_vernier_offset          0
# define            hptdc_dll_control          1
# define              hptdc_dead_time          0
# define            hptdc_test_invert          0
# define              hptdc_test_mode          0
# define        hptdc_enable_trailing          1
# define         hptdc_enable_leading          1
# define    hptdc_mode_rc_compression          0
# define                hptdc_mode_rc          0
# define               hptdc_dll_mode          0
# define            hptdc_pll_control          4
# define     hptdc_serial_clock_delay          0
# define         hptdc_io_clock_delay          0
# define       hptdc_core_clock_delay          0
# define        hptdc_dll_clock_delay          0
# define    hptdc_serial_clock_source          0
# define        hptdc_io_clock_source          0
# define      hptdc_core_clock_source          0
# define       hptdc_dll_clock_source          1
# define              hptdc_roll_over       3563
# define        hptdc_enable_matching          1
# define            hptdc_enable_pair          0
# define      hptdc_enable_ttl_serial          0
# define     hptdc_enable_ttl_control          0
# define       hptdc_enable_ttl_reset          0
# define       hptdc_enable_ttl_clock          0
# define         hptdc_enable_ttl_hit          0
# define           hptdc_setup_parity          0

*/

/*

//1/29 parameter settings

# define             hptdc_test_select         			14
# define       hptdc_enable_error_mark          		1
# define     hptdc_enable_error_bypass          		0
# define            hptdc_enable_error       2047
# define      hptdc_readout_sincyspeed          0
# define            hptdc_serial_delay          0
# define           hptdc_strobe_select          0
# define    hptdc_readout_speed_select          1
# define             hptdc_token_delay          0
# define    hptdc_enable_local_trailer          1
# define     hptdc_enable_local_header          1
# define   hptdc_enable_global_trailer          0
# define    hptdc_enable_global_header          0
# define              hptdc_keep_token          1
# define                  hptdc_master          1
# define         hptdc_enable_bytewise          0
# define           hptdc_enable_serial          1
# define     hptdc_enable_jtag_readout          0
# define                  hptdc_tdc_id          0
# define    hptdc_select_bypass_inputs          0
# define       hptdc_readout_fifo_size          7
# define     hptdc_reject_count_offset         3975
# define           hptdc_search_window         75
# define            hptdc_match_window         50
# define      hptdc_leading_resolution          1
# define          hptdc_fixed_pattern           0
# define   hptdc_enable_fixed_pattern           0
# define         hptdc_max_event_size           9
# define hptdc_reject_readout_fifo_full         1
# define hptdc_enable_readout_occupancy         0
# define hptdc_enable_readout_separator         0
# define hptdc_enable_overflow_detect           1
# define        hptdc_enable_relative           1
# define hptdc_enable_automatic_reject          1
# define     hptdc_event_count_offset           0
# define   hptdc_trigger_count_offset          4015
# define  hptdc_enable_set_counbunres           1
# define hptdc_enable_master_reset_code         1
# define hptdc_enable_mast_res_evnt_res         1
# define hptdc_enable_res_chn_buf_sepa          0
# define hptdc_enable_sepa_on_evnt_res          0
# define hptdc_enable_sepa_on_bnch_res          0
# define hptdc_enable_direct_evnt_res          0
# define hptdc_enable_direct_bnch_res          0
# define  hptdc_enable_direct_trigger          0
# define               hptdc_offset31          0
# define               hptdc_offset30          0
# define               hptdc_offset29          0
# define               hptdc_offset28          0
# define               hptdc_offset27          0
# define               hptdc_offset26          0
# define               hptdc_offset25          0
# define               hptdc_offset24          0
# define               hptdc_offset23          0
# define               hptdc_offset22          0
# define               hptdc_offset21          0
# define               hptdc_offset20          0
# define               hptdc_offset19          0
# define               hptdc_offset18          0
# define               hptdc_offset17          0
# define               hptdc_offset16          0
# define               hptdc_offset15          0
# define               hptdc_offset14          0
# define               hptdc_offset13          0
# define               hptdc_offset12          0
# define               hptdc_offset11          0
# define               hptdc_offset10          0
# define                hptdc_offset9          0
# define                hptdc_offset8          0
# define                hptdc_offset7          0
# define                hptdc_offset6          0
# define                hptdc_offset5          0
# define                hptdc_offset4          0
# define                hptdc_offset3          0
# define                hptdc_offset2          0
# define                hptdc_offset1          0
# define                hptdc_offset0          0
# define    hptdc_coarse_count_offset          0
# define         hptdc_dll_tap_adjust          0
# define              hptdc_rc_adjust          0
# define           hptdc_not_used_567          0
# define         hptdc_low_power_mode          0
# define           hptdc_width_select          1
# define         hptdc_vernier_offset          0
# define            hptdc_dll_control          1
# define              hptdc_dead_time          0
# define            hptdc_test_invert          0
# define              hptdc_test_mode          0
# define        hptdc_enable_trailing          1
# define         hptdc_enable_leading          1
# define    hptdc_mode_rc_compression          0
# define                hptdc_mode_rc          0
# define               hptdc_dll_mode          0
# define            hptdc_pll_control          4
# define     hptdc_serial_clock_delay          0
# define         hptdc_io_clock_delay          0
# define       hptdc_core_clock_delay          0
# define        hptdc_dll_clock_delay          0
# define    hptdc_serial_clock_source          0
# define        hptdc_io_clock_source          0
# define      hptdc_core_clock_source          0
# define       hptdc_dll_clock_source          1
# define              hptdc_roll_over          3563
# define        hptdc_enable_matching          1
# define            hptdc_enable_pair          0
# define      hptdc_enable_ttl_serial          0
# define     hptdc_enable_ttl_control          0
# define       hptdc_enable_ttl_reset          0
# define       hptdc_enable_ttl_clock          0
# define         hptdc_enable_ttl_hit          0
# define           hptdc_setup_parity          1



//End

*/

//2/15 parameter settings

# define             hptdc_test_select         			14
# define       hptdc_enable_error_mark          		1
# define     hptdc_enable_error_bypass          		0
# define            hptdc_enable_error       2047
# define      hptdc_readout_sincyspeed          0
# define            hptdc_serial_delay          0
# define           hptdc_strobe_select          0
# define    hptdc_readout_speed_select          1
# define             hptdc_token_delay          0
# define    hptdc_enable_local_trailer          1
# define     hptdc_enable_local_header          1
# define   hptdc_enable_global_trailer          0
# define    hptdc_enable_global_header          0
# define              hptdc_keep_token          1
# define                  hptdc_master          1
# define         hptdc_enable_bytewise          0
# define           hptdc_enable_serial          1
# define     hptdc_enable_jtag_readout          0
# define                  hptdc_tdc_id          0
# define    hptdc_select_bypass_inputs          0
# define       hptdc_readout_fifo_size          7
# define     hptdc_reject_count_offset         3975
# define           hptdc_search_window         71
# define            hptdc_match_window         61
# define      hptdc_leading_resolution          1
# define          hptdc_fixed_pattern           0
# define   hptdc_enable_fixed_pattern           0
# define         hptdc_max_event_size           9
# define hptdc_reject_readout_fifo_full         1
# define hptdc_enable_readout_occupancy         0
# define hptdc_enable_readout_separator         0
# define hptdc_enable_overflow_detect           1
// # define        hptdc_enable_relative           1
# define        hptdc_enable_relative           0
# define hptdc_enable_automatic_reject          1
# define     hptdc_event_count_offset           0
# define   hptdc_trigger_count_offset          4015
# define  hptdc_enable_set_counbunres           1
# define hptdc_enable_master_reset_code         1
# define hptdc_enable_mast_res_evnt_res         1
# define hptdc_enable_res_chn_buf_sepa          0
# define hptdc_enable_sepa_on_evnt_res          0
# define hptdc_enable_sepa_on_bnch_res          0
# define hptdc_enable_direct_evnt_res          0
# define hptdc_enable_direct_bnch_res          0
# define  hptdc_enable_direct_trigger          0
# define               hptdc_offset31          0
# define               hptdc_offset30          0
# define               hptdc_offset29          0
# define               hptdc_offset28          0
# define               hptdc_offset27          0
# define               hptdc_offset26          0
# define               hptdc_offset25          0
# define               hptdc_offset24          0
# define               hptdc_offset23          0
# define               hptdc_offset22          0
# define               hptdc_offset21          0
# define               hptdc_offset20          0
# define               hptdc_offset19          0
# define               hptdc_offset18          0
# define               hptdc_offset17          0
# define               hptdc_offset16          0
# define               hptdc_offset15          0
# define               hptdc_offset14          0
# define               hptdc_offset13          0
# define               hptdc_offset12          0
# define               hptdc_offset11          0
# define               hptdc_offset10          0
# define                hptdc_offset9          0
# define                hptdc_offset8          0
# define                hptdc_offset7          0
# define                hptdc_offset6          0
# define                hptdc_offset5          0
# define                hptdc_offset4          0
# define                hptdc_offset3          0
# define                hptdc_offset2          0
# define                hptdc_offset1          0
# define                hptdc_offset0          0
# define    hptdc_coarse_count_offset          0
# define         hptdc_dll_tap_adjust          0
# define              hptdc_rc_adjust          0
# define           hptdc_not_used_567          0
# define         hptdc_low_power_mode          0
# define           hptdc_width_select          1
# define         hptdc_vernier_offset          0
# define            hptdc_dll_control          1
# define              hptdc_dead_time          0
# define            hptdc_test_invert          0
# define              hptdc_test_mode          0
# define        hptdc_enable_trailing          1
# define         hptdc_enable_leading          1
# define    hptdc_mode_rc_compression          0
# define                hptdc_mode_rc          0
# define               hptdc_dll_mode          0
# define            hptdc_pll_control          4
# define     hptdc_serial_clock_delay          0
# define         hptdc_io_clock_delay          0
# define       hptdc_core_clock_delay          0
# define        hptdc_dll_clock_delay          0
# define    hptdc_serial_clock_source          0
# define        hptdc_io_clock_source          0
# define      hptdc_core_clock_source          0
# define       hptdc_dll_clock_source          1
# define              hptdc_roll_over          4095
# define        hptdc_enable_matching          1
# define            hptdc_enable_pair          0
# define      hptdc_enable_ttl_serial          0
# define     hptdc_enable_ttl_control          0
# define       hptdc_enable_ttl_reset          0
# define       hptdc_enable_ttl_clock          0
# define         hptdc_enable_ttl_hit          0
# define           hptdc_setup_parity          0


//End

char mezzSetupFileName[281];
unsigned int numberSetupArrayError[AMTS_SETUP+AMTSETUP_LENGTH][MAXNUMBERMEZZANINE];
unsigned int numberMezzSetupBitError[MAXNUMBERMEZZANINE];
unsigned int numberMezzSetupError[MAXNUMBERMEZZANINE];
int basicSetupArray[AMTS_SETUP+AMTSETUP_LENGTH],   // arrays for one mezzanine card only
    mezzSetupArray[AMTS_SETUP+AMTSETUP_LENGTH][MAXNUMBERMEZZANINE+1],
    statusSetupCheck[AMTS_SETUP+AMTSETUP_LENGTH],
    AMTStatusArray[AMTSTATUSLENGTH][MAXNUMBERMEZZANINE];

	
int IndMezzEnableHandle, AMTSetupHandle, AMTErrorHandle, ASDSetupHandle,
    CopyMezzSetupHandle;
int nbMezzCard, mezzCardNb, mezzEnables, mezzCardSetupAll, downloadMezzSetup;
int mezzButtonID[MAXNUMBERMEZZANINE], mezzButtonForCopy[MAXNUMBERMEZZANINE+1],
    mezzButtonForMenu[MAXNUMBERMEZZANINE], mezzInitStatus[MAXNUMBERMEZZANINE];
int selectedASD, applyToASD1, applyToASD2 , applyToASD3;
int nominalThresholdUsed, NTHyst[3][MAXNUMBERMEZZANINE], NTThre[3][MAXNUMBERMEZZANINE];
int downloadMezzSetupDone, AMTIDCode[MAXNUMBERMEZZANINE+1];
int lEdgeOn[MAXNUMBERMEZZANINE], tEdgeOn[MAXNUMBERMEZZANINE], pairOn[MAXNUMBERMEZZANINE]; 
int AMTEdgesOn, AMTPairOn, widthSelection, integrationGate, rundownCurrent;

//
// Function prototypes
//
void MezzCardParameterInit(void);
void CheckNumberMezzCard(int panel);
void UpAMTSetupAllControl(void);
void UpMezz00AMTSetupControl(void);
void UpMezz01AMTSetupControl(void);
void UpMezz02AMTSetupControl(void);
void UpMezz03AMTSetupControl(void);
void UpMezz04AMTSetupControl(void);
void UpMezz05AMTSetupControl(void);
void UpMezz06AMTSetupControl(void);
void UpMezz07AMTSetupControl(void);
void UpMezz08AMTSetupControl(void);
void UpMezz09AMTSetupControl(void);
void UpMezz10AMTSetupControl(void);
void UpMezz11AMTSetupControl(void);
void UpMezz12AMTSetupControl(void);
void UpMezz13AMTSetupControl(void);
void UpMezz14AMTSetupControl(void);
void UpMezz15AMTSetupControl(void);
void UpMezz16AMTSetupControl(void);
void UpMezz17AMTSetupControl(void);
void DisableAllIndividualMezzSettings(void);
int MezzCardSetupPanelAndButton(void);
void DownloadAllMezzCardSetup(void);
void AMTGlobalReset(void);
int MezzCardsClockMode(void);

// AMT Setup Panel
void AMTDone(void);
void AMTCancel(void);
void AMTMezzCard(void);
void AMTStrobeSelection(void);
void AMTChannelAllOn(void);
void AMTChannelAllOff(void);
void AMTChannel0To7On(void);
void AMTChannel0To7Off(void);
void AMTChannel8To15On(void);
void AMTChannel8To15Off(void);
void AMTChannel16To23On(void);
void AMTChannel16To23Off(void);
void ASD1Control(void);
void ASD2Control(void);
void ASD3Control(void);
void SaveMezzCardSetup(void);
int WriteSetupFile(void);
void AMTErrorControl(void);
void CopyMezzSetupControl(void);
void DisablePair(void);
void DisableLeadingAndTrailing(void);
void LoadNominalASDMainThreshold(void);
void UpSerialNumberAndLoadNominalASDMainThreshold(void);
void UpSerialNumberAndReadNominalASDMainThreshold(void);
void LoadAMTSetupArray(void);
void RecallMezzanineSetup(void);

// AMT Error Panel
void AMTErrorControlDone(void);
void AMTErrorControlCancel(void);

// ASD Setup Panel 
void ASDDone(void);
void ASDCancel(void);
void ASDAllChannelOn(void);
void ASDAllChannelOff(void);
void ASDChannelsModeActive(void);
void ASDChannelsModeHigh(void);
void ASDChannelsModeLow(void);
void ApplyToAllASD(void);
void ApplyTo1stNextASD(void);
void ApplyTo2rdNextASD(void);
void LoadASDSetupArray(int ASDNumber);
 
// Copy Mezzanine Card Setup Panel
void CopyMezzSetup(void);
void CopyMezzSetupDone(void);
void CopyMezzSetupFrom(void);
void SelectAllForCopy(void);
void UnselectAllForCopy(void);
void SetMezzanineAMTChannel(int mezz);
void SetMezzanineASDChannel(int mezz, int ASDNumber);
void SetMezzanineThreshold(int mezz, int ASDNumber);

//Modified by Xiangting


void LoadHPTDCSetupArray(void);
void LoadHPTDCControlArray_step1(void);
void LoadHPTDCControlArray_step2(void);
void LoadHPTDCControlArray_step3(void);
void LoadHPTDCControlArray_step4(void);
void LoadHPTDCControlArray_step5(void);
void LoadHPTDCControlArray_step6(void);

int basicSetupArray_a3p250[A3P250SETUP_LENGTH];

//int basicSetupArray_a3p250[159];  
int basicSetupArray_h[647];
int basicControlArray_h1[40];
int basicControlArray_h2[40];
int basicControlArray_h3[40];
int basicControlArray_h4[40];
int basicControlArray_h5[40];
int basicControlArray_h6[40];
int HPTDCControl_array[40];


#define TDC_SETUP_MAX_LENGTH 200
int TDC_setup_array[TDC_SETUP_MAX_LENGTH];
int TDC_setup_length;

int LoadTDCSetup0Array(void);
int LoadTDCSetup1Array(void);
int LoadTDCSetup2Array(void);
int LoadTDCControl0Array(void);
int LoadTDCControl1Array(void);
int LoadA3P250SetupArray_new(void);
int LoadA3P250SetupArray_old(void);
void LoadTDCControl0Array_step1(void);
void LoadTDCControl0Array_step2(void);
void LoadTDCControl0Array_step3(void);





//umich TDC version 1

#define TDC_SETUP0_LENGTH 							115  


#define TDC_ENABLE_NEW_TTC_INDEX 					0
#define TDC_ENABLE_MASTER_RESET_CODE_INDEX			1  
#define TDC_ENABLE_DIRECT_BUNCH_RESET_INDEX			2
#define TDC_ENABLE_DIRECT_EVENT_RESET_INDEX			3
#define TDC_ENABLE_DIRECT_TRIGGER_INDEX				4
#define TDC_ENABLE_AUTO_ROLL_OVER_INDEX				5				
#define TDC_BYPASS_BCR_DISTRIBUTION_INDEX			6
#define TDC_ENABLE_TRIGGER_INDEX					7			
#define TDC_CHANNEL_DATA_DEBUG_INDEX				8				
#define TDC_ENABLE_LEADING_INDEX					9			
#define TDC_ENABLE_PAIR_INDEX						10		
#define TDC_ENABLE_FAKE_HIT_INDEX					11			
#define TDC_RISING_IS_LEADING_INDEX					12			
#define TDC_CHANNEL_ENABLE_R_INDEX					36			
#define TDC_CHANNEL_ENABLE_F_INDEX					60			
#define TDC_INNER_ID_INDEX							84
#define TDC_ENABLE_TRIGGER_TIMEOUT_INDEX			103					
#define TDC_ENABLE_HIGH_SPEED_INDEX					104			
#define TDC_ENABLE_LEGACY_INDEX						105		
#define TDC_FULL_WIDTH_RES_INDEX					106			
#define TDC_WIDTH_SELECT_INDEX						107		
#define TDC_ENABLE_8B10B_INDEX						110		
#define TDC_ENABLE_INSERT_INDEX						111		
#define TDC_ENABLE_ERROR_PACKET_INDEX				112				
#define TDC_ENABLE_ID_INDEX							113	
#define TDC_ENABLE_ERROR_NOTIFY_INDEX				114				



#define TDC_ENABLE_NEW_TTC 						0
#define TDC_ENABLE_MASTER_RESET_CODE			0  
#define TDC_ENABLE_DIRECT_BUNCH_RESET			0
#define TDC_ENABLE_DIRECT_EVENT_RESET			0
#define TDC_ENABLE_DIRECT_TRIGGER				0
#define TDC_ENABLE_AUTO_ROLL_OVER				1				
#define TDC_BYPASS_BCR_DISTRIBUTION				0
#define TDC_ENABLE_TRIGGER						1			
#define TDC_CHANNEL_DATA_DEBUG					0				
#define TDC_ENABLE_LEADING						0			
#define TDC_ENABLE_PAIR							1		
#define TDC_ENABLE_FAKE_HIT						1			
//#define TDC_RISING_IS_LEADING					0xFFFFFF
#define TDC_RISING_IS_LEADING					0x000000
//#define TDC_CHANNEL_ENABLE_R					0x000000
#define TDC_CHANNEL_ENABLE_R					0xFFFFFF
//#define TDC_CHANNEL_ENABLE_F					0x000000
#define TDC_CHANNEL_ENABLE_F					0xFFFFFF
#define TDC_INNER_ID							0x7AAAA
#define TDC_ENABLE_TRIGGER_TIMEOUT				0					
#define TDC_ENABLE_HIGH_SPEED					0			
#define TDC_ENABLE_LEGACY						1		
#define TDC_FULL_WIDTH_RES						0			
#define TDC_WIDTH_SELECT						0		
#define TDC_ENABLE_8B10B						0		
#define TDC_ENABLE_INSERT						0		
#define TDC_ENABLE_ERROR_PACKET					0				
#define TDC_ENABLE_ID							0	
#define TDC_ENABLE_ERROR_NOTIFY					0


#define TDC_SETUP1_LENGTH							94


#define TDC_COMBINE_TIME_OUT_CONFIG_INDEX			0				
#define TDC_FAKE_HIT_TIME_INTERVAL_INDEX			10				
#define TDC_SYN_PACKET_NUMBER_INDEX					22		
#define TDC_ROLL_OVER_INDEX							34
#define TDC_COARSE_COUNT_OFFSET_INDEX				46			
#define TDC_BUCH_OFFSET_INDEX						58	
#define TDC_EVENT_OFFSET_INDEX						70	
#define TDC_MATCH_WINDOW_INDEX						82

#define TDC_COMBINE_TIME_OUT_CONFIG				40				
#define TDC_FAKE_HIT_TIME_INTERVAL				256				
#define TDC_SYN_PACKET_NUMBER					0xFFF		
#define TDC_ROLL_OVER							0xFFF
#define TDC_COARSE_COUNT_OFFSET					0			
#define TDC_BUCH_OFFSET							0x000	//0xF9C	
#define TDC_EVENT_OFFSET						0x000	
#define TDC_MATCH_WINDOW						0x020


#define TDC_SETUP2_LENGTH							36


#define TDC_FINE_SEL_INDEX							0
#define TDC_LUT_INDEX 								4

#define TDC_FINE_SEL							0x3
#define TDC_LUT									0x19CA01CC



#define TDC_CONTROL0_LENGTH							8


#define TDC_RESET_EPLL_INDEX						0
#define TDC_RESET_JTAG_IN_INDEX						1
#define TDC_EVENT_RESET_JTAG_IN_INDEX				2		
#define TDC_CHNL_FIFO_OVERFLOW_CLEAR_INDEX			3			
#define TDC_DEBUG_PORT_SELECT_INDEX					4

#define TDC_RESET_EPLL							0
#define TDC_RESET_JTAG_IN						0
#define TDC_EVENT_RESET_JTAG_IN					0		
#define TDC_CHNL_FIFO_OVERFLOW_CLEAR			0			
#define TDC_DEBUG_PORT_SELECT					0x0


#define TDC_CONTROL1_LENGTH							47


#define TDC_PHASE_CLK160_INDEX						0
#define TDC_PHASE_CLK320_0_INDEX					5	
#define TDC_PHASE_CLK320_1_INDEX					9	
#define TDC_PHASE_CLK320_2_INDEX					13	
#define TDC_EPLL_RES_A_INDEX						17
#define TDC_EPLL_ICP_A_INDEX						21
#define TDC_EPLL_CAP_A_INDEX						25
#define TDC_EPLL_RES_B_INDEX						27
#define TDC_EPLL_ICP_B_INDEX						31
#define TDC_EPLL_CAP_B_INDEX						35
#define TDC_EPLL_RES_C_INDEX						37
#define TDC_EPLL_ICP_C_INDEX						41
#define TDC_EPLL_CAP_C_INDEX						45

#define TDC_PHASE_CLK160						0x00
#define TDC_PHASE_CLK320_0						0x4	
#define TDC_PHASE_CLK320_1						0x0	
#define TDC_PHASE_CLK320_2						0x2	
#define TDC_EPLL_RES_A							0x2
#define TDC_EPLL_ICP_A							0x4
#define TDC_EPLL_CAP_A							0x2
#define TDC_EPLL_RES_B							0x2
#define TDC_EPLL_ICP_B							0x4
#define TDC_EPLL_CAP_B							0x2
#define TDC_EPLL_RES_C							0x2
#define TDC_EPLL_ICP_C							0x4
#define TDC_EPLL_CAP_C							0x2


#define TDC_STATUS0_LENGTH							33 

#define TDC_INSTRUCTION_ERROR_INDEX					0
#define TDC_CRC_INDEX								1


#define TDC_STATUS1_LENGTH							25

#define TDC_EPLL_LOCK_INDEX							0
#define TDC_CHNL_FIFO_OVERFLOW_INDEX				1


#endif
