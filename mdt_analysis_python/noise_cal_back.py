fr = open("run00187727_20190327_dataword.txt","r")
fw = open("run00187727_20190327_dataword_chNoiseRate.txt","w")


lines = fr.readlines()

count=len(lines)

i=0
trigger=0
trigger_cnt=0
nontrigger=0


m1c0_cnt=0
m1c1_cnt=0
m1c2_cnt=0
m1c3_cnt=0
m1c4_cnt=0
m1c5_cnt=0
m1c6_cnt=0
m1c7_cnt=0
m1c8_cnt=0
m1c9_cnt=0
m1c10_cnt=0
m1c11_cnt=0
m1c12_cnt=0
m1c13_cnt=0
m1c14_cnt=0
m1c15_cnt=0
m1c16_cnt=0
m1c17_cnt=0
m1c18_cnt=0
m1c19_cnt=0
m1c20_cnt=0
m1c21_cnt=0
m1c22_cnt=0
m1c23_cnt=0


m8c0_cnt=0
m8c1_cnt=0
m8c2_cnt=0
m8c3_cnt=0
m8c4_cnt=0
m8c5_cnt=0
m8c6_cnt=0
m8c7_cnt=0
m8c8_cnt=0
m8c9_cnt=0
m8c10_cnt=0
m8c11_cnt=0
m8c12_cnt=0
m8c13_cnt=0
m8c14_cnt=0
m8c15_cnt=0
m8c16_cnt=0
m8c17_cnt=0
m8c18_cnt=0
m8c19_cnt=0
m8c20_cnt=0
m8c21_cnt=0
m8c22_cnt=0
m8c23_cnt=0

m9c0_cnt=0
m9c1_cnt=0
m9c2_cnt=0
m9c3_cnt=0
m9c4_cnt=0
m9c5_cnt=0
m9c6_cnt=0
m9c7_cnt=0
m9c8_cnt=0
m9c9_cnt=0
m9c10_cnt=0
m9c11_cnt=0
m9c12_cnt=0
m9c13_cnt=0
m9c14_cnt=0
m9c15_cnt=0
m9c16_cnt=0
m9c17_cnt=0
m9c18_cnt=0
m9c19_cnt=0
m9c20_cnt=0
m9c21_cnt=0
m9c22_cnt=0
m9c23_cnt=0


m10c0_cnt=0
m10c1_cnt=0
m10c2_cnt=0
m10c3_cnt=0
m10c4_cnt=0
m10c5_cnt=0
m10c6_cnt=0
m10c7_cnt=0
m10c8_cnt=0
m10c9_cnt=0
m10c10_cnt=0
m10c11_cnt=0
m10c12_cnt=0
m10c13_cnt=0
m10c14_cnt=0
m10c15_cnt=0
m10c16_cnt=0
m10c17_cnt=0
m10c18_cnt=0
m10c19_cnt=0
m10c20_cnt=0
m10c21_cnt=0
m10c22_cnt=0
m10c23_cnt=0

m11c0_cnt=0
m11c1_cnt=0
m11c2_cnt=0
m11c3_cnt=0
m11c4_cnt=0
m11c5_cnt=0
m11c6_cnt=0
m11c7_cnt=0
m11c8_cnt=0
m11c9_cnt=0
m11c10_cnt=0
m11c11_cnt=0
m11c12_cnt=0
m11c13_cnt=0
m11c14_cnt=0
m11c15_cnt=0
m11c16_cnt=0
m11c17_cnt=0
m11c18_cnt=0
m11c19_cnt=0
m11c20_cnt=0
m11c21_cnt=0
m11c22_cnt=0
m11c23_cnt=0


#######noise rate for each channel######

m1c0_noiserate=0.0
m1c1_noiserate=0.0
m1c2_noiserate=0.0
m1c3_noiserate=0.0
m1c4_noiserate=0.0
m1c5_noiserate=0.0
m1c6_noiserate=0.0
m1c7_noiserate=0.0
m1c8_noiserate=0.0
m1c9_noiserate=0.0
m1c10_noiserate=0.0
m1c11_noiserate=0.0
m1c12_noiserate=0.0
m1c13_noiserate=0.0
m1c14_noiserate=0.0
m1c15_noiserate=0.0
m1c16_noiserate=0.0
m1c17_noiserate=0.0
m1c18_noiserate=0.0
m1c19_noiserate=0.0
m1c20_noiserate=0.0
m1c21_noiserate=0.0
m1c22_noiserate=0.0
m1c23_noiserate=0.0

m8c0_noiserate=0.0
m8c1_noiserate=0.0
m8c2_noiserate=0.0
m8c3_noiserate=0.0
m8c4_noiserate=0.0
m8c5_noiserate=0.0
m8c6_noiserate=0.0
m8c7_noiserate=0.0
m8c8_noiserate=0.0
m8c9_noiserate=0.0
m8c10_noiserate=0.0
m8c11_noiserate=0.0
m8c12_noiserate=0.0
m8c13_noiserate=0.0
m8c14_noiserate=0.0
m8c15_noiserate=0.0
m8c16_noiserate=0.0
m8c17_noiserate=0.0
m8c18_noiserate=0.0
m8c19_noiserate=0.0
m8c20_noiserate=0.0
m8c21_noiserate=0.0
m8c22_noiserate=0.0
m8c23_noiserate=0.0




m9c0_noiserate=0.0
m9c1_noiserate=0.0
m9c2_noiserate=0.0
m9c3_noiserate=0.0
m9c4_noiserate=0.0
m9c5_noiserate=0.0
m9c6_noiserate=0.0
m9c7_noiserate=0.0
m9c8_noiserate=0.0
m9c9_noiserate=0.0
m9c10_noiserate=0.0
m9c11_noiserate=0.0
m9c12_noiserate=0.0
m9c13_noiserate=0.0
m9c14_noiserate=0.0
m9c15_noiserate=0.0
m9c16_noiserate=0.0
m9c17_noiserate=0.0
m9c18_noiserate=0.0
m9c19_noiserate=0.0
m9c20_noiserate=0.0
m9c21_noiserate=0.0
m9c22_noiserate=0.0
m9c23_noiserate=0.0


m10c0_noiserate=0.0
m10c1_noiserate=0.0
m10c2_noiserate=0.0
m10c3_noiserate=0.0
m10c4_noiserate=0.0
m10c5_noiserate=0.0
m10c6_noiserate=0.0
m10c7_noiserate=0.0
m10c8_noiserate=0.0
m10c9_noiserate=0.0
m10c10_noiserate=0.0
m10c11_noiserate=0.0
m10c12_noiserate=0.0
m10c13_noiserate=0.0
m10c14_noiserate=0.0
m10c15_noiserate=0.0
m10c16_noiserate=0.0
m10c17_noiserate=0.0
m10c18_noiserate=0.0
m10c19_noiserate=0.0
m10c20_noiserate=0.0
m10c21_noiserate=0.0
m10c22_noiserate=0.0
m10c23_noiserate=0.0

m11c0_noiserate=0.0
m11c1_noiserate=0.0
m11c2_noiserate=0.0
m11c3_noiserate=0.0
m11c4_noiserate=0.0
m11c5_noiserate=0.0
m11c6_noiserate=0.0
m11c7_noiserate=0.0
m11c8_noiserate=0.0
m11c9_noiserate=0.0
m11c10_noiserate=0.0
m11c11_noiserate=0.0
m11c12_noiserate=0.0
m11c13_noiserate=0.0
m11c14_noiserate=0.0
m11c15_noiserate=0.0
m11c16_noiserate=0.0
m11c17_noiserate=0.0
m11c18_noiserate=0.0
m11c19_noiserate=0.0
m11c20_noiserate=0.0
m11c21_noiserate=0.0
m11c22_noiserate=0.0
m11c23_noiserate=0.0

m8c0_noiserate=0.0
m8c1_noiserate=0.0
m8c2_noiserate=0.0
m8c3_noiserate=0.0
m8c4_noiserate=0.0
m8c5_noiserate=0.0
m8c6_noiserate=0.0
m8c7_noiserate=0.0
m8c8_noiserate=0.0
m8c9_noiserate=0.0
m8c10_noiserate=0.0
m8c11_noiserate=0.0
m8c12_noiserate=0.0
m8c13_noiserate=0.0
m8c14_noiserate=0.0
m8c15_noiserate=0.0
m8c16_noiserate=0.0
m8c17_noiserate=0.0
m8c18_noiserate=0.0
m8c19_noiserate=0.0
m8c20_noiserate=0.0
m8c21_noiserate=0.0
m8c22_noiserate=0.0
m8c23_noiserate=0.0

while i<count-1:
    if lines[i][0:2]=="00":
        trigger=trigger+1
    elif lines[i][0:4]=="2019":
        nontrigger=nontrigger+1

    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==0):
        m1c0_cnt=m1c0_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==1):
        m1c1_cnt=m1c1_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==2):
        m1c2_cnt=m1c2_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==3):
        m1c3_cnt=m1c3_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==4):
        m1c4_cnt=m1c4_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==5):
        m1c5_cnt=m1c5_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==6):
        m1c6_cnt=m1c6_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==7):
        m1c7_cnt=m1c7_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==8):
        m1c8_cnt=m1c8_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==9):
        m1c9_cnt=m1c9_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==10):
        m1c10_cnt=m1c10_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==11):
        m1c11_cnt=m1c11_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==12):
        m1c12_cnt=m1c12_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==13):
        m1c13_cnt=m1c13_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==14):
        m1c14_cnt=m1c14_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==15):
        m1c15_cnt=m1c15_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==16):
        m1c16_cnt=m1c16_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==17):
        m1c17_cnt=m1c17_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==18):
        m1c18_cnt=m1c18_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==19):
        m1c19_cnt=m1c19_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==20):
        m1c20_cnt=m1c20_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==21):
        m1c21_cnt=m1c21_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==22):
        m1c22_cnt=m1c22_cnt+1
    elif (lines[i][0:2]=="41") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==23):
        m1c23_cnt=m1c23_cnt+1   
        
        
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==0):
        m8c0_cnt=m8c0_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==1):
        m8c1_cnt=m8c1_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==2):
        m8c2_cnt=m8c2_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==3):
        m8c3_cnt=m8c3_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==4):
        m8c4_cnt=m8c4_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==5):
        m8c5_cnt=m8c5_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==6):
        m8c6_cnt=m8c6_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==7):
        m8c7_cnt=m8c7_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==8):
        m8c8_cnt=m8c8_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==9):
        m8c9_cnt=m8c9_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==10):
        m8c10_cnt=m8c10_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==11):
        m8c11_cnt=m8c11_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==12):
        m8c12_cnt=m8c12_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==13):
        m8c13_cnt=m8c13_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==14):
        m8c14_cnt=m8c14_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==15):
        m8c15_cnt=m8c15_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==16):
        m8c16_cnt=m8c16_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==17):
        m8c17_cnt=m8c17_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==18):
        m8c18_cnt=m8c18_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==19):
        m8c19_cnt=m8c19_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==20):
        m8c20_cnt=m8c20_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==21):
        m8c21_cnt=m8c21_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==22):
        m8c22_cnt=m8c22_cnt+1
    elif (lines[i][0:2]=="48") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==23):
        m8c23_cnt=m8c23_cnt+1


    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==0):
        m9c0_cnt=m9c0_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==1):
        m9c1_cnt=m9c1_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==2):
        m9c2_cnt=m9c2_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==3):
        m9c3_cnt=m9c3_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==4):
        m9c4_cnt=m9c4_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==5):
        m9c5_cnt=m9c5_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==6):
        m9c6_cnt=m9c6_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==7):
        m9c7_cnt=m9c7_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==8):
        m9c8_cnt=m9c8_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==9):
        m9c9_cnt=m9c9_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==10):
        m9c10_cnt=m9c10_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==11):
        m9c11_cnt=m9c11_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==12):
        m9c12_cnt=m9c12_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==13):
        m9c13_cnt=m9c13_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==14):
        m9c14_cnt=m9c14_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==15):
        m9c15_cnt=m9c15_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==16):
        m9c16_cnt=m9c16_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==17):
        m9c17_cnt=m9c17_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==18):
        m9c18_cnt=m9c18_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==19):
        m9c19_cnt=m9c19_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==20):
        m9c20_cnt=m9c20_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==21):
        m9c21_cnt=m9c21_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==22):
        m9c22_cnt=m9c22_cnt+1
    elif (lines[i][0:2]=="49") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==23):
        m9c23_cnt=m9c23_cnt+1        


    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==0):
        m10c0_cnt=m10c0_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==1):
        m10c1_cnt=m10c1_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==2):
        m10c2_cnt=m10c2_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==3):
        m10c3_cnt=m10c3_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==4):
        m10c4_cnt=m10c4_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==5):
        m10c5_cnt=m10c5_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==6):
        m10c6_cnt=m10c6_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==7):
        m10c7_cnt=m10c7_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==8):
        m10c8_cnt=m10c8_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==9):
        m10c9_cnt=m10c9_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==10):
        m10c10_cnt=m10c10_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==11):
        m10c11_cnt=m10c11_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==12):
        m10c12_cnt=m10c12_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==13):
        m10c13_cnt=m10c13_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==14):
        m10c14_cnt=m10c14_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==15):
        m10c15_cnt=m10c15_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==16):
        m10c16_cnt=m10c16_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==17):
        m10c17_cnt=m10c17_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==18):
        m10c18_cnt=m10c18_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==19):
        m10c19_cnt=m10c19_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==20):
        m10c20_cnt=m10c20_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==21):
        m10c21_cnt=m10c21_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==22):
        m10c22_cnt=m10c22_cnt+1
    elif (lines[i][0:2]=="4a") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==23):
        m10c23_cnt=m10c23_cnt+1   



    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==0):
        m11c0_cnt=m11c0_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==1):
        m11c1_cnt=m11c1_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==2):
        m11c2_cnt=m11c2_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==3):
        m11c3_cnt=m11c3_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==4):
        m11c4_cnt=m11c4_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==5):
        m11c5_cnt=m11c5_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==6):
        m11c6_cnt=m11c6_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==7):
        m11c7_cnt=m11c7_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==8):
        m11c8_cnt=m11c8_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==9):
        m11c9_cnt=m11c9_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==10):
        m11c10_cnt=m11c10_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==11):
        m11c11_cnt=m11c11_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==12):
        m11c12_cnt=m11c12_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==13):
        m11c13_cnt=m11c13_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==14):
        m11c14_cnt=m11c14_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==15):
        m11c15_cnt=m11c15_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==16):
        m11c16_cnt=m11c16_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==17):
        m11c17_cnt=m11c17_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==18):
        m11c18_cnt=m11c18_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==19):
        m11c19_cnt=m11c19_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==20):
        m11c20_cnt=m11c20_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==21):
        m11c21_cnt=m11c21_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==22):
        m11c22_cnt=m11c22_cnt+1
    elif (lines[i][0:2]=="4b") and (int(bin(int('1'+lines[i][0:8],16))[3:][8:13],2)==23):
        m11c23_cnt=m11c23_cnt+1   
        
    i=i+1
        
trigger_cnt=trigger-nontrigger*11

m1c0_noiserate= m1c0_cnt/(trigger_cnt*0.00000155 )                             
m1c1_noiserate= m1c1_cnt/(trigger_cnt*0.00000155 )  
m1c2_noiserate= m1c2_cnt/(trigger_cnt*0.00000155 )    
m1c3_noiserate= m1c3_cnt/(trigger_cnt*0.00000155 ) 
m1c4_noiserate= m1c4_cnt/(trigger_cnt*0.00000155 ) 
m1c5_noiserate= m1c5_cnt/(trigger_cnt*0.00000155 ) 
m1c6_noiserate= m1c6_cnt/(trigger_cnt*0.00000155 ) 
m1c7_noiserate= m1c7_cnt/(trigger_cnt*0.00000155 ) 
m1c8_noiserate= m1c8_cnt/(trigger_cnt*0.00000155 ) 
m1c9_noiserate= m1c9_cnt/(trigger_cnt*0.00000155 ) 
m1c10_noiserate= m1c10_cnt/(trigger_cnt*0.00000155 ) 
m1c11_noiserate= m1c11_cnt/(trigger_cnt*0.00000155 ) 
m1c12_noiserate= m1c12_cnt/(trigger_cnt*0.00000155 ) 
m1c13_noiserate= m1c13_cnt/(trigger_cnt*0.00000155 ) 
m1c14_noiserate= m1c14_cnt/(trigger_cnt*0.00000155 ) 
m1c15_noiserate= m1c15_cnt/(trigger_cnt*0.00000155 ) 
m1c16_noiserate= m1c16_cnt/(trigger_cnt*0.00000155 ) 
m1c17_noiserate= m1c17_cnt/(trigger_cnt*0.00000155 ) 
m1c18_noiserate= m1c18_cnt/(trigger_cnt*0.00000155 ) 
m1c19_noiserate= m1c19_cnt/(trigger_cnt*0.00000155 ) 
m1c20_noiserate= m1c20_cnt/(trigger_cnt*0.00000155 ) 
m1c21_noiserate= m1c21_cnt/(trigger_cnt*0.00000155 ) 
m1c22_noiserate= m1c22_cnt/(trigger_cnt*0.00000155 ) 
m1c23_noiserate= m1c23_cnt/(trigger_cnt*0.00000155 ) 

                
m8c0_noiserate= m8c0_cnt/(trigger_cnt*0.00000155 )                              
m8c1_noiserate= m8c1_cnt/(trigger_cnt*0.00000155 )  
m8c2_noiserate= m8c2_cnt/(trigger_cnt*0.00000155 )    
m8c3_noiserate= m8c3_cnt/(trigger_cnt*0.00000155 ) 
m8c4_noiserate= m8c4_cnt/(trigger_cnt*0.00000155 ) 
m8c5_noiserate= m8c5_cnt/(trigger_cnt*0.00000155 ) 
m8c6_noiserate= m8c6_cnt/(trigger_cnt*0.00000155 ) 
m8c7_noiserate= m8c7_cnt/(trigger_cnt*0.00000155 ) 
m8c8_noiserate= m8c8_cnt/(trigger_cnt*0.00000155 ) 
m8c9_noiserate= m8c9_cnt/(trigger_cnt*0.00000155 ) 
m8c10_noiserate= m8c10_cnt/(trigger_cnt*0.00000155 ) 
m8c11_noiserate= m8c11_cnt/(trigger_cnt*0.00000155 ) 
m8c12_noiserate= m8c12_cnt/(trigger_cnt*0.00000155 ) 
m8c13_noiserate= m8c13_cnt/(trigger_cnt*0.00000155 ) 
m8c14_noiserate= m8c14_cnt/(trigger_cnt*0.00000155 ) 
m8c15_noiserate= m8c15_cnt/(trigger_cnt*0.00000155 ) 
m8c16_noiserate= m8c16_cnt/(trigger_cnt*0.00000155 ) 
m8c17_noiserate= m8c17_cnt/(trigger_cnt*0.00000155 ) 
m8c18_noiserate= m8c18_cnt/(trigger_cnt*0.00000155 ) 
m8c19_noiserate= m8c19_cnt/(trigger_cnt*0.00000155 ) 
m8c20_noiserate= m8c20_cnt/(trigger_cnt*0.00000155 ) 
m8c21_noiserate= m8c21_cnt/(trigger_cnt*0.00000155 ) 
m8c22_noiserate= m8c22_cnt/(trigger_cnt*0.00000155 ) 
m8c23_noiserate= m8c23_cnt/(trigger_cnt*0.00000155 ) 


m9c0_noiserate= m9c0_cnt/(trigger_cnt*0.00000155 )                              
m9c1_noiserate= m9c1_cnt/(trigger_cnt*0.00000155 )  
m9c2_noiserate= m9c2_cnt/(trigger_cnt*0.00000155 )    
m9c3_noiserate= m9c3_cnt/(trigger_cnt*0.00000155 ) 
m9c4_noiserate= m9c4_cnt/(trigger_cnt*0.00000155 ) 
m9c5_noiserate= m9c5_cnt/(trigger_cnt*0.00000155 ) 
m9c6_noiserate= m9c6_cnt/(trigger_cnt*0.00000155 ) 
m9c7_noiserate= m9c7_cnt/(trigger_cnt*0.00000155 ) 
m9c8_noiserate= m9c8_cnt/(trigger_cnt*0.00000155 ) 
m9c9_noiserate= m9c9_cnt/(trigger_cnt*0.00000155 ) 
m9c10_noiserate= m9c10_cnt/(trigger_cnt*0.00000155 ) 
m9c11_noiserate= m9c11_cnt/(trigger_cnt*0.00000155 ) 
m9c12_noiserate= m9c12_cnt/(trigger_cnt*0.00000155 ) 
m9c13_noiserate= m9c13_cnt/(trigger_cnt*0.00000155 ) 
m9c14_noiserate= m9c14_cnt/(trigger_cnt*0.00000155 ) 
m9c15_noiserate= m9c15_cnt/(trigger_cnt*0.00000155 ) 
m9c16_noiserate= m9c16_cnt/(trigger_cnt*0.00000155 ) 
m9c17_noiserate= m9c17_cnt/(trigger_cnt*0.00000155 ) 
m9c18_noiserate= m9c18_cnt/(trigger_cnt*0.00000155 ) 
m9c19_noiserate= m9c19_cnt/(trigger_cnt*0.00000155 ) 
m9c20_noiserate= m9c20_cnt/(trigger_cnt*0.00000155 ) 
m9c21_noiserate= m9c21_cnt/(trigger_cnt*0.00000155 ) 
m9c22_noiserate= m9c22_cnt/(trigger_cnt*0.00000155 ) 
m9c23_noiserate= m9c23_cnt/(trigger_cnt*0.00000155 ) 

m10c0_noiserate= m10c0_cnt/(trigger_cnt*0.00000155 )                              
m10c1_noiserate= m10c1_cnt/(trigger_cnt*0.00000155 )  
m10c2_noiserate= m10c2_cnt/(trigger_cnt*0.00000155 )    
m10c3_noiserate= m10c3_cnt/(trigger_cnt*0.00000155 ) 
m10c4_noiserate= m10c4_cnt/(trigger_cnt*0.00000155 ) 
m10c5_noiserate= m10c5_cnt/(trigger_cnt*0.00000155 ) 
m10c6_noiserate= m10c6_cnt/(trigger_cnt*0.00000155 ) 
m10c7_noiserate= m10c7_cnt/(trigger_cnt*0.00000155 ) 
m10c8_noiserate= m10c8_cnt/(trigger_cnt*0.00000155 ) 
m10c9_noiserate= m10c9_cnt/(trigger_cnt*0.00000155 ) 
m10c10_noiserate= m10c10_cnt/(trigger_cnt*0.00000155 ) 
m10c11_noiserate= m10c11_cnt/(trigger_cnt*0.00000155 ) 
m10c12_noiserate= m10c12_cnt/(trigger_cnt*0.00000155 ) 
m10c13_noiserate= m10c13_cnt/(trigger_cnt*0.00000155 ) 
m10c14_noiserate= m10c14_cnt/(trigger_cnt*0.00000155 ) 
m10c15_noiserate= m10c15_cnt/(trigger_cnt*0.00000155 ) 
m10c16_noiserate= m10c16_cnt/(trigger_cnt*0.00000155 ) 
m10c17_noiserate= m10c17_cnt/(trigger_cnt*0.00000155 ) 
m10c18_noiserate= m10c18_cnt/(trigger_cnt*0.00000155 ) 
m10c19_noiserate= m10c19_cnt/(trigger_cnt*0.00000155 ) 
m10c20_noiserate= m10c20_cnt/(trigger_cnt*0.00000155 ) 
m10c21_noiserate= m10c21_cnt/(trigger_cnt*0.00000155 ) 
m10c22_noiserate= m10c22_cnt/(trigger_cnt*0.00000155 ) 
m10c23_noiserate= m10c23_cnt/(trigger_cnt*0.00000155 ) 

m11c0_noiserate= m11c0_cnt/(trigger_cnt*0.00000155 )                              
m11c1_noiserate= m11c1_cnt/(trigger_cnt*0.00000155 )  
m11c2_noiserate= m11c2_cnt/(trigger_cnt*0.00000155 )    
m11c3_noiserate= m11c3_cnt/(trigger_cnt*0.00000155 ) 
m11c4_noiserate= m11c4_cnt/(trigger_cnt*0.00000155 ) 
m11c5_noiserate= m11c5_cnt/(trigger_cnt*0.00000155 ) 
m11c6_noiserate= m11c6_cnt/(trigger_cnt*0.00000155 ) 
m11c7_noiserate= m11c7_cnt/(trigger_cnt*0.00000155 ) 
m11c8_noiserate= m11c8_cnt/(trigger_cnt*0.00000155 ) 
m11c9_noiserate= m11c9_cnt/(trigger_cnt*0.00000155 ) 
m11c10_noiserate= m11c10_cnt/(trigger_cnt*0.00000155 ) 
m11c11_noiserate= m11c11_cnt/(trigger_cnt*0.00000155 ) 
m11c12_noiserate= m11c12_cnt/(trigger_cnt*0.00000155 ) 
m11c13_noiserate= m11c13_cnt/(trigger_cnt*0.00000155 ) 
m11c14_noiserate= m11c14_cnt/(trigger_cnt*0.00000155 ) 
m11c15_noiserate= m11c15_cnt/(trigger_cnt*0.00000155 ) 
m11c16_noiserate= m11c16_cnt/(trigger_cnt*0.00000155 ) 
m11c17_noiserate= m11c17_cnt/(trigger_cnt*0.00000155 ) 
m11c18_noiserate= m11c18_cnt/(trigger_cnt*0.00000155 ) 
m11c19_noiserate= m11c19_cnt/(trigger_cnt*0.00000155 ) 
m11c20_noiserate= m11c20_cnt/(trigger_cnt*0.00000155 ) 
m11c21_noiserate= m11c21_cnt/(trigger_cnt*0.00000155 ) 
m11c22_noiserate= m11c22_cnt/(trigger_cnt*0.00000155 ) 
m11c23_noiserate= m11c23_cnt/(trigger_cnt*0.00000155 ) 

print(m8c0_noiserate)    
print(m8c1_noiserate)  


fw.writelines("trigger count: "+str(trigger_cnt) + '\n')



fw.writelines("m1c0 noise rate: "+ str(m1c0_noiserate) + '\n')
fw.writelines("m1c1 noise rate: "+ str(m1c1_noiserate) + '\n')
fw.writelines("m1c2 noise rate: "+ str(m1c2_noiserate) + '\n')
fw.writelines("m1c3 noise rate: "+ str(m1c3_noiserate) + '\n')
fw.writelines("m1c4 noise rate: "+ str(m1c4_noiserate) + '\n')
fw.writelines("m1c5 noise rate: "+ str(m1c5_noiserate) + '\n')
fw.writelines("m1c6 noise rate: "+ str(m1c6_noiserate) + '\n')
fw.writelines("m1c7 noise rate: "+ str(m1c7_noiserate) + '\n')
fw.writelines("m1c8 noise rate: "+ str(m1c8_noiserate) + '\n')
fw.writelines("m1c9 noise rate: "+ str(m1c9_noiserate) + '\n')
fw.writelines("m1c10 noise rate: "+ str(m1c10_noiserate) + '\n')
fw.writelines("m1c11 noise rate: "+ str(m1c11_noiserate) + '\n')
fw.writelines("m1c12 noise rate: "+ str(m1c12_noiserate) + '\n')
fw.writelines("m1c13 noise rate: "+ str(m1c13_noiserate) + '\n')
fw.writelines("m1c14 noise rate: "+ str(m1c14_noiserate) + '\n')
fw.writelines("m1c15 noise rate: "+ str(m1c15_noiserate) + '\n')
fw.writelines("m1c16 noise rate: "+ str(m1c16_noiserate) + '\n')
fw.writelines("m1c17 noise rate: "+ str(m1c17_noiserate) + '\n')
fw.writelines("m1c18 noise rate: "+ str(m1c18_noiserate) + '\n')
fw.writelines("m1c19 noise rate: "+ str(m1c19_noiserate) + '\n')
fw.writelines("m1c20 noise rate: "+ str(m1c20_noiserate) + '\n')
fw.writelines("m1c21 noise rate: "+ str(m1c21_noiserate) + '\n')
fw.writelines("m1c22 noise rate: "+ str(m1c22_noiserate) + '\n')
fw.writelines("m1c23 noise rate: "+ str(m1c23_noiserate) + '\n')

fw.writelines("m8c0 noise rate: "+ str(m8c0_noiserate) + '\n')
fw.writelines("m8c1 noise rate: "+ str(m8c1_noiserate) + '\n')
fw.writelines("m8c2 noise rate: "+ str(m8c2_noiserate) + '\n')
fw.writelines("m8c3 noise rate: "+ str(m8c3_noiserate) + '\n')
fw.writelines("m8c4 noise rate: "+ str(m8c4_noiserate) + '\n')
fw.writelines("m8c5 noise rate: "+ str(m8c5_noiserate) + '\n')
fw.writelines("m8c6 noise rate: "+ str(m8c6_noiserate) + '\n')
fw.writelines("m8c7 noise rate: "+ str(m8c7_noiserate) + '\n')
fw.writelines("m8c8 noise rate: "+ str(m8c8_noiserate) + '\n')
fw.writelines("m8c9 noise rate: "+ str(m8c9_noiserate) + '\n')
fw.writelines("m8c10 noise rate: "+ str(m8c10_noiserate) + '\n')
fw.writelines("m8c11 noise rate: "+ str(m8c11_noiserate) + '\n')
fw.writelines("m8c12 noise rate: "+ str(m8c12_noiserate) + '\n')
fw.writelines("m8c13 noise rate: "+ str(m8c13_noiserate) + '\n')
fw.writelines("m8c14 noise rate: "+ str(m8c14_noiserate) + '\n')
fw.writelines("m8c15 noise rate: "+ str(m8c15_noiserate) + '\n')
fw.writelines("m8c16 noise rate: "+ str(m8c16_noiserate) + '\n')
fw.writelines("m8c17 noise rate: "+ str(m8c17_noiserate) + '\n')
fw.writelines("m8c18 noise rate: "+ str(m8c18_noiserate) + '\n')
fw.writelines("m8c19 noise rate: "+ str(m8c19_noiserate) + '\n')
fw.writelines("m8c20 noise rate: "+ str(m8c20_noiserate) + '\n')
fw.writelines("m8c21 noise rate: "+ str(m8c21_noiserate) + '\n')
fw.writelines("m8c22 noise rate: "+ str(m8c22_noiserate) + '\n')
fw.writelines("m8c23 noise rate: "+ str(m8c23_noiserate) + '\n')

fw.writelines("m9c0 noise rate: "+ str(m9c0_noiserate) + '\n')
fw.writelines("m9c1 noise rate: "+ str(m9c1_noiserate) + '\n')
fw.writelines("m9c2 noise rate: "+ str(m9c2_noiserate) + '\n')
fw.writelines("m9c3 noise rate: "+ str(m9c3_noiserate) + '\n')
fw.writelines("m9c4 noise rate: "+ str(m9c4_noiserate) + '\n')
fw.writelines("m9c5 noise rate: "+ str(m9c5_noiserate) + '\n')
fw.writelines("m9c6 noise rate: "+ str(m9c6_noiserate) + '\n')
fw.writelines("m9c7 noise rate: "+ str(m9c7_noiserate) + '\n')
fw.writelines("m9c8 noise rate: "+ str(m9c8_noiserate) + '\n')
fw.writelines("m9c9 noise rate: "+ str(m9c9_noiserate) + '\n')
fw.writelines("m9c10 noise rate: "+ str(m9c10_noiserate) + '\n')
fw.writelines("m9c11 noise rate: "+ str(m9c11_noiserate) + '\n')
fw.writelines("m9c12 noise rate: "+ str(m9c12_noiserate) + '\n')
fw.writelines("m9c13 noise rate: "+ str(m9c13_noiserate) + '\n')
fw.writelines("m9c14 noise rate: "+ str(m9c14_noiserate) + '\n')
fw.writelines("m9c15 noise rate: "+ str(m9c15_noiserate) + '\n')
fw.writelines("m9c16 noise rate: "+ str(m9c16_noiserate) + '\n')
fw.writelines("m9c17 noise rate: "+ str(m9c17_noiserate) + '\n')
fw.writelines("m9c18 noise rate: "+ str(m9c18_noiserate) + '\n')
fw.writelines("m9c19 noise rate: "+ str(m9c19_noiserate) + '\n')
fw.writelines("m9c20 noise rate: "+ str(m9c20_noiserate) + '\n')
fw.writelines("m9c21 noise rate: "+ str(m9c21_noiserate) + '\n')
fw.writelines("m9c22 noise rate: "+ str(m9c22_noiserate) + '\n')
fw.writelines("m9c23 noise rate: "+ str(m9c23_noiserate) + '\n')

fw.writelines("m10c0 noise rate: "+ str(m10c0_noiserate) + '\n')
fw.writelines("m10c1 noise rate: "+ str(m10c1_noiserate) + '\n')
fw.writelines("m10c2 noise rate: "+ str(m10c2_noiserate) + '\n')
fw.writelines("m10c3 noise rate: "+ str(m10c3_noiserate) + '\n')
fw.writelines("m10c4 noise rate: "+ str(m10c4_noiserate) + '\n')
fw.writelines("m10c5 noise rate: "+ str(m10c5_noiserate) + '\n')
fw.writelines("m10c6 noise rate: "+ str(m10c6_noiserate) + '\n')
fw.writelines("m10c7 noise rate: "+ str(m10c7_noiserate) + '\n')
fw.writelines("m10c8 noise rate: "+ str(m10c8_noiserate) + '\n')
fw.writelines("m10c9 noise rate: "+ str(m10c9_noiserate) + '\n')
fw.writelines("m10c10 noise rate: "+ str(m10c10_noiserate) + '\n')
fw.writelines("m10c11 noise rate: "+ str(m10c11_noiserate) + '\n')
fw.writelines("m10c12 noise rate: "+ str(m10c12_noiserate) + '\n')
fw.writelines("m10c13 noise rate: "+ str(m10c13_noiserate) + '\n')
fw.writelines("m10c14 noise rate: "+ str(m10c14_noiserate) + '\n')
fw.writelines("m10c15 noise rate: "+ str(m10c15_noiserate) + '\n')
fw.writelines("m10c16 noise rate: "+ str(m10c16_noiserate) + '\n')
fw.writelines("m10c17 noise rate: "+ str(m10c17_noiserate) + '\n')
fw.writelines("m10c18 noise rate: "+ str(m10c18_noiserate) + '\n')
fw.writelines("m10c19 noise rate: "+ str(m10c19_noiserate) + '\n')
fw.writelines("m10c20 noise rate: "+ str(m10c20_noiserate) + '\n')
fw.writelines("m10c21 noise rate: "+ str(m10c21_noiserate) + '\n')
fw.writelines("m10c22 noise rate: "+ str(m10c22_noiserate) + '\n')
fw.writelines("m10c23 noise rate: "+ str(m10c23_noiserate) + '\n')


fw.writelines("m11c0 noise rate: "+ str(m11c0_noiserate) + '\n')
fw.writelines("m11c1 noise rate: "+ str(m11c1_noiserate) + '\n')
fw.writelines("m11c2 noise rate: "+ str(m11c2_noiserate) + '\n')
fw.writelines("m11c3 noise rate: "+ str(m11c3_noiserate) + '\n')
fw.writelines("m11c4 noise rate: "+ str(m11c4_noiserate) + '\n')
fw.writelines("m11c5 noise rate: "+ str(m11c5_noiserate) + '\n')
fw.writelines("m11c6 noise rate: "+ str(m11c6_noiserate) + '\n')
fw.writelines("m11c7 noise rate: "+ str(m11c7_noiserate) + '\n')
fw.writelines("m11c8 noise rate: "+ str(m11c8_noiserate) + '\n')
fw.writelines("m11c9 noise rate: "+ str(m11c9_noiserate) + '\n')
fw.writelines("m11c10 noise rate: "+ str(m11c10_noiserate) + '\n')
fw.writelines("m11c11 noise rate: "+ str(m11c11_noiserate) + '\n')
fw.writelines("m11c12 noise rate: "+ str(m11c12_noiserate) + '\n')
fw.writelines("m11c13 noise rate: "+ str(m11c13_noiserate) + '\n')
fw.writelines("m11c14 noise rate: "+ str(m11c14_noiserate) + '\n')
fw.writelines("m11c15 noise rate: "+ str(m11c15_noiserate) + '\n')
fw.writelines("m11c16 noise rate: "+ str(m11c16_noiserate) + '\n')
fw.writelines("m11c17 noise rate: "+ str(m11c17_noiserate) + '\n')
fw.writelines("m11c18 noise rate: "+ str(m11c18_noiserate) + '\n')
fw.writelines("m11c19 noise rate: "+ str(m11c19_noiserate) + '\n')
fw.writelines("m11c20 noise rate: "+ str(m11c20_noiserate) + '\n')
fw.writelines("m11c21 noise rate: "+ str(m11c21_noiserate) + '\n')
fw.writelines("m11c22 noise rate: "+ str(m11c22_noiserate) + '\n')
fw.writelines("m11c23 noise rate: "+ str(m11c23_noiserate) + '\n')


fw.writelines("m1c0 count: "+ str(m1c0_cnt) + '\n')
fw.writelines("m1c1 count: "+ str(m1c1_cnt) + '\n')
fw.writelines("m1c2 count: "+ str(m1c2_cnt) + '\n')
fw.writelines("m1c3 count: "+ str(m1c3_cnt) + '\n')
fw.writelines("m1c4 count: "+ str(m1c4_cnt) + '\n')
fw.writelines("m1c5 count: "+ str(m1c5_cnt) + '\n')
fw.writelines("m1c6 count: "+ str(m1c6_cnt) + '\n')
fw.writelines("m1c7 count: "+ str(m1c7_cnt) + '\n')
fw.writelines("m1c8 count: "+ str(m1c8_cnt) + '\n')
fw.writelines("m1c9 count: "+ str(m1c9_cnt) + '\n')
fw.writelines("m1c10 count: "+ str(m1c10_cnt) + '\n')
fw.writelines("m1c11 count: "+ str(m1c11_cnt) + '\n')
fw.writelines("m1c12 count: "+ str(m1c12_cnt) + '\n')
fw.writelines("m1c13 count: "+ str(m1c13_cnt) + '\n')
fw.writelines("m1c14 count: "+ str(m1c14_cnt) + '\n')
fw.writelines("m1c15 count: "+ str(m1c15_cnt) + '\n')
fw.writelines("m1c16 count: "+ str(m1c16_cnt) + '\n')
fw.writelines("m1c17 count: "+ str(m1c17_cnt) + '\n')
fw.writelines("m1c18 count: "+ str(m1c18_cnt) + '\n')
fw.writelines("m1c19 count: "+ str(m1c19_cnt) + '\n')
fw.writelines("m1c20 count: "+ str(m1c20_cnt) + '\n')
fw.writelines("m1c21 count: "+ str(m1c21_cnt) + '\n')
fw.writelines("m1c22 count: "+ str(m1c22_cnt) + '\n')
fw.writelines("m1c23 count: "+ str(m1c23_cnt) + '\n')

fw.writelines("m8c0 count: "+ str(m8c0_cnt) + '\n')
fw.writelines("m8c1 count: "+ str(m8c1_cnt) + '\n')
fw.writelines("m8c2 count: "+ str(m8c2_cnt) + '\n')
fw.writelines("m8c3 count: "+ str(m8c3_cnt) + '\n')
fw.writelines("m8c4 count: "+ str(m8c4_cnt) + '\n')
fw.writelines("m8c5 count: "+ str(m8c5_cnt) + '\n')
fw.writelines("m8c6 count: "+ str(m8c6_cnt) + '\n')
fw.writelines("m8c7 count: "+ str(m8c7_cnt) + '\n')
fw.writelines("m8c8 count: "+ str(m8c8_cnt) + '\n')
fw.writelines("m8c9 count: "+ str(m8c9_cnt) + '\n')
fw.writelines("m8c10 count: "+ str(m8c10_cnt) + '\n')
fw.writelines("m8c11 count: "+ str(m8c11_cnt) + '\n')
fw.writelines("m8c12 count: "+ str(m8c12_cnt) + '\n')
fw.writelines("m8c13 count: "+ str(m8c13_cnt) + '\n')
fw.writelines("m8c14 count: "+ str(m8c14_cnt) + '\n')
fw.writelines("m8c15 count: "+ str(m8c15_cnt) + '\n')
fw.writelines("m8c16 count: "+ str(m8c16_cnt) + '\n')
fw.writelines("m8c17 count: "+ str(m8c17_cnt) + '\n')
fw.writelines("m8c18 count: "+ str(m8c18_cnt) + '\n')
fw.writelines("m8c19 count: "+ str(m8c19_cnt) + '\n')
fw.writelines("m8c20 count: "+ str(m8c20_cnt) + '\n')
fw.writelines("m8c21 count: "+ str(m8c21_cnt) + '\n')
fw.writelines("m8c22 count: "+ str(m8c22_cnt) + '\n')
fw.writelines("m8c23 count: "+ str(m8c23_cnt) + '\n')

fw.writelines("m9c0 count: "+ str(m9c0_cnt) + '\n')
fw.writelines("m9c1 count: "+ str(m9c1_cnt) + '\n')
fw.writelines("m9c2 count: "+ str(m9c2_cnt) + '\n')
fw.writelines("m9c3 count: "+ str(m9c3_cnt) + '\n')
fw.writelines("m9c4 count: "+ str(m9c4_cnt) + '\n')
fw.writelines("m9c5 count: "+ str(m9c5_cnt) + '\n')
fw.writelines("m9c6 count: "+ str(m9c6_cnt) + '\n')
fw.writelines("m9c7 count: "+ str(m9c7_cnt) + '\n')
fw.writelines("m9c8 count: "+ str(m9c8_cnt) + '\n')
fw.writelines("m9c9 count: "+ str(m9c9_cnt) + '\n')
fw.writelines("m9c10 count: "+ str(m9c10_cnt) + '\n')
fw.writelines("m9c11 count: "+ str(m9c11_cnt) + '\n')
fw.writelines("m9c12 count: "+ str(m9c12_cnt) + '\n')
fw.writelines("m9c13 count: "+ str(m9c13_cnt) + '\n')
fw.writelines("m9c14 count: "+ str(m9c14_cnt) + '\n')
fw.writelines("m9c15 count: "+ str(m9c15_cnt) + '\n')
fw.writelines("m9c16 count: "+ str(m9c16_cnt) + '\n')
fw.writelines("m9c17 count: "+ str(m9c17_cnt) + '\n')
fw.writelines("m9c18 count: "+ str(m9c18_cnt) + '\n')
fw.writelines("m9c19 count: "+ str(m9c19_cnt) + '\n')
fw.writelines("m9c20 count: "+ str(m9c20_cnt) + '\n')
fw.writelines("m9c21 count: "+ str(m9c21_cnt) + '\n')
fw.writelines("m9c22 count: "+ str(m9c22_cnt) + '\n')
fw.writelines("m9c23 count: "+ str(m9c23_cnt) + '\n')

fw.writelines("m10c0 count: "+ str(m10c0_cnt) + '\n')
fw.writelines("m10c1 count: "+ str(m10c1_cnt) + '\n')
fw.writelines("m10c2 count: "+ str(m10c2_cnt) + '\n')
fw.writelines("m10c3 count: "+ str(m10c3_cnt) + '\n')
fw.writelines("m10c4 count: "+ str(m10c4_cnt) + '\n')
fw.writelines("m10c5 count: "+ str(m10c5_cnt) + '\n')
fw.writelines("m10c6 count: "+ str(m10c6_cnt) + '\n')
fw.writelines("m10c7 count: "+ str(m10c7_cnt) + '\n')
fw.writelines("m10c8 count: "+ str(m10c8_cnt) + '\n')
fw.writelines("m10c9 count: "+ str(m10c9_cnt) + '\n')
fw.writelines("m10c10 count: "+ str(m10c10_cnt) + '\n')
fw.writelines("m10c11 count: "+ str(m10c11_cnt) + '\n')
fw.writelines("m10c12 count: "+ str(m10c12_cnt) + '\n')
fw.writelines("m10c13 count: "+ str(m10c13_cnt) + '\n')
fw.writelines("m10c14 count: "+ str(m10c14_cnt) + '\n')
fw.writelines("m10c15 count: "+ str(m10c15_cnt) + '\n')
fw.writelines("m10c16 count: "+ str(m10c16_cnt) + '\n')
fw.writelines("m10c17 count: "+ str(m10c17_cnt) + '\n')
fw.writelines("m10c18 count: "+ str(m10c18_cnt) + '\n')
fw.writelines("m10c19 count: "+ str(m10c19_cnt) + '\n')
fw.writelines("m10c20 count: "+ str(m10c20_cnt) + '\n')
fw.writelines("m10c21 count: "+ str(m10c21_cnt) + '\n')
fw.writelines("m10c22 count: "+ str(m10c22_cnt) + '\n')
fw.writelines("m10c23 count: "+ str(m10c23_cnt) + '\n')


fw.writelines("m11c0 count: "+ str(m11c0_cnt) + '\n')
fw.writelines("m11c1 count: "+ str(m11c1_cnt) + '\n')
fw.writelines("m11c2 count: "+ str(m11c2_cnt) + '\n')
fw.writelines("m11c3 count: "+ str(m11c3_cnt) + '\n')
fw.writelines("m11c4 count: "+ str(m11c4_cnt) + '\n')
fw.writelines("m11c5 count: "+ str(m11c5_cnt) + '\n')
fw.writelines("m11c6 count: "+ str(m11c6_cnt) + '\n')
fw.writelines("m11c7 count: "+ str(m11c7_cnt) + '\n')
fw.writelines("m11c8 count: "+ str(m11c8_cnt) + '\n')
fw.writelines("m11c9 count: "+ str(m11c9_cnt) + '\n')
fw.writelines("m11c10 count: "+ str(m11c10_cnt) + '\n')
fw.writelines("m11c11 count: "+ str(m11c11_cnt) + '\n')
fw.writelines("m11c12 count: "+ str(m11c12_cnt) + '\n')
fw.writelines("m11c13 count: "+ str(m11c13_cnt) + '\n')
fw.writelines("m11c14 count: "+ str(m11c14_cnt) + '\n')
fw.writelines("m11c15 count: "+ str(m11c15_cnt) + '\n')
fw.writelines("m11c16 count: "+ str(m11c16_cnt) + '\n')
fw.writelines("m11c17 count: "+ str(m11c17_cnt) + '\n')
fw.writelines("m11c18 count: "+ str(m11c18_cnt) + '\n')
fw.writelines("m11c19 count: "+ str(m11c19_cnt) + '\n')
fw.writelines("m11c20 count: "+ str(m11c20_cnt) + '\n')
fw.writelines("m11c21 count: "+ str(m11c21_cnt) + '\n')
fw.writelines("m11c22 count: "+ str(m11c22_cnt) + '\n')
fw.writelines("m11c23 count: "+ str(m11c23_cnt) + '\n')





    
fr.close()
fw.close()

