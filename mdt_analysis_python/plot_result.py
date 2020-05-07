import csv
from matplotlib import pyplot as plt
path = './newTDC/ch2_curr'
filename = path + '/width_result.csv'
with open(filename, 'r') as csvfile:
    reader = csv.reader(csvfile)
    data = [row for row in reader]
amp_in = [float(x[0]) for x in data]
amp_out = [float(x[1]) for x in data]
res = [float(x[2]) for x in data]
figname1 = path + '/TransCurve.png'
plt.plot(amp_in,amp_out,linestyle='-',marker='.', markersize=10, mfc='r',mec='r')
plt.xlim(0,8)
plt.xlabel('Rundown Current Code')
plt.ylabel('Output Width (ns)')
plt.savefig(figname1)
plt.show()
figname2 = path + '/TimeRes.png'
plt.plot(amp_in,res,linestyle='-',marker='.', markersize=10, mfc='r',mec='r')
plt.xlabel('Rundown Current Code')
plt.ylabel('Output Width Res (ns)')
plt.xlim(0,8)
plt.savefig(figname2)
plt.show()

