#!/bin/bash

if [ "$1" != "" ]; then 
  LOCALHOST=$1
fi
if [ "$2" != "" ]; then 
  LOCALPORT=$2
fi
if [ "$3" != "" ]; then
  REMHOST=$3
fi
if [ "$4" != "" ]; then
  REMPORT=$4
fi

# Settable parameters. If empty or commented out they'll be asked for...

#LOCALHOST="localhost"
#LOCALPORT=1972
#REMHOST="pclip9.cern.ch"
#REMPORT=9090

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
if [ "$REMHOST" == "" ] ; then
  REMHOST=$HOSTNAME
fi
if [ "$REMPORT" == "" ]; then
  REMPORT=9090
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
if [ -e MonitorClient.xml ]; then
    rm MonitorClient.xml
fi

sed -e "s/.lport/${LOCALPORT}/g" -e "s/.lhost/${LOCALHOST}/g" -e "s/.pwd/${TEST_PATH}/g" .profile.xml > profile.xml
sed -e "s/.lport/${LOCALPORT}/g" -e "s/.lhost/${LOCALHOST}/g" -e "s/.rport/${REMPORT}/g" -e "s/.rhost/${REMHOST}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath1/${LIB1}/g"  -e "s/.libpath2/${LIB2}/g" .EBMonitorClientWithWebInterface.xml > EBMonitorClientWithWebInterface.xml 
sed -e "s/.lport/${LOCALPORT}/g" -e "s/.lhost/${LOCALHOST}/g" -e "s/.rport/${REMPORT}/g" -e "s/.rhost/${REMHOST}/g" -e "s/.pwd/${TEST_PATH}/g" -e "s/.libpath1/${LIB1}/g"  -e "s/.libpath2/${LIB2}/g" .MonitorClient.xml > MonitorClient.xml 
/bin/sed -e "s/.lport/$LOCALPORT/g" -e "s/.lhost/$LOCALHOST/g" .runEBMonitorClientWithWebInterface.sh > runEBMonitorClientWithWebInterface.sh
/bin/sed -e "s/.lport/$LOCALPORT/g" -e "s/.lhost/$LOCALHOST/g" .runMonitorClient.sh > runMonitorClient.sh

/bin/chmod 751 runEBMonitorClientWithWebInterface.sh
/bin/chmod 751 runMonitorClient.sh

