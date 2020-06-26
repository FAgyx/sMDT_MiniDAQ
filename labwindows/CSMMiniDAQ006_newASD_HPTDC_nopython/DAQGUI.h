/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2019. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  BGOCTRL_P                        1
#define  BGOCTRL_P_INHIBIT3DURATION       2       /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_INHIBIT3DELAY          3       /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_INHIBIT2DURATION       4       /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_INHIBIT2DELAY          5       /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_INHIBIT1DURATION       6       /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_INHIBIT1DELAY          7       /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_INHIBIT0DURATION       8       /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_INHIBIT0DELAY          9       /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_DONE                   10      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_DECORATION1            11      /* control type: deco, callback function: (none) */
#define  BGOCTRL_P_DECORATION0            12      /* control type: deco, callback function: (none) */
#define  BGOCTRL_P_TTCRXACCESS3           13      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_TTCRXACCESS2           14      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_TTCRXACCESS1           15      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_MODE3                  16      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_STARTNOTEMPTY3         17      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_LOADBGOSETUP           18      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_SAVEBGOSETUP           19      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_CYCLE3                 20      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_FPENABLE3              21      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_TTCRXACCESS0           22      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_FILLBGOFIFOS           23      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_MODE2                  24      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_STARTNOTEMPTY2         25      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_DATAORCOMMANDINPUT     26      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_SUBADDRINPUT           27      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_TTCRXADDRINPUT         28      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_TTCRXREGINPUT          29      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_FORMATINPUT            30      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_BOARDRESET             31      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_FORMAT3                32      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_CYCLE2                 33      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_FPENABLE2              34      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_TTCRXACCESS            35      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_DEFAULT                36      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_DATAORCOMMAND3         37      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_SUBADDRESS3            38      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_TTCRXADDRESS3          39      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_MODE1                  40      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_STARTNOTEMPTY1         41      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_FORMAT2                42      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_RETRANSMITBGOFIFO3     43      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_RETRANSMITBGOFIFO2     44      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_RETRANSMITBGOFIFO1     45      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_RETRANSMITBGOFIFO0     46      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_CYCLE1                 47      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_FPENABLE1              48      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_DATAORCOMMAND2         49      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_MODE0                  50      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_SUBADDRESS2            51      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_TTCRXADDRESS2          52      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_STARTNOTEMPTY0         53      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_CYCLE0                 54      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_FORMAT1                55      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_FPENABLE0              56      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_DATAORCOMMAND1         57      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_FORMAT0                58      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_FORMAT                 59      /* control type: textButton, callback function: (none) */
#define  BGOCTRL_P_SUBADDRESS1            60      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_DATAORCOMMAND0         61      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_TTCRXADDRESS1          62      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_DATAORCOMMAND          63      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_SUBADDRESS             64      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_SUBADDRESS0            65      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_BGOFIFO3ADDRESS        66      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_BGOFIFO2ADDRESS        67      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_BGOFIFO1ADDRESS        68      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_BGOFIFO0ADDRESS        69      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_TTCRXADDRESS0          70      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_TTCRXADDRESS           71      /* control type: numeric, callback function: (none) */
#define  BGOCTRL_P_ALLNO                  72      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_ALLYES                 73      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_ALLASYNCHRONOUS        74      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_ALLSYNCHRONOUS         75      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_ALLREPETIVE            76      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_ALLSINGLE              77      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_RETRANSMITDISABLEALL   78      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_FPDISABLEALL           79      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_GENERATEALLBGOSIGNAL   80      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_GENERATEBGO3SIGNAL     81      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_GENERATEBGO2SIGNAL     82      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_GENERATEBGO1SIGNAL     83      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_GENERATEBGO0SIGNAL     84      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_WRITEBGOFIFOALL        85      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_WRITEBGOFIFO3          86      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_WRITEBGOFIFO2          87      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_WRITEBGOFIFO1          88      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_WRITEBGOFIFO0          89      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_RESETBGOFIFOALL        90      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_BGOSIGNALETITLE        91      /* control type: textMsg, callback function: (none) */
#define  BGOCTRL_P_RESETBGOFIFO3          92      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_RESETBGOFIFO2          93      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_RESETBGOFIFO1          94      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_RETRANSMITENABLEALL    95      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_RESETBGOFIFO0          96      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_FPENABLEALL            97      /* control type: command, callback function: (none) */
#define  BGOCTRL_P_WRITEBGOFIFOETITLE     98      /* control type: textMsg, callback function: (none) */
#define  BGOCTRL_P_RETRANSMITBGOFIFO      99      /* control type: textMsg, callback function: (none) */
#define  BGOCTRL_P_EXECUTEBCHANNEL        100     /* control type: command, callback function: (none) */
#define  BGOCTRL_P_BGO3TITLE              101     /* control type: textMsg, callback function: (none) */
#define  BGOCTRL_P_BGO1TITLE              102     /* control type: textMsg, callback function: (none) */
#define  BGOCTRL_P_BGO0TITLE              103     /* control type: textMsg, callback function: (none) */
#define  BGOCTRL_P_RESETBGOFIFOTITLE      104     /* control type: textMsg, callback function: (none) */
#define  BGOCTRL_P_BGOMODETITLE           105     /* control type: textMsg, callback function: (none) */
#define  BGOCTRL_P_BGO2TITLE              106     /* control type: textBox, callback function: (none) */

#define  P_AMTERROR                       2
#define  P_AMTERROR_DONE                  2       /* control type: command, callback function: (none) */
#define  P_AMTERROR_AMT_ERR_MARK          3       /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_AMT_ERR_COARSE        4       /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_AMT_JTAG_INSTR        5       /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_AMT_ERR_SETUP_PAR     6       /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_AMT_ERR_RO_STATE      7       /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_AMT_ERR_RO_FIFO_PAR   8       /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_AMT_TRIG_MATCH        9       /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_AMT_TRIG_FIFO_PAR     10      /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_ERRORFLAGTEST         11      /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_AMT_ERR_L1_BUFFER_PAR 12      /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_AMT_ERR_CH_SEL        13      /* control type: textButton, callback function: (none) */
#define  P_AMTERROR_CANCEL                14      /* control type: command, callback function: (none) */

#define  P_AMTSETUP                       3
#define  P_AMTSETUP_LOADTHRESHOLD         2       /* control type: command, callback function: (none) */
#define  P_AMTSETUP_MEZZCARD              3       /* control type: numeric, callback function: (none) */
#define  P_AMTSETUP_COARSE_OFFSET         4       /* control type: numeric, callback function: (none) */
#define  P_AMTSETUP_BC_OFFSET             5       /* control type: numeric, callback function: (none) */
#define  P_AMTSETUP_EC_OFFSET             6       /* control type: numeric, callback function: (none) */
#define  P_AMTSETUP_RC_OFFSET             7       /* control type: numeric, callback function: (none) */
#define  P_AMTSETUP_MASK_WINDOW           8       /* control type: numeric, callback function: (none) */
#define  P_AMTSETUP_SEARCH_WINDOW         9       /* control type: numeric, callback function: (none) */
#define  P_AMTSETUP_MATCH_WINDOW          10      /* control type: numeric, callback function: (none) */
#define  P_AMTSETUP_PULSE_WIDTH           11      /* control type: ring, callback function: (none) */
#define  P_AMTSETUP_EN_SERIAL             12      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_DONE                  13      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_ASD3CONTROL           14      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CNT_ROLLOVER          15      /* control type: numeric, callback function: (none) */
#define  P_AMTSETUP_STROBE_SEL            16      /* control type: ring, callback function: (none) */
#define  P_AMTSETUP_RD_SPEED              17      /* control type: ring, callback function: (none) */
#define  P_AMTSETUP_COPYSETUP             18      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_SAVESETUP             19      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_ASD2CONTROL           20      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CHANNEL23             21      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL22             22      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL21             23      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL20             24      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL19             25      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL18             26      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL17             27      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL16             28      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL15             29      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL14             30      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL13             31      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL12             32      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_LNTCONTROL            33      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_GLOBALRESET           34      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL11             35      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL10             36      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL9              37      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_ALLCHANNELOFF         38      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_ALLCHANNELON          39      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CHANNEL0TO7OFF        40      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CHANNEL0TO7ON         41      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CHANNEL8TO15OFF       42      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CHANNEL8TO15ON        43      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CHANNEL16TO23OFF      44      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CHANNEL16TO23ON       45      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CHANNEL8              46      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL7              47      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL6              48      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_ASD1CONTROL           49      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CHANNEL5              50      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL4              51      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL3              52      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL2              53      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL1              54      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_CHANNEL0              55      /* control type: textButton, callback function: (none) */
#define  P_AMTSETUP_RELATIVE              56      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_LEADING               57      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_TRAILING              58      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_PAIR                  59      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_MASK_FLAGS            60      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_ENC_MASTER_RESET      61      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_MASTER_RESET_EVT      62      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_TRIG_MATCH            63      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_INCLKBOOST            64      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_ERR_MARK_REJ          65      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_DIR_CNTL              66      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_SEP_EVT_RESET         67      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_SEP_BNCH_RESET        68      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_RST_CHAN_BUFF_SEP     69      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_L1_BUFF_OCC_RO        70      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_SEPARATOR_RO          71      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_ERR_MARK_OVERFLOW     72      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_AUTO_REJ              73      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_BUFF_OVERFLOW_DET     74      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_SET_CTRS_BNCH_RST     75      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_ERRORCONTROL          76      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_DEFAULT               77      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_CANCEL                78      /* control type: command, callback function: (none) */
#define  P_AMTSETUP_TEST_MODE             79      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_TEST_INVERT_MODE      80      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_TRIG_FULL_REJ         81      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_L1_FULL_REJ           82      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_RO_FULL_REJ           83      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_TRAILER               84      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_HEADER                85      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_PLL_MULT              86      /* control type: ring, callback function: (none) */
#define  P_AMTSETUP_CLKOUT_MODE           87      /* control type: ring, callback function: (none) */
#define  P_AMTSETUP_DIS_RING              88      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_ERST_BCECRST          89      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_DIS_ENCODE            90      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_ENABLE_REJECTED       91      /* control type: binary, callback function: (none) */
#define  P_AMTSETUP_CHANNELCONTROL        92      /* control type: textMsg, callback function: (none) */

#define  P_ASDSETUP                       4
#define  P_ASDSETUP_CHANNEL4MODE          2       /* control type: slide, callback function: (none) */
#define  P_ASDSETUP_CHANNEL3MODE          3       /* control type: slide, callback function: (none) */
#define  P_ASDSETUP_CHANNEL2MODE          4       /* control type: slide, callback function: (none) */
#define  P_ASDSETUP_CHANNEL1MODE          5       /* control type: slide, callback function: (none) */
#define  P_ASDSETUP_CHANNEL7MODE          6       /* control type: slide, callback function: (none) */
#define  P_ASDSETUP_CHANNEL6MODE          7       /* control type: slide, callback function: (none) */
#define  P_ASDSETUP_CHANNEL5MODE          8       /* control type: slide, callback function: (none) */
#define  P_ASDSETUP_CHANNEL0MODE          9       /* control type: slide, callback function: (none) */
#define  P_ASDSETUP_MAINTHRESHOLD         10      /* control type: numeric, callback function: (none) */
#define  P_ASDSETUP_DEADTIME              11      /* control type: ring, callback function: (none) */
#define  P_ASDSETUP_HYSTERESIS            12      /* control type: ring, callback function: (none) */
#define  P_ASDSETUP_WIL_INT               13      /* control type: ring, callback function: (none) */
#define  P_ASDSETUP_APPLYTO2RDNEXTASD     14      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_APPLYTO1STNEXTASD     15      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_APPLYTOALLASD         16      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_WIL_THRESH            17      /* control type: ring, callback function: (none) */
#define  P_ASDSETUP_CALIBCAPACITOR        18      /* control type: ring, callback function: (none) */
#define  P_ASDSETUP_WIL_CUR               19      /* control type: ring, callback function: (none) */
#define  P_ASDSETUP_CHANNEL7              20      /* control type: textButton, callback function: (none) */
#define  P_ASDSETUP_CHANNEL6              21      /* control type: textButton, callback function: (none) */
#define  P_ASDSETUP_CHANNEL5              22      /* control type: textButton, callback function: (none) */
#define  P_ASDSETUP_CHANNEL4              23      /* control type: textButton, callback function: (none) */
#define  P_ASDSETUP_CHANNEL3              24      /* control type: textButton, callback function: (none) */
#define  P_ASDSETUP_CHANNEL2              25      /* control type: textButton, callback function: (none) */
#define  P_ASDSETUP_CHANNEL1              26      /* control type: textButton, callback function: (none) */
#define  P_ASDSETUP_CHANNEL0              27      /* control type: textButton, callback function: (none) */
#define  P_ASDSETUP_DONE                  28      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_CHANNELSMODELOW       29      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_CHANNELSMODEACTIVE    30      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_CHANNELSMODEHIGH      31      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_DEFAULT               32      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_CANCEL                33      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_ALLCHANNELOFF         34      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_ALLCHANNELON          35      /* control type: command, callback function: (none) */
#define  P_ASDSETUP_CHMODETEXT            36      /* control type: textBox, callback function: (none) */
#define  P_ASDSETUP_CALENATEXT            37      /* control type: textBox, callback function: (none) */
#define  P_ASDSETUP_CHIPMODE              38      /* control type: binary, callback function: (none) */

#define  P_AUTORUN                        5
#define  P_AUTORUN_RESTARTHANGTIME        2       /* control type: numeric, callback function: (none) */
#define  P_AUTORUN_STOPAFTERNERR          3       /* control type: numeric, callback function: (none) */
#define  P_AUTORUN_STOPAFTERRUNS          4       /* control type: numeric, callback function: (none) */
#define  P_AUTORUN_STOPATEVENT            5       /* control type: numeric, callback function: (none) */
#define  P_AUTORUN_DONE                   6       /* control type: command, callback function: (none) */
#define  P_AUTORUN_RESTARTAFTERSEC        7       /* control type: numeric, callback function: (none) */
#define  P_AUTORUN_RESTARTATEVENT         8       /* control type: numeric, callback function: (none) */
#define  P_AUTORUN_RESTARTHANGON          9       /* control type: textButton, callback function: (none) */
#define  P_AUTORUN_STOPAFTERNERRON        10      /* control type: textButton, callback function: (none) */
#define  P_AUTORUN_STOPAFTERRUNSON        11      /* control type: textButton, callback function: (none) */
#define  P_AUTORUN_RESTARTAFTERSECON      12      /* control type: textButton, callback function: (none) */
#define  P_AUTORUN_STOPEVENTON            13      /* control type: textButton, callback function: (none) */
#define  P_AUTORUN_RESTARTEVENTON         14      /* control type: textButton, callback function: (none) */
#define  P_AUTORUN_ALLON                  15      /* control type: command, callback function: (none) */
#define  P_AUTORUN_RUNCONTROLFILE         16      /* control type: command, callback function: (none) */
#define  P_AUTORUN_ALLOFF                 17      /* control type: command, callback function: (none) */
#define  P_AUTORUN_DECORATION             18      /* control type: deco, callback function: (none) */
#define  P_AUTORUN_SCANCONTROL            19      /* control type: textButton, callback function: (none) */
#define  P_AUTORUN_RUNS                   20      /* control type: textMsg, callback function: (none) */
#define  P_AUTORUN_SECONDS                21      /* control type: textMsg, callback function: (none) */
#define  P_AUTORUN_INITDAQCONTROL         22      /* control type: textButton, callback function: (none) */
#define  P_AUTORUN_AUTOSTARTRUN           23      /* control type: textButton, callback function: (none) */

#define  P_COPYMEZZ                       6
#define  P_COPYMEZZ_COPYINGTOMEZZ         2       /* control type: numeric, callback function: (none) */
#define  P_COPYMEZZ_UNSELECTALL           3       /* control type: command, callback function: (none) */
#define  P_COPYMEZZ_SELECTALL             4       /* control type: command, callback function: (none) */
#define  P_COPYMEZZ_STARTCOPY             5       /* control type: command, callback function: (none) */
#define  P_COPYMEZZ_DONE                  6       /* control type: command, callback function: (none) */
#define  P_COPYMEZZ_FROM                  7       /* control type: ring, callback function: (none) */
#define  P_COPYMEZZ_ASDCHANNEL            8       /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_AMTCHANNEL            9       /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MAINTHRESHOLD         10      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ17                11      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ16                12      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ15                13      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ14                14      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ13                15      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ12                16      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ11                17      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ10                18      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ09                19      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ08                20      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ07                21      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ06                22      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ05                23      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ04                24      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ03                25      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ02                26      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ01                27      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_MEZZ00                28      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_ALLSETUP              29      /* control type: textButton, callback function: (none) */
#define  P_COPYMEZZ_DECORATION            30      /* control type: deco, callback function: (none) */
#define  P_COPYMEZZ_INCLUDING             31      /* control type: textMsg, callback function: (none) */
#define  P_COPYMEZZ_TO                    32      /* control type: textMsg, callback function: (none) */

#define  P_CSM                            7
#define  P_CSM_DETAIL                     2       /* control type: command, callback function: (none) */
#define  P_CSM_DONE                       3       /* control type: command, callback function: (none) */
#define  P_CSM_CANCEL                     4       /* control type: command, callback function: (none) */
#define  P_CSM_DEFAULT                    5       /* control type: command, callback function: (none) */
#define  P_CSM_TTCRXCONTROL               6       /* control type: command, callback function: (none) */
#define  P_CSM_GOLCONTROL                 7       /* control type: command, callback function: (none) */
#define  P_CSM_SAVESETUP                  8       /* control type: command, callback function: (none) */
#define  P_CSM_DECORATION                 9       /* control type: deco, callback function: (none) */
#define  P_CSM_INFORTEXT                  10      /* control type: textMsg, callback function: (none) */
#define  P_CSM_VERSIONDATE                11      /* control type: numeric, callback function: (none) */
#define  P_CSM_CSMVERSION                 12      /* control type: numeric, callback function: (none) */
#define  P_CSM_PHASEERROR                 13      /* control type: textButton, callback function: (none) */
#define  P_CSM_ORUNUSED                   14      /* control type: textButton, callback function: (none) */
#define  P_CSM_TTCRXREADY                 15      /* control type: textButton, callback function: (none) */
#define  P_CSM_CSMERROR                   16      /* control type: textButton, callback function: (none) */
#define  P_CSM_I2COPERATION               17      /* control type: textButton, callback function: (none) */
#define  P_CSM_TTCRXSETUPERRDUMP          18      /* control type: textButton, callback function: (none) */
#define  P_CSM_TTCRXSETUPERRI2C           19      /* control type: textButton, callback function: (none) */
#define  P_CSM_GOLREADY                   20      /* control type: textButton, callback function: (none) */
#define  P_CSM_TTCLOADSTATUS              21      /* control type: textButton, callback function: (none) */
#define  P_CSM_GOLLD                      22      /* control type: ring, callback function: (none) */
#define  P_CSM_XMTLOCK2                   23      /* control type: textButton, callback function: (none) */
#define  P_CSM_XMTLOCK1                   24      /* control type: textButton, callback function: (none) */
#define  P_CSM_CSMNEXTSTATE               25      /* control type: ring, callback function: (none) */
#define  P_CSM_CSMSTATE                   26      /* control type: ring, callback function: (none) */
#define  P_CSM_CSMPAIRDEBUG               27      /* control type: textButton, callback function: (none) */
#define  P_CSM_CSMMODE                    28      /* control type: textButton, callback function: (none) */
#define  P_CSM_CSMMEZZLINK                29      /* control type: textButton, callback function: (none) */
#define  P_CSM_LHCLOCK                    30      /* control type: textButton, callback function: (none) */
#define  P_CSM_FIFOTHRESHOLD2             31      /* control type: numeric, callback function: (none) */
#define  P_CSM_FIFOTHRESHOLD1             32      /* control type: numeric, callback function: (none) */
#define  P_CSM_MAXALLOWEDMEZZS            33      /* control type: numeric, callback function: (none) */
#define  P_CSM_AMTCOMMANDDELAY            34      /* control type: numeric, callback function: (none) */
#define  P_CSM_SPARE                      35      /* control type: numeric, callback function: (none) */
#define  P_CSM_CSMTYPE                    36      /* control type: ring, callback function: (none) */
#define  P_CSM_AMTPHASEERROR              37      /* control type: numeric, callback function: (none) */
#define  P_CSM_PARITYERROR                38      /* control type: numeric, callback function: (none) */
#define  P_CSM_MEZZ17                     39      /* control type: textButton, callback function: (none) */
#define  P_CSM_PAIRCONTROL                40      /* control type: ring, callback function: (none) */
#define  P_CSM_PWIDTHRESOLUTION           41      /* control type: ring, callback function: (none) */
#define  P_CSM_MEZZ16                     42      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ15                     43      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ14                     44      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ13                     45      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ12                     46      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ11                     47      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ10                     48      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ09                     49      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ08                     50      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ07                     51      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ06                     52      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ05                     53      /* control type: textButton, callback function: (none) */
#define  P_CSM_GOLDIFF                    54      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ04                     55      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ03                     56      /* control type: textButton, callback function: (none) */
#define  P_CSM_CLEARALLMEZZ               57      /* control type: command, callback function: (none) */
#define  P_CSM_SETALLMEZZ                 58      /* control type: command, callback function: (none) */
#define  P_CSM_MEZZ02                     59      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ01                     60      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZ00                     61      /* control type: textButton, callback function: (none) */
#define  P_CSM_CSMCMD                     62      /* control type: textButton, callback function: (none) */
#define  P_CSM_GOLTDI                     63      /* control type: textButton, callback function: (none) */
#define  P_CSM_TTCRXTDI                   64      /* control type: textButton, callback function: (none) */
#define  P_CSM_GOLMODE                    65      /* control type: textButton, callback function: (none) */
#define  P_CSM_GOLNEG                     66      /* control type: textButton, callback function: (none) */
#define  P_CSM_PASSALLAMTDATA             67      /* control type: textButton, callback function: (none) */
#define  P_CSM_SYNCHWORDCONTROL           68      /* control type: textButton, callback function: (none) */
#define  P_CSM_DROPAMTHDTR                69      /* control type: textButton, callback function: (none) */
#define  P_CSM_GOLLASER                   70      /* control type: textButton, callback function: (none) */
#define  P_CSM_EMPTYCYCLECONTROL          71      /* control type: textButton, callback function: (none) */
#define  P_CSM_GOLPLL                     72      /* control type: textButton, callback function: (none) */
#define  P_CSM_TTCRXUSEPROM               73      /* control type: textButton, callback function: (none) */
#define  P_CSM_CSMDAQCONTROL              74      /* control type: textButton, callback function: (none) */
#define  P_CSM_BCIDMATCH                  75      /* control type: textButton, callback function: (none) */
#define  P_CSM_EVIDMATCH                  76      /* control type: textButton, callback function: (none) */
#define  P_CSM_MEZZJTAGENABLE             77      /* control type: textButton, callback function: (none) */
#define  P_CSM_TTCRXSTATUSBITS            78      /* control type: textBox, callback function: (none) */
#define  P_CSM_TTCRXSETUPBITS             79      /* control type: textBox, callback function: (none) */

#define  P_CSMIO                          8
#define  P_CSMIO_READIOS                  2       /* control type: command, callback function: (none) */
#define  P_CSMIO_SETIOS                   3       /* control type: command, callback function: (none) */
#define  P_CSMIO_PROGRAMFPGA              4       /* control type: command, callback function: (none) */
#define  P_CSMIO_RESETCSM                 5       /* control type: command, callback function: (none) */
#define  P_CSMIO_DONE                     6       /* control type: command, callback function: (none) */
#define  P_CSMIO_SELECTHARDTDC            7       /* control type: command, callback function: (none) */
#define  P_CSMIO_SELECTSOFTTDC            8       /* control type: command, callback function: (none) */
#define  P_CSMIO_DECORATION               9       /* control type: deco, callback function: (none) */
#define  P_CSMIO_INFORTEXT                10      /* control type: textMsg, callback function: (none) */
#define  P_CSMIO_READYS                   11      /* control type: textButton, callback function: (none) */
#define  P_CSMIO_CSMERROR                 12      /* control type: textButton, callback function: (none) */
#define  P_CSMIO_I2COPERATION             13      /* control type: textButton, callback function: (none) */
#define  P_CSMIO_BSOFTWARETDO             14      /* control type: textButton, callback function: (none) */
#define  P_CSMIO_BHARDWARETDO             15      /* control type: textButton, callback function: (none) */
#define  P_CSMIO_READBACKIOS              16      /* control type: numeric, callback function: (none) */
#define  P_CSMIO_WANTEDIOS                17      /* control type: numeric, callback function: (none) */
#define  P_CSMIO_RESETLEVEL               18      /* control type: textButton, callback function: (none) */
#define  P_CSMIO_PROGRAMFPGABAR           19      /* control type: textButton, callback function: (none) */
#define  P_CSMIO_DEFAULT                  20      /* control type: command, callback function: (none) */
#define  P_CSMIO_CANCEL                   21      /* control type: command, callback function: (none) */

#define  P_DAQINFOR                       9
#define  P_DAQINFOR_CLOSE                 2       /* control type: command, callback function: (none) */
#define  P_DAQINFOR_DATARATE_MAX          3       /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_EVENTSIZE_MAX         4       /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_TRIGRATE_MAX          5       /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_DAQRATE_MAX           6       /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_DATARATE_MIN          7       /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_TRIGRATE_MIN          8       /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_EVENTSIZE_MIN         9       /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_DAQRATE_MIN           10      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_TRIGRATE_AVER         11      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_DATARATE_AVER         12      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_EVENTSIZE_AVER        13      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_TRIGRATE_CURR         14      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_DAQRATE_AVER          15      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_DATARATE_CURR         16      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_EVENTSIZE_CURR        17      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_DAQRATE_CURR          18      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_NUMBEREVENT           19      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_RUNNUMBER             20      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_EVENTID               21      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_DAQTIME               22      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_GRAPH_DATARATE        23      /* control type: graph, callback function: (none) */
#define  P_DAQINFOR_GRAPH_DAQRATE         24      /* control type: graph, callback function: (none) */
#define  P_DAQINFOR_DATETIME              25      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_STOPTIME              26      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_STARTTIME             27      /* control type: textBox, callback function: (none) */
#define  P_DAQINFOR_DISPLAYPLOTS          28      /* control type: textButton, callback function: (none) */
#define  P_DAQINFOR_CURRENT               29      /* control type: textMsg, callback function: (none) */
#define  P_DAQINFOR_AVERAGE               30      /* control type: textMsg, callback function: (none) */
#define  P_DAQINFOR_MINIMUM               31      /* control type: textMsg, callback function: (none) */
#define  P_DAQINFOR_MAXIMUM               32      /* control type: textMsg, callback function: (none) */

#define  P_GOL                            10
#define  P_GOL_DONE                       2       /* control type: command, callback function: (none) */
#define  P_GOL_CANCEL                     3       /* control type: command, callback function: (none) */
#define  P_GOL_DEFAULT                    4       /* control type: command, callback function: (none) */
#define  P_GOL_CSMCONTROL                 5       /* control type: command, callback function: (none) */
#define  P_GOL_TTCRXCONTROL               6       /* control type: command, callback function: (none) */
#define  P_GOL_SAVESETUP                  7       /* control type: command, callback function: (none) */
#define  P_GOL_DECORATION                 8       /* control type: deco, callback function: (none) */
#define  P_GOL_INFORTEXT                  9       /* control type: textMsg, callback function: (none) */
#define  P_GOL_LINKMODE                   10      /* control type: ring, callback function: (none) */
#define  P_GOL_LINKSTATEC                 11      /* control type: ring, callback function: (none) */
#define  P_GOL_LINKSTATEB                 12      /* control type: ring, callback function: (none) */
#define  P_GOL_LINKSTATEA                 13      /* control type: ring, callback function: (none) */
#define  P_GOL_LOSSOFLOCKTIME             14      /* control type: ring, callback function: (none) */
#define  P_GOL_PLLCURRENT                 15      /* control type: ring, callback function: (none) */
#define  P_GOL_PLLLOCKTIME                16      /* control type: ring, callback function: (none) */
#define  P_GOL_WAITTIME                   17      /* control type: ring, callback function: (none) */
#define  P_GOL_CLOCK                      18      /* control type: ring, callback function: (none) */
#define  P_GOL_DRIVERSTRENGTH             19      /* control type: numeric, callback function: (none) */
#define  P_GOL_SELECTTESTSIGNALS          20      /* control type: numeric, callback function: (none) */
#define  P_GOL_HAMMINGCHECKSUM            21      /* control type: numeric, callback function: (none) */
#define  P_GOL_NBLOSSOFLOCK               22      /* control type: numeric, callback function: (none) */
#define  P_GOL_LOCKLOSSCOUNTERCTRL        23      /* control type: textButton, callback function: (none) */
#define  P_GOL_LOCKLOSSCONTROL            24      /* control type: textButton, callback function: (none) */
#define  P_GOL_STRENGTHTEXT               25      /* control type: textBox, callback function: (none) */
#define  P_GOL_DRIVER                     26      /* control type: textButton, callback function: (none) */
#define  P_GOL_CURRENTCONTROL             27      /* control type: textButton, callback function: (none) */
#define  P_GOL_FLAGBITSINGLINK            28      /* control type: textButton, callback function: (none) */
#define  P_GOL_READY                      29      /* control type: textButton, callback function: (none) */
#define  P_GOL_TESTSHIFT                  30      /* control type: textButton, callback function: (none) */
#define  P_GOL_TXEN                       31      /* control type: textButton, callback function: (none) */
#define  P_GOL_TXER                       32      /* control type: textButton, callback function: (none) */
#define  P_GOL_RESET                      33      /* control type: textButton, callback function: (none) */
#define  P_GOL_FORCELOCK                  34      /* control type: textButton, callback function: (none) */
#define  P_GOL_SELFTEST                   35      /* control type: textButton, callback function: (none) */

#define  P_INDMEZZS                       11
#define  P_INDMEZZS_MEZZ15                2       /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ17                3       /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ16                4       /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ13                5       /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ14                6       /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ11                7       /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ12                8       /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ09                9       /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ10                10      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ06                11      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ08                12      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ07                13      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ04                14      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ05                15      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ02                16      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ03                17      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ00                18      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_MEZZ01                19      /* control type: binary, callback function: (none) */
#define  P_INDMEZZS_TEXTMSG               20      /* control type: textMsg, callback function: (none) */

#define  P_JTAGCTRL                       12
#define  P_JTAGCTRL_QUIT                  2       /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_MESSAGE               3       /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_NEWDOSE               4       /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_KILLALLTCPCLIENTS     5       /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_INITDAQ               6       /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_STARTORSTOPDAQ        7       /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_DATAOUT               8       /* control type: ring, callback function: (none) */
#define  P_JTAGCTRL_DAQTIME               9       /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_NUMBERERROR           10      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_PAUSEORRESUMEDAQ      11      /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_TRIGRATE              12      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_DAQRATE               13      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_TCPSTATUS             14      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_FILENAME              15      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_DECORATION0           16      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_NUMBEREVENT           17      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_RESETCSMCHIP          18      /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_SETUPMULTCSMRDOUT     19      /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_SAMPLEAMTPHASE        20      /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_STOPTIME              21      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_STARTTIME             22      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_JTAGCHAINCONTROL      23      /* control type: textButton, callback function: (none) */
#define  P_JTAGCTRL_STARTORSTOP           24      /* control type: command, callback function: (none) */
#define  P_JTAGCTRL_MEZZSETUPSTATUS       25      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_TTCRXSETUPSTATUS      26      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_GOLSETUPSTATUS        27      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_CSMSETUPSTATUS        28      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_DAQINFOR              29      /* control type: textButton, callback function: (none) */
#define  P_JTAGCTRL_CSMCHIP               30      /* control type: textButton, callback function: (none) */
#define  P_JTAGCTRL_MEZZCARDS             31      /* control type: textButton, callback function: (none) */
#define  P_JTAGCTRL_CSM                   32      /* control type: textButton, callback function: (none) */
#define  P_JTAGCTRL_TTCRX                 33      /* control type: textButton, callback function: (none) */
#define  P_JTAGCTRL_GOL                   34      /* control type: textButton, callback function: (none) */
#define  P_JTAGCTRL_CSMTYPE               35      /* control type: ring, callback function: (none) */
#define  P_JTAGCTRL_RUNTYPE               36      /* control type: ring, callback function: (none) */
#define  P_JTAGCTRL_PROM                  37      /* control type: textButton, callback function: (none) */
#define  P_JTAGCTRL_DATETIME              38      /* control type: textBox, callback function: (none) */
#define  P_JTAGCTRL_LINE7                 39      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_LINE                  40      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_LINE6                 41      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_LINE0                 42      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_TDO                   43      /* control type: textMsg, callback function: (none) */
#define  P_JTAGCTRL_TDI                   44      /* control type: textMsg, callback function: (none) */
#define  P_JTAGCTRL_LINE1                 45      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_LINE2                 46      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_LINE3                 47      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_LINE5                 48      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_LINE4                 49      /* control type: deco, callback function: (none) */
#define  P_JTAGCTRL_CSMBOARDSTATUS        50      /* control type: ring, callback function: (none) */
#define  P_JTAGCTRL_JTAGRATEDIVISOR       51      /* control type: numeric, callback function: (none) */
#define  P_JTAGCTRL_CSMNUMBER             52      /* control type: numeric, callback function: (none) */
#define  P_JTAGCTRL_NUMBERCSM             53      /* control type: numeric, callback function: (none) */
#define  P_JTAGCTRL_NBTCPNODES            54      /* control type: numeric, callback function: (none) */
#define  P_JTAGCTRL_RUNNUMBER             55      /* control type: numeric, callback function: (none) */

#define  P_JTAGDIAG                       13
#define  P_JTAGDIAG_DIAGDONE              2       /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_MEZZCARDID            3       /* control type: numeric, callback function: (none) */
#define  P_JTAGDIAG_IDCODE                4       /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_CHECKSTATUS           5       /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_VIEWSETUPALL          6       /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_CHECKSETUPALL         7       /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_LOOPTESTSETUPALL      8       /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_VIEWINDSETUP          9       /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_LOOPTESTALLASIND      10      /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_LOOPTESTINDSETUP      11      /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_CHECKINDSETUP         12      /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_GLOBALRESET           13      /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_LOOPBACK              14      /* control type: command, callback function: (none) */
#define  P_JTAGDIAG_APPLYTOALLTEXT        15      /* control type: textMsg, callback function: (none) */

#define  P_MEZZINFO                       14
#define  P_MEZZINFO_MEZZCARDID            2       /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_COARSE_OFFSET     3       /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_BC_OFFSET         4       /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_EC_OFFSET         5       /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_RC_OFFSET         6       /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_NFAILEDMEZZCARD       7       /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_NUMBERMISMATCHBIT     8       /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_ASD3MAINTHRESHOLD     9       /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_ASD3DEADTIME          10      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2DEADTIME          11      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1DEADTIME          12      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2MAINTHRESHOLD     13      /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_ASD3HYSTERESIS        14      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2HYSTERESIS        15      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1HYSTERESIS        16      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1MAINTHRESHOLD     17      /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_ASD3WINTGATEWIDTH     18      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD3CALIBCAPACITOR    19      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CALIBCAPACITOR    20      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_MEZZCARDTEXT          21      /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_SETUPFAILUREFLAGS     22      /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_SETUPFLAGS            23      /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_ASD1CALIBCAPACITOR    24      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD3WDISCCURRENT      25      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2WDISCCURRENT      26      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1WDISCCURRENT      27      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2WINTGATEWIDTH     28      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1WINTGATEWIDTH     29      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_AMT_MASK_WINDOW       30      /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_SEARCH_WINDOW     31      /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_MATCH_WINDOW      32      /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_DIR_CNTL          33      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_ASD3WILTHRESHOLD      34      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2WILTHRESHOLD      35      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1WILTHRESHOLD      36      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_AMT_TEST_MODE         37      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_L1_FULL_REJ       38      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_RO_FULL_REJ       39      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_MST_RST_ON_EVT_RS 40      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ENC_MASTER_RST    41      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_BUFF_OVRFLW_DET   42      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_MRK_OVRFLOW   43      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_RST_CH_ON_SEP     44      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_SEP_BNCH_RST      45      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CHIPMODE          46      /* control type: binary, callback function: (none) */
#define  P_MEZZINFO_ASD2CHIPMODE          47      /* control type: binary, callback function: (none) */
#define  P_MEZZINFO_ASD1CHIPMODE          48      /* control type: binary, callback function: (none) */
#define  P_MEZZINFO_AMT_SEP_EVT_RST       49      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_SEP_RO            50      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_AUTO_REJ          51      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_EN_PAIR           52      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_EN_TRAILING       53      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_EN_LEADING        54      /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CHANNEL7          55      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CHANNEL6          56      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CHANNEL5          57      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CHANNEL4          58      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CHANNEL3          59      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CHANNEL2          60      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CHANNEL1          61      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CHANNEL0          62      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD2CHANNEL7          63      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD2CHANNEL6          64      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD3CH7MODE           65      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD3CH6MODE           66      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD3CH5MODE           67      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD3CH4MODE           68      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD3CH3MODE           69      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD3CH2MODE           70      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD3CH1MODE           71      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD3CH0MODE           72      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CH7MODE           73      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CH6MODE           74      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CH5MODE           75      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CH4MODE           76      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CH3MODE           77      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CH2MODE           78      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CH1MODE           79      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CH0MODE           80      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1CH7MODE           81      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1CH6MODE           82      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1CH5MODE           83      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1CH4MODE           84      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1CH3MODE           85      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1CH2MODE           86      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1CH1MODE           87      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD1CH0MODE           88      /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_ASD2CHANNEL5          89      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD2CHANNEL4          90      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD2CHANNEL3          91      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD2CHANNEL2          92      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD2CHANNEL1          93      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD2CHANNEL0          94      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD1CHANNEL7          95      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD1CHANNEL6          96      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD1CHANNEL5          97      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD1CHANNEL4          98      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD1CHANNEL3          99      /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD1CHANNEL2          100     /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD1CHANNEL1          101     /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_ASD1CHANNEL0          102     /* control type: textButton, callback function: (none) */
#define  P_MEZZINFO_AMT_EN_REL            103     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_CNT_ROLLOVER      104     /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_TEST_INVERT       105     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_TOKEN_DELAY       106     /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_SERIAL_DELAY      107     /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_TRIG_FULL_REJ     108     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_MRK_REJ       109     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_SET_CTRS_BNCH_RST 110     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_HEADER            111     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_TRAILER           112     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_INCLKBOOST        113     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ENABLE_REJECTED   114     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_GLOBAL_RESET      115     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_TRIG_MATCH        116     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_EN_MASK_FLAGS     117     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_EN_SERIAL         118     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_JTAG_INSTR    119     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_SETUP_PAR     120     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_RO_STATE      121     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_RO_FIFO_PAR   122     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_TRIG_MATCH    123     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_TRIG_FIFO_PAR 124     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_L1_BUFF_OCC_RO    125     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_L1_BUFF_PAR   126     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_CHANNEL_SEL   127     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_COARSE        128     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERR_MARK          129     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_GLOBAL_TYPE       130     /* control type: numeric, callback function: (none) */
#define  P_MEZZINFO_AMT_CHANNELS_0        131     /* control type: listBox, callback function: (none) */
#define  P_MEZZINFO_AMT_CHANNELS_2        132     /* control type: listBox, callback function: (none) */
#define  P_MEZZINFO_AMT_CHANNELS_1        133     /* control type: listBox, callback function: (none) */
#define  P_MEZZINFO_AMT_PULSE_WIDTH       134     /* control type: listBox, callback function: (none) */
#define  P_MEZZINFO_AMT_STROBE_SEL        135     /* control type: listBox, callback function: (none) */
#define  P_MEZZINFO_CLOSE                 136     /* control type: command, callback function: (none) */
#define  P_MEZZINFO_AMT_DIS_RING          137     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_ERST_BCECRST      138     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_DIS_ENCODE        139     /* control type: radioButton, callback function: (none) */
#define  P_MEZZINFO_AMT_RD_SPEED          140     /* control type: listBox, callback function: (none) */
#define  P_MEZZINFO_AMT_CLKOUT_MODE       141     /* control type: listBox, callback function: (none) */
#define  P_MEZZINFO_AMT_PLL_MULT          142     /* control type: ring, callback function: (none) */
#define  P_MEZZINFO_CHIPMODETXTBOX        143     /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_CAPSELECTTXTBOX       144     /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_DEADTIMETXTBOX        145     /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_WDISCCURRENTTXTBOX    146     /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_WINTGATETXTBOX        147     /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_WTHRESHOLDTXTBOX      148     /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_HYSTERESISTXTBOX      149     /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_MTHRESHOLDTXTBOX      150     /* control type: textBox, callback function: (none) */
#define  P_MEZZINFO_CHANNELCTRLTEXT       151     /* control type: textMsg, callback function: (none) */
#define  P_MEZZINFO_ASD2TEXT              152     /* control type: textMsg, callback function: (none) */
#define  P_MEZZINFO_ASD1TEXT              153     /* control type: textMsg, callback function: (none) */
#define  P_MEZZINFO_ASD3TEXT              154     /* control type: textMsg, callback function: (none) */
#define  P_MEZZINFO_CHANNELMODETEXT       155     /* control type: textMsg, callback function: (none) */
#define  P_MEZZINFO_ASD3TEXT_2            156     /* control type: textMsg, callback function: (none) */
#define  P_MEZZINFO_ASD2TEXT_2            157     /* control type: textMsg, callback function: (none) */
#define  P_MEZZINFO_ASD1TEXT_2            158     /* control type: textMsg, callback function: (none) */

#define  P_MEZZJTAG                       15
#define  P_MEZZJTAG_CLOSE                 2       /* control type: command, callback function: (none) */
#define  P_MEZZJTAG_REDO                  3       /* control type: command, callback function: (none) */
#define  P_MEZZJTAG_CHECKINGMEZZ          4       /* control type: numeric, callback function: (none) */
#define  P_MEZZJTAG_YELLOWMESSAGE         5       /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_GREENMESSAGE          6       /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_REDMESSAGE            7       /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ00                8       /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ01                9       /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ02                10      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ03                11      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ04                12      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ05                13      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ06                14      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ07                15      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ08                16      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ09                17      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ10                18      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ11                19      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ12                20      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ13                21      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ14                22      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ15                23      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ16                24      /* control type: textMsg, callback function: (none) */
#define  P_MEZZJTAG_MEZZ17                25      /* control type: textMsg, callback function: (none) */

#define  P_MULTCSM                        16
#define  P_MULTCSM_OPERATOR               2       /* control type: string, callback function: (none) */
#define  P_MULTCSM_SECTORSN               3       /* control type: string, callback function: (none) */
#define  P_MULTCSM_CSM00                  4       /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN00                   5       /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE00              6       /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE00             7       /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP00                   8       /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM01                  9       /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN01                   10      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE01              11      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE01             12      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP01                   13      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM02                  14      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN02                   15      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE02              16      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE02             17      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP02                   18      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM03                  19      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN03                   20      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE03              21      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE03             22      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP03                   23      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM04                  24      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN04                   25      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE04              26      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE04             27      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP04                   28      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM05                  29      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN05                   30      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE05              31      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE05             32      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP05                   33      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM06                  34      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN06                   35      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE06              36      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE06             37      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP06                   38      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM07                  39      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN07                   40      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE07              41      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE07             42      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP07                   43      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM08                  44      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN08                   45      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE08              46      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE08             47      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP08                   48      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM09                  49      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN09                   50      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE09              51      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE09             52      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP09                   53      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM10                  54      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN10                   55      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE10              56      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE10             57      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP10                   58      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CSM11                  59      /* control type: textButton, callback function: (none) */
#define  P_MULTCSM_SN11                   60      /* control type: string, callback function: (none) */
#define  P_MULTCSM_CANNODE11              61      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_ELMBNODE11             62      /* control type: numeric, callback function: (none) */
#define  P_MULTCSM_UP11                   63      /* control type: command, callback function: (none) */
#define  P_MULTCSM_LOADSETUP              64      /* control type: command, callback function: (none) */
#define  P_MULTCSM_NEXT                   65      /* control type: command, callback function: (none) */
#define  P_MULTCSM_PREVIOUS               66      /* control type: command, callback function: (none) */
#define  P_MULTCSM_DONE                   67      /* control type: command, callback function: (none) */
#define  P_MULTCSM_CLEAR                  68      /* control type: command, callback function: (none) */
#define  P_MULTCSM_ALLCSMOFF              69      /* control type: command, callback function: (none) */
#define  P_MULTCSM_ELMBNODE               70      /* control type: textMsg, callback function: (none) */
#define  P_MULTCSM_CANNODE                71      /* control type: textMsg, callback function: (none) */
#define  P_MULTCSM_SERIALNUMBER           72      /* control type: textMsg, callback function: (none) */
#define  P_MULTCSM_ALLCSMON               73      /* control type: command, callback function: (none) */

#define  P_MZONOFF                        17
#define  P_MZONOFF_CLOSE                  2       /* control type: command, callback function: (none) */
#define  P_MZONOFF_REDO                   3       /* control type: command, callback function: (none) */
#define  P_MZONOFF_CHECKINGMEZZ           4       /* control type: numeric, callback function: (none) */
#define  P_MZONOFF_YELLOWMESSAGE          5       /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_GREENMESSAGE           6       /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_REDMESSAGE             7       /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ00                 8       /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ01                 9       /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ02                 10      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ03                 11      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ04                 12      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ05                 13      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ06                 14      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ07                 15      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ08                 16      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ09                 17      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ10                 18      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ11                 19      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ12                 20      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ13                 21      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ14                 22      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ15                 23      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ16                 24      /* control type: textMsg, callback function: (none) */
#define  P_MZONOFF_MEZZ17                 25      /* control type: textMsg, callback function: (none) */

#define  P_REPORT                         18
#define  P_REPORT_CLOSE                   2       /* control type: command, callback function: (none) */
#define  P_REPORT_TDCTRLEVIDERROR         3       /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMTRLEVIDERROR         4       /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMWCERROR              5       /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCIDERRINTDCTR         6       /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCCONTSAMEEDGE         7       /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCWCMISMATCH           8       /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCUNEXPECTEDDATA       9       /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMPARITYERROR          10      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCTRAHEADITSDATA       11      /* control type: numeric, callback function: (none) */
#define  P_REPORT_WARNINGEVENT            12      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCEDGEERROR            13      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCHDAFTERITSDATA       14      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCTIMEWRONGORDER       15      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EXTRATDCTRAILER         16      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCEDGEERRW20           17      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EXTRACSMTRAILER         18      /* control type: numeric, callback function: (none) */
#define  P_REPORT_ANALYSEDEVENT           19      /* control type: textBox, callback function: (none) */
#define  P_REPORT_TDCIDERRINTDCHD         20      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCMASKEMPTY            21      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCHDREVIDERROR         22      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EVENTONLYTDCHT          23      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EVENTNOTDCHITS          24      /* control type: numeric, callback function: (none) */
#define  P_REPORT_UNEXPWRONGTDCWC         25      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EXPWRONGTDCWC           26      /* control type: numeric, callback function: (none) */
#define  P_REPORT_PRESCALEERROR           27      /* control type: numeric, callback function: (none) */
#define  P_REPORT_PAIRERROR               28      /* control type: numeric, callback function: (none) */
#define  P_REPORT_WRONGCSMOVERFLOW        29      /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMFIFOOVERFLOW2        30      /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMFIFOOVERFLOW1        31      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCDEBUGWORD            32      /* control type: numeric, callback function: (none) */
#define  P_REPORT_DISCARDEDTDCWORDS       33      /* control type: numeric, callback function: (none) */
#define  P_REPORT_NUMBERHUGEEVENT         34      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCMASKWORD             35      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCERROREMPTY           36      /* control type: numeric, callback function: (none) */
#define  P_REPORT_NREQUESTEDPAUSE         37      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCHARDERROR            38      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCSOFTERROR            39      /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMBCIDERROR            40      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EXTRATDCHEADER          41      /* control type: numeric, callback function: (none) */
#define  P_REPORT_MISSTDCTRAILER          42      /* control type: numeric, callback function: (none) */
#define  P_REPORT_TDCBCIDERROR            43      /* control type: numeric, callback function: (none) */
#define  P_REPORT_MISSTDCHEADER           44      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EXTRACSMHEADER          45      /* control type: numeric, callback function: (none) */
#define  P_REPORT_MISSCSMTRAILER          46      /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMABTEVIDERROR         47      /* control type: numeric, callback function: (none) */
#define  P_REPORT_MISSCSMHEADER           48      /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMHDREVIDERROR         49      /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMABORT                50      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EXTRAWORD               51      /* control type: numeric, callback function: (none) */
#define  P_REPORT_CSMERROR                52      /* control type: numeric, callback function: (none) */
#define  P_REPORT_BCIDMISMATCH            53      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EMPTYEVENT              54      /* control type: numeric, callback function: (none) */
#define  P_REPORT_EVIDMISMATCH            55      /* control type: numeric, callback function: (none) */
#define  P_REPORT_WCERROR                 56      /* control type: numeric, callback function: (none) */
#define  P_REPORT_WRONGEVENT              57      /* control type: numeric, callback function: (none) */
#define  P_REPORT_RUNNUMBER               58      /* control type: numeric, callback function: (none) */

#define  P_RUNCOND                        19
#define  P_RUNCOND_DONE                   2       /* control type: command, callback function: (none) */
#define  P_RUNCOND_SAVERAWDATA            3       /* control type: textButton, callback function: (none) */
#define  P_RUNCOND_TDCTRAILER             4       /* control type: textButton, callback function: (none) */
#define  P_RUNCOND_TDCHEADER              5       /* control type: textButton, callback function: (none) */
#define  P_RUNCOND_TRIGGERDEADTIME        6       /* control type: numeric, callback function: (none) */
#define  P_RUNCOND_NTHRESHOLD             7       /* control type: numeric, callback function: (none) */
#define  P_RUNCOND_CSMTRAILER             8       /* control type: textButton, callback function: (none) */
#define  P_RUNCOND_SAVEFLAGGEDEVT         9       /* control type: textButton, callback function: (none) */
#define  P_RUNCOND_MAPPINGMDTCHAMBER      10      /* control type: textButton, callback function: (none) */
#define  P_RUNCOND_CHECKSYSTEMNOISE       11      /* control type: textButton, callback function: (none) */
#define  P_RUNCOND_CSMHEADER              12      /* control type: textButton, callback function: (none) */
#define  P_RUNCOND_ALLON                  13      /* control type: command, callback function: (none) */
#define  P_RUNCOND_ALLOFF                 14      /* control type: command, callback function: (none) */
#define  P_RUNCOND_CHAMBERHV              15      /* control type: ring, callback function: (none) */
#define  P_RUNCOND_DECORATION0            16      /* control type: deco, callback function: (none) */
#define  P_RUNCOND_ANALYSIS               17      /* control type: ring, callback function: (none) */

#define  P_SELACT                         20
#define  P_SELACT_INITDAQ                 2       /* control type: command, callback function: (none) */
#define  P_SELACT_DONOTHING               3       /* control type: command, callback function: (none) */
#define  P_SELACT_DOWNLOAD                4       /* control type: command, callback function: (none) */
#define  P_SELACT_INFORTEXT               5       /* control type: textMsg, callback function: (none) */

#define  P_SELVFP                         21
#define  P_SELVFP_LOADPROM                2       /* control type: command, callback function: (none) */
#define  P_SELVFP_DONOTHING               3       /* control type: command, callback function: (none) */
#define  P_SELVFP_SELECTRBDFILE           4       /* control type: command, callback function: (none) */
#define  P_SELVFP_DEFAULT                 5       /* control type: command, callback function: (none) */
#define  P_SELVFP_TEXTINFOBOX             6       /* control type: textBox, callback function: (none) */
#define  P_SELVFP_VERIFYFPGATIME          7       /* control type: numeric, callback function: (none) */
#define  P_SELVFP_SECONDS_2               8       /* control type: textMsg, callback function: (none) */
#define  P_SELVFP_VERIFYPROMTIME          9       /* control type: numeric, callback function: (none) */
#define  P_SELVFP_SECONDS                 10      /* control type: textMsg, callback function: (none) */
#define  P_SELVFP_PAUSEDAQTOVERIFYPROM    11      /* control type: textButton, callback function: (none) */
#define  P_SELVFP_PAUSEDAQTOVERIFYFPGA    12      /* control type: textButton, callback function: (none) */
#define  P_SELVFP_CHECKMODE               13      /* control type: ring, callback function: (none) */

#define  P_SNUMBER                        22
#define  P_SNUMBER_OPERATOR               2       /* control type: string, callback function: (none) */
#define  P_SNUMBER_CHAMBERSN              3       /* control type: string, callback function: (none) */
#define  P_SNUMBER_MBSN                   4       /* control type: string, callback function: (none) */
#define  P_SNUMBER_CSMSN                  5       /* control type: string, callback function: (none) */
#define  P_SNUMBER_NIKHEFID               6       /* control type: string, callback function: (none) */
#define  P_SNUMBER_MDTDCSSN               7       /* control type: string, callback function: (none) */
#define  P_SNUMBER_EXPECTEDELMBNODE       8       /* control type: numeric, callback function: (none) */
#define  P_SNUMBER_STARTMEZZCARD          9       /* control type: numeric, callback function: (none) */
#define  P_SNUMBER_MEZZ00SN               10      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ00SNSTS            11      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ01SN               12      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ01SNSTS            13      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ02SN               14      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ02SNSTS            15      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ03SN               16      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ03SNSTS            17      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ04SN               18      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ04SNSTS            19      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ05SN               20      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ05SNSTS            21      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ06SN               22      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ06SNSTS            23      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ07SN               24      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ07SNSTS            25      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ08SN               26      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ08SNSTS            27      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ09SN               28      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ09SNSTS            29      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ10SN               30      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ10SNSTS            31      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ11SN               32      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ11SNSTS            33      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ12SN               34      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ12SNSTS            35      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ13SN               36      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ13SNSTS            37      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ14SN               38      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ14SNSTS            39      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ15SN               40      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ15SNSTS            41      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ16SN               42      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ16SNSTS            43      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_MEZZ17SN               44      /* control type: string, callback function: (none) */
#define  P_SNUMBER_MEZZ17SNSTS            45      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_ALLCHECKED             46      /* control type: command, callback function: (none) */
#define  P_SNUMBER_EXPECTEDMEZZCARD       47      /* control type: numeric, callback function: (none) */
#define  P_SNUMBER_ACTUALELMBNODE         48      /* control type: numeric, callback function: (none) */
#define  P_SNUMBER_DONE                   49      /* control type: command, callback function: (none) */
#define  P_SNUMBER_LOADPREVIOUSSN         50      /* control type: command, callback function: (none) */
#define  P_SNUMBER_USEDCSONCHAMBER        51      /* control type: textButton, callback function: (none) */
#define  P_SNUMBER_DECORATION0            52      /* control type: deco, callback function: (none) */

#define  P_TITLE                          23
#define  P_TITLE_PICTURE                  2       /* control type: pictButton, callback function: (none) */

#define  P_TTCREG                         24
#define  P_TTCREG_CLOSE                   2       /* control type: command, callback function: (none) */
#define  P_TTCREG_RBI2CID0005             3       /* control type: numeric, callback function: (none) */
#define  P_TTCREG_I2CID0005               4       /* control type: numeric, callback function: (none) */
#define  P_TTCREG_RBID0813                5       /* control type: numeric, callback function: (none) */
#define  P_TTCREG_ID0813                  6       /* control type: numeric, callback function: (none) */
#define  P_TTCREG_RBID0007                7       /* control type: numeric, callback function: (none) */
#define  P_TTCREG_STATUS                  8       /* control type: numeric, callback function: (none) */
#define  P_TTCREG_ID0007                  9       /* control type: numeric, callback function: (none) */
#define  P_TTCREG_RBCONTROL               10      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_CONTROL                 11      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_RBCONFIGURATION3        12      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_CONFIGURATION3          13      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_RBCONFIGURATION2        14      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_CONFIGURATION2          15      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_RBCONFIGURATION1        16      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_CONFIGURATION1          17      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_RBCOARSEDELAY           18      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_PLLREADY                19      /* control type: textButton, callback function: (none) */
#define  P_TTCREG_DLLREADY                20      /* control type: textButton, callback function: (none) */
#define  P_TTCREG_FRAMESYNCH              21      /* control type: textButton, callback function: (none) */
#define  P_TTCREG_COARSEDELAY             22      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_RBFINEDELAY2            23      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_FINEDELAY2              24      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_RBFINEDELAY1            25      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_SEUERROR                26      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_DOUBLEERROR             27      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_SINGLEERROR             28      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_BUNCHNUMBER             29      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_EVENTNUMBER             30      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_FINEDELAY1              31      /* control type: numeric, callback function: (none) */
#define  P_TTCREG_UNUSEDBITS              32      /* control type: textButton, callback function: (none) */
#define  P_TTCREG_AUTORESET               33      /* control type: textButton, callback function: (none) */

#define  P_TTCRX                          25
#define  P_TTCRX_CLOCK2COARSEDELAY        2       /* control type: numeric, callback function: (none) */
#define  P_TTCRX_CLOCK2FINEDELAY          3       /* control type: numeric, callback function: (none) */
#define  P_TTCRX_CLOCK1COARSEDELAY        4       /* control type: numeric, callback function: (none) */
#define  P_TTCRX_REDMESSAGE               5       /* control type: textMsg, callback function: (none) */
#define  P_TTCRX_YELLOWMESSAGE_2          6       /* control type: textMsg, callback function: (none) */
#define  P_TTCRX_YELLOWMESSAGE            7       /* control type: textMsg, callback function: (none) */
#define  P_TTCRX_WHITEMESSAGE             8       /* control type: textMsg, callback function: (none) */
#define  P_TTCRX_GREENMESSAGE             9       /* control type: textMsg, callback function: (none) */
#define  P_TTCRX_PLLCURRENT               10      /* control type: numeric, callback function: (none) */
#define  P_TTCRX_FREQCHECKPERIOD          11      /* control type: numeric, callback function: (none) */
#define  P_TTCRX_TESTOUTPUTS              12      /* control type: numeric, callback function: (none) */
#define  P_TTCRX_DLLCURRENT               13      /* control type: numeric, callback function: (none) */
#define  P_TTCRX_I2CBASEADDRESS           14      /* control type: numeric, callback function: (none) */
#define  P_TTCRX_TTCRXADDRESS             15      /* control type: numeric, callback function: (none) */
#define  P_TTCRX_CLOCK1FINEDELAY          16      /* control type: numeric, callback function: (none) */
#define  P_TTCRX_DONE                     17      /* control type: command, callback function: (none) */
#define  P_TTCRX_CANCEL                   18      /* control type: command, callback function: (none) */
#define  P_TTCRX_DEFAULT                  19      /* control type: command, callback function: (none) */
#define  P_TTCRX_CLOCK2COARSETEXT         20      /* control type: textBox, callback function: (none) */
#define  P_TTCRX_CLOCK2FINETEXT           21      /* control type: textBox, callback function: (none) */
#define  P_TTCRX_CSMCONTROL               22      /* control type: command, callback function: (none) */
#define  P_TTCRX_GOLCONTROL               23      /* control type: command, callback function: (none) */
#define  P_TTCRX_SAVESETUP                24      /* control type: command, callback function: (none) */
#define  P_TTCRX_CLOCK1COARSETEXT         25      /* control type: textBox, callback function: (none) */
#define  P_TTCRX_CLOCK2TOTALTEXT          26      /* control type: textBox, callback function: (none) */
#define  P_TTCRX_CLOCK1TOTALTEXT          27      /* control type: textBox, callback function: (none) */
#define  P_TTCRX_CYCLETEXT                28      /* control type: textBox, callback function: (none) */
#define  P_TTCRX_CLOCK1FINETEXT           29      /* control type: textBox, callback function: (none) */
#define  P_TTCRX_EVENTCOUNTER             30      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_SERIALBOUTPUT            31      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_CLOCKL1ACCEPTOUTPUT      32      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_PARALLELOUTPUTBUS        33      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_BUNCHCOUNTER             34      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_CLOCKOUTPUT              35      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_MASTERMODEBTEXT          36      /* control type: textMsg, callback function: (none) */
#define  P_TTCRX_COLORCODETEXT            37      /* control type: textMsg, callback function: (none) */
#define  P_TTCRX_ATTENTION                38      /* control type: textMsg, callback function: (none) */
#define  P_TTCRX_MASTERMODEATEXT          39      /* control type: textMsg, callback function: (none) */
#define  P_TTCRX_DECORATION               40      /* control type: deco, callback function: (none) */
#define  P_TTCRX_CLOCK2OUTPUT             41      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_HAMMINGCHECKB            42      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_TESTMODEB                43      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_SPCONVERTERB             44      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_SELECTCLOCK2             45      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_HAMMINGCHECKA            46      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_TESTMODEA                47      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_SPCONVERTERA             48      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_TESTINOUTPUTS            49      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_WATCHDOGCIRCUIT          50      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_DLLTESTRESETLINE         51      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_PLLTESTRESETLINE         52      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_INPUT                    53      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_EXTSIGNALFORPLL          54      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_AUTOFREQINCREASE         55      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_HAMMINGDATAERROR         56      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_TESTINPUTPSHFT2          57      /* control type: textButton, callback function: (none) */
#define  P_TTCRX_TESTINPUTPSHFT1          58      /* control type: textButton, callback function: (none) */

#define  TRIGSEL_P                        26
#define  TRIGSEL_P_DONE                   2       /* control type: command, callback function: (none) */
#define  TRIGSEL_P_CALIBTRIGGER           3       /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG100KHZ         4       /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG50KHZ          5       /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_CALIBTRIGGERRATE       6       /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_MEASURETRIGGERRATE     7       /* control type: command, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG100KHZRATE     8       /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG25KHZ          9       /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG50KHZRATE      10      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG10KHZ          11      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG25KHZRATE      12      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG5KHZ           13      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG10KHZRATE      14      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG1KHZ           15      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG5KHZRATE       16      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG100HZ          17      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG1KHZRATE       18      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG1HZ            19      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG100HZRATE      20      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_VMEONESHOT             21      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_SOFTTRIG1HZRATE        22      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_L1ACHANNEL3            23      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_VMEONESHOTRATE         24      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_L1ACHANNEL2            25      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_L1ACHANNEL3RATE        26      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_L1ACHANNEL1            27      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_L1ACHANNEL2RATE        28      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_L1ACHANNEL0            29      /* control type: textButton, callback function: (none) */
#define  TRIGSEL_P_L1ACHANNEL1RATE        30      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_RATETEXT               31      /* control type: textMsg, callback function: (none) */
#define  TRIGSEL_P_L1ACHANNEL0RATE        32      /* control type: numeric, callback function: (none) */
#define  TRIGSEL_P_MESSAGE                33      /* control type: textBox, callback function: (none) */

#define  TTCCTRL_P                        27
#define  TTCCTRL_P_DONE                   2       /* control type: command, callback function: (none) */
#define  TTCCTRL_P_RATESELECTION          3       /* control type: ring, callback function: (none) */
#define  TTCCTRL_P_TRIGGERSELECTION       4       /* control type: ring, callback function: (none) */
#define  TTCCTRL_P_BCDELAY                5       /* control type: textBox, callback function: (none) */
#define  TTCCTRL_P_CURRENTEVTNB           6       /* control type: numeric, callback function: (none) */
#define  TTCCTRL_P_EVTNBOFFSET            7       /* control type: numeric, callback function: (none) */
#define  TTCCTRL_P_VMESTATUS              8       /* control type: textBox, callback function: (none) */
#define  TTCCTRL_P_INFORTEXT              9       /* control type: textMsg, callback function: (none) */
#define  TTCCTRL_P_L1AFIFOSTATUS          10      /* control type: textBox, callback function: (none) */
#define  TTCCTRL_P_BGOFIFO3STATUS         11      /* control type: textBox, callback function: (none) */
#define  TTCCTRL_P_BGOFIFO2STATUS         12      /* control type: textBox, callback function: (none) */
#define  TTCCTRL_P_BGOFIFO1STATUS         13      /* control type: textBox, callback function: (none) */
#define  TTCCTRL_P_BASE                   14      /* control type: numeric, callback function: (none) */
#define  TTCCTRL_P_BGOFIFO0STATUS         15      /* control type: textBox, callback function: (none) */
#define  TTCCTRL_P_RUNTYPE                16      /* control type: ring, callback function: (none) */
#define  TTCCTRL_P_DECORATION1            17      /* control type: deco, callback function: (none) */
#define  TTCCTRL_P_DECORATION4            18      /* control type: deco, callback function: (none) */
#define  TTCCTRL_P_DECORATION3            19      /* control type: deco, callback function: (none) */
#define  TTCCTRL_P_DECORATION0            20      /* control type: deco, callback function: (none) */
#define  TTCCTRL_P_RESETBGOFIFO4          21      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_RESETBGOFIFO3          22      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_RESETBGOFIFO2          23      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_RESETBGOFIFO1          24      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_SELECTCOUNT            25      /* control type: textButton, callback function: (none) */
#define  TTCCTRL_P_RESETEVTORBITCOUNT     26      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_READEVENTNB            27      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_BOARDRESET             28      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_RESETBGOFIFO0          29      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_SOFTWARETRIGGER        30      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_RESETL1AFIFO           31      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_BGOFIFO3               32      /* control type: textMsg, callback function: (none) */
#define  TTCCTRL_P_BGOFIFO2               33      /* control type: textMsg, callback function: (none) */
#define  TTCCTRL_P_BGOFIFO1               34      /* control type: textMsg, callback function: (none) */
#define  TTCCTRL_P_TTCRXACCESS            35      /* control type: textButton, callback function: (none) */
#define  TTCCTRL_P_BGOFIFO0               36      /* control type: textMsg, callback function: (none) */
#define  TTCCTRL_P_CSR2TITLE              37      /* control type: textMsg, callback function: (none) */
#define  TTCCTRL_P_CSR1TITLE              38      /* control type: textMsg, callback function: (none) */
#define  TTCCTRL_P_VMETRANSFER            39      /* control type: textButton, callback function: (none) */
#define  TTCCTRL_P_ORBITSIGNAL            40      /* control type: textButton, callback function: (none) */
#define  TTCCTRL_P_BGOCONTROL             41      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_BOARDID                42      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_ECBCRESET              43      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_BCRESET                44      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_ECRESET                45      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_RETRANSMITBGOFIFO3     46      /* control type: textButton, callback function: (none) */
#define  TTCCTRL_P_RETRANSMITBGOFIFO2     47      /* control type: textButton, callback function: (none) */
#define  TTCCTRL_P_RETRANSMITBGOFIFO1     48      /* control type: textButton, callback function: (none) */
#define  TTCCTRL_P_RETRANSMITBGOFIFO0     49      /* control type: textButton, callback function: (none) */
#define  TTCCTRL_P_RETRANSMITDISABLEALL   50      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_RETRANSMITENABLEALL    51      /* control type: command, callback function: (none) */
#define  TTCCTRL_P_FORMAT                 52      /* control type: textButton, callback function: (none) */
#define  TTCCTRL_P_SUBADDRESS             53      /* control type: numeric, callback function: (none) */
#define  TTCCTRL_P_TTCRXADDRESS           54      /* control type: numeric, callback function: (none) */
#define  TTCCTRL_P_SUBADDRTEXT            55      /* control type: textMsg, callback function: (none) */

#define  TTCMSG_P                         28
#define  TTCMSG_P_BASE                    2       /* control type: numeric, callback function: (none) */
#define  TTCMSG_P_MESSAGE                 3       /* control type: textBox, callback function: (none) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENU00                           1
#define  MENU00_FILE                      2
#define  MENU00_FILE_LOADPROFILE          3
#define  MENU00_FILE_SAVEPROFILE          4
#define  MENU00_FILE_SEPARATOR00          5
#define  MENU00_FILE_SAVEDEFAULTPROFILE   6
#define  MENU00_FILE_CLEARDEFAULTPROFILE  7
#define  MENU00_FILE_SEPARATOR01          8
#define  MENU00_FILE_GTEXTFILES           9
#define  MENU00_FILE_GACTSEQFILES         10
#define  MENU00_FILE_SEPARATOR02          11
#define  MENU00_FILE_QUIT                 12
#define  MENU00_EXE                       13
#define  MENU00_EXE_CSMIO                 14
#define  MENU00_EXE_PROGRAMFPGAVIAIO      15
#define  MENU00_EXE_RESETCSM              16
#define  MENU00_EXE_SELECTHARDTDO         17
#define  MENU00_EXE_SELECTSOFTTDO         18
#define  MENU00_EXE_SEPARATOR30           19
#define  MENU00_EXE_INITDAQ               20
#define  MENU00_EXE_SEARCHJTAGDRIVER      21
#define  MENU00_EXE_SEPARATOR31           22
#define  MENU00_EXE_RUNCONDITION          23
#define  MENU00_EXE_AUTORUNSETUP          24
#define  MENU00_EXE_SEPARATOR32           25
#define  MENU00_EXE_CHECKMEZZONOFF        26
#define  MENU00_EXE_CHECKALLHARDWARE      27
#define  MENU00_EXE_RDIAG                 28
#define  MENU00_EXE_RDIAG_SUBMENU         29
#define  MENU00_EXE_RDIAG_GETRAWDATA      30
#define  MENU00_EXE_RDIAG_GETONEEVENT     31
#define  MENU00_EXE_RDIAG_GETONESEQEVENT  32
#define  MENU00_EXE_SEPARATOR33           33
#define  MENU00_EXE_PROGRAMFPGA           34
#define  MENU00_EXE_PROGRAMPROM           35
#define  MENU00_EXE_PROGRAMFPGAFROMPROM   36
#define  MENU00_EXE_SEPARATOR34           37
#define  MENU00_EXE_VERIFYFPGA            38
#define  MENU00_EXE_VERIFYPROM            39
#define  MENU00_EXE_SEPARATOR35           40
#define  MENU00_EXE_SHUTDOWNFPGA          41
#define  MENU00_EXE_FPGACONFSTATUS        42
#define  MENU00_EXE_SEPARATOR36           43
#define  MENU00_EXE_SAVEERRREPORT         44
#define  MENU00_EXE_ERRORREPORT           45
#define  MENU00_JTAG                      46
#define  MENU00_JTAG_SETUPJTAGCHAIN       47
#define  MENU00_JTAG_RESETTAP             48
#define  MENU00_JTAG_SEPARATOR10          49
#define  MENU00_JTAG_SETUPCSM             50
#define  MENU00_JTAG_SETUPTTCRX           51
#define  MENU00_JTAG_SEPARATOR11          52
#define  MENU00_JTAG_SETUPGOL             53
#define  MENU00_JTAG_SEPARATOR12          54
#define  MENU00_JTAG_MEZZSETUPALL         55
#define  MENU00_JTAG_MEZZSETUPIND         56
#define  MENU00_JTAG_MEZZSETUPIND_SUBMENU 57
#define  MENU00_JTAG_MEZZSETUPIND_M00     58
#define  MENU00_JTAG_MEZZSETUPIND_M01     59
#define  MENU00_JTAG_MEZZSETUPIND_M02     60
#define  MENU00_JTAG_MEZZSETUPIND_M03     61
#define  MENU00_JTAG_MEZZSETUPIND_M04     62
#define  MENU00_JTAG_MEZZSETUPIND_M05     63
#define  MENU00_JTAG_MEZZSETUPIND_M06     64
#define  MENU00_JTAG_MEZZSETUPIND_M07     65
#define  MENU00_JTAG_MEZZSETUPIND_M08     66
#define  MENU00_JTAG_MEZZSETUPIND_M09     67
#define  MENU00_JTAG_MEZZSETUPIND_M10     68
#define  MENU00_JTAG_MEZZSETUPIND_M11     69
#define  MENU00_JTAG_MEZZSETUPIND_M12     70
#define  MENU00_JTAG_MEZZSETUPIND_M13     71
#define  MENU00_JTAG_MEZZSETUPIND_M14     72
#define  MENU00_JTAG_MEZZSETUPIND_M15     73
#define  MENU00_JTAG_MEZZSETUPIND_M16     74
#define  MENU00_JTAG_MEZZSETUPIND_M17     75
#define  MENU00_JTAG_NOMINALTHRESHOLD     76
#define  MENU00_JTAG_SEPARATOR13          77
#define  MENU00_JTAG_DIAG                 78
#define  MENU00_JTAG_DIAG_SUBMENU         79
#define  MENU00_JTAG_DIAG_CSMID           80
#define  MENU00_JTAG_DIAG_CSMVERSIONDATE  81
#define  MENU00_JTAG_DIAG_SEPERATORD0     82
#define  MENU00_JTAG_DIAG_AMTPARITYERROR  83
#define  MENU00_JTAG_DIAG_AMTPHASEERROR   84
#define  MENU00_JTAG_DIAG_AMTFULLPHASE    85
#define  MENU00_JTAG_DIAG_SEPERATORD1     86
#define  MENU00_JTAG_DIAG_CSMSTATUS       87
#define  MENU00_JTAG_DIAG_TTCRXSTATUS     88
#define  MENU00_JTAG_DIAG_CSMROBITS       89
#define  MENU00_JTAG_DIAG_SEPERATORD2     90
#define  MENU00_JTAG_DIAG_CSMFULLRW       91
#define  MENU00_JTAG_DIAG_CSMFULLRO       92
#define  MENU00_JTAG_DIAG_SEPERATORD3     93
#define  MENU00_JTAG_DIAG_CSMCONFIGRW     94
#define  MENU00_JTAG_DIAG_CSMCONFIGRO     95
#define  MENU00_JTAG_DIAG_SEPERATORD4     96
#define  MENU00_JTAG_DIAG_CSMPARARW       97
#define  MENU00_JTAG_DIAG_CSMPARARO       98
#define  MENU00_JTAG_SEARCHMEZZCARD       99
#define  MENU00_JTAG_SEPARATOR14          100
#define  MENU00_JTAG_GETDEVICELIST        101
#define  MENU00_JTAG_GETALLDEVICEID       102
#define  MENU00_JTAG_SEPARATOR15          103
#define  MENU00_JTAG_GETPROMID            104
#define  MENU00_JTAG_GETCSMCHIPID         105
#define  MENU00_JTAG_GETTTCRXID           106
#define  MENU00_JTAG_GETGOLID             107
#define  MENU00_JTAG_GETCSMID             108
#define  MENU00_JTAG_GETALLAMTID          109
#define  MENU00_JTAG_SEPARATOR16          110
#define  MENU00_JTAG_CLEARSETUPSTATUS     111
#define  MENU00_JTAG_RESETCSM             112
#define  MENU00_TTCVI                     113
#define  MENU00_TTCVI_STATUSANDCONTROL    114
#define  MENU00_TTCVI_BGOCONTROL          115
#define  MENU00_TTCVI_SEPARATOR20         116
#define  MENU00_TTCVI_RESETBOARD          117
#define  MENU00_TTCVI_RESETL1AFIFO        118
#define  MENU00_TTCVI_SEPARATOR21         119
#define  MENU00_TTCVI_ECRESET             120
#define  MENU00_TTCVI_BCRESET             121
#define  MENU00_TTCVI_ECBCRESET           122
#define  MENU00_TTCVI_SEPARATOR22         123
#define  MENU00_TTCVI_SOFTWARETRIGGER     124
#define  MENU00_TTCVI_RANDOMTRIG          125
#define  MENU00_TTCVI_RANDOMTRIG_SUBMENU  126
#define  MENU00_TTCVI_RANDOMTRIG_1HZ      127
#define  MENU00_TTCVI_RANDOMTRIG_100HZ    128
#define  MENU00_TTCVI_RANDOMTRIG_1KHZ     129
#define  MENU00_TTCVI_RANDOMTRIG_5KHZ     130
#define  MENU00_TTCVI_RANDOMTRIG_10KHZ    131
#define  MENU00_TTCVI_RANDOMTRIG_25KHZ    132
#define  MENU00_TTCVI_RANDOMTRIG_50KHZ    133
#define  MENU00_TTCVI_RANDOMTRIG_100KHZ   134
#define  MENU00_TTCVI_SEPARATOR23         135
#define  MENU00_TTCVI_EXTERNALTRIG        136
#define  MENU00_TTCVI_EXTERNALTRIG_SUBMENU 137
#define  MENU00_TTCVI_EXTERNALTRIG_CH0    138
#define  MENU00_TTCVI_EXTERNALTRIG_CH1    139
#define  MENU00_TTCVI_EXTERNALTRIG_CH2    140
#define  MENU00_TTCVI_EXTERNALTRIG_CH3    141
#define  MENU00_TTCVI_DISABLETRIGGER      142
#define  MENU00_TTCVI_SEPARATOR24         143
#define  MENU00_TTCVI_TRIGSELECT          144
#define  MENU00_HELP                      145
#define  MENU00_HELP_BUTTONS              146
#define  MENU00_HELP_ABOUT                147


     /* (no callbacks specified in the resource file) */ 


#ifdef __cplusplus
    }
#endif
