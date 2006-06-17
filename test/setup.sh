#!/bin/bash

if [ "$1" != "" ]; then 
  LOCALHOST=$1
fi
if [ "$2" != "" ]; then 
  LOCALPORT=$2
fi

# Settable parameters. If empty or commented out they'll be asked for...

#LOCALHOST="pclip9.cern.ch"
#LOCALPORT=1972

# End of settable parameters

eval `scramv1 ru -sh`

HOSTNAME=$(echo `/bin/hostname` | sed 's/\//\\\//g')
echo "The local hostname is = $HOSTNAME"

TEST_PATH=$(echo "${PWD}" | sed 's/\//\\\//g')
echo "The current directory is = $PWD"

if [ "$LOCALHOST" == "" ] ; then
  LOCALHOST=$HOSTNAME
fi
if [ "$LOCALPORT" == "" ]; then
  LOCALPORT=1972
fi

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

sed -e "s/.lport/${LOCALPORT}/g" -e "s/.lhost/${LOCALHOST}/g" -e "s/.pwd/${TEST_PATH}/g" .profile.xml > profile.xml
sed -e "s/.lport/${LOCALPORT}/g" -e "s/.lhost/${LOCALHOST}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath1/${LIB1}/g"  -e "s/.libpath2/${LIB2}/g" .EBMonitorClientWithWebInterface.xml > EBMonitorClientWithWebInterface.xml 
/bin/sed -e "s/.lport/$LOCALPORT/g" -e "s/.lhost/$LOCALHOST/g" .runMonitorClient.sh > runMonitorClient.sh

/bin/chmod 751 runMonitorClient.sh

