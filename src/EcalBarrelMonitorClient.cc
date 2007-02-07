/*
 * \file EcalBarrelMonitorClient.cc
 *
 * $Date: 2007/02/06 18:19:50 $
 * $Revision: 1.211 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#include "TStyle.h"
#include "TGaxis.h"
#include "TColor.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DQMServices/Core/interface/DaqMonitorBEInterface.h"
#include "DQMServices/Daemon/interface/MonitorDaemon.h"

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "DataFormats/EcalRawData/interface/EcalRawDataCollections.h"

#include "OnlineDB/EcalCondDB/interface/EcalCondDBInterface.h"
#include "OnlineDB/EcalCondDB/interface/RunTag.h"
#include "OnlineDB/EcalCondDB/interface/RunDat.h"
#include "OnlineDB/EcalCondDB/interface/MonRunDat.h"

#include <DQM/EcalBarrelMonitorClient/interface/EcalBarrelMonitorClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBMUtilsClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EcalErrorMask.h>

using namespace cms;
using namespace edm;
using namespace std;

EcalBarrelMonitorClient::EcalBarrelMonitorClient(const ParameterSet& ps, MonitorUserInterface* mui){

  enableStateMachine_ = true;

  mui_ = mui;

  this->initialize(ps);

}

EcalBarrelMonitorClient::EcalBarrelMonitorClient(const ParameterSet& ps){

  enableStateMachine_ = false;

  mui_ = 0;

  this->initialize(ps);

}

void EcalBarrelMonitorClient::initialize(const ParameterSet& ps){

  cout << endl;
  cout << " *** Ecal Barrel Generic Monitor Client ***" << endl;
  cout << endl;

  // Set runTypes

  runTypes_.resize( 13 );
  for ( unsigned int i=0; i<runTypes_.size(); ++i ) runTypes_[i] =  "UNKNOWN"; 
  runTypes_[EcalDCCHeaderBlock::COSMIC]                 = "COSMIC";
  runTypes_[EcalDCCHeaderBlock::BEAMH4]                 = "BEAM";
  runTypes_[EcalDCCHeaderBlock::BEAMH2]                 = "BEAM";
  runTypes_[EcalDCCHeaderBlock::MTCC]                   = "PHYSICS";
  runTypes_[EcalDCCHeaderBlock::LASER_STD]              = "LASER";
  runTypes_[EcalDCCHeaderBlock::TESTPULSE_MGPA]         = "TEST_PULSE";
  runTypes_[EcalDCCHeaderBlock::PEDESTAL_STD]           = "PEDESTAL";
  runTypes_[EcalDCCHeaderBlock::PEDESTAL_OFFSET_SCAN]   = "PEDESTAL-OFFSET";

  clients_.clear();
  clientNames_.clear();

  begin_run_ = false;
  end_run_   = false;

  forced_status_ = false;

  forced_update_ = false;

  h_ = 0;

  status_  = "unknown";
  run_     = -1;
  evt_     = -1;
  runtype_ = -1;

  subrun_  = -1;

  last_jevt_   = -1;
  last_update_ = 0;

  unknowns_ = 0;

  // DQM ROOT input

  inputFile_ = ps.getUntrackedParameter<string>("inputFile", "");

  if ( inputFile_.size() != 0 ) {
    cout << " Reading DQM from inputFile = '" << inputFile_ << "'" << endl;
  }

  // DQM ROOT output

  outputFile_ = ps.getUntrackedParameter<string>("outputFile", "");

  if ( outputFile_.size() != 0 ) {
    cout << " Writing DQM to outputFile = '" << outputFile_ << "'" << endl;
  }

  // Ecal Cond DB

  dbName_ = ps.getUntrackedParameter<string>("dbName", "");
  dbHostName_ = ps.getUntrackedParameter<string>("dbHostName", "");
  dbHostPort_ = ps.getUntrackedParameter<int>("dbHostPort", 1521);
  dbUserName_ = ps.getUntrackedParameter<string>("dbUserName", "");
  dbPassword_ = ps.getUntrackedParameter<string>("dbPassword", "");

  if ( dbName_.size() != 0 ) {
    cout << " Using Ecal Cond DB, "
         << " dbName = '" << dbName_ << "'"
         << " dbHostName = '" << dbHostName_ << "'"
         << " dbHostPort = '" << dbHostPort_ << "'"
         << " dbUserName = '" << dbUserName_ << "'" << endl;
  } else {
    cout << " Ecal Cond DB is not enabled" << endl;
  }

  // Mask file

  maskFile_ = ps.getUntrackedParameter<string>("maskFile", "");

  if ( maskFile_.size() != 0 ) {
    cout << " Using maskFile = '" << maskFile_ << "'" << endl;
  }

  // enableSubRun switch

  enableSubRun_ = ps.getUntrackedParameter<bool>("enableSubRun", false);

  // location

  location_ =  ps.getUntrackedParameter<string>("location", "H4");

  // base Html output directory

  baseHtmlDir_ = ps.getUntrackedParameter<string>("baseHtmlDir", "");

  if ( baseHtmlDir_.size() != 0 ) {
    cout << " HTML output will go to"
         << " baseHtmlDir = '" << baseHtmlDir_ << "'" << endl;
  } else {
    cout << " HTML output is not enabled" << endl;
  }

  // collateSources switch

  collateSources_ = ps.getUntrackedParameter<bool>("collateSources", false);

  if ( collateSources_ ) {
    cout << " collateSources switch is ON" << endl;
  } else {
    cout << " collateSources switch is OFF" << endl;
  }

  // cloneME switch

  cloneME_ = ps.getUntrackedParameter<bool>("cloneME", true);

  if ( cloneME_ ) {
    cout << " cloneME switch is ON" << endl;
  } else {
    cout << " cloneME switch is OFF" << endl;
  }

  // enableQT switch

  enableQT_ = ps.getUntrackedParameter<bool>("enableQT", true);

  if ( enableQT_ ) {
    cout << " enableQT switch is ON" << endl;
  } else {
    cout << " enableQT switch is OFF" << endl;
  }

  // enableTCC switch

  enableTCC_ = ps.getUntrackedParameter<bool>("enableTCC", false);

  if ( enableTCC_ ) {
    cout << " enableTCC switch is ON" << endl;
  } else {
    cout << " enableTCC switch is OFF" << endl;
  }

  // enableExit switch

  enableExit_ = ps.getUntrackedParameter<bool>("enableExit", true);

  if ( enableExit_ ) {
    cout << " enableExit switch is ON" << endl;
  } else {
    cout << " enableExit switch is OFF" << endl;
  } 

  // verbosity switch

  verbose_ = ps.getUntrackedParameter<bool>("verbose", false);

  if ( verbose_ ) {
    cout << " verbose switch is ON" << endl;
  } else {
    cout << " verbose switch is OFF" << endl;
  }

  // MonitorDaemon switch

  enableMonitorDaemon_ = ps.getUntrackedParameter<bool>("enableMonitorDaemon", true);

  if ( enableMonitorDaemon_ ) {
    cout << " enableMonitorDaemon switch is ON" << endl;
  } else {
    cout << " enableMonitorDaemon switch is OFF" << endl;
  }

  // prefix to ME paths

  prefixME_ = ps.getUntrackedParameter<string>("prefixME", "");

  // DQM Client name

  clientName_ = ps.getUntrackedParameter<string>("clientName", "EcalBarrelMonitorClient");

  if ( ! enableStateMachine_ ) {
    if ( enableMonitorDaemon_ ) {

      // DQM Collector hostname

      hostName_ = ps.getUntrackedParameter<string>("hostName", "localhost");

      // DQM Collector port

      hostPort_ = ps.getUntrackedParameter<int>("hostPort", 9090);

      cout << " Client '" << clientName_ << "' " << endl
           << " Collector on host '" << hostName_ << "'"
           << " on port '" << hostPort_ << "'" << endl;

    }
  }


  // Server switch

  enableServer_ = ps.getUntrackedParameter<bool>("enableServer", false);
  serverPort_   = ps.getUntrackedParameter<int>("serverPort", 9900);

  if ( enableServer_ ) {
    if ( enableMonitorDaemon_ && hostPort_ != serverPort_ ) {
      cout << " Forcing the same port for Collector and Server" << endl;
      serverPort_ = hostPort_;
    }
    cout << " Server on port '" << serverPort_ << "' is enabled" << endl;
  } else {
    cout << " Server is not enabled" << endl;
  }

  // vector of selected Super Modules (Defaults to all 36).

  superModules_.reserve(36);
  for ( unsigned int i = 1; i < 37; i++ ) superModules_.push_back(i);

  superModules_ = ps.getUntrackedParameter<vector<int> >("superModules", superModules_); 

  cout << " Selected SMs:" << endl;

  for ( unsigned int i = 0; i < superModules_.size(); i++ ) {
    cout << " " << setw(2) << setfill('0') << superModules_[i];
  }

  cout << endl;

  // global ROOT style

  gStyle->Reset("Default");

  gStyle->SetCanvasColor(10);
  gStyle->SetPadColor(10);
  gStyle->SetFillColor(10);
  gStyle->SetStatColor(10);
  gStyle->SetTitleColor(10);
  gStyle->SetTitleFillColor(10);

  TGaxis::SetMaxDigits(4);

  gStyle->SetOptTitle(kTRUE);
  gStyle->SetTitleX(0.00);
  gStyle->SetTitleY(1.00);
  gStyle->SetTitleW(0.00);
  gStyle->SetTitleH(0.05);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(43, "c");
  gStyle->SetTitleFontSize(11);

  gStyle->SetOptStat(kFALSE);
  gStyle->SetStatX(0.99);
  gStyle->SetStatY(0.99);
  gStyle->SetStatW(0.25);
  gStyle->SetStatH(0.20);
  gStyle->SetStatBorderSize(1);
  gStyle->SetStatFont(43);
  gStyle->SetStatFontSize(10);

  gStyle->SetOptFit(kFALSE);

  gROOT->ForceStyle();

  // Define new color palette

  float rgb[6][3] = {{1.00, 0.00, 0.00}, {0.00, 1.00, 0.00}, {1.00, 0.96, 0.00},
                     {0.50, 0.00, 0.20}, {0.00, 0.50, 0.40}, {0.94, 0.78, 0.00}};

  for( int i=0; i<6; i++ ) {
    TColor* color;
    if( ! gROOT->GetColor( 301+i )) {
      color = new TColor( 301+i, rgb[i][0], rgb[i][1], rgb[i][2], "" );
    }
    else {
      color = gROOT->GetColor( 301+i );
      color->SetRGB( rgb[i][0], rgb[i][1], rgb[i][2] );
    }
  }  

  // clients' constructors

  clients_.reserve(8);
  clientNames_.reserve(8);

  clients_.push_back( new EBIntegrityClient(ps) );
  clientNames_.push_back( "Integrity" );
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_OFFSET_SCAN ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::MTCC ));

  clients_.push_back( new EBCosmicClient(ps) );
  clientNames_.push_back( "Cosmic" );
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::MTCC ));

  clients_.push_back(  new EBLaserClient(ps) );
  clientNames_.push_back( "Laser" );
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));

  clients_.push_back(  new EBPedestalClient(ps) );
  clientNames_.push_back( "Pedestal" );
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));

  clients_.push_back(  new EBPedestalOnlineClient(ps) );
  clientNames_.push_back( "PedestalOnLine" );
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::MTCC ));

  clients_.push_back(  new EBTestPulseClient(ps) );
  clientNames_.push_back( "TestPulse" );
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));

  clients_.push_back(  new EBBeamCaloClient(ps) );
  clientNames_.push_back( "BeamCalo" );
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));

  clients_.push_back(  new EBBeamHodoClient(ps) );
  clientNames_.push_back( "BeamHodo" );
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));

  if ( enableTCC_ ) {
    clients_.push_back( new EBTriggerTowerClient(ps) );
    clientNames_.push_back( "TriggerTower" );
    chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
  }

#if 0
  clients_.push_back(  new EBClusterClient(ps) );
  clientNames_.push_back( "Cluster" );
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
  chb_.insert( EBCIMMap::value_type( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
#endif

  cout << endl;

}

EcalBarrelMonitorClient::~EcalBarrelMonitorClient(){

  cout << "Exit ..." << endl;

  for ( unsigned int i=0; i<clients_.size(); i++ ) {
    delete clients_[i];
  }

  if ( ! enableStateMachine_ ) {
    mui_->disconnect();
    delete mui_;
  }

}

void EcalBarrelMonitorClient::beginJob(void){

  if ( verbose_ ) cout << "EcalBarrelMonitorClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

  // start DQM user interface instance
  // will attempt to reconnect upon connection problems (w/ a 5-sec delay)

  if ( ! enableStateMachine_ ) {
    if ( enableMonitorDaemon_ ) {
      if ( enableServer_ ) {
        mui_ = new MonitorUIRoot(hostName_, hostPort_, clientName_, 5, true);
      } else {
        mui_ = new MonitorUIRoot(hostName_, hostPort_, clientName_, 5, false);
      }
    } else {
      mui_ = new MonitorUIRoot();
      if ( enableServer_ ) {
        mui_->actAsServer(serverPort_, clientName_);
      }
    }
  }

  if ( verbose_ ) {
    mui_->setVerbose(1);
  } else {
    mui_->setVerbose(0);
  }

  if ( ! enableStateMachine_ ) {
    if ( ! enableMonitorDaemon_ ) {
      if ( inputFile_.size() != 0 ) {
        DaqMonitorBEInterface* dbe = mui_->getBEInterface();
        dbe->open(inputFile_);
      }
    }
  }

  mui_->setMaxAttempts2Reconnect(99999);

  for ( unsigned int i=0; i<clients_.size(); i++ ) {
    clients_[i]->beginJob(mui_);
  }

  this->subscribe();

}

void EcalBarrelMonitorClient::beginRun(void){

  begin_run_ = true;
  end_run_   = false;

  if ( verbose_ ) cout << "EcalBarrelMonitorClient: beginRun" << endl;

  jevt_ = 0;

  this->setup();

  this->beginRunDb();

  for ( int i=0; i<int(clients_.size()); i++ ) {
    clients_[i]->cleanup();
    bool started; started = false;
    for ( EBCIMMap::iterator j = chb_.lower_bound(clients_[i]); j != chb_.upper_bound(clients_[i]); ++j ) {
      if ( runtype_ != -1 && runtype_ == (*j).second && !started ) { started = true; clients_[i]->beginRun(); }
    }
  }
  
}

void EcalBarrelMonitorClient::endJob(void) {

  // check last event

  if ( ! end_run_ ) {

    cout << endl;
    cout << "Checking last event at endJob() ... " << endl;
    cout << endl;

    forced_update_ = true;
    this->analyze();

    if ( ! end_run_ ) {

      cout << endl;
      cout << "Forcing endRun() ... " << endl;
      cout << endl;

      forced_status_ = true;
      this->endRun();

    }

  }

  if ( verbose_ ) cout << "EcalBarrelMonitorClient: endJob, ievt = " << ievt_ << endl;

  this->unsubscribe();

  this->cleanup();

  for ( unsigned int i=0; i<clients_.size(); i++ ) {
    clients_[i]->endJob();
  }

}

void EcalBarrelMonitorClient::endRun(void) {

  begin_run_ = false;
  end_run_   = true;

  if ( verbose_ ) cout << "EcalBarrelMonitorClient: endRun, jevt = " << jevt_ << endl;

  if ( outputFile_.size() != 0 ) mui_->save(outputFile_);

  if ( subrun_ != -1 ) {

    this->writeDb();
    this->endRunDb();

  }

  if ( baseHtmlDir_.size() != 0 ) this->htmlOutput();

  if ( subrun_ != -1 ) this->softReset();

  for ( int i=0; i<int(clients_.size()); i++ ) {
    bool ended; ended = false;
    for ( EBCIMMap::iterator j = chb_.lower_bound(clients_[i]); j != chb_.upper_bound(clients_[i]); ++j ) {
      if ( runtype_ != -1 && runtype_ == (*j).second && !ended ) { ended = true; clients_[i]->endRun(); }
    }
  }
  
  this->cleanup();

  status_  = "unknown";
  run_     = -1;
  evt_     = -1;
  runtype_ = -1;

  subrun_ = -1;

  last_jevt_   = -1;
  last_update_ = 0;

  if ( ! enableStateMachine_ ) {
    if ( enableMonitorDaemon_ ) {

      // in this way we avoid ROOT memory leaks ...

      if ( enableExit_ ) {

        cout << endl;
        cout << ">>> endJob() after endRun() <<<" << endl;
        cout << endl;
        this->endJob();
        throw exception();

      }

    }
  }

}

void EcalBarrelMonitorClient::setup(void) {

}

void EcalBarrelMonitorClient::cleanup(void) {

  if ( cloneME_ ) {
    if ( h_ ) delete h_;
  }

  h_ = 0;

}

void EcalBarrelMonitorClient::beginRunDb(void) {

  subrun_ = 0;

  current_time_ = time(NULL);
  last_time_ = current_time_;

  EcalCondDBInterface* econn;

  econn = 0;

  if ( dbName_.size() != 0 ) {
    try {
      cout << "Opening DB connection ..." << endl;
      econn = new EcalCondDBInterface(dbHostName_, dbName_, dbUserName_, dbPassword_, dbHostPort_);
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  // create the objects necessary to identify a dataset

  LocationDef locdef;

  locdef.setLocation(location_);

  RunTypeDef rundef;

  rundef.setRunType( runtype_ == -1 ? "UNKNOWN" : runTypes_[runtype_]  );
 
  RunTag runtag;

  runtag.setLocationDef(locdef);
  runtag.setRunTypeDef(rundef);

  runtag.setGeneralTag( runtype_ == -1 ? "UNKNOWN" : runTypes_[runtype_] );

  // fetch the RunIOV from the DB

  bool foundRunIOV = false;

  if ( econn ) {
    try {
      cout << "Fetching RunIOV ... " << flush;
      runiov_ = econn->fetchRunIOV(&runtag, run_);
//      runiov_ = econn->fetchRunIOV(location_, run_);
      cout << "done." << endl;
      foundRunIOV = true;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
      foundRunIOV = false;
    }
  }

  // begin - setup the RunIOV (on behalf of the DAQ)

  if ( ! foundRunIOV ) {

    Tm startRun;

    startRun.setToCurrentGMTime();

    runiov_.setRunNumber(run_);
    runiov_.setRunStart(startRun);
    runiov_.setRunTag(runtag);

    if ( econn ) {
      try {
        cout << "Inserting RunIOV ... " << flush;
        econn->insertRunIOV(&runiov_);
        cout << "done." << endl;
      } catch (runtime_error &e) {
        cerr << e.what() << endl;
        try {
          cout << "Fetching RunIOV (again) ... " << flush;
          runiov_ = econn->fetchRunIOV(&runtag, run_);
//          runiov_ = econn->fetchRunIOV(location_, run_);
          cout << "done." << endl;
          foundRunIOV = true;
        } catch (runtime_error &e) {
          cerr << e.what() << endl;
          foundRunIOV = false;
        }
      }
    }

  }

  // end - setup the RunIOV (on behalf of the DAQ)

  string st = runiov_.getRunTag().getRunTypeDef().getRunType();
  if ( st == "UNKNOWN" ) runtype_ = -1; 
  else for ( unsigned int i=0; i<runTypes_.size(); i++ ) if ( st == runTypes_[i] ) runtype_ = i;

  cout << endl;
  cout << "=============RunIOV:" << endl;
  cout << "Run Number:         " << runiov_.getRunNumber() << endl;
  cout << "Run Start:          " << runiov_.getRunStart().str() << endl;
  cout << "Run End:            " << runiov_.getRunEnd().str() << endl;
  cout << "====================" << endl;
  cout << endl;
  cout << "=============RunTag:" << endl;
  cout << "GeneralTag:         " << runiov_.getRunTag().getGeneralTag() << endl;
  cout << "Location:           " << runiov_.getRunTag().getLocationDef().getLocation() << endl;
  cout << "Run Type:           " << runiov_.getRunTag().getRunTypeDef().getRunType() << endl;
  cout << "====================" << endl;
  cout << endl;

  if ( maskFile_.size() != 0 ) {
    try {
      cout << "Fetching masked channels from file ... " << flush;
      EcalErrorMask::readFile(maskFile_, verbose_);
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  } else {
    if ( econn ) {
      try {
	cout << "Fetching masked channels from DB ... " << flush;
	EcalErrorMask::readDB(econn, &runiov_);
	cout << "done." << endl;
      } catch (runtime_error &e) {
	cerr << e.what() << endl;
      }
    }
  }

  cout << endl;

  if ( econn ) {
    try {
      cout << "Closing DB connection ..." << endl;
      delete econn;
      econn = 0;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  cout << endl;

}

void EcalBarrelMonitorClient::writeDb(void) {

  subrun_++;

  last_time_ = current_time_;

  EcalCondDBInterface* econn;

  econn = 0;

  if ( dbName_.size() != 0 ) {
    try {
      cout << "Opening DB connection ..." << endl;
      econn = new EcalCondDBInterface(dbHostName_, dbName_, dbUserName_, dbPassword_, dbHostPort_);
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  MonVersionDef monverdef;

  monverdef.setMonitoringVersion("test01");

  MonRunTag montag;

  montag.setMonVersionDef(monverdef);
  montag.setGeneralTag("CMSSW");

  Tm startSubRun;

  startSubRun.setToCurrentGMTime();

  // setup the MonIOV

  moniov_.setRunIOV(runiov_);
  moniov_.setSubRunNumber(subrun_);

  if ( enableMonitorDaemon_ ) {
    moniov_.setSubRunStart(startSubRun);
  } else {
    moniov_.setSubRunStart(runiov_.getRunStart());
  }

  moniov_.setMonRunTag(montag);

  cout << endl;
  cout << "==========MonRunIOV:" << endl;
  cout << "SubRun Number:      " << moniov_.getSubRunNumber() << endl;
  cout << "SubRun Start:       " << moniov_.getSubRunStart().str() << endl;
  cout << "SubRun End:         " << moniov_.getSubRunEnd().str() << endl;
  cout << "====================" << endl;
  cout << endl;
  cout << "==========MonRunTag:" << endl;
  cout << "GeneralTag:         " << moniov_.getMonRunTag().getGeneralTag() << endl;
  cout << "Monitoring Ver:     " << moniov_.getMonRunTag().getMonVersionDef().getMonitoringVersion() << endl;
  cout << "====================" << endl;
  cout << endl;

  int taskl = 0x0;
  int tasko = 0x0;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    for ( int j = 0; j<int(clients_.size()); ++j ) {
      bool written; written = false;
      for ( EBCIMMap::iterator k = chb_.lower_bound(clients_[j]); k != chb_.upper_bound(clients_[j]); ++k ) {
        if ( h_ && h_->GetBinContent((*k).second+1) != 0 && runtype_ != -1 && runtype_ == (*k).second && !written ) { 
          if ( clientNames_[j] == "Laser" && h_->GetBinContent(EcalDCCHeaderBlock::LASER_STD+1) == 0 ) continue;
          written = true; 
          taskl |= 0x1 << j;
          if ( clients_[j]->writeDb(econn, &runiov_, &moniov_, ism) ) {
            tasko |= 0x1 << j;
          } else {
            tasko |= 0x0 << j;
          }
        }
      }
      if ( ((taskl >> j) & 0x1) ) {
        cout << " Task output for " << clientNames_[j] << " = " << ((tasko >> j) & 0x1) << endl;
      }
    }

    EcalLogicID ecid;
    MonRunDat md;
    map<EcalLogicID, MonRunDat> dataset;

    MonRunOutcomeDef monRunOutcomeDef;

    monRunOutcomeDef.setShortDesc("success");

    float nevt = -1.;

    if ( h_ ) nevt = h_->GetEntries();

    md.setNumEvents(int(nevt));
    md.setMonRunOutcomeDef(monRunOutcomeDef);
    md.setRootfileName(outputFile_);
    md.setTaskList(taskl);
    md.setTaskOutcome(tasko);

    if ( econn ) {
      try {
        ecid = econn->getEcalLogicID("ECAL");
        dataset[ecid] = md;
      } catch (runtime_error &e) {
        cerr << e.what() << endl;
      }
    }

    if ( econn ) {
      try {
        cout << "Inserting MonRunDat ... " << flush;
        econn->insertDataSet(&dataset, &moniov_);
        cout << "done." << endl;
      } catch (runtime_error &e) {
        cerr << e.what() << endl;
      }
    }

  }

  if ( econn ) {
    try {
      cout << "Closing DB connection ..." << endl;
      delete econn;
      econn = 0;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  cout << endl;

}

void EcalBarrelMonitorClient::endRunDb(void) {

  EcalCondDBInterface* econn;

  econn = 0;

  if ( dbName_.size() != 0 ) {
    try {
      cout << "Opening DB connection ..." << endl;
      econn = new EcalCondDBInterface(dbHostName_, dbName_, dbUserName_, dbPassword_, dbHostPort_);
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  EcalLogicID ecid;
  RunDat rd;
  map<EcalLogicID, RunDat> dataset;

  float nevt = -1.;

  if ( h_ ) nevt = h_->GetEntries();

  rd.setNumEvents(int(nevt));

  // fetch the RunDat from the DB

  bool foundRunDat = false;

  if ( econn ) {
    try {
      cout << "Fetching RunDat ... " << flush;
      econn->fetchDataSet(&dataset, &runiov_);
      cout << "done." << endl;
      foundRunDat = true;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
      foundRunDat = false;
    }
  }

  // begin - setup the RunDat (on behalf of the DAQ)

  if ( ! foundRunDat ) {

    if ( econn ) {
      try {
        ecid = econn->getEcalLogicID("ECAL");
        dataset[ecid] = rd;
      } catch (runtime_error &e) {
        cerr << e.what() << endl;
      }
    }

    if ( econn ) {
      try {
        cout << "Inserting RunDat ... " << flush;
        econn->insertDataSet(&dataset, &runiov_);
        cout << "done." << endl;
      } catch (runtime_error &e) {
        cerr << e.what() << endl;
      }
    }

  }

  // end - setup the RunDat (on behalf of the DAQ)

  if ( econn ) {
    try {
      cout << "Closing DB connection ..." << endl;
      delete econn;
      econn = 0;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

}

void EcalBarrelMonitorClient::subscribe(void){

  if ( verbose_ ) cout << "EcalBarrelMonitorClient: subscribe" << endl;

  mui_->subscribe("*/EcalBarrel/EcalInfo/STATUS");
  mui_->subscribe("*/EcalBarrel/EcalInfo/RUN");
  mui_->subscribe("*/EcalBarrel/EcalInfo/EVT");
  mui_->subscribe("*/EcalBarrel/EcalInfo/EVTTYPE");
  mui_->subscribe("*/EcalBarrel/EcalInfo/RUNTYPE");

  if ( collateSources_ ) {

    if ( verbose_ ) cout << "EcalBarrelMonitorClient: collate" << endl;

    Char_t histo[200];

    sprintf(histo, "EVTTYPE");
    me_h_ = mui_->collate1D(histo, histo, "EcalBarrel/Sums/EcalInfo");
    sprintf(histo, "*/EcalBarrel/EcalInfo/EVTTYPE");
    mui_->add(me_h_, histo);

  }

}

void EcalBarrelMonitorClient::subscribeNew(void){

  mui_->subscribeNew("*/EcalBarrel/EcalInfo/STATUS");
  mui_->subscribeNew("*/EcalBarrel/EcalInfo/RUN");
  mui_->subscribeNew("*/EcalBarrel/EcalInfo/EVT");
  mui_->subscribeNew("*/EcalBarrel/EcalInfo/EVTTYPE");
  mui_->subscribeNew("*/EcalBarrel/EcalInfo/RUNTYPE");

}

void EcalBarrelMonitorClient::unsubscribe(void) {

  if ( verbose_ ) cout << "EcalBarrelMonitorClient: unsubscribe" << endl;

  if ( collateSources_ ) {

    if ( verbose_ ) cout << "EcalBarrelMonitorClient: uncollate" << endl;

    if ( mui_ ) {

      mui_->removeCollate(me_h_);

    }

  }

  mui_->unsubscribe("*/EcalBarrel/EcalInfo/STATUS");
  mui_->unsubscribe("*/EcalBarrel/EcalInfo/RUN");
  mui_->unsubscribe("*/EcalBarrel/EcalInfo/EVT");
  mui_->unsubscribe("*/EcalBarrel/EcalInfo/EVTTYPE");
  mui_->unsubscribe("*/EcalBarrel/EcalInfo/RUNTYPE");

}

void EcalBarrelMonitorClient::softReset(void) {

  for ( int i=0; i<int(clients_.size()); i++ ) {
    bool done; done = false;
    for ( EBCIMMap::iterator j = chb_.lower_bound(clients_[i]); j != chb_.upper_bound(clients_[i]); ++j ) {
      if ( runtype_ != -1 && runtype_ == (*j).second && !done ) { done = true; clients_[i]->softReset(); }
    }
  }

}

void EcalBarrelMonitorClient::analyze(void){

  current_time_ = time(NULL);

  ievt_++;
  jevt_++;

  if ( ievt_ % 10 == 0 ) {
    if ( verbose_ ) cout << "EcalBarrelMonitorClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;
  }

  // # of full monitoring cycles processed
  int updates = mui_->getNumUpdates();

  if ( ! enableStateMachine_ ) {
    if ( enableQT_ ) mui_->runQTests();
    mui_->doMonitoring();
  }

  this->subscribeNew();

  Char_t histo[200];

  MonitorElement* me;
  string s;

  bool update = false;

  if ( updates != last_update_ || updates == -1 || forced_update_ ) {

    sprintf(histo, (prefixME_+"EcalBarrel/EcalInfo/STATUS").c_str());
    me = mui_->get(histo);
    if ( me ) {
      s = me->valueString();
      status_ = "unknown";
      if ( s.substr(2,1) == "0" ) status_ = "begin-of-run";
      if ( s.substr(2,1) == "1" ) status_ = "running";
      if ( s.substr(2,1) == "2" ) status_ = "end-of-run";
      if ( verbose_ ) cout << "Found '" << histo << "'" << endl;
    }

    if ( inputFile_.size() != 0 ) {
      if ( ievt_ == 1 ) {
        cout << endl;
        cout << " Reading DQM from file, forcing 'begin-of-run'" << endl;
        cout << endl;
        status_ = "begin-of-run";
      }
    }

    sprintf(histo, (prefixME_+"EcalBarrel/EcalInfo/RUN").c_str());
    me = mui_->get(histo);
    if ( me ) {
      s = me->valueString();
      sscanf((s.substr(2,s.length()-2)).c_str(), "%d", &run_);
      if ( verbose_ ) cout << "Found '" << histo << "'" << endl;
    }

    sprintf(histo, (prefixME_+"EcalBarrel/EcalInfo/EVT").c_str());
    me = mui_->get(histo);
    if ( me ) {
      s = me->valueString();
      sscanf((s.substr(2,s.length()-2)).c_str(), "%d", &evt_);
      if ( verbose_ ) cout << "Found '" << histo << "'" << endl;
    }

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EcalInfo/EVTTYPE");
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EcalInfo/EVTTYPE").c_str());
    }
    me = mui_->get(histo);
    h_ = EBMUtilsClient::getHisto<TH1F*>( me, cloneME_, h_ );

    sprintf(histo, (prefixME_+"EcalBarrel/EcalInfo/RUNTYPE").c_str());
    me = mui_->get(histo);
    if ( me ) {
      s = me->valueString();
      runtype_ = atoi(s.substr(2,s.size()-2).c_str());
      if ( verbose_ ) cout << "Found '" << histo << "'" << endl;
    }

    if ( verbose_ ) cout << " updates = "  << updates << endl;

    cout << " run = "      << run_      <<
            " event = "    << evt_      <<
            " status = "   << status_   << endl;

    cout << " runtype = "  << ( runtype_ == -1 ? "UNKNOWN" : runTypes_[runtype_] ) <<
            " location = " << location_ << flush;

    if ( h_ ) {
      if ( h_->GetEntries() != 0 ) {
        cout << "  ( " << flush;
        for ( int i=0; i<int(runTypes_.size()); ++i ) {
          if ( runTypes_[i] != "UNKNOWN" && h_->GetBinContent(i+1) != 0 ) {
            string s = runTypes_[i];
            transform( s.begin(), s.end(), s.begin(), (int(*)(int))tolower );
            cout << s << " ";
          }
        }
        cout << ")" << flush;
      }
    }
    cout << endl;
    
    update = true;
    
    last_update_ = updates;
    
    last_jevt_ = jevt_;

  }

  for ( int i=0; i<int(clients_.size()); i++ ) {
    bool subscribed; subscribed = false;
    for ( EBCIMMap::iterator j = chb_.lower_bound(clients_[i]); j != chb_.upper_bound(clients_[i]); ++j ) {
      if ( runtype_ != -1 && runtype_ == (*j).second && !subscribed ) { subscribed = true; clients_[i]->subscribeNew(); }
    }
  }

  if ( status_ == "begin-of-run" ) {

    if ( ! begin_run_ ) {

      this->beginRun();

      forced_status_ = false;

    }
    
  }

  if ( status_ == "begin-of-run" || status_ == "running" || status_ == "end-of-run" ) {

    if ( begin_run_ && ! end_run_ ) {

      if ( ( update ) || ( updates == -1 && jevt_ % 10 == 0 ) || status_ == "end-of-run" || forced_update_ ) {

        for ( int i=0; i<int(clients_.size()); i++ ) {
          bool analyzed; analyzed = false;
          for ( EBCIMMap::iterator j = chb_.lower_bound(clients_[i]); j != chb_.upper_bound(clients_[i]); ++j ) {
            if ( runtype_ != -1 && runtype_ == (*j).second && !analyzed ) { analyzed = true; clients_[i]->analyze(); }
          }
        }

        if ( status_ == "end-of-run" || forced_update_ ) {

          if ( enableQT_ ) {

            cout << endl;
            switch ( mui_->getSystemStatus() ) {
              case dqm::qstatus::ERROR:
                cout << " Error(s)";
                break;
              case dqm::qstatus::WARNING:
                cout << " Warning(s)";
                break;
              case dqm::qstatus::OTHER:
                cout << " Some tests did not run;";
                break;
              default:
                cout << " No problems";
            }
            cout << " reported after running the quality tests" << endl;
            cout << endl;

          }

        }

        forced_update_ = false;
        
      }
      
      if ( enableSubRun_ ) {
        time_t seconds = 15 * 60;
        if ( (current_time_ - last_time_) > seconds ) {
          if ( runtype_ == EcalDCCHeaderBlock::COSMIC || 
               runtype_ == EcalDCCHeaderBlock::BEAMH2 || 
               runtype_ == EcalDCCHeaderBlock::BEAMH4 ) this->writeDb();
        }
      }

    }

  }
  
  if ( status_ == "end-of-run" ) {
    
    if ( begin_run_ && ! end_run_ ) {

      this->endRun();
      
      forced_status_ = false;
      
    }
    
  }

  // BEGIN: run-time fixes for missing state transitions
  
  if ( status_ == "unknown" ) {
    
    if ( update ) unknowns_++;
    
    if ( unknowns_ >= 50 ) {
      
      cout << endl;
      cout << "Too many 'unknown' states ..." << endl;
      cout << endl;
      
      if ( enableExit_ ) throw exception();

    }
    
  }
  
  if ( status_ == "running" ) {
    
    if ( ! forced_status_ ) {
      
      if ( run_ > 0 && evt_ > 0 && runtype_ != -1 ) {
        
        if ( ! begin_run_ ) {

          cout << endl;
          cout << "Forcing beginRun() ... NOW !" << endl;
          cout << endl;

          this->beginRun();

          forced_status_ = true;

        }

        if ( begin_run_ ) {

          if ( ( jevt_ - last_jevt_ ) > 200 ) {

            cout << endl;
            cout << "Forcing endRun() ... NOW !" << endl;
            cout << endl;

            this->endRun();

            forced_status_ = true;

          }

        }

      }

    }

  }
  
  // END: run-time fixes for missing state transitions

}

void EcalBarrelMonitorClient::htmlOutput(void){

  cout << endl;
  cout << "Preparing EcalBarrelMonitorClient html output ..." << endl;

  char tmp[10];

  sprintf(tmp, "%09d", run_);

  string htmlDir = baseHtmlDir_ + "/" + tmp + "/";

  system(("/bin/mkdir -p " + htmlDir).c_str());

  ofstream htmlFile;

  htmlFile.open((htmlDir + "index.html").c_str());

  // html page header
  htmlFile << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">  " << endl;
  htmlFile << "<html>  " << endl;
  htmlFile << "<head>  " << endl;
  htmlFile << "  <meta content=\"text/html; charset=ISO-8859-1\"  " << endl;
  htmlFile << " http-equiv=\"content-type\">  " << endl;
  htmlFile << "  <title>Monitor:Executed Tasks index</title> " << endl;
  htmlFile << "</head>  " << endl;
  htmlFile << "<body>  " << endl;
  htmlFile << "<br>  " << endl;
  htmlFile << "<h2>Executed tasks for run:&nbsp&nbsp&nbsp" << endl;
  htmlFile << "<span style=\"color: rgb(0, 0, 153);\">" << run_ <<"</span></h2> " << endl;
  htmlFile << "<h2>Run type:&nbsp&nbsp&nbsp" << endl;
  htmlFile << "<span style=\"color: rgb(0, 0, 153);\">" << ( runtype_ == -1 ? "UNKNOWN" : runTypes_[runtype_] ) <<"</span></h2> " << endl;
  htmlFile << "<hr>" << endl;

  htmlFile << "<ul>" << endl;

  string htmlName;

  for ( int j = 0; j<int(clients_.size()); ++j ) {
    bool written; written = false;
    for ( EBCIMMap::iterator k = chb_.lower_bound(clients_[j]); k != chb_.upper_bound(clients_[j]); ++k ) {
      if ( h_ && h_->GetBinContent((*k).second+1) != 0 && runtype_ != -1 && runtype_ == (*k).second && !written ) { 
        if ( clientNames_[j] == "Laser" && h_->GetBinContent(EcalDCCHeaderBlock::LASER_STD+1) == 0 ) continue;
        written = true; 
        htmlName = "EB" + clientNames_[j] + "Client.html";
        clients_[j]->htmlOutput(run_, htmlDir, htmlName);
        htmlFile << "<li><a href=\"" << htmlName << "\">Data " << clientNames_[j] << "</a></li>" << endl;
      }
    }
  }

  htmlFile << "</ul>" << endl;

  // html page footer
  htmlFile << "</body> " << endl;
  htmlFile << "</html> " << endl;

  htmlFile.close();

}

