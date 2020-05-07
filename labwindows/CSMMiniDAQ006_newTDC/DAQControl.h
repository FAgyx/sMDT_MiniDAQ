#ifndef DAQControl_H
#define DAQControl_H
//
// Filename: DAQControl.h
// Author: T.S. Dai
//
// Description:
// This file contains prototypes for all of the DAQ Control functions.
//

#define MAXIMUMRERUN                      2
#define TYPEAMT1                          1
#define TYPEAMT2                          2
#define TYPEAMT3                          3
#define TYPEHPTDC						  4


#define CMDPROBE                          1
#define CMDDETAILSTATUS                   2
#define CMDGETONEEVENTINIT               10
#define CMDINITDAQ                       11
#define CMDSTARTRUN                      12
#define CMDSTOPRUN                       13
#define CMDPAUSERUN                      14
#define CMDRESUMERUN                     15
#define CMDREQRAWDATA                    20
#define CMDREREQRAWDATA                  21
#define CMDREQONEEVENT                   22
#define CMDREREQONEEVENT                 23
#define CMDREQONESEQEVENT                24
#define CMDREREQONESEQEVENT              25
#define CMDREQEDGES                      30
#define CMDREQPAIR                       31
#define CMDREQHITS                       32
#define CMDREQAVERAGETDCTIME             33
#define CMDREQFRACTIONUSED               34
#define CMDREQAVERAGEWIDTH               35
#define CMDREQDAQINFOR                   50
#define CMDREREQDAQINFOR                 51
#define CMDREQALLDAQINFOR                52
#define CMDQUIT                         255

#define TCPLENGTH                         0
#define TCPCOMMAND                        1
#define TCPCSMNUMBER                      2
#define TCPCSMVERSION                     3
#define TCPSYNCHWORDCONTROL               4
#define TCPMEZZENABLES                    5
#define TCPEDGEPAIRENABLES0               6
#define TCPEDGEPAIRENABLES1               7
#define TCPGOLACARDPAGESIZE               8
#define TCPAMTTYPE                        9
#define TCPSAVERAWDATA                   10
#define TCPCHAMBERTYPE                   11
#define TCPCHAMBERHV                     12
#define TCPSUPPRESSCSMHEADER             13
#define TCPSUPPRESSCSMTRAILER            14
#define TCPSUPPRESSAMTHEADER             15
#define TCPSUPPRESSAMTTRAILER            16
#define TCPRUNNUMBER                     17
#define TCPNUMBEROFEVENT                 18
#define TCPNOMINALTHRESHOLD              19
#define TCPINTEGRATIONGATE               20
#define TCPRUNDOWNCURRENT                21
#define TCPWIDTHSELECTION                22
#define TCPPAIRDEBUG                     23
#define TCPCHECKSYSTEMNOISE              24
#define TCPMAPPINGMDTCHAMBER             25
#define TCPTRIGGERSELECTION              26
#define TCPTRIGGERRATESELECTION          27
#define TCPEXPECTEDTRIGGERRATE           28
#define TCPMAXALLOWEDMEZZS               29
#define TCPANALYSISCONTROL               30
#define TCPSTARTMEZZCARD                 31
#define TCPSPARE07                       32
#define TCPSPARE06                       33
#define TCPSPARE05                       34
#define TCPSPARE04                       35
#define TCPSPARE03                       36
#define TCPSPARE02                       37
#define TCPSPARE01                       38
#define TCPFILENAME                      39

#define CLIENTRUNSTATE                    0
#define CLIENTANAINSTALLED                1
#define CLIENTCERNLIBINSTALLED            2
#define CLIENTPROCESSINGDATA              3

#define LEADINGEDGEENABLE                 0
#define TRAILINGEDGEENABLE                1
#define PAIRENABLE                        2

// Define AMT and CSM data type
#define TDC_HEADER                      0xa  // ID code of TDC header
#define TDC_TRAILER                     0xc  // ID code of TDC trailer
#define TDC_MASK                        0x2  // ID code of TDC masked hit
#define TDC_EDGE                        0x3  // ID code of TDC edge timing  
#define TDC_PAIR                        0x4  // ID code of TDC paired timing
#define TDC_ERROR                       0x6  // ID code of TDC error
#define TDC_DEBUG                       0x7  // ID code of TDC debug
#define CSM_WORD                        0x5  // ID code of CSM words
#define CSM_HEAD                        0x9  // sub ID code of CSM header
#define CSM_GOOD                        0xb  // sub ID code of CSM good event trailer
#define	CSM_ABORT                       0xd  // sub ID code of CSM aborted event trailer
#define	CSM_PARITY                      0x1  // sub ID code of CSM parity error code
#define	CSM_ERROR                       0x0  // sub ID code of CSM error code for abort

#define WORDCOUNTBIT0LOCATION             0
#define WORDCOUNTBITS                 0xFFF
#define BCIDBIT0LOCATION                  0
#define BCIDBITS                      0xFFF
#define EVIDBIT0LOCATION                 12
#define EVIDBITS                      0xFFF
#define MAINIDBIT0LOCATION               28
#define MAINIDBITS                      0xF
#define SUBIDBIT0LOCATION                24
#define SUBIDBITS                       0xF
#define TDCNUMBERBIT0LOCATION            24
#define TDCNUMBERBITS                   0xF
#define TDCNUMBERMASK            0x0F000000
#define CHANNELNUMBERBIT0LOCATION        19
#define CHANNELNUMBERBITS              0x1F
#define CHANNELNUMBERMASK        0x00F80000
#define WIRENUMBERBIT0LOCATION           19
#define WIRENUMBERBITS                0x1FF
#define WIRENUMBERMASK           0x0FF80000
#define FINETIMEBIT0LOCATION              0
#define FINETIMEBITS                   0x1F
#define MASKFLAGSBIT0LOCATION             0
#define MASKFLAGSBITS              0xFFFFFF
#define STYPEBIT0LOCATION                18
#define STYPEBITS                       0x1
#define SERRORBIT0LOCATION               17
#define SERRORBITS                      0x1
#define STDCTIMEBIT0LOCATION              0
#define STDCTIMEBITS                0x1FFFF
#define SCOARSETIMEBIT0LOCATION           5
#define SCOARSETIMEBITS               0xFFF
#define PTDCTIMEBIT0LOCATION              0
#define PTDCTIMEBITS                  0x7FF
#define PCOARSETIMEBIT0LOCATION           5
#define PCOARSETIMEBITS                0x3F
#define PWIDTHBIT0LOCATION               11
#define PWIDTHBITS                     0xFF
#define CSMPARITYBIT0LOCATION            27
#define CSMPARITYBITS            0x08000000
#define ODDPARITYBIT0LOCATION            26
#define ODDPARITYBITS            0x04000000
#define CSMFIFOOV2BIT0LOCATION           25
#define CSMFIFOOV2BITS           0x02000000
#define CSMFIFOOV1BIT0LOCATION           24
#define CSMFIFOOV1BITS           0x01000000
#define NTRIGBIT0LOCATION                24
#define NTRIGBITS                       0xF
#define NEVENTBIT0LOCATION               28
#define NEVENTBITS                      0x7
#define WCONLY                          0x1
#define WCONLYBIT0LOCATION               31
#define WCONLYBITS                      0x1

// Define parameter for raw data
#define ID_SYNCHWORD                    0xD
#define ID_IDLEWORD                     0x0
#define SWORDHEADERMASK          0xF0000000
#define SWORDHEADER              0xD0000000
#define CSMIDLEWORD              0x04000000
#define CSMPARITYERRORBIT        0x08000000
#define CSMODDPARITYBIT          0x04000000
#define CSMDATABIT0LOCATION               0
#define CSMDATABITS              0x03FFFFFF

// Constant for Analysis
#define NUMBERANALYSISCOUNTER           800
#define ANALYSEDEVENT                     0
#define ANALYSEDEVENTHIGH                 1
#define WRONGEVENT                        2
#define WARNINGEVENT                      3
#define DISCARDEDEVENT                    4
#define NUMBERHUGEEVENT                   5
#define WORDCOUNTERERROR                  6
#define CSMWORDCOUNTERERR                 7
#define EVIDMISMATCH                      8
#define BCIDMISMATCH                      9
#define EMPTYEVENT                       10
#define EXTRAWORD                        11
#define EXTRAWORDINTDC                   12   // 18 TDCs, 0 for EvtBuilt
#define CSMHEADEREVIDMISMATCH            30
#define CSMTRAILEREVIDMISMATCH           31
#define CSMABORTEVIDMISMATCH             32
#define CSMBCIDMISMATCH                  33
#define CSMERROR                         34
#define CSMERRORINTDC                    35   // 18 TDCs
#define CSMABORT                         53
#define MISSCSMHEADER                    54
#define EXTRACSMHEADER                   55
#define MISSCSMTRAILER                   56
#define EXTRACSMTRAILER                  57
#define CSMPARITYERROR                   58
#define PARITYERRORINTDC                 59   // 18 TDCs
#define TDCBCIDMISMATCH                  77
#define TDCBCIDMISMATCHINTDC             78   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCHEADEREVIDMISMATCH            96
#define TDCHEADEREVIDMISMATCHINTDC       97   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCTRAILEREVIDMISMATCH          115
#define TDCTRAILEREVIDMISMATCHINTDC     116   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCEDGEERROR                    134
#define TDCEDGEERRORINTDC               135   // 18 TDCs
#define TDCEDGEERRW20                   153
#define TDCEDGEERRW20INTDC              154   // 18 TDCs
#define MISSTDCHEADER                   172
#define EXTRATDCHEADER                  173
#define MISSTDCTRAILER                  174
#define EXTRATDCTRAILER                 175
#define TDCSOFTERROR                    176
#define TDCHARDERROR                    177
#define TDCERROR                        178
#define TDCERRORLIST                    179   // 14 different errors in TDC error word
#define TDCERRORINTDC                   193   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCERROREMPTY                   211
#define TDCMASKWORD                     212
#define TDCMASKWORDINTDC                213   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCMASKEMPTY                    231
#define TDCDEBUGWORD                    232
#define TDCDEBUGWORDINTDC               233   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCHEADERWRONGTDCID             251
#define TDCHEADERMISSTDCID              252   // 18 TDCs, 16 TDC IDs in TDC header (4 bits) for EvtBuilt
#define TDCHEADEREXTRATDCID             270   // 18 TDCs, 16 TDC IDs in TDC header (4 bits) for EvtBuilt
#define TDCTRAILERWRONGTDCID            288
#define TDCTRAILERMISSTDCID             289   // 18 TDCs, 16 TDC IDs in TDC trailer (4 bits) for EvtBuilt
#define TDCTRAILEREXTRATDCID            307   // 18 TDCs, 16 TDC IDs in TDC trailer (4 bits) for EvtBuilt
#define TDCHEADERAFTERITSDATA           325
#define TDCHEADERAFTERITSDATALIST       326   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCTRAILERAHEADDATA             344
#define TDCTRAILERAHEADDATALIST         345   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCWCMISMATCH                   363
#define TDCWCMISMATCHLIST               364   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCCONTSAMEEDGE                 382
#define TDCCONTSAMEEDGEINTDC            383   // 18 TDCs
#define TDCCONTTRAILINGEDGE             401
#define TDCCONTTRAILINGEDGEINTDC        402   // 18 TDCs
#define TDCUNEXPECTEDDATA               420
#define TDCUNEXPECTEDDATAINTDC          421   // 18 TDCs
#define TDCTIMEWRONGORDER               439
#define TDCTIMEWRONGORDERINTDC          440   // 18 TDCs
#define EVENTNOTDCHITS                  458
#define EVENTNOTDCHITSLIST              459   // 18 TDCs
#define EVENTONLYTDCHDANDTR             477
#define EVENTONLYTDCHDANDTRLIST         478   // 18 TDCs
#define NBCIDERRDUETRIGFIFOOV           496
#define DISCARDEDTRAILINGONLY           497
#define DISCARDEDTRAILINGONLYINTDC      498   // 18 TDCs
#define DISCARDEDLEADINGONLY            516
#define DISCARDEDLEADINGONLYINTDC       517   // 18 TDCs
#define DISCARDEDTDCWORDS               535
#define DISCARDEDTDCWORDSINTDC          536   // 18 TDCs
#define EXPECTEDTDCWCERROR              554
#define EXPECTEDTDCWCERRORINTDC         555   // 18 TDCs
#define UNEXPECTEDTDCWCERROR            573
#define UNEXPECTEDTDCWCERRORINTDC       574   // 18 TDCs
#define WRONGCSMFIFOOVERFLOW            592
#define WRONGCSMFIFOOVERFLOWINTDC       593   // 18 TDCs
#define CSMFIFOOVERFLOW1                611
#define CSMFIFOOVERFLOW1INTDC           612   // 18 TDCs
#define CSMFIFOOVERFLOW2                630
#define CSMFIFOOVERFLOW2INTDC           631   // 18 TDCs
#define CSMFIFOOVERFLOWS                649
#define CSMFIFOOVERFLOWSINTDC           650   // 18 TDCs
#define CSMEVTPRESCALEERR               668
#define CSMEVTPRESCALEERRINTDC          669   // 18 TDCs
#define CSMPAIRERROR                    687
#define CSMPAIRERRORINTDC               688   // 18 TDCs
#define NREQUESTEDSTOP                  706
#define NREQUESTEDPAUSE                 707
#define NTOTALWORD                      708
#define NTOTALWORDHIGH                  709
#define PROCESSEDEVENT                  710
#define PROCESSEDEVENTHIGH              711
#define NDATAWORD                       712
#define NDATAWORDHIGH                   713
#define NEVENTWORD                      714
#define NEVENTWORDHIGH                  715
#define NGOODCYCLE                      716
#define NGOODCYCLEHIGH                  717
#define NEMPTYCYCLE                     718
#define NEMPTYCYCLEHIGH                 719
#define NBADCYCLE                       720
#define NBADCYCLEATBEGINNING            721
#define CURREVENTSIZE                   722
#define MINEVENTSIZE                    723
#define MAXEVENTSIZE                    724
#define CURREVENTID                     725
#define NBUFFERPOINTERERROR             726
#define TXPARITYERROR                   727
#define TXPARITYERRORINTDC              728   // 18 TDC slots
#define LHCCLOCKUNLOCKED                746
#define XMT1CLOCKUNLOCKED               747
#define XMT2CLOCKUNLOCKED               748
#define CSMPHASEERROR                   749
#define I2COPERATIONFAILED              750
#define UNEXPECTEDTTCRXSETUP            751
#define CSMHASERROR                     752
#define NUMBERTRIGGER                   753
#define NUMBERSAMETRIGGER               754
#define NUMBERTRIGGERLOW                755
#define NUMBERUNMATCHEDTRIGGERLOW       756
#define NUMBERTRIGGERHIGH               757
#define NUMBERUNMATCHEDTRIGGERHIGH      758
#define UNKNOWNSWORD                    759

// Define synch word bits
//
#define STATUSTRIGTIMEINSYNCHWORD         1
#define SWORDBIT27               0x08000000
#define SWORDODDPARITY           0x04000000
#define SWORDBIT25               0x02000000
#define SWORDLHCLOCKBIT          0x01000000
#define SWORDXMTLOCK1BIT         0x00800000
#define SWORDXMTLOCK2BIT         0x00400000
#define SWORDPHASEERRORBIT       0x00200000
#define SWORDI2COPERATIONBIT     0x00100000
#define SWORDTTCRXI2CCHECKERROR  0x00080000
#define SWORDCSMERRORBIT         0x00040000
#define SWORDCSMSTATUSMASK       0x01FC0000
#define SWORDCSMSTATUSBIT0LOC            18
#define SWORDCSMSTATUSBITS             0x7F
#define SWORDCONTROLMASK         0x00030000
#define SWORDCONTROLBIT0LOC              16
#define SWORDCONTROLBITS                0x3
#define SWORDSTATUSONLY          0x00000000
#define SWORDTRIGGERTIMELOW      0x00010000
#define SWORDTRIGGERTIMEHIGH     0x00020000
#define SWORDTRIGGERTIMEMASK     0x0000FFFF
#define SWORDTRIGGERTIMEMASK     0x0000FFFF
#define SWORDTRIGGERTIMEBIT0LOC           0
#define SWORDTRIGGERTIMEBITS         0xFFFF

// Define MDT Chamber Type
#define CNUMBERMASK              0x000000FF
#define CTYPEMASK                0x0000FF00
#define CTYPEUNKNOWN             0x0000FFFF
#define CTYPEINDB                0x0000FE00
#define CTYPEEMA                 0x00000100
#define CTYPEEMC                 0x00000200
#define CTYPEEIA                 0x00000300
#define CTYPEEIC                 0x00000400
#define CTYPEEOA                 0x00000500
#define CTYPEEOC                 0x00000600
#define CTYPEEEA                 0x00000700
#define CTYPEEEC                 0x00000800
//
// Define MDT endcap chambers
//
#define CTYPEEML1A               0x00001000
#define CTYPEEML1C               0x00001100
#define CTYPEEML2A               0x00001200
#define CTYPEEML2C               0x00001300
#define CTYPEEML3A               0x00001400
#define CTYPEEML3C               0x00001500
#define CTYPEEML4A               0x00001600
#define CTYPEEML4C               0x00001700
#define CTYPEEML5A               0x00001800
#define CTYPEEML5C               0x00001900
#define CTYPEEMS1A               0x00002000
#define CTYPEEMS1C               0x00002100
#define CTYPEEMS2A               0x00002200
#define CTYPEEMS2C               0x00002300
#define CTYPEEMS3A               0x00002400
#define CTYPEEMS3C               0x00002500
#define CTYPEEMS4A               0x00002600
#define CTYPEEMS4C               0x00002700
#define CTYPEEMS5A               0x00002800
#define CTYPEEMS5C               0x00002900
#define CTYPEEIL1A               0x00003000
#define CTYPEEIL1C               0x00003100
#define CTYPEEIL2A               0x00003200
#define CTYPEEIL2C               0x00003300
#define CTYPEEIL3A               0x00003400
#define CTYPEEIL3C               0x00003500
#define CTYPEEIL4A               0x00003600
#define CTYPEEIL4C               0x00003700
#define CTYPEEIL5A               0x00003800
#define CTYPEEIL5C               0x00003900
#define CTYPEEIL23A              0x00003a00
#define CTYPEEIL23C              0x00003b00
#define CTYPEEIS1A               0x00004000
#define CTYPEEIS1C               0x00004100
#define CTYPEEIS2A               0x00004200
#define CTYPEEIS2C               0x00004300
#define CTYPEEIS3A               0x00004400
#define CTYPEEIS3C               0x00004500
#define CTYPEEIS4A               0x00004600
#define CTYPEEIS4C               0x00004700
#define CTYPEEIS5A               0x00004800
#define CTYPEEIS5C               0x00004900
#define CTYPEEOL1A               0x00005000
#define CTYPEEOL1C               0x00005100
#define CTYPEEOL2A               0x00005200
#define CTYPEEOL2C               0x00005300
#define CTYPEEOL3A               0x00005400
#define CTYPEEOL3C               0x00005500
#define CTYPEEOL4A               0x00005600
#define CTYPEEOL4C               0x00005700
#define CTYPEEOL5A               0x00005800
#define CTYPEEOL5C               0x00005900
#define CTYPEEOL6A               0x00005a00
#define CTYPEEOL6C               0x00005b00
#define CTYPEEOS1A               0x00006000
#define CTYPEEOS1C               0x00006100
#define CTYPEEOS2A               0x00006200
#define CTYPEEOS2C               0x00006300
#define CTYPEEOS3A               0x00006400
#define CTYPEEOS3C               0x00006500
#define CTYPEEOS4A               0x00006600
#define CTYPEEOS4C               0x00006700
#define CTYPEEOS5A               0x00006800
#define CTYPEEOS5C               0x00006900
#define CTYPEEOS6A               0x00006a00
#define CTYPEEOS6C               0x00006b00
#define CTYPEEEL1A               0x00007000
#define CTYPEEEL1C               0x00007100
#define CTYPEEEL2A               0x00007200
#define CTYPEEEL2C               0x00007300
#define CTYPEEEL3A               0x00007400
#define CTYPEEEL3C               0x00007500
#define CTYPEEEL4A               0x00007600
#define CTYPEEEL4C               0x00007700
#define CTYPEEEL5A               0x00007800
#define CTYPEEEL5C               0x00007900
#define CTYPEEES1A               0x00008000
#define CTYPEEES1C               0x00008100
#define CTYPEEES2A               0x00008200
#define CTYPEEES2C               0x00008300
#define CTYPEEES3A               0x00008400
#define CTYPEEES3C               0x00008500
#define CTYPEEES4A               0x00008600
#define CTYPEEES4C               0x00008700
#define CTYPEEES5A               0x00008800
#define CTYPEEES5C               0x00008900
//
// Define MDT barrel chambers
//
#define CTYPEBIL1A               0x00009000
#define CTYPEBIL1C               0x00009100
#define CTYPEBIL2A               0x00009200
#define CTYPEBIL2C               0x00009300
#define CTYPEBIL3A               0x00009400
#define CTYPEBIL3C               0x00009500
#define CTYPEBIL4A               0x00009600
#define CTYPEBIL4C               0x00009700
#define CTYPEBIL5A               0x00009800
#define CTYPEBIL5C               0x00009900
#define CTYPEBIL6A               0x00009a00
#define CTYPEBIL6C               0x00009b00
#define CTYPEBIL7A               0x00009c00
#define CTYPEBIL7C               0x00009d00
#define CTYPEBIM1A               0x00009e00
#define CTYPEBIM1C               0x00009f00
#define CTYPEBIM2A               0x0000a000
#define CTYPEBIM2C               0x0000a100
#define CTYPEBIM3A               0x0000a200
#define CTYPEBIM3C               0x0000a300
#define CTYPEBIM4A               0x0000a400
#define CTYPEBIM4C               0x0000a500
#define CTYPEBIM5A               0x0000a600
#define CTYPEBIM5C               0x0000a700
#define CTYPEBIM6A               0x0000a800
#define CTYPEBIM6C               0x0000a900
#define CTYPEBIS1A               0x0000aa00
#define CTYPEBIS1C               0x0000ab00
#define CTYPEBIS2A               0x0000ac00
#define CTYPEBIS2C               0x0000ad00
#define CTYPEBIS3A               0x0000ae00
#define CTYPEBIS3C               0x0000af00
#define CTYPEBIS4A               0x0000b000
#define CTYPEBIS4C               0x0000b100
#define CTYPEBIS5A               0x0000b200
#define CTYPEBIS5C               0x0000b300
#define CTYPEBIS6A               0x0000b400
#define CTYPEBIS6C               0x0000b500
#define CTYPEBIS7A               0x0000b600
#define CTYPEBIS7C               0x0000b700
#define CTYPEBIS8A               0x0000b800
#define CTYPEBIS8C               0x0000b900
#define CTYPEBIR1A               0x0000ba00
#define CTYPEBIR1C               0x0000bb00
#define CTYPEBIR2A               0x0000bc00
#define CTYPEBIR2C               0x0000bd00
#define CTYPEBIR3A               0x0000be00
#define CTYPEBIR3C               0x0000bf00
#define CTYPEBIR4A               0x0000c000
#define CTYPEBIR4C               0x0000c100
#define CTYPEBIR5A               0x0000c200
#define CTYPEBIR5C               0x0000c300
#define CTYPEBIR6A               0x0000c400
#define CTYPEBIR6C               0x0000c500
#define CTYPEBIR7A               0x0000c600
#define CTYPEBIR7C               0x0000c700
#define CTYPEBMF1A               0x0000c800
#define CTYPEBMF1C               0x0000c900
#define CTYPEBMF2A               0x0000ca00
#define CTYPEBMF2C               0x0000cb00
#define CTYPEBMF3A               0x0000cc00
#define CTYPEBMF3C               0x0000cd00
#define CTYPEBMF4A               0x0000ce00
#define CTYPEBMF4C               0x0000cf00
#define CTYPEBMF5A               0x0000d000
#define CTYPEBMF5C               0x0000d100
#define CTYPEBML1A               0x0000d200
#define CTYPEBML1C               0x0000d300
#define CTYPEBML2A               0x0000d400
#define CTYPEBML2C               0x0000d500
#define CTYPEBML3A               0x0000d600
#define CTYPEBML3C               0x0000d700
#define CTYPEBML4A               0x0000d800
#define CTYPEBML4C               0x0000d900
#define CTYPEBML5A               0x0000da00
#define CTYPEBML5C               0x0000db00
#define CTYPEBML6A               0x0000dc00
#define CTYPEBML6C               0x0000dd00
#define CTYPEBML7A               0x0000de00
#define CTYPEBML7C               0x0000df00
#define CTYPEBMS1A               0x0000e000
#define CTYPEBMS1C               0x0000e100
#define CTYPEBMS2A               0x0000e200
#define CTYPEBMS2C               0x0000e300
#define CTYPEBMS3A               0x0000e400
#define CTYPEBMS3C               0x0000e500
#define CTYPEBMS4A               0x0000e600
#define CTYPEBMS4C               0x0000e700
#define CTYPEBMS5A               0x0000e800
#define CTYPEBMS5C               0x0000e900
#define CTYPEBMS6A               0x0000ea00
#define CTYPEBMS6C               0x0000eb00
#define CTYPEBOL1A               0x0000f000
#define CTYPEBOL1C               0x0000f100
#define CTYPEBOL2A               0x0000f200
#define CTYPEBOL2C               0x0000f300
#define CTYPEBOL3A               0x0000f400
#define CTYPEBOL3C               0x0000f500
#define CTYPEBOL4A               0x0000f600
#define CTYPEBOL4C               0x0000f700
#define CTYPEBOL5A               0x0000f800
#define CTYPEBOL5C               0x0000f900
#define CTYPEBOL6A               0x0000fa00
#define CTYPEBOL6C               0x0000fb00
#define CTYPEBOL7A               0x0000fc00
#define CTYPEBOL7C               0x0000fd00
#define CTYPEBOS1A               0x00001a00
#define CTYPEBOS1C               0x00001b00
#define CTYPEBOS2A               0x00001c00
#define CTYPEBOS2C               0x00001d00
#define CTYPEBOS3A               0x00001e00
#define CTYPEBOS3C               0x00001f00
#define CTYPEBOS4A               0x00002a00
#define CTYPEBOS4C               0x00002b00
#define CTYPEBOS5A               0x00002c00
#define CTYPEBOS5C               0x00002d00
#define CTYPEBOS6A               0x00002e00
#define CTYPEBOS6C               0x00002f00
#define CTYPEBOSXA               0x00003c00
#define CTYPEBOSXC               0x00003d00
#define CTYPEBOG0A               0x00003e00
#define CTYPEBOG0B               0x00003f00
#define CTYPEBOG0C               0x00004a00
#define CTYPEBOG2A               0x00004b00
#define CTYPEBOG2B               0x00004c00
#define CTYPEBOG2C               0x00004d00
#define CTYPEBOG4A               0x00004e00
#define CTYPEBOG4B               0x00004f00
#define CTYPEBOG4C               0x00005c00
#define CTYPEBOG6A               0x00005d00
#define CTYPEBOG6B               0x00005e00
#define CTYPEBOG6C               0x00005f00
#define CTYPEBOG8A               0x00006c00
#define CTYPEBOG8B               0x00006d00
#define CTYPEBOG8C               0x00006e00
#define CTYPEBOF1A               0x00007a00
#define CTYPEBOF1C               0x00007b00
#define CTYPEBOF3A               0x00007c00
#define CTYPEBOF3C               0x00007d00
#define CTYPEBOF5A               0x00007e00
#define CTYPEBOF5C               0x00007f00
#define CTYPEBOF7A               0x00008a00
#define CTYPEBOF7C               0x00008b00
#define CTYPEBEE1A               0x00008c00
#define CTYPEBEE1C               0x00008d00
#define CTYPEBEE2A               0x00008e00
#define CTYPEBEE2C               0x00008f00


// Define MDT Chamber HV status
//
#define MDTCHAMBERHVOFF                   0
#define MDTCHAMBERHVON                    1
#define MDTCHAMBERHVUNKNOWN               2
#define MDTCHAMBERHVNOTAPPLY              3

// Define MDT Chamber Readout Channel Status
//
#define RDOUTCHOK                         0
#define RDOUTCHDEAD                       1
#define RDOUTCHLOWEFFICIENCY              2
#define RDOUTCHHOT                        3
#define RDOUTCHVERYHOT                    4
#define RDOUTCHASDNOISE                   5
#define RDOUTCHABNORMALOFFSET             6
#define RDOUTCHABNORMALSIGMA              7
#define RDOUTCHHASCROSSTALK               8
#define RDOUTCHBADRESOLUTION              9
#define RDOUTCHBADSLOPE                  10
#define RDOUTCHBADLINEARITY              11
#define RDOUTCHBADGAINFIT                12
#define RDOUTCHBADGAINLINEARITY          13
#define RDOUTCHABNORMALW0                14
#define RDOUTCHBADWIDTHRESOLUTION        15

// Define MDT Chamber Mezzanine Card Status
//
#define MEZZOK                    RDOUTCHOK
#define MEZZNOTINDATABASE                 1
#define MEZZLOWCOEFFI                     2
#define MEZZASD1LOWCOEFFI                 3
#define MEZZASD2LOWCOEFFI                 4
#define MEZZASD3LOWCOEFFI                 5

// Define Offsets for MDT Chamber Readout Status
//
#define RDOUTSTATUSOFFSETHVOFF            0
#define RDOUTSTATUSOFFSETHVON           100

// Define MDT Chamber related constants
//
#define MAXNUMBERROW                      8
#define MAXNUMBERTUBE                    72

int DAQControlHandle, DAQInforHandle, ErrorReportHandle, SerialNumberHandle, RunConditionHandle;
int CheckMezzOnOffHandle;
int checkCSM, checkCSMJTAG, checkCSMIO, CSMTestStatus;
int numberInitDAQ, numberBadInitDAQ, numberReInitDAQ, initDAQStatus;
int mezzOnOffStatus, allDAQHardwareStatus, checkAllDAQHardware, scanSerialNumber;
int requestStopByClient, requestStopTime0, checkMezzOnOff, snButton[MAXNUMBERMEZZANINE];
int newRunNumber, runStartedWithTCPClient, validBCIDPresetValue, requestBackNormal;
int eventReady, getOneEventInitDone, getOneSequentialEventInitDone, initDAQDone, synchWordControl;
int reqThresholdScan, thresholdScanPaired, thresholdScanStartTime, needLoadNominalThreshold;
int reqInjectionScan, injectionScanPaired, injectionScanStartTime, thresholdScanTrigWarning;
int reqLinearityScan, linearityScanPaired, linearityScanStartTime;
int reqGainScan, gainScanPaired, gainScanStartTime, differentScanMixed, chamberIndex;
int willDoThresholdScan, runDiagnostics, pauseRunAsRequested, useOnChamberDCS, expectedELMBNode;
unsigned int verifyFPGATime, verifyPROMTime, requestStartOrStop, restartRunAfterStop;
int MDTChamberControl, MDTChamberNumberMezz, MDTChamberNumberRow, startMezzCard;
int nChamberEntry, nMLayer[5000], nMezzMLayer[5000], nLayer[5000], mezzControl[5000];
char chamberSNumber[5000][30], chamberName[5000][30];
char MDTChamberSN[30], MDTDCSSN[30], motherboardSN[30], CSMSN[30], operatorName[30], serialNumber[30];
char MDTChamberName[30], MDTChamberSumFilename[256], MDTChamberWarningFilename[256], dataDirName[30];
char MDTDCSNIKHEFID[30], CSMName[30];
FILE *MDTChamberSumFile, *MDTChamberWarningFile;

// Variables for client
int clientRunState, clientAnaInstalled, clientCERNLibInstalled, clientProcessingData;

// Variables for fit
int gotMezzDatabase, largeVOffSpan, numberScanRun, numberBadScanRun, numberIgnoredScanRun;
int useExternalTrigger, thresholdScanMethod, numberSubScans, subScans;
int readNominalThresholdOnly, getVOff[MAXNUMBERMEZZANINE], errCount[MAXNUMBERMEZZANINE][3][200];
int hitCount[MAXNUMBERMEZZANINE][24][200], trigCount[3][200];
int threshs[3][200], injMasks[200], calibTrigDelay[200], calibCapacitor[200];
int channelStatusHVOff[24][MAXNUMBERMEZZANINE], channelStatusHVOn[24][MAXNUMBERMEZZANINE];
float timeAverage[24][MAXNUMBERMEZZANINE], timeSigma[24][MAXNUMBERMEZZANINE];
float fracUsed[24][MAXNUMBERMEZZANINE], fracUsedInWidth[24][MAXNUMBERMEZZANINE];
float widthAverage[24][MAXNUMBERMEZZANINE], widthSigma[24][MAXNUMBERMEZZANINE];
float measuredTDCTime[200][24][MAXNUMBERMEZZANINE], measuredTDCTimeResolution[200][24][MAXNUMBERMEZZANINE];
float measuredAverageWidth[200][24][MAXNUMBERMEZZANINE], measuredWidthResolution[200][24][MAXNUMBERMEZZANINE];
float fracUsedInMeasureTDCTime[200][24][MAXNUMBERMEZZANINE], fracUsedInMeasureWidth[200][24][MAXNUMBERMEZZANINE];
float thresholdScanResults[5][24][MAXNUMBERMEZZANINE], benchThresholdScan[5][24][MAXNUMBERMEZZANINE];
float injectionScanResults[2][24][MAXNUMBERMEZZANINE], benchInjectionScan[2][24][MAXNUMBERMEZZANINE];
float linearityScanResults[7][24][MAXNUMBERMEZZANINE], gainScanResults[14][24][MAXNUMBERMEZZANINE];
float coeffiFromThresholdScan[4][MAXNUMBERMEZZANINE], widthBin, allowedFractionBadCycles;

// Analysis controls and variables
int enableAnalysis, firstCallAnalysis, showErrorReport, eventID, dataType, toTCPClients;
unsigned int analysisCounter[NUMBERANALYSISCOUNTER], totalNumberEvent, totalNumberEventHigh;
unsigned int nEdge[2][24][MAXNUMBERMEZZANINE], nPair[24][MAXNUMBERMEZZANINE], nEvents[MAXNUMBERMEZZANINE];
unsigned int nGoodHit[24][MAXNUMBERMEZZANINE], nASDNoise[24][MAXNUMBERMEZZANINE];
double noiseRate[24][MAXNUMBERMEZZANINE], ASDNoiseRate[24][MAXNUMBERMEZZANINE];
double relativeEfficiency[MAXNUMBERTUBE][MAXNUMBERTUBE];

// Cut Variables
double ASDVOffSpanCut, mezzCoeffiCut, ASDCoeffiCut, threScanNDFCut, VOffsetCut, threScanSigmaCut;
double injEffiCut0, injEffiCut1, injEffiCut2, injEffiCut3, injXTalkCut0, injXTalkCut1;
double linMSErrorCut, minTimeSigmaCut, timeSigmaCut, linSlopeCut; 
double gainMSErrorCut, gainCHI2Cut, widthSigmaCut, gainRatioCut, minIntTimeCut, maxIntTimeCut;
double ASDNoiseWarningCut, ASDNoiseCut, noiseCut0, noiseCut1, effiCut0, effiCut1, effiCut2, effiCut3;
double fracCRRateCut0, fracCRRateCut1;

//
// Function prototypes
//
void DAQControlStartUp(void);
int DAQControlButton(void);
void Quit(void);
void StartOrStopDAQ(void);
void StartDAQ(void);
void StopDAQ(void);
void PauseOrResumeDAQ(void);
void PauseDAQ(void);
void ResumeDAQ(void);
void InitDAQ(void);
void SearchJTAGDriver(void);
void SetDAQRunType(void);
void SetRunNumberButton(void);
void SetRunNumber(void);
void NewDose(void);
void DisconnectAllTCPClients(void);
void GetRawData(void);
void GetOneEvent(void);
void GetOneSequentialEvent(void);
int CheckClientInstallations(int warningControl);
int SendTCPCommand(int GOLANumber, int command);
int SendShortTCPCommand(int GOLANumber, int command);
int WaitForTCPClientReply(int port);
void UpdateTCPStatus(int newConnection);
void HandleVariesData(unsigned int data[]);
void PrintOutData(unsigned int *data, FILE *file);
void RawDataInterpretation(unsigned int data, FILE *file);
void DataInterpretation(unsigned int data, FILE *file);
void SetupJTAGRelatedControlButtons(void);
int CheckAMTDataMode(int giveWarning);

// Serial Number Panel
void UpSerialNumber(void);
void LoadPreviousSerialNumber(void);
void OnChamberDCSBoxUsage(void);
void CheckedAllMezzCardSerialNumber(void);
void ReadMDTChamberDatabase(void);

// DAQ error report and data monitor routine prototype
//
void UpErrorReport(void);
void ErrorReportClose(void);
void SetErrorReportCounter(int panelIndex, int counterIndex);
void SaveErrorSummaryFile(void);
void ErrorSummaryPrint(FILE *sumFile, int i, char name[], int maxLength);
void SaveRunLogfile(void);
void SaveResultFile(void);
void SaveScanResults(void);
void SaveEfficiencyAndNoiseRate(void);

// DAQ Infor Routine Prototype
//
void DAQControlInforColor(void);
void CloseDAQInfor(void);
void DAQInforControl(void);
void CollectAndDisplayDAQInfor(void);
void CollectDAQInfor(int counter[]);
void CopyDAQInfor(int counter[]);

// Function prototype to check mezzanine card on/off status
//
void UpCheckMezzCardOnOff(void);
void CheckMezzCardOnOff(void);
void CheckAllDAQHardware(void);

// Variables and routines for Run Condition
int suppressCSMHeader, suppressCSMTrailer, suppressTDCHeader, suppressTDCTrailer;
int MDTChamberHVStatus, saveRawData, checkSystemNoise, mappingMDTChamber;
int analysisControl, saveFlaggedEvt, triggerDeadtime, TTCviTriggerDisabled;
// Setup Run Condition Routine Prototypes
//
int RunConditionPanelAndButton(void);
void UpRunCondition(void);
void RunConditionDone(void);
void RunConditionAllOn(void);
void RunConditionAllOff(void);
void SetMDTChamberHVStatus(void);
void GetRunCondition(void);

// Variables and routines for Auto Run
int AutoRunSetupHandle, autoRunSetupDone;
unsigned int stopRunAtEventOn, stopRunAtEvent, restartRunAtEventOn, restartRunAtEvent;
unsigned int stopAfterNRunOn, stopAfterNRun, restartRunAfterSecOn, restartRunAfterSec;
unsigned int restartRunAtHangTimeOn, restartRunAtHangTime, numberAutoRun, numberBadAutoRun;
unsigned int stopAfterNError, stopAfterNErrorOn, totalRun, totalEvent, processAutoRun;
unsigned int scanControl, initDAQControl, initDAQDoneForRun, autoStartRunControl;
unsigned int scanRun, reqRerun, numberRerun, numberAutoRerun, reqProcessData;
// Auto Run Routine Prototypes
//
int AutoRunSetupPanelAndButton(void);
void UpAutoRunSetup(void);
void AutoRunSetupDone(void);
void AutoRunSetupAllOn(void);
void AutoRunSetupAllOff(void);
void GetAutoRunControls(void);
void AutoRunControl(int restartRun);
void AutoRunSelectControlFile(void);
int GetNewRunSetups(int downloadSetups);

// Chamber related routines
void GetNumberMezzAndRow(int chamberID, int *numberMezz, int *numberRow);
void MapChamberFromMezzCard(int ctype, int mezz, int ch, int *mappedCh, int *row, int *tube);
void MapMezzCardFromChamber(int ctype, int row, int tube, int *mezz, int *ch, int *mappedCh);
int MultilayerNumber(char *chamberName, int row);
double TubeLength(char *chamberName, int row, int tube);
void EnableMezzCardsAccordingMDTChamber(void);

// Fit routines
//
// void GaussFit(int crd, int chn, int* NDF, float* chisq, float* x0, float* sigma, float* logN0);
void InjEff(int crd, int chn, float* eff, float* xtalk);
int FitThresholdScanResults(void);
int FitInjectionScanResults(void);
int FitLinearityScanResults(void);
int FitGainScanResults(void);

#endif
