#!/bin/bash

eval `scramv1 ru -sh`

HOSTNAME=$(echo `/bin/hostname` | sed 's/\//\\\//g')
echo "The hostname is = $HOSTNAME"

TEST_PATH=$(echo "${PWD}" | sed 's/\//\\\//g')
echo "The current directory is = $PWD"

MWC_LIB1="${LOCALRT}/lib/slc3_ia32_gcc323/libEBMonitorClientStateMachine.so"
echo "Looking for the EBMonitorClientStateMachine library... $MWC_LIB1"
if [ ! -f $MWC_LIB1 ]; then
    echo "Not Found! Will pick it up from the release area..."
    MWC_LIB1="${CMSSW_RELEASE_BASE}/lib/slc3_ia32_gcc323/libEBMonitorClientStateMachine.so"
else 
    echo "Found!"
fi

MWC_LIB=$(echo "$MWC_LIB1" | sed 's/\//\\\//g')
echo $MWC_LIB1

if [ -e profile.xml ]; then
    rm profile.xml
fi 
if [ -e EBMonitorClientStateMachine.xml ]; then
    rm EBMonitorClientStateMachine.xml
fi
if [ -e startEBMonitorClientStateMachine ]; then
    rm startEBMonitorClientStateMachine
fi

sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath/${MWC_LIB}/g" .profile.xml > profile.xml
sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath/${MWC_LIB}/g" .EBMonitorClientStateMachine.xml > EBMonitorClientStateMachine.xml 
sed -e "s/.portn/1972/g" -e "s/.host/${HOSTNAME}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath/${MWC_LIB}/g" .startEBMonitorClientStateMachine > startEBMonitorClientStateMachine

chmod 751 profile.xml
chmod 751 EBMonitorClientStateMachine.xml
chmod 751 startEBMonitorClientStateMachine



