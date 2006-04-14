#!/bin/bash

eval `scramv1 ru -sh`

HOSTNAME=$(echo `/bin/hostname` | sed 's/\//\\\//g')
echo "The hostname is = $HOSTNAME"

TEST_PATH=$(echo "${PWD}" | sed 's/\//\\\//g')
echo "The current directory is = $PWD"

MWC_LIB1="${LOCALRT}/lib/slc3_ia32_gcc323/libEBMonitorClientWebInterface.so"
echo "Looking for the MonitorWebClient library... $MWC_LIB1"
if [ ! -f $MWC_LIB1 ]; then
    echo "Not Found! Will pick it up from the release area..."
    MWC_LIB1="/afs/cern.ch/cms/Releases/CMSSW/prerelease/${CMSSW_VERSION}/lib/slc3_ia32_gcc323/libEBMonitorClientWebInterface.so"
else 
    echo "Found!"
fi
echo $MWC_LIB1
MWC_LIB1=$(echo "$MWC_LIB1" | sed 's/\//\\\//g')


MWC_LIB2="${LOCALRT}/lib/slc3_ia32_gcc323/libDQMServicesXdaqCollector.so"
echo "Looking for the DQMServicesXdaqCollector library... $MWC_LIB1"
if [ ! -f $MWC_LIB2 ]; then
    echo "Not Found! Will pick it up from the release area..."
    MWC_LIB2="${CMSSW_RELEASE_BASE}/lib/slc3_ia32_gcc323/libDQMServicesXdaqCollector.so"
else 
    echo "Found!"
fi
echo $MWC_LIB2
MWC_LIB2=$(echo "$MWC_LIB2" | sed 's/\//\\\//g')


if [ -e profile.xml ]; then
    rm profile.xml
fi 
if [ -e EBMonitorClientWithWebInterface.xml ]; then
    rm EBMonitorClientWithWebInterface.xml
fi
if [ -e startEBMonitorClient ]; then
    rm startEBMonitorClient
fi

sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" .profile.xml > profile.xml
sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath1/${MWC_LIB1}/g"  -e "s/.libpath2/${MWC_LIB2}/g" .EBMonitorClientWithWebInterface.xml > EBMonitorClientWithWebInterface.xml 
sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" .startEBMonitorClient > startEBMonitorClient

chmod 751 profile.xml
chmod 751 EBMonitorClientWithWebInterface.xml
chmod 751 startEBMonitorClient



