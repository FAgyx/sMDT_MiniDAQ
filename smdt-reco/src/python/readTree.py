import ROOT
import numpy as np
import os

f = ROOT.TFile('output/run188566/fitResiduals/Nominal/trackDiagnostics.root')
t = f.Get('trackFitTree')

nImport = 50000
maxNHit = 8

count = 0
dataArr   = np.zeros((nImport, 8, 3))
targetArr = np.zeros((nImport, 2))
for event in t:
    count += 1
    if count == nImport: 
        break

    targetArr[count][0] = event.angle_optimized
    targetArr[count][1] = event.impact_par_opt
    for i_h in range(event.hitX.size()):
        dataArr[count][i_h][0] = event.hitX[i_h]
        dataArr[count][i_h][1] = event.hitY[i_h]
        dataArr[count][i_h][2] = event.hitR[i_h]

print(count)
np.save(os.path.join('output', 'energyflow', 'dataArr.npy'), dataArr)
np.save(os.path.join('output', 'energyflow', 'targetArr.npy'), targetArr)
