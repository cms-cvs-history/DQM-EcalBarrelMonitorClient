// $Id: writeMaskToDB.cpp,v 1.12 2007/02/08 08:36:08 benigno Exp $

/*!
  \file writeMaskFromDB.cpp
  \brief It reads errors masks from a file and updates database
  \author B. Gobbo 
  \version $Revision: 1.12 $
  \date $Date: 2007/02/08 08:36:08 $
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
     -t, --run-type=RUN TYPE      : run type \n\
     -i, --self-iov               : self-made IOV \n\
     -v, --verbose                : verbosity on \n\
     -V, --verify-syntax          : just verify file text syntax \n\n";
}

void printTag( const RunTag* tag) {
  std::cout << "------------ RunTag:" << std::endl;
  std::cout << "GeneralTag:         " << tag->getGeneralTag() << std::endl;
  std::cout << "Location:           " << tag->getLocationDef().getLocation() << std::endl;
  std::cout << "Run Type:           " << tag->getRunTypeDef().getRunType() << std::endl;
  std::cout << "--------------------" << std::endl;
}

void printIOV( const RunIOV* iov) {
  std::cout << std::endl;
  RunTag tag = iov->getRunTag();
  printTag(&tag);
  std::cout << "------------ RunIOV:" << std::endl;
  std::cout << "Run Number:         " << iov->getRunNumber() << std::endl;
  std::cout << "Run Start:          " << iov->getRunStart().str() << std::endl;
  std::cout << "Run End:            " << iov->getRunEnd().str() << std::endl;
  std::cout << "--------------------" << std::endl;
  std::cout << std::endl;
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
  bool smi             = false;
  bool verbose         = false;
  bool verifySyntax    = false;
  bool errors          = false;

  // ------------------

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
	{ "help",          0, 0, 'h' },     
	{ "sid",           1, 0, 's' },     
	{ "host-name",     1, 0, 'H' },
	{ "user-name",     1, 0, 'u' },
	{ "password",      1, 0, 'p' },
	{ "location",      1, 0, 'l' },
	{ "run-number",    1, 0, 'r' },
	{ "run-type",      1, 0, 't' },
	{ "self-iov",      0, 0, 'i' },
	{ "verbose",       0, 0, 'v' },
	{ "verify-syntax", 0, 0, 'V' },
	{ 0, 0, 0, 0 }
      };

      c = getopt_long( argc, argv, "hs:H:u:p:l:r:t:ivV", long_options, &option_index );
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
      case 'i':
	smi = true;
	break;
      case 'v':
	verbose = true;
	break;
      case 'V':
        verifySyntax = true;
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

  std::cout << std::endl;

  if( !verifySyntax ) {
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
    
    std::cout << std::endl;
    std::cout << "hostname  : " << hostName << std::endl
	      << "sid       : " << sid << std::endl
	      << "user      : " << user << std::endl
	      << "password  : " << passwd << std::endl
	      << "runNumber : " << runNb << std::endl
	      << "location  : " << location << std::endl
	      << "runType   : " << runType << std::endl;
  }

  std::cout << "file      : " << fileName << std::endl;

  // OK, from here there's all what's needed...

  try {
    if( verifySyntax ) {
      EcalErrorMask::readFile( fileName, verbose, true );
    }
    else {
      EcalErrorMask::readFile( fileName, verbose );
    }
  } catch( std::runtime_error &e ) {
    std::cerr << e.what() << std::endl;
    if( !verifySyntax ) {
      return -1;
    }
    else {
      errors = true;
    }
  }

  if( !verifySyntax ) {
    EcalCondDBInterface* eConn;
    try {
      eConn = new EcalCondDBInterface( hostName, sid, user, passwd );
    } catch( std::runtime_error &e ) {
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
    
    if( smi ) {
      
      Tm startTm;
      startTm.setToCurrentGMTime();
      startTm.setToMicrosTime( startTm.microsTime() );
      
      RunIOV new_runiov;
      new_runiov.setRunNumber( runNb );
      new_runiov.setRunStart( startTm );
      new_runiov.setRunTag( runtag );

      printIOV(&new_runiov);
      std::string yesno;
      std::cout << "Inserting self-made IOV. Are you sure? [y/N] ";
      std::cin >> yesno;
      if( yesno == "y" || yesno == "Y" || yesno == "yes" || yesno == "YES" ) {
        eConn->insertRunIOV(&new_runiov);
      }
    }
    
    RunIOV runiov = eConn->fetchRunIOV( &runtag, runNb );
    printIOV(&runiov);
    
    std::string yesno;
    std::cout << "Inserting masking table. Are you sure? [y/N] ";
    std::cin >> yesno;
    if( yesno == "y" || yesno == "Y" || yesno == "yes" || yesno == "YES" ) { 
      EcalErrorMask::writeDB( eConn, &runiov );
    }
  
    delete eConn;
  }

  return 0;
}


