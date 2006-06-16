#!/bin/bash

eval `scramv1 ru -sh`

HOSTNAME=$(echo `/bin/hostname` | sed 's/\//\\\//g')
echo "The hostname is = $HOSTNAME"

TEST_PATH=$(echo "${PWD}" | sed 's/\//\\\//g')
echo "The current directory is = $PWD"

LIB1="${LOCALRT}/lib/slc3_ia32_gcc323/libEBMonitorClientWebInterface.so"
echo "Looking for the MonitorWebClient library... $LIB1"
if [ ! -f $LIB1 ]; then
    echo "Not Found! Will pick it up from the release area..."
    LIB1="/afs/cern.ch/cms/Releases/CMSSW/prerelease/${CMSSW_VERSION}/lib/slc3_ia32_gcc323/libEBMonitorClientWebInterface.so"
else 
    echo "Found!"
fi
echo $LIB1
LIB1=$(echo "$LIB1" | sed 's/\//\\\//g')

if [ -e profile.xml ]; then
    rm profile.xml
fi 
if [ -e EBMonitorClientWithWebInterface.xml ]; then
    rm EBMonitorClientWithWebInterface.xml
fi

sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" .profile.xml > profile.xml
sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath1/${LIB1}/g"  -e "s/.libpath2/${LIB2}/g" .EBMonitorClientWithWebInterface.xml > EBMonitorClientWithWebInterface.xml 

