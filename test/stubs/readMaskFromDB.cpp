// $Id: readMaskFromDB.cpp,v 1.4 2007/01/23 12:18:45 benigno Exp $

/*!
  \file readMaskFromDB.cpp
  \brief It reads errors masks from database and writes them to an output file
  \author B. Gobbo 
  \version $Revision: 1.4 $
  \date $Date: 2007/01/23 12:18:45 $
*/


#include <iostream>
#include <string>
#include <cstdlib>
#include <getopt.h>

#include "OnlineDB/EcalCondDB/interface/EcalCondDBInterface.h"
#include "OnlineDB/EcalCondDB/interface/RunIOV.h"
#include "DQM/EcalBarrelMonitorClient/interface/EcalErrorMask.h"

void usage( char* cp ) {
  std::cout <<
"\n\
usage: " << cp << " [OPTIONS] file\n\n\
     -h, --help                   : print this help message \n\
     -s, --sid=SID                : data base sid \n\
     -H, --host-name=HOST NAME    : data base server host name \n\
     -u, --user-name=DB USER      : data base user name \n\
     -p, --password=DB PASSWORD   : data base password \n\
     -l, --location=LOCATION      : location H4, 867-1, ...\n\
     -r, --run-number=RUN NUMBER  : run number \n\
     -t, --run-type=RUN TYPE      : run type \n\n";
}

int main( int argc, char **argv ) {

  char* cp;

  // --------------------------------------------------------------------------
  // If you like, you can set variables to some default here

  std::string user     = "";
  std::string passwd   = "";
  int runNb            = 0;
  std::string fileName = "";
  std::string hostName = "";
  std::string sid      = "";
  std::string location = "";
  std::string runType  = "";

  if(( cp = (char*) strrchr( argv[0], '/' )) != NULL ) {
    ++cp;
  }
  else {
    cp = argv[0];
  }
  
  // Arguments and Options
  if( argc > 1 ) {
    int c;
    while(1) {
      int option_index;
      static struct option long_options[] = {
	{ "help", 0, 0, 'h' },     
	{ "sid", 1, 0, 's' },     
	{ "host-name", 1, 0, 'H' },
	{ "user-name", 1, 0, 'u' },
	{ "password", 1, 0, 'p' },
	{ "location", 1, 0, 'l' },
	{ "run-number", 1, 0, 'r' },
	{ "run-type", 1, 0, 't' },
	{ 0, 0, 0, 0 }
      };

      c = getopt_long( argc, argv, "hs:H:u:p:l:r:t:", long_options, &option_index );
      if( c == -1 ) break;

      switch( c ) {
      case 'h':
	usage(cp);
	return(0);
	break;
      case 's':
	sid = optarg;
	break;
      case 'H':
	hostName = optarg;
	break;
      case 'u':
	user = optarg;
	break;
      case 'p':
	passwd = optarg;
	break;
      case 'l':
	location = optarg;
	break;
      case 'r':
	runNb = atoi(optarg);
	break;
      case 't':
	runType = optarg;
	break;
      default:
	break;
      }
    }
  }	
  if( optind+1 == argc ) {
    fileName = argv[optind++];
  }
  else {
    usage(cp);
    return -1;
  }

  if( hostName == "" ) {
    std::cout << "hostname: ";
    std::cin >> hostName;
  }
  if( sid == "" ) {
    std::cout << "sid     : ";
    std::cin >> sid;
  }
  if( user == "" ) {
    std::cout << "username: ";
    std::cin >> user;
  }
  if( passwd == "" ) {
    std::cout << "password: ";
    std::cin >> passwd;
  }
  if( runNb == 0 ) {
    std::cout << "run Nb  : ";
    std::cin >> runNb;
  }
  if( location == "" ) {
    std::cout << "location: ";
    std::cin >> location;
  }
  if( runType == "" ) {
    std::cout << "run type: ";
    std::cin >> runType;
  }

  std::cout << "hostname  : " << hostName << std::endl
	    << "sid       : " << sid << std::endl
	    << "user      : " << user << std::endl
	    << "password  : " << passwd << std::endl
	    << "runNumber : " << runNb << std::endl
	    << "location  : " << location << std::endl
	    << "runType   : " << runType << std::endl
	    << "file      : " << fileName << std::endl;
  
  // OK, from here there's all what's needed...

  EcalCondDBInterface* eConn;
  try {
     eConn = new EcalCondDBInterface( hostName, sid, user, passwd );
  } catch( runtime_error &e ) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  
  LocationDef locdef;
  RunTypeDef rundef;
  RunTag     runtag;

  locdef.setLocation( location );

  rundef.setRunType( runType );

  runtag.setLocationDef( locdef );
  runtag.setRunTypeDef( rundef );

  runtag.setGeneralTag( runType );

  RunIOV runiov;
  runiov.setRunTag( runtag );
  runiov.setRunNumber( runNb );
  EcalErrorMask::readDB( eConn, &runiov );
  EcalErrorMask::writeFile( fileName );

  delete eConn;

  return 0;
}


