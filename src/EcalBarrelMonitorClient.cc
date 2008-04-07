/*
 * \file EcalBarrelMonitorClient.cc
 *
 * $Date: 2008/04/06 18:07:20 $
 * $Revision: 1.406 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DQMServices/Core/interface/MonitorElement.h"

#include "DQMServices/Core/interface/DQMOldReceiver.h"
#include "DQMServices/Core/interface/DQMStore.h"

#include "DataFormats/EcalRawData/interface/EcalDCCHeaderBlock.h"

#include "OnlineDB/EcalCondDB/interface/EcalCondDBInterface.h"
#include "OnlineDB/EcalCondDB/interface/RunTag.h"
#include "OnlineDB/EcalCondDB/interface/RunDat.h"
#include "OnlineDB/EcalCondDB/interface/MonRunDat.h"

#include "DQM/EcalCommon/interface/ColorPalette.h"
#include "DQM/EcalCommon/interface/EcalErrorMask.h"
#include <DQM/EcalCommon/interface/UtilsClient.h>
#include <DQM/EcalCommon/interface/Numbers.h>
#include <DQM/EcalCommon/interface/LogicID.h>

#include <DQM/EcalBarrelMonitorClient/interface/EcalBarrelMonitorClient.h>

#include <DQM/EcalBarrelMonitorClient/interface/EBIntegrityClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBStatusFlagsClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBOccupancyClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBCosmicClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBLaserClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBPedestalClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBPedestalOnlineClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBTestPulseClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBBeamCaloClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBBeamHodoClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBTriggerTowerClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBClusterClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBTimingClient.h>

#include "xgi/Method.h"
#include "xgi/Utils.h"

#include "cgicc/Cgicc.h"
#include "cgicc/FormEntry.h"
#include "cgicc/HTMLClasses.h"

#include "TStyle.h"
#include "TGaxis.h"
#include "TColor.h"

using namespace cms;
using namespace edm;
using namespace std;

EcalBarrelMonitorClient::EcalBarrelMonitorClient(const ParameterSet& ps) : ModuleWeb("EcalBarrelMonitorClient"){

  this->initialize(ps);

}

void EcalBarrelMonitorClient::initialize(const ParameterSet& ps){

  cout << endl;
  cout << " *** Ecal Barrel Generic Monitor Client ***" << endl;
  cout << endl;

  // DQM ROOT input file

  inputFile_ = ps.getUntrackedParameter<string>("inputFile", "");

  if ( inputFile_.size() != 0 ) {
    cout << " Reading DQM data from inputFile = '" << inputFile_ << "'" << endl;
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
         << " dbUserName = '" << dbUserName_ << "'";
    if ( dbUserName_.size() != 0 ) {
      cout << " dbHostName = '" << dbHostName_ << "'"
           << " dbHostPort = '" << dbHostPort_ << "'";
    }
    cout << endl;
  } else {
    cout << " Ecal Cond DB is OFF" << endl;
  }

  // Mask file

  maskFile_ = ps.getUntrackedParameter<string>("maskFile", "");

  if ( maskFile_.size() != 0 ) {
    cout << " Using maskFile = '" << maskFile_ << "'" << endl;
  }

  // mergeRuns switch

  mergeRuns_ = ps.getUntrackedParameter<bool>("mergeRuns", false);

  if ( mergeRuns_ ) {
    cout << " mergeRuns switch is ON" << endl;
  } else {
    cout << " mergeRuns switch is OFF" << endl;
  }

  // enableSubRunDb switch

  enableSubRunDb_ = ps.getUntrackedParameter<bool>("enableSubRunDb", false);
  dbRefreshTime_  = ps.getUntrackedParameter<int>("dbRefreshTime", 15);

  if ( enableSubRunDb_ ) {
    cout << " enableSubRunDb switch is ON" << endl;
    cout << " dbRefreshTime is " << dbRefreshTime_ << " minutes" << endl;
  } else {
    cout << " enableSubRunDb switch is OFF" << endl;
  }

  // enableSubRunHtml switch

  enableSubRunHtml_ = ps.getUntrackedParameter<bool>("enableSubRunHtml", false);
  htmlRefreshTime_  = ps.getUntrackedParameter<int>("htmlRefreshTime", 5);

  if ( enableSubRunHtml_ ) {
    cout << " enableSubRunHtml switch is ON" << endl;
    cout << " htmlRefreshTime is " << htmlRefreshTime_ << " minutes" << endl;
  } else {
    cout << " enableSubRunHtml switch is OFF" << endl;
  }

  // location

  location_ =  ps.getUntrackedParameter<string>("location", "H4");

  // base Html output directory

  baseHtmlDir_ = ps.getUntrackedParameter<string>("baseHtmlDir", "");

  if ( baseHtmlDir_.size() != 0 ) {
    cout << " HTML output will go to"
         << " baseHtmlDir = '" << baseHtmlDir_ << "'" << endl;
  } else {
    cout << " HTML output is OFF" << endl;
  }

  // cloneME switch

  cloneME_ = ps.getUntrackedParameter<bool>("cloneME", true);

  if ( cloneME_ ) {
    cout << " cloneME switch is ON" << endl;
  } else {
    cout << " cloneME switch is OFF" << endl;
  }

  // debug switch

  debug_ = ps.getUntrackedParameter<bool>("debug", false);

  if ( debug_ ) {
    cout << " debug switch is ON" << endl;
  } else {
    cout << " debug switch is OFF" << endl;
  }

  // enableMonitorDaemon switch

  enableMonitorDaemon_ = ps.getUntrackedParameter<bool>("enableMonitorDaemon", false);

  if ( enableMonitorDaemon_ ) {
    cout << " enableMonitorDaemon switch is ON" << endl;
  } else {
    cout << " enableMonitorDaemon switch is OFF" << endl;
  }

  enableCleanup_ = ps.getUntrackedParameter<bool>("enableCleanup", false);

  if ( enableCleanup_ ) {
    cout << " enableCleanup switch is ON" << endl;
  } else {
    cout << " enableCleanup switch is OFF" << endl;
  }

  // enableUpdate switch

  enableUpdate_ = ps.getUntrackedParameter<bool>("enableUpdate", false);

  if ( enableUpdate_ ) {
    cout << " enableUpdate switch is ON" << endl;
  } else {
    cout << " enableUpdate switch is OFF" << endl;
  }

  // DQM Client name

  clientName_ = ps.getUntrackedParameter<string>("clientName", "EcalBarrelMonitorClient");

  if ( enableMonitorDaemon_ ) {

    // DQM Collector hostname

    hostName_ = ps.getUntrackedParameter<string>("hostName", "localhost");

    // DQM Collector port

    hostPort_ = ps.getUntrackedParameter<int>("hostPort", 9090);

    cout << " Client '" << clientName_ << "' " << endl
         << " Collector on host '" << hostName_ << "'"
         << " on port '" << hostPort_ << "'" << endl;

  }

  // vector of selected Super Modules (Defaults to all 36).

  superModules_.reserve(36);
  for ( unsigned int i = 1; i <= 36; i++ ) superModules_.push_back(i);

  superModules_ = ps.getUntrackedParameter<vector<int> >("superModules", superModules_);

  cout << " Selected SMs:" << endl;

  for ( unsigned int i = 0; i < superModules_.size(); i++ ) {
    cout << " " << setw(2) << setfill('0') << superModules_[i];
  }

  cout << endl;

  // vector of enabled Clients (defaults)

  enabledClients_.push_back("Integrity");
  enabledClients_.push_back("StatusFlags");
  enabledClients_.push_back("PedestalOnline");
  enabledClients_.push_back("Summary");

  enabledClients_ = ps.getUntrackedParameter<vector<string> >("enabledClients", enabledClients_);

  cout << " Enabled Clients:" << endl;

  for ( unsigned int i = 0; i < enabledClients_.size(); i++ ) {
    cout << " " << enabledClients_[i];
  }

  cout << endl;

  // global ROOT style

  gStyle->Reset("Default");

  gStyle->SetCanvasColor(10);
  gStyle->SetPadColor(10);
  gStyle->SetFillColor(10);
  gStyle->SetStatColor(10);
  gStyle->SetTitleFillColor(10);

  TGaxis::SetMaxDigits(4);

  gStyle->SetOptTitle(kTRUE);
  gStyle->SetTitleX(0.01);
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

  for( int i=0; i<6; i++ ) {
    TColor* color = gROOT->GetColor( 301+i );
    if ( ! color ) color = new TColor( 301+i, 0, 0, 0, "");
    color->SetRGB( ecdqm::rgb[i][0], ecdqm::rgb[i][1], ecdqm::rgb[i][2] );
  }

  for( int i=0; i<10; i++ ) {
    TColor* color = gROOT->GetColor( 401+i );
    if ( ! color ) color = new TColor( 401+i, 0, 0, 0, "");
    color->SetRGB( ecdqm::rgb2[i][0], ecdqm::rgb2[i][1], ecdqm::rgb2[i][2] );
  }

  for( int i=0; i<10; i++ ) {
    TColor* color = gROOT->GetColor( 501+i );
    if ( ! color ) color = new TColor( 501+i, 0, 0, 0, "");
    color->SetRGB( ecdqm::rgb2[i][1], 0, 0 );
  }

  // set runTypes (use resize() on purpose!)

  runTypes_.resize(30);
  for ( unsigned int i = 0; i < runTypes_.size(); i++ ) runTypes_[i] =  "UNKNOWN";

  runTypes_[EcalDCCHeaderBlock::COSMIC]               = "COSMIC";
  runTypes_[EcalDCCHeaderBlock::BEAMH4]               = "BEAM";
  runTypes_[EcalDCCHeaderBlock::BEAMH2]               = "BEAM";
  runTypes_[EcalDCCHeaderBlock::MTCC]                 = "PHYSICS";
  runTypes_[EcalDCCHeaderBlock::LASER_STD]            = "LASER";
  runTypes_[EcalDCCHeaderBlock::LED_STD]              = "LED";
  runTypes_[EcalDCCHeaderBlock::TESTPULSE_MGPA]       = "TEST_PULSE";
  runTypes_[EcalDCCHeaderBlock::PEDESTAL_STD]         = "PEDESTAL";
  runTypes_[EcalDCCHeaderBlock::PEDESTAL_OFFSET_SCAN] = "PEDESTAL-OFFSET";

  runTypes_[EcalDCCHeaderBlock::COSMICS_GLOBAL]       = "COSMIC";
  runTypes_[EcalDCCHeaderBlock::PHYSICS_GLOBAL]       = "PHYSICS";
  runTypes_[EcalDCCHeaderBlock::HALO_GLOBAL]          = "HALO";
  runTypes_[EcalDCCHeaderBlock::COSMICS_LOCAL]        = "COSMIC";
  runTypes_[EcalDCCHeaderBlock::PHYSICS_LOCAL]        = "PHYSICS";
  runTypes_[EcalDCCHeaderBlock::HALO_LOCAL]           = "HALO";

  runTypes_[EcalDCCHeaderBlock::LASER_GAP]            = "LASER";
  runTypes_[EcalDCCHeaderBlock::LED_GAP]              = "LED";
  runTypes_[EcalDCCHeaderBlock::TESTPULSE_GAP]        = "TEST_PULSE";
  runTypes_[EcalDCCHeaderBlock::PEDESTAL_GAP]         = "PEDESTAL";

  runTypes_[EcalDCCHeaderBlock::CALIB_LOCAL]          = "CALIB";

  // clients' constructors

  clients_.reserve(12);
  clientsNames_.reserve(12);

  if ( find(enabledClients_.begin(), enabledClients_.end(), "Integrity" ) != enabledClients_.end() ) {

    clients_.push_back( new EBIntegrityClient(ps) );
    clientsNames_.push_back( "Integrity" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_OFFSET_SCAN ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::MTCC ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_GAP ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "StatusFlags" ) != enabledClients_.end() ) {

    clients_.push_back( new EBStatusFlagsClient(ps) );
    clientsNames_.push_back( "StatusFlags" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_OFFSET_SCAN ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::MTCC ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_GAP ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "Occupancy" ) != enabledClients_.end() ) {

    clients_.push_back( new EBOccupancyClient(ps) );
    clientsNames_.push_back( "Occupancy" );
    
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_OFFSET_SCAN ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::MTCC ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_GAP ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "Cosmic" ) != enabledClients_.end() ) {

    clients_.push_back( new EBCosmicClient(ps) );
    clientsNames_.push_back( "Cosmic" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::MTCC ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_GAP ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "Laser" ) != enabledClients_.end() ) {

    clients_.push_back( new EBLaserClient(ps) );
    clientsNames_.push_back( "Laser" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_GAP ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "Pedestal" ) != enabledClients_.end() ) {

    clients_.push_back( new EBPedestalClient(ps) );
    clientsNames_.push_back( "Pedestal" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_GAP ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "PedestalOnline" ) != enabledClients_.end() ) {

    clients_.push_back( new EBPedestalOnlineClient(ps) );
    clientsNames_.push_back( "PedestalOnline" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::MTCC ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_GAP ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "TestPulse" ) != enabledClients_.end() ) {

    clients_.push_back( new EBTestPulseClient(ps) );
    clientsNames_.push_back( "TestPulse" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_STD ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_MGPA ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::LASER_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::TESTPULSE_GAP ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PEDESTAL_GAP ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "BeamCalo" ) != enabledClients_.end() ) {

    clients_.push_back( new EBBeamCaloClient(ps) );
    clientsNames_.push_back( "BeamCalo" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "BeamHodo" ) != enabledClients_.end() ) {

    clients_.push_back( new EBBeamHodoClient(ps) );
    clientsNames_.push_back( "BeamHodo" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "TriggerTower" ) != enabledClients_.end() ) {

    clients_.push_back( new EBTriggerTowerClient(ps) );
    clientsNames_.push_back( "TriggerTower" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::MTCC ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "Cluster" ) != enabledClients_.end() ) {

    clients_.push_back( new EBClusterClient(ps) );
    clientsNames_.push_back( "Cluster" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH4 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::BEAMH2 ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::MTCC ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));

  }

  if ( find(enabledClients_.begin(), enabledClients_.end(), "Timing" ) != enabledClients_.end() ) {

    clients_.push_back( new EBTimingClient(ps) );
    clientsNames_.push_back( "Timing" );

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMIC ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::MTCC ));

    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_GLOBAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::COSMICS_LOCAL ));
    clientsRuns_.insert(pair<EBClient*,int>( clients_.back(), EcalDCCHeaderBlock::PHYSICS_LOCAL ));

  }

  // define status bits

  clientsStatus_.insert(pair<string,int>( "Integrity",       0 ));
  clientsStatus_.insert(pair<string,int>( "Cosmic",          1 ));
  clientsStatus_.insert(pair<string,int>( "Laser",           2 ));
  clientsStatus_.insert(pair<string,int>( "Pedestal",        3 ));
  clientsStatus_.insert(pair<string,int>( "PedestalOnline",  4 ));
  clientsStatus_.insert(pair<string,int>( "TestPulse",       5 ));
  clientsStatus_.insert(pair<string,int>( "BeamCalo",        6 ));
  clientsStatus_.insert(pair<string,int>( "BeamHodo",        7 ));
  clientsStatus_.insert(pair<string,int>( "TriggerTower",    8 ));
  clientsStatus_.insert(pair<string,int>( "Cluster",         9 ));
  clientsStatus_.insert(pair<string,int>( "Timing",         10 ));
  clientsStatus_.insert(pair<string,int>( "Led",            11 ));
  clientsStatus_.insert(pair<string,int>( "StatusFlags",    12 ));
  clientsStatus_.insert(pair<string,int>( "Occupancy",      13 ));

  if ( find(enabledClients_.begin(), enabledClients_.end(), "Summary" ) != enabledClients_.end() ) {

    summaryClient_ = new EBSummaryClient(ps);

  }

  if ( summaryClient_ ) summaryClient_->setFriends(clients_);

  cout << endl;

}

EcalBarrelMonitorClient::~EcalBarrelMonitorClient(){

  cout << "Exit ..." << endl;

  for ( unsigned int i=0; i<clients_.size(); i++ ) {
    delete clients_[i];
  }

  if ( summaryClient_ ) delete summaryClient_;

  if ( enableMonitorDaemon_ ) delete mui_;

}

void EcalBarrelMonitorClient::beginJob(const EventSetup &c) {

  begin_run_ = false;
  end_run_   = false;

  forced_status_ = false;
  forced_update_ = false;

  h_ = 0;

  status_  = "unknown";
  run_     = -1;
  evt_     = -1;
  runType_ = -1;
  evtType_ = -1;

  last_run_ = -1;

  subrun_  = -1;

  if ( debug_ ) cout << "EcalBarrelMonitorClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

  current_time_ = time(NULL);
  last_time_db_ = current_time_;
  last_time_html_ = current_time_;

  if ( enableMonitorDaemon_ ) {

    // start DQM user interface instance
    // will attempt to reconnect upon connection problems (w/ a 5-sec delay)

    mui_ = new DQMOldReceiver(hostName_, hostPort_, clientName_, 5);
    dbe_ = mui_->getBEInterface();

  } else {
    
    // get hold of back-end interface

    mui_ = 0;
    dbe_ = Service<DQMStore>().operator->();
  
  }

  if ( debug_ ) {
    dbe_->setVerbose(1);
  } else {
    dbe_->setVerbose(0);
  }

  if ( ! enableMonitorDaemon_ ) {
    if ( inputFile_.size() != 0 ) {
      if ( dbe_ ) {
        dbe_->open(inputFile_);
      }
    }
  }

  for ( unsigned int i=0; i<clients_.size(); i++ ) {
    clients_[i]->beginJob(dbe_);
  }

  if ( summaryClient_ ) summaryClient_->beginJob(dbe_);

  Numbers::initGeometry(c);

}

void EcalBarrelMonitorClient::beginRun(void){

  begin_run_ = true;
  end_run_   = false;

  last_run_  = run_;

  if ( debug_ ) cout << "EcalBarrelMonitorClient: beginRun" << endl;

  jevt_ = 0;

  current_time_ = time(NULL);
  last_time_db_ = current_time_;
  last_time_html_ = current_time_;

  this->setup();

  this->beginRunDb();

  for ( int i=0; i<int(clients_.size()); i++ ) {
    clients_[i]->cleanup();
    bool done = false;
    for ( multimap<EBClient*,int>::iterator j = clientsRuns_.lower_bound(clients_[i]); j != clientsRuns_.upper_bound(clients_[i]); j++ ) {
      if ( runType_ != -1 && runType_ == (*j).second && !done ) {
        done = true;
        clients_[i]->beginRun();
      }
    }
  }

  if ( summaryClient_ ) summaryClient_->beginRun();

}

void EcalBarrelMonitorClient::beginRun(const Run& r, const EventSetup& c) {

  cout << endl;
  cout << "Standard beginRun() for run " << r.id().run() << endl;
  cout << endl;

  run_ = r.id().run();

  jevt_ = 0;

}

void EcalBarrelMonitorClient::endJob(void) {

  // check last event

  if ( ! end_run_ ) {

    cout << endl;
    cout << "Checking last event at endJob() ... " << endl;
    cout << endl;

    forced_update_ = true;
    this->analyze();

    if ( begin_run_ && ! end_run_ ) {

      cout << endl;
      cout << "Forcing endRun() ... " << endl;
      cout << endl;

      forced_status_ = true;
      this->endRun();

    }

  }

  if ( debug_ ) cout << "EcalBarrelMonitorClient: endJob, ievt = " << ievt_ << endl;

  this->cleanup();

  for ( unsigned int i=0; i<clients_.size(); i++ ) {
    clients_[i]->endJob();
  }

  if ( summaryClient_ ) summaryClient_->endJob();

}

void EcalBarrelMonitorClient::endRun(void) {

  begin_run_ = false;
  end_run_   = true;

  if ( debug_ ) cout << "EcalBarrelMonitorClient: endRun, jevt = " << jevt_ << endl;

  if ( baseHtmlDir_.size() != 0 ) this->htmlOutput();

  if ( subrun_ != -1 ) {

    this->writeDb();

    this->endRunDb();

  }

  for ( int i=0; i<int(clients_.size()); i++ ) {
    bool done = false;
    for ( multimap<EBClient*,int>::iterator j = clientsRuns_.lower_bound(clients_[i]); j != clientsRuns_.upper_bound(clients_[i]); j++ ) {
      if ( runType_ != -1 && runType_ == (*j).second && !done ) {
        done = true;
        clients_[i]->endRun();
      }
    }
  }

  if ( summaryClient_ ) summaryClient_->endRun();

  this->cleanup();

  status_  = "unknown";
  run_     = -1;
  evt_     = -1;
  runType_ = -1;
  evtType_ = -1;

  subrun_ = -1;

}

void EcalBarrelMonitorClient::endRun(const Run& r, const EventSetup& c) {

  cout << endl;
  cout << "Standard endRun() for run " << r.id().run() << endl;
  cout << endl;

  if ( run_ != -1 && evt_ != -1 && runType_ != -1 ) {

    forced_update_ = true;
    this->analyze();

    if ( ! mergeRuns_ ) {

      if ( begin_run_ && ! end_run_ ) {

        forced_status_ = false;
        this->endRun();

      }

    }

  }

}

void EcalBarrelMonitorClient::beginLuminosityBlock(const LuminosityBlock &l, const EventSetup &c) {

  cout << endl;
  cout << "Standard beginLuminosityBlock() for run " << l.id().run() << endl;
  cout << endl;

}

void EcalBarrelMonitorClient::endLuminosityBlock(const LuminosityBlock &l, const EventSetup &c) {

  cout << endl;
  cout << "Standard endLuminosityBlock() for run " << l.id().run() << endl;
  cout << endl;

  if ( run_ != -1 && evt_ != -1 && runType_ != -1 ) {

    forced_update_ = true;
    this->analyze();

  }

}

void EcalBarrelMonitorClient::setup(void) {

}

void EcalBarrelMonitorClient::cleanup(void) {

  if ( ! enableCleanup_ ) return;

  if ( cloneME_ ) {
    if ( h_ ) delete h_;
  }

  h_ = 0;

}

void EcalBarrelMonitorClient::beginRunDb(void) {

  subrun_ = 0;

  EcalCondDBInterface* econn;

  econn = 0;

  if ( dbName_.size() != 0 ) {
    try {
      cout << "Opening DB connection with TNS_ADMIN ..." << endl;
      econn = new EcalCondDBInterface(dbName_, dbUserName_, dbPassword_);
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
      if ( dbHostName_.size() != 0 ) {
        try {
          cout << "Opening DB connection without TNS_ADMIN ..." << endl;
          econn = new EcalCondDBInterface(dbHostName_, dbName_, dbUserName_, dbPassword_, dbHostPort_);
          cout << "done." << endl;
        } catch (runtime_error &e) {
          cerr << e.what() << endl;
        }
      }
    }
  }

  // create the objects necessary to identify a dataset

  LocationDef locdef;

  locdef.setLocation(location_);

  RunTypeDef rundef;

  rundef.setRunType( this->getRunType()  );

  RunTag runtag;

  runtag.setLocationDef(locdef);
  runtag.setRunTypeDef(rundef);

  runtag.setGeneralTag( this->getRunType() );

  // fetch the RunIOV from the DB

  bool foundRunIOV = false;

  if ( econn ) {
    try {
      cout << "Fetching RunIOV ..." << endl;
//      runiov_ = econn->fetchRunIOV(&runtag, run_);
      runiov_ = econn->fetchRunIOV(location_, run_);
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
        cout << "Inserting RunIOV ..." << endl;
        econn->insertRunIOV(&runiov_);
        cout << "done." << endl;
      } catch (runtime_error &e) {
        cerr << e.what() << endl;
        try {
          cout << "Fetching RunIOV (again) ..." << endl;
//          runiov_ = econn->fetchRunIOV(&runtag, run_);
          runiov_ = econn->fetchRunIOV(location_, run_);
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

  string rt = runiov_.getRunTag().getRunTypeDef().getRunType();
  if ( strcmp(rt.c_str(), "UNKNOWN") == 0 ) {
    runType_ = -1;
  } else {
    for ( unsigned int i = 0; i < runTypes_.size(); i++ ) {
      if ( rt == runTypes_[i] ) {
        if ( runType_ != int(i) ) {
          cout << endl;
          cout << "Taking Run Type from DB: " << runTypes_[i] << endl;
          cout << endl;
          runType_ = i;
        }
        break;
      }
    }
  }

  if ( maskFile_.size() != 0 ) {
    try {
      cout << "Fetching masked channels from file ..." << endl;
      EcalErrorMask::readFile(maskFile_, debug_);
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  } else {
    if ( econn ) {
      try {
        cout << "Fetching masked channels from DB ..." << endl;
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
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  cout << endl;

}

void EcalBarrelMonitorClient::writeDb(void) {

  subrun_++;

  EcalCondDBInterface* econn;

  econn = 0;

  if ( dbName_.size() != 0 ) {
    try {
      cout << "Opening DB connection with TNS_ADMIN ..." << endl;
      econn = new EcalCondDBInterface(dbName_, dbUserName_, dbPassword_);
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
      if ( dbHostName_.size() != 0 ) {
        try {
          cout << "Opening DB connection without TNS_ADMIN ..." << endl;
          econn = new EcalCondDBInterface(dbHostName_, dbName_, dbUserName_, dbPassword_, dbHostPort_);
          cout << "done." << endl;
        } catch (runtime_error &e) {
          cerr << e.what() << endl;
        }
      }
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

  for ( int i=0; i<int(clients_.size()); i++ ) {
    bool done = false;
    for ( multimap<EBClient*,int>::iterator j = clientsRuns_.lower_bound(clients_[i]); j != clientsRuns_.upper_bound(clients_[i]); j++ ) {
      if ( h_ && runType_ != -1 && runType_ == (*j).second && !done ) {
        if ( strcmp(clientsNames_[i].c_str(), "Cosmic") == 0 && runType_ != runTypes_[EcalDCCHeaderBlock::COSMIC] && runType_ != runTypes_[EcalDCCHeaderBlock::COSMICS_LOCAL] && runType_ != runTypes_[EcalDCCHeaderBlock::COSMICS_GLOBAL] && runType_ != runTypes_[EcalDCCHeaderBlock::PHYSICS_GLOBAL] && runType_ != runTypes_[EcalDCCHeaderBlock::PHYSICS_LOCAL] && h_->GetBinContent(2+EcalDCCHeaderBlock::COSMIC) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::COSMICS_LOCAL) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::COSMICS_GLOBAL) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::PHYSICS_GLOBAL) && h_->GetBinContent(2+EcalDCCHeaderBlock::PHYSICS_LOCAL) ) continue;
        if ( strcmp(clientsNames_[i].c_str(), "Laser") == 0 && runType_ != runTypes_[EcalDCCHeaderBlock::LASER_STD] && runType_ != runTypes_[EcalDCCHeaderBlock::LASER_GAP] && h_->GetBinContent(2+EcalDCCHeaderBlock::LASER_STD) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::LASER_GAP) == 0 ) continue;
        if ( strcmp(clientsNames_[i].c_str(), "Pedestal") == 0 && runType_ != runTypes_[EcalDCCHeaderBlock::PEDESTAL_STD] && runType_ != runTypes_[EcalDCCHeaderBlock::PEDESTAL_GAP] && h_->GetBinContent(2+EcalDCCHeaderBlock::PEDESTAL_STD) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::PEDESTAL_GAP) == 0 ) continue;
        if ( strcmp(clientsNames_[i].c_str(), "TestPulse") == 0 && runType_ != runTypes_[EcalDCCHeaderBlock::TESTPULSE_MGPA] && runType_ != runTypes_[EcalDCCHeaderBlock::TESTPULSE_GAP] && h_->GetBinContent(2+EcalDCCHeaderBlock::TESTPULSE_MGPA) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::TESTPULSE_GAP) == 0 ) continue;
        done = true;
        taskl |= 0x1 << clientsStatus_[clientsNames_[i]];
        cout << " Writing " << clientsNames_[i] << " results to DB " << endl;
        cout << endl;
        if ( clients_[i]->writeDb(econn, &runiov_, &moniov_) ) {
          tasko |= 0x1 << clientsStatus_[clientsNames_[i]];
        } else {
          tasko |= 0x0 << clientsStatus_[clientsNames_[i]];
        }
      }
    }
    if ( ((taskl >> clientsStatus_[clientsNames_[i]]) & 0x1) ) {
      cout << " Task output for "
           << clientsNames_[i]
           << " = "
           << ((tasko >> clientsStatus_[clientsNames_[i]]) & 0x1) << endl;
      cout << endl;
    }
  }

  if ( summaryClient_ ) summaryClient_->writeDb(econn, &runiov_, &moniov_);

  EcalLogicID ecid;
  MonRunDat md;
  map<EcalLogicID, MonRunDat> dataset;

  MonRunOutcomeDef monRunOutcomeDef;

  monRunOutcomeDef.setShortDesc("success");

  float nevt = -1.;

  if ( h_ ) nevt = h_->GetEntries();

  md.setNumEvents(int(nevt));
  md.setMonRunOutcomeDef(monRunOutcomeDef);

//  string fileName = "";
//  md.setRootfileName(fileName);

  md.setTaskList(taskl);
  md.setTaskOutcome(tasko);

  if ( econn ) {
    try {
      ecid = LogicID::getEcalLogicID("EB");
      dataset[ecid] = md;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  if ( econn ) {
    try {
      cout << "Inserting MonRunDat ..." << endl;
      econn->insertDataSet(&dataset, &moniov_);
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  if ( econn ) {
    try {
      cout << "Closing DB connection ..." << endl;
      delete econn;
      econn = 0;
      cout << "done." << endl;
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
      cout << "Opening DB connection with TNS_ADMIN ..." << endl;
      econn = new EcalCondDBInterface(dbName_, dbUserName_, dbPassword_);
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
      if ( dbHostName_.size() != 0 ) {
        try {
          cout << "Opening DB connection without TNS_ADMIN ..." << endl;
          econn = new EcalCondDBInterface(dbHostName_, dbName_, dbUserName_, dbPassword_, dbHostPort_);
          cout << "done." << endl;
        } catch (runtime_error &e) {
          cerr << e.what() << endl;
        }
      }
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
      cout << "Fetching RunDat ..." << endl;
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
        ecid = LogicID::getEcalLogicID("EB");
        dataset[ecid] = rd;
      } catch (runtime_error &e) {
        cerr << e.what() << endl;
      }
    }

    if ( econn ) {
      try {
        cout << "Inserting RunDat ..." << endl;
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
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

}

void EcalBarrelMonitorClient::analyze(void){

  current_time_ = time(NULL);

  ievt_++;
  jevt_++;

  if ( debug_ ) cout << "EcalBarrelMonitorClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;

  // update MEs (online mode)
  if ( enableUpdate_ ) {
    if ( enableMonitorDaemon_ ) mui_->doMonitoring();
  }

  MonitorElement* me;
  string s;

  me = dbe_->get("EcalBarrel/EcalInfo/STATUS");
  if ( me ) {
    status_ = "unknown";
    s = me->valueString();
    if ( strcmp(s.c_str(), "i=0") == 0 ) status_ = "begin-of-run";
    if ( strcmp(s.c_str(), "i=1") == 0 ) status_ = "running";
    if ( strcmp(s.c_str(), "i=2") == 0 ) status_ = "end-of-run";
    if ( debug_ ) cout << "Found 'EcalBarrel/EcalInfo/STATUS'" << endl;
  }

  if ( inputFile_.size() != 0 ) {
    if ( ievt_ == 1 ) {
      cout << endl;
      cout << " Reading DQM from file, forcing 'begin-of-run'" << endl;
      cout << endl;
      status_ = "begin-of-run";
    }
  }

  int ecal_run = -1;
  me = dbe_->get("EcalBarrel/EcalInfo/RUN");
  if ( me ) {
    s = me->valueString();
    sscanf(s.c_str(), "i=%d", &ecal_run);
    if ( debug_ ) cout << "Found 'EcalBarrel/EcalInfo/RUN'" << endl;
  }

  int ecal_evt = -1;
  me = dbe_->get("EcalBarrel/EcalInfo/EVT");
  if ( me ) {
    s = me->valueString();
    sscanf(s.c_str(), "i=%d", &ecal_evt);
    if ( debug_ ) cout << "Found 'EcalBarrel/EcalInfo/EVT'" << endl;
  }

  me = dbe_->get("EcalBarrel/EcalInfo/EVTTYPE");
  h_ = UtilsClient::getHisto<TH1F*>( me, cloneME_, h_ );

  me = dbe_->get("EcalBarrel/EcalInfo/RUNTYPE");
  if ( me ) {
    s = me->valueString();
    sscanf(s.c_str(), "i=%d", &evtType_);
    if ( runType_ == -1 ) runType_ = evtType_;
    if ( debug_ ) cout << "Found 'EcalBarrel/EcalInfo/RUNTYPE'" << endl;
  }

  // if the run number from the Event is less than zero,
  // use the run number from the ECAL DCC header
  if ( run_ <= 0 ) run_ = ecal_run;

  if ( ! mergeRuns_ && run_ != last_run_ ) forced_update_ = true;

  bool update = ( jevt_ <   10                      ) ||
                ( jevt_ <  100 && jevt_ %   10 == 0 ) ||
                ( jevt_ < 1000 && jevt_ %  100 == 0 ) ||
                (                 jevt_ % 1000 == 0 );

  if ( update || strcmp(status_.c_str(), "begin-of-run") == 0 || strcmp(status_.c_str(), "end-of-run") == 0 || forced_update_ ) {

    cout << " RUN status = \"" << status_ << "\"" << endl;
    cout << "   CMS  run/event number = " << run_ << "/" << evt_ << endl;
    cout << "   ECAL run/event number = " << ecal_run << "/" << ecal_evt << endl;
    cout << "   ECAL location = " << location_ << endl;
    cout << "   ECAL run/event type = " << this->getRunType() << "/" << ( evtType_ == -1 ? "UNKNOWN" : runTypes_[evtType_] ) << flush;

    if ( h_ ) {
      if ( h_->GetEntries() != 0 ) {
        cout << " ( " << flush;
        for ( unsigned int i = 0; i < runTypes_.size(); i++ ) {
          if ( runTypes_[i] != "UNKNOWN" && h_->GetBinContent(2+i) != 0 ) {
            string s = runTypes_[i];
            transform( s.begin(), s.end(), s.begin(), (int(*)(int))tolower );
            cout << s << " ";
          }
        }
        cout << ")" << flush;
      }
    }
    cout << endl;

  }

  if ( strcmp(status_.c_str(), "begin-of-run") == 0 ) {

    if ( run_ != -1 && evt_ != -1 && runType_ != -1 ) {

      if ( ! begin_run_ ) {

        forced_status_ = false;
        this->beginRun();

      }

    }

  }

  if ( strcmp(status_.c_str(), "begin-of-run") == 0 || strcmp(status_.c_str(), "running") == 0 || strcmp(status_.c_str(), "end-of-run") == 0 ) {

    if ( begin_run_ && ! end_run_ ) {

      bool update = ( jevt_ < 3 || jevt_ % 1000 == 0 );

      if ( update || strcmp(status_.c_str(), "begin-of-run") == 0 || strcmp(status_.c_str(), "end-of-run") == 0 || forced_update_ ) {

        for ( int i=0; i<int(clients_.size()); i++ ) {
          bool done = false;
          for ( multimap<EBClient*,int>::iterator j = clientsRuns_.lower_bound(clients_[i]); j != clientsRuns_.upper_bound(clients_[i]); j++ ) {
            if ( runType_ != -1 && runType_ == (*j).second && !done ) {
              done = true;
              clients_[i]->analyze();
            }
          }
        }

        if ( summaryClient_ ) summaryClient_->analyze();

      }

      forced_update_ = false;

      if ( enableSubRunHtml_ ) {
        if ( (current_time_ - last_time_html_) > 60 * htmlRefreshTime_ ) {
          last_time_html_ = current_time_;
          this->htmlOutput( true );
        }
      }

      if ( enableSubRunDb_ ) {
        if ( (current_time_ - last_time_db_) > 60 * dbRefreshTime_ ) {
          if ( runType_ == EcalDCCHeaderBlock::COSMIC ||
               runType_ == EcalDCCHeaderBlock::BEAMH2 ||
               runType_ == EcalDCCHeaderBlock::BEAMH4 ) this->writeDb();
          last_time_db_ = current_time_;
        }
      }

    }

  }

  if ( strcmp(status_.c_str(), "end-of-run") == 0 ) {

    if ( run_ != -1 && evt_ != -1 && runType_ != -1 ) {

      if ( begin_run_ && ! end_run_ ) {

        forced_status_ = false;
        this->endRun();

      }

    }

  }

  // BEGIN: run-time fixes for missing state transitions

  // run number transition

  if ( strcmp(status_.c_str(), "running") == 0 ) {

    if ( run_ != -1 && evt_ != -1 && runType_ != -1 ) {

      if ( ! mergeRuns_ ) {

        int new_run_ = run_;
        int old_run_ = last_run_;

        if ( new_run_ != old_run_ ) {

          if ( begin_run_ && ! end_run_ ) {

            cout << endl;
            cout << " Old run has finished, issuing endRun() ... " << endl;
            cout << endl;

            // end old_run_
            run_ = old_run_;

            forced_status_ = false;
            this->endRun();

          }

          if ( ! begin_run_ ) {

            cout << endl;
            cout << " New run has started, issuing beginRun() ... " << endl;
            cout << endl;

            // start new_run_
            run_ = new_run_;

            forced_status_ = false;
            this->beginRun();

          }

        }

      }

    }

  }

  // 'running' state without a previous 'begin-of-run' state

  if ( strcmp(status_.c_str(), "running") == 0 ) {

    if ( run_ != -1 && evt_ != -1 && runType_ != -1 ) {

      if ( ! forced_status_ ) {

        if ( ! begin_run_ ) {

          cout << endl;
          cout << "Forcing beginRun() ... NOW !" << endl;
          cout << endl;

          forced_status_ = true;
          this->beginRun();

        }

      }

    }

  }

  // END: run-time fixes for missing state transitions

}

void EcalBarrelMonitorClient::analyze(const Event &e, const EventSetup &c) {

  run_ = e.id().run();
  evt_ = e.id().event();

  this->analyze();

}

void EcalBarrelMonitorClient::htmlOutput( bool current ){

  time_t start = time(NULL);

  cout << endl;
  cout << "Preparing EcalBarrelMonitorClient html output ..." << endl;

  char tmp[10];

  sprintf(tmp, "%09d", run_);

  string htmlDir;
  if( current ) {
    htmlDir = baseHtmlDir_ + "/current/";
  }
  else {
    htmlDir = baseHtmlDir_ + "/" + tmp + "/";
  }

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
  htmlFile << "<span style=\"color: rgb(0, 0, 153);\">" << this->getRunType() <<"</span></h2> " << endl;
  htmlFile << "<hr>" << endl;

  htmlFile << "<ul>" << endl;

  string htmlName;

  for ( int i=0; i<int(clients_.size()); i++ ) {
    bool done = false;
    for ( multimap<EBClient*,int>::iterator j = clientsRuns_.lower_bound(clients_[i]); j != clientsRuns_.upper_bound(clients_[i]); j++ ) {
      if ( h_ && runType_ != -1 && runType_ == (*j).second && !done ) {
        if ( strcmp(clientsNames_[i].c_str(), "Cosmic") == 0 && runType_ != runTypes_[EcalDCCHeaderBlock::COSMIC] && runType_ != runTypes_[EcalDCCHeaderBlock::COSMICS_LOCAL] && runType_ != runTypes_[EcalDCCHeaderBlock::COSMICS_GLOBAL] && runType_ != runTypes_[EcalDCCHeaderBlock::PHYSICS_GLOBAL] && runType_ != runTypes_[EcalDCCHeaderBlock::PHYSICS_LOCAL] && h_->GetBinContent(2+EcalDCCHeaderBlock::COSMIC) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::COSMICS_LOCAL) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::COSMICS_GLOBAL) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::PHYSICS_GLOBAL) && h_->GetBinContent(2+EcalDCCHeaderBlock::PHYSICS_LOCAL) ) continue;
        if ( strcmp(clientsNames_[i].c_str(), "Laser") == 0 && runType_ != runTypes_[EcalDCCHeaderBlock::LASER_STD] && runType_ != runTypes_[EcalDCCHeaderBlock::LASER_GAP] && h_->GetBinContent(2+EcalDCCHeaderBlock::LASER_STD) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::LASER_GAP) == 0 ) continue;
        if ( strcmp(clientsNames_[i].c_str(), "Pedestal") == 0 && runType_ != runTypes_[EcalDCCHeaderBlock::PEDESTAL_STD] && runType_ != runTypes_[EcalDCCHeaderBlock::PEDESTAL_GAP] && h_->GetBinContent(2+EcalDCCHeaderBlock::PEDESTAL_STD) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::PEDESTAL_GAP) == 0 ) continue;
        if ( strcmp(clientsNames_[i].c_str(), "TestPulse") == 0 && runType_ != runTypes_[EcalDCCHeaderBlock::TESTPULSE_MGPA] && runType_ != runTypes_[EcalDCCHeaderBlock::TESTPULSE_GAP] && h_->GetBinContent(2+EcalDCCHeaderBlock::TESTPULSE_MGPA) == 0 && h_->GetBinContent(2+EcalDCCHeaderBlock::TESTPULSE_GAP) == 0 ) continue;
        done = true;
        htmlName = "EB" + clientsNames_[i] + "Client.html";
        clients_[i]->htmlOutput(run_, htmlDir, htmlName);
        htmlFile << "<li><a href=\"" << htmlName << "\">Data " << clientsNames_[i] << "</a></li>" << endl;
      }
    }
  }

  if ( summaryClient_ ) {

    htmlName = "EBSummaryClient.html";
    summaryClient_->htmlOutput(run_, htmlDir, htmlName);
    htmlFile << "<li><a href=\"" << htmlName << "\">Data " << "Summary" << "</a></li>" << endl;

    htmlFile << "<br>" << endl;

    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    htmlFile << "<td><img src=\"EB_global_summary.png\" border=0></td>" << endl;

    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

  }

  htmlFile << "</ul>" << endl;

  // html page footer
  htmlFile << "</body> " << endl;
  htmlFile << "</html> " << endl;

  htmlFile.close();

  cout << endl;

  if( current ) {
    time_t elapsed = time(NULL) - start;
    cout << "==========> htmlOutput Elapsed Time: " << elapsed << endl;
  }

}

void EcalBarrelMonitorClient::defaultWebPage(xgi::Input *in, xgi::Output *out){

  string path;
  string mname;

  static bool autorefresh_ = false;

  try {

    cgicc::Cgicc cgi(in);

    if ( xgi::Utils::hasFormElement(cgi,"autorefresh") ) {
      autorefresh_ = xgi::Utils::getFormElement(cgi, "autorefresh")->getIntegerValue() != 0;
    }

    if ( xgi::Utils::hasFormElement(cgi,"module") ) {
      mname = xgi::Utils::getFormElement(cgi, "module")->getValue();
    }

    cgicc::CgiEnvironment cgie(in);
    path = cgie.getPathInfo() + "?" + cgie.getQueryString();

  } catch (exception &e) {

    cerr << "Standard C++ exception : " << e.what() << endl;

  }

  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict)            << endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr")           << endl;

  *out << "<html>"                                                   << endl;

  *out << "<head>"                                                   << endl;

  *out << "<title>" << typeid(EcalBarrelMonitorClient).name()
       << " MAIN</title>"                                            << endl;

  if ( autorefresh_ ) {
    *out << "<meta http-equiv=\"refresh\" content=\"3\">"            << endl;
  }

  *out << "</head>"                                                  << endl;

  *out << "<body>"                                                   << endl;

  *out << cgicc::form().set("method","GET").set("action", path )
       << endl;
  *out << cgicc::input().set("type","hidden").set("name","module").set("value", mname)
       << endl;
  *out << cgicc::input().set("type","hidden").set("name","autorefresh").set("value", autorefresh_?"0":"1")
       << endl;
  *out << cgicc::input().set("type","submit").set("value",autorefresh_?"Toggle AutoRefresh OFF":"Toggle AutoRefresh ON")
       << endl;
  *out << cgicc::form()                                              << endl;

  *out << cgicc::h3( "EcalBarrelMonitorClient Status" ).set( "style", "font-family:arial" ) << endl;

  *out << "<table style=\"font-family: arial\"><tr><td>" << endl;

  *out << "<p style=\"font-family: arial\">"
       << "<table border=1>"
       << "<tr><th>Cycle</th><td align=right>" << this->getEvtPerJob();
  int nevt = 0;
  if ( this->getEntryHisto() != 0 ) nevt = int( this->getEntryHisto()->GetEntries());
  *out << "<tr><th>Event</th><td align=right>" << nevt
       << "</td><tr><th>Run</th><td align=right>" << this->getRun()
       << "</td><tr><th>Run Type</th><td align=right> " << this->getRunType()
       << "</td></table></p>" << endl;

  *out << "</td><td>" << endl;

  *out << "<p style=\"font-family: arial\">"
       << "<table border=1>"
       << "<tr><th>Evt Type</th><th>Evt/Run</th><th>Evt Type</th><th>Evt/Run</th>" << endl;
  vector<string> runTypes = this->getRunTypes();
  for( unsigned int i=0, j=0; i<runTypes.size(); i++ ) {
    if ( runTypes[i] != "UNKNOWN" ) {
      if ( j++%2 == 0 ) *out << "<tr>";
      nevt = 0;
      if ( this->getEntryHisto() != 0 ) nevt = int( this->getEntryHisto()->GetBinContent(i+1));
      *out << "<td>" << runTypes[i]
           << "</td><td align=right>" << nevt << endl;
    }
  }
  *out << "</td></table></p>" << endl;

  *out << "</td><tr><td colspan=2>" << endl;

  *out << "<p style=\"font-family: arial\">"
       << "<table border=1>"
       << "<tr><th>Client</th><th>Cyc/Job</th><th>Cyc/Run</th><th>Client</th><th>Cyc/Job</th><th>Cyc/Run</th>" << endl;
  const vector<EBClient*> clients = this->getClients();
  const vector<string> clientsNames = this->getClientsNames();
  for( unsigned int i=0; i<clients.size(); i++ ) {
    if ( clients[i] != 0 ) {
      if ( i%2 == 0 ) *out << "<tr>";
      *out << "<td>" << clientsNames[i]
           << "</td><td align=right>" << clients[i]->getEvtPerJob()
           << "</td><td align=right>" << clients[i]->getEvtPerRun() << endl;
    }
  }
  *out << "</td></table></p>" << endl;

  *out << "</td><tr><td>" << endl;


  *out << "<p style=\"font-family: arial\">"
       << "<table border=1>"
       << "<tr><th colspan=2>RunIOV</th>"
       << "<tr><td>Run Number</td><td align=right> " << this->getRunIOV().getRunNumber()
       << "</td><tr><td>Run Start</td><td align=right> " << this->getRunIOV().getRunStart().str()
       << "</td><tr><td>Run End</td><td align=right> " << this->getRunIOV().getRunEnd().str()
       << "</td></table></p>" << endl;

  *out << "</td><td colsapn=2>" << endl;

  *out << "<p style=\"font-family: arial\">"
       << "<table border=1>"
       << "<tr><th colspan=2>RunTag</th>"
       << "<tr><td>GeneralTag</td><td align=right> " << this->getRunIOV().getRunTag().getGeneralTag()
       << "</td><tr><td>Location</td><td align=right> " << this->getRunIOV().getRunTag().getLocationDef().getLocation()
       << "</td><tr><td>Run Type</td><td align=right> " << this->getRunIOV().getRunTag().getRunTypeDef().getRunType()
       << "</td></table></p>" << endl;

  *out << "</td><tr><td>" << endl;

  *out << "<p style=\"font-family: arial\">"
       << "<table border=1>"
       << "<tr><th colspan=2>MonRunIOV</th>"
       << "<tr><td>SubRun Number</td><td align=right> " << this->getMonIOV().getSubRunNumber()
       << "</td><tr><td>SubRun Start</td><td align=right> " << this->getMonIOV().getSubRunStart().str()
       << "</td><tr><td>SubRun End</td><td align=right> " << this->getMonIOV().getSubRunEnd().str()
       << "</td></table></p>" << endl;

  *out << "</td><td colspan=2>" << endl;

  *out << "<p style=\"font-family: arial\">"
       << "<table border=1>"
       << "<tr><th colspan=2>MonRunTag</th>"
       << "<tr><td>GeneralTag</td><td align=right> " << this->getMonIOV().getMonRunTag().getGeneralTag()
       << "</td><tr><td>Monitoring Version</td><td align=right> " << this->getMonIOV().getMonRunTag().getMonVersionDef().getMonitoringVersion()
       << "</td></table></p>" << endl;

  *out << "</td><table>" << endl;


  *out << "</body>"                                                  << endl;

  *out << "</html>"                                                  << endl;

}

