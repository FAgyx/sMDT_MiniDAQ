"""An example using Energy Flow Networks (EFNs), which were introduced in
[1810.05165](https://arxiv.org/abs/1810.05165), to classify quark and gluon
jets. The [`EFN`](../docs/archs/#efn) class is used to construct the network
architecture. The output of the example is a plot of the ROC curves obtained
by the EFN as well as the jet mass and constituent multiplicity observables.
"""

# standard library imports
from __future__ import absolute_import, division, print_function
import os, sys

# standard numerical library imports
import numpy as np

# energyflow imports
import energyflow as ef
from energyflow.archs import PFN
from energyflow.datasets import qg_jets
from energyflow.utils import data_split, to_categorical

from sklearn.metrics import roc_auc_score, roc_curve
import matplotlib
print(matplotlib.rcsetup.interactive_bk)
matplotlib.use('Agg')
import matplotlib.pyplot as plt
plt.subplots_adjust(wspace=0.3)
# ROOT imports
#from ROOT import TCanvas

################################### SETTINGS ##################################
# the commented values correspond to those in 1810.05165
###############################################################################

# data controls, can go up to 2000000 total for full dataset
train, val, test = 37500, 5000, 7500
# train, val, test = 1000000, 200000, 200000

# network architecture parameters
Phi_sizes, F_sizes = (100, 100, 128), (100, 100, 100)
# Phi_sizes, F_sizes = (100, 100, 256), (100, 100, 100)

# network training parameters
num_epoch = 5
batch_size = 500

loss_function = 'mean_squared_error'
if len(sys.argv) > 1:
    print(sys.argv)
    loss_function = sys.argv[1]
    print('Using loss function: ' + loss_function)

###############################################################################

# load data
X = np.copy(np.load(os.path.join('output', 'energyflow', 'dataArr.npy'), mmap_mode='r'))
Y = np.copy(np.load(os.path.join('output', 'energyflow', 'targetArr.npy'), mmap_mode='r'))

print('Loaded sMDT hit information')
print('Before preprocessing:')
print('Mean x:', np.mean(X[:1000,:,0]))
print('Mean y:', np.mean(X[:1000,:,1]))
print('Mean r:', np.mean(X[:1000,:,2]))
print('Mean theta:', np.mean(Y[:1000, 0]))
print('Mean impact: ', np.mean(Y[:1000,1]))
print('size of X: ', X.shape)
print('size of nonzero X: ', X[X!=0].shape)
print('number of zeros in hitx:', [len(a) for a in np.where(X==0)])

# preprocess by centering input distributions near 0 and map outputs to [0,1]

meanX = 276.5
meanY = 139.2
maxR  = 7.1
meanR = 3.5
for x in X:
    x[:,0] = (x[:,0] - meanX)/meanX
    x[:,1] = (x[:,1] - meanY)/meanX
    x[:,2] = (x[:,2] - meanR)/meanR

for y in Y:
    y[0] = y[0]/2 + 0.5
    y[1] = (y[1] - meanX)/meanX/2.0 + 0.5

print('Finished preprocessing')
print('After preprocessing:')
print('Mean x:', np.mean(X[:1000,:,0]))
print('Mean y:', np.mean(X[:1000,:,1]))
print('Mean r:', np.mean(X[:1000,:,2]))
print('Mean theta:', np.mean(Y[:1000, 0]))
print('Mean impact: ', np.mean(Y[:1000, 1]))

fig, axs = plt.subplots(2, 3)
axs[0, 0].hist(X[:,:,0].flatten(), bins=25)
axs[0, 1].hist(X[:,:,1].flatten(), bins=25)
axs[0, 2].hist(X[:,:,2].flatten(), bins=25)
axs[1, 0].hist(Y[:,0].flatten(), bins=25)
axs[1, 1].hist(Y[:,1].flatten(), bins=25)
fig.savefig('output/energyflow/postProcessed.png')
# do train/val/test split 
(X_train, X_val, X_test, 
 Y_train, Y_val, Y_test) = data_split(X, Y, val=val, test=test)

print('Done train/val/test split')
print('Model summary:')

# build architecture
# output dim is by default 2
pfn = PFN(loss=loss_function, input_dim=X.shape[-1], Phi_sizes=Phi_sizes, F_sizes=F_sizes)

# train model
pfn.fit(X_train, Y_train,
        epochs=num_epoch,
        batch_size=batch_size,
        validation_data=(X_val, Y_val),
        verbose=1)

# get predictions on test data
preds = pfn.predict(X_test, batch_size=1000)
thetaTest  = 2.0*(Y_test[:,0]-0.5)
thetaPred  = 2.0*(preds[:,0]-0.5)
deltaTheta = thetaPred - thetaTest
impactTest = (Y_test[:,1]-0.5)*2.0*meanX + meanX
impactPred = (preds[:,1]-0.5)*2.0*meanX + meanX
deltaImpact = impactTest - impactPred

textstr = '\n'.join((
    r'$\mu=%.2f$' % (np.mean(deltaTheta),),
    r'$\sigma=%.2f$' % (np.std(deltaTheta),)))

print(preds[:10])
print(Y_test[:10])
fig, axs = plt.subplots(2, 2)
axs[0,0].hist(deltaTheta, bins=100)
axs[0,0].set_xlabel(r"$\theta-\theta_0$ [radians]")
axs[0,0].set_ylabel('Number of events')
axs[0,0].set_title(loss_function)
axs[0,0].annotate(textstr, xy=(0.05, 0.85), xycoords='axes fraction')
axs[0,1].scatter(thetaTest, deltaTheta)
axs[0,1].set_xlabel(r"$\theta$ [radians]")
axs[0,1].set_ylabel(r"$\theta-\theta_0$ [radians]")
axs[1,0].hist(deltaImpact, bins=100)
axs[1,0].set_xlabel(r"$b-b_0$ [mm]")
axs[1,0].set_ylabel("Number of events")
axs[1,1].scatter(impactTest, deltaImpact)
axs[1,1].set_xlabel(r"$b_0$ [mm]")
axs[1,1].set_ylabel(r"$b-b_0$ [mm]")
fig.subplots_adjust(wspace=0.4,hspace=0.4)
fig.savefig(os.path.join('output', 'energyflow', loss_function + '.png'))
