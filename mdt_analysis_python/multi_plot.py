import csv
from matplotlib import pyplot as plt
path = './newASD'
filename = path + '/newASD_result.csv'
with open(filename, 'r') as csvfile:
    reader = csv.reader(csvfile)
    data = [row for row in reader]
amp_in = [float(x[0]) for x in data]
amp_out_1 = [float(x[1]) for x in data]
amp_out_2 = [float(x[3]) for x in data]
amp_out_3 = [float(x[5]) for x in data]
res_1 = [float(x[2]) for x in data]
res_2 = [float(x[4]) for x in data]
res_3 = [float(x[6]) for x in data]
figname1 = path + '/TransCurve.png'
plt.plot(amp_in,amp_out_1,label='config_1')
plt.plot(amp_in,amp_out_2,label='config_2')
plt.plot(amp_in,amp_out_3,label='config_3')
plt.xlim(0,10)
plt.legend()
plt.xlabel('Input Pulse Amp (mV)')
plt.ylabel('Output Width (ns)')
plt.savefig(figname1)
plt.show()
figname2 = path + '/TimeRes.png'
plt.plot(amp_in,res_1,label='config_1')
plt.plot(amp_in,res_2,label='config_2')
plt.plot(amp_in,res_3,label='config_3')
plt.xlabel('Input Pulse Amp (mV)')
plt.ylabel('Output Width Res (ns)')
plt.xlim(0,10)
plt.legend()
plt.savefig(figname2)
plt.show()

