# .bashrc

# Athena Setup
#export ATLAS_LOCAL_ROOT_BASE=/software/atlas/ATLASLocalRootBase
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
alias setupATLAS='source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh'
setupATLAS
#localSetupROOT
#localSetupROOT 6.02.05-x86_64-slc6-gcc48-opt
#localSetupROOT 5.34.25-x86_64-slc6-gcc48-opt

#export DISPLAY=ui05.lcg.ustc.edu.cn:12.0
#source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
#asetup 17.3.4.1
#alias asetup='source $AtlasSetup/scripts/asetup.sh'

# DQ2
#alias dq2-ls='shopts="$SHELLOPTS"; set -f; noglob dq2-ls'
#alias dq2-get='shopts="$SHELLOPTS"; set -f; noglob dq2-get'
#alias dq2-put='shopts="$SHELLOPTS"; set -f; noglob dq2-put'

# Root Setup
#

# RootCore Setup
#source $HOME/workarea/ROOTTutorial/RootCore/scripts/setup.sh
#echo 'Setting up RootCore ...'
#echo 'RootCore Directory is' $ROOTCOREDIR
