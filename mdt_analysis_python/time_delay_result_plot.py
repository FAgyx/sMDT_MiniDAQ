import csv
from matplotlib import pyplot as plt
path = './newTDC/run2'
filename = path + '/time_delay_result.csv'
with open(filename, 'r') as csvfile:
    reader = csv.reader(csvfile)
    data = [row for row in reader]
time_in = [float(x[0]) for x in data]
time_out = [float(x[1]) for x in data]
figname1 = path + '/Time_delay_result.png'
plt.plot(time_in,time_out,linestyle='-',marker='.',mfc='r',mec='r')
plt.xlim(-5,45)
plt.xlabel('Input Pulse Delay (ns)')
plt.ylabel('Output Difference (ns)')
plt.savefig(figname1)
plt.show()