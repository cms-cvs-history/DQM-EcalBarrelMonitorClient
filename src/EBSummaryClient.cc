/*
 * \file EBSummaryClient.cc
 *
 * $Date: 2007/11/02 12:36:27 $
 * $Revision: 1.80 $
 * \author G. Della Ricca
 *
*/

#include <memory>
#include <iostream>
#include <iomanip>
#include <map>

#include "TStyle.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DQMServices/UI/interface/MonitorUIRoot.h"
#include "DQMServices/Core/interface/QTestStatus.h"
#include "DQMServices/QualityTests/interface/QCriterionRoot.h"

#include "OnlineDB/EcalCondDB/interface/RunTag.h"
#include "OnlineDB/EcalCondDB/interface/RunIOV.h"

#include <DQM/EcalCommon/interface/UtilsClient.h>
#include <DQM/EcalCommon/interface/Numbers.h>

#include <DQM/EcalBarrelMonitorClient/interface/EBCosmicClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBIntegrityClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBLaserClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBPedestalClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBPedestalOnlineClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBTestPulseClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBBeamCaloClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBBeamHodoClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBTriggerTowerClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBClusterClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBTimingClient.h>

#include <DQM/EcalBarrelMonitorClient/interface/EBSummaryClient.h>

using namespace cms;
using namespace edm;
using namespace std;

EBSummaryClient::EBSummaryClient(const ParameterSet& ps){

  // cloneME switch
  cloneME_ = ps.getUntrackedParameter<bool>("cloneME", true);

  // enableQT switch
  enableQT_ = ps.getUntrackedParameter<bool>("enableQT", true);

  // verbosity switch
  verbose_ = ps.getUntrackedParameter<bool>("verbose", false);

  // MonitorDaemon switch
  enableMonitorDaemon_ = ps.getUntrackedParameter<bool>("enableMonitorDaemon", true);

  // prefix to ME paths
  prefixME_ = ps.getUntrackedParameter<string>("prefixME", "");

  // vector of selected Super Modules (Defaults to all 36).
  superModules_.reserve(36);
  for ( unsigned int i = 1; i <= 36; i++ ) superModules_.push_back(i);
  superModules_ = ps.getUntrackedParameter<vector<int> >("superModules", superModules_);

  meIntegrity_      = 0;
  meOccupancy_      = 0;
  mePedestalOnline_ = 0;
  meLaserL1_        = 0;
  meLaserL1PN_      = 0;
  mePedestal_       = 0;
  mePedestalPN_     = 0;
  meTestPulse_      = 0;
  meTestPulsePN_    = 0;
  meGlobalSummary_  = 0;

  meCosmic_         = 0;
  meTiming_         = 0;
  meTriggerTowerEt_        = 0;
  meTriggerTowerEmulError_ = 0;

  qtg01_ = 0;
  qtg02_ = 0;
  qtg03_ = 0;
  qtg04_ = 0;
  qtg04PN_ = 0;
  qtg05_ = 0;
  qtg05PN_ = 0;
  qtg06_ = 0;
  qtg06PN_ = 0;

  qtg07_ = 0;
  qtg08_ = 0;
  qtg09_ = 0;
  qtg10_ = 0;

  qtg99_  = 0;

}

EBSummaryClient::~EBSummaryClient(){

}

void EBSummaryClient::beginJob(MonitorUserInterface* mui){

  mui_ = mui;
  dbe_ = mui->getBEInterface();

  if ( verbose_ ) cout << "EBSummaryClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

  if ( enableQT_ ) {

    Char_t qtname[200];

    sprintf(qtname, "EBIT summary quality test");
    qtg01_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBOT summary quality test");
    qtg02_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBPOT summary quality test");
    qtg03_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBLT summary quality test L1");
    qtg04_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBLT PN summary quality test L1");
    qtg04PN_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBPT summary quality test");
    qtg05_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBPT PN summary quality test");
    qtg05PN_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBTPT summary quality test");
    qtg06_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBTPT PN summary quality test");
    qtg06PN_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBCT summary quality test");
    qtg07_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBTMT summary quality test");
    qtg08_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBTTT Et summary quality test");
//     qtg09_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBTTT emulator error summary quality test");
    qtg10_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EB global summary quality test");
    qtg99_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    qtg01_->setMeanRange(1., 6.);
//    qtg02_->setMeanRange(1., 6.);
    qtg03_->setMeanRange(1., 6.);
    qtg04_->setMeanRange(1., 6.);
    qtg04PN_->setMeanRange(1., 6.);
    qtg05_->setMeanRange(1., 6.);
    qtg05PN_->setMeanRange(1., 6.);
    qtg06_->setMeanRange(1., 6.);
    qtg06PN_->setMeanRange(1., 6.);

//    qtg07_->setMeanRange(1., 6.);
    qtg08_->setMeanRange(1., 6.);
//    qtg09_->setMeanRange(1., 6.); 
    qtg10_->setMeanRange(1., 6.);

    qtg99_->setMeanRange(1., 6.);

    qtg01_->setErrorProb(1.00);
//    qtg02_->setErrorProb(1.00);
    qtg03_->setErrorProb(1.00);
    qtg04_->setErrorProb(1.00);
    qtg04PN_->setErrorProb(1.00);
    qtg05_->setErrorProb(1.00);
    qtg05PN_->setErrorProb(1.00);
    qtg06_->setErrorProb(1.00);
    qtg06PN_->setErrorProb(1.00);

//    qtg07_->setErrorProb(1.00);
    qtg08_->setErrorProb(1.00);
//    qtg09_->setErrorProb(1.00);
    qtg10_->setErrorProb(1.00);

    qtg99_->setErrorProb(1.00);

  }

}

void EBSummaryClient::beginRun(void){

  if ( verbose_ ) cout << "EBSummaryClient: beginRun" << endl;

  jevt_ = 0;

  this->setup();

  this->subscribe();

}

void EBSummaryClient::endJob(void) {

  if ( verbose_ ) cout << "EBSummaryClient: endJob, ievt = " << ievt_ << endl;

  this->unsubscribe();

  this->cleanup();

}

void EBSummaryClient::endRun(void) {

  if ( verbose_ ) cout << "EBSummaryClient: endRun, jevt = " << jevt_ << endl;

  this->unsubscribe();
  
  this->cleanup();

}

void EBSummaryClient::setup(void) {

  Char_t histo[200];

  dbe_->setCurrentFolder( "EcalBarrel/EBSummaryClient" );

  if ( meIntegrity_ ) dbe_->removeElement( meIntegrity_->getName() );
  sprintf(histo, "EBIT integrity quality summary");
  meIntegrity_ = dbe_->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if ( meOccupancy_ ) dbe_->removeElement( meOccupancy_->getName() );
  sprintf(histo, "EBOT occupancy summary");
  meOccupancy_ = dbe_->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if ( mePedestalOnline_ ) dbe_->removeElement( mePedestalOnline_->getName() );
  sprintf(histo, "EBPOT pedestal quality summary G12");
  mePedestalOnline_ = dbe_->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if ( meLaserL1_ ) dbe_->removeElement( meLaserL1_->getName() );
  sprintf(histo, "EBLT laser quality summary L1");
  meLaserL1_ = dbe_->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if ( meLaserL1PN_ ) dbe_->removeElement( meLaserL1PN_->getName() );
  sprintf(histo, "EBLT PN laser quality summary L1");
  meLaserL1PN_ = dbe_->book2D(histo, histo, 90, 0., 90., 20, -10., 10.);

  if( mePedestal_ ) dbe_->removeElement( mePedestal_->getName() );
  sprintf(histo, "EBPT pedestal quality summary");
  mePedestal_ = dbe_->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if( mePedestalPN_ ) dbe_->removeElement( mePedestalPN_->getName() );
  sprintf(histo, "EBPT PN pedestal quality summary");
  mePedestalPN_ = dbe_->book2D(histo, histo, 90, 0., 90., 20, -10, 10.);

  if( meTestPulse_ ) dbe_->removeElement( meTestPulse_->getName() );
  sprintf(histo, "EBTPT test pulse quality summary");
  meTestPulse_ = dbe_->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if( meTestPulsePN_ ) dbe_->removeElement( meTestPulsePN_->getName() );
  sprintf(histo, "EBTPT PN test pulse quality summary");
  meTestPulsePN_ = dbe_->book2D(histo, histo, 90, 0., 90., 20, -10., 10.);

  if( meCosmic_ ) dbe_->removeElement( meCosmic_->getName() );
  sprintf(histo, "EBCT cosmic quality summary");
  meCosmic_ = dbe_->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if( meTiming_ ) dbe_->removeElement( meTiming_->getName() );
  sprintf(histo, "EBTMT timing quality summary");
  meTiming_ = dbe_->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if( meTriggerTowerEt_ ) dbe_->removeElement( meTriggerTowerEt_->getName() );
  sprintf(histo, "EBTTT Et trigger tower quality summary");
  meTriggerTowerEt_ = dbe_->book3D(histo, histo, 72, 0., 72., 34, -17., 17., 256, 0., 256.);

  if( meTriggerTowerEmulError_ ) dbe_->removeElement( meTriggerTowerEmulError_->getName() );
  sprintf(histo, "EBTTT emulator error quality summary");
  meTriggerTowerEmulError_ = dbe_->book2D(histo, histo, 72, 0., 72., 34, -17., 17.);

  if( meGlobalSummary_ ) dbe_->removeElement( meGlobalSummary_->getName() );
  sprintf(histo, "EB global summary");
  meGlobalSummary_ = dbe_->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

}

void EBSummaryClient::cleanup(void) {

  dbe_->setCurrentFolder( "EcalBarrel/EBSummaryClient" );

  if ( meIntegrity_ ) dbe_->removeElement( meIntegrity_->getName() );
  meIntegrity_ = 0;

  if ( meOccupancy_ ) dbe_->removeElement( meOccupancy_->getName() );
  meOccupancy_ = 0;

  if ( mePedestalOnline_ ) dbe_->removeElement( mePedestalOnline_->getName() );
  mePedestalOnline_ = 0;

  if ( meLaserL1_ ) dbe_->removeElement( meLaserL1_->getName() );
  meLaserL1_ = 0;

  if ( meLaserL1PN_ ) dbe_->removeElement( meLaserL1PN_->getName() );
  meLaserL1PN_ = 0;

  if ( mePedestal_ ) dbe_->removeElement( mePedestal_->getName() );
  mePedestal_ = 0;

  if ( mePedestalPN_ ) dbe_->removeElement( mePedestalPN_->getName() );
  mePedestalPN_ = 0;

  if ( meTestPulse_ ) dbe_->removeElement( meTestPulse_->getName() );
  meTestPulse_ = 0;

  if ( meTestPulsePN_ ) dbe_->removeElement( meTestPulsePN_->getName() );
  meTestPulsePN_ = 0;

  if ( meCosmic_ ) dbe_->removeElement( meCosmic_->getName() );
  meCosmic_ = 0;

  if ( meTiming_ ) dbe_->removeElement( meTiming_->getName() );
  meTiming_ = 0;

  if ( meTriggerTowerEt_ ) dbe_->removeElement( meTriggerTowerEt_->getName() );
  meTriggerTowerEt_ = 0;

  if ( meTriggerTowerEmulError_ ) dbe_->removeElement( meTriggerTowerEmulError_->getName() );
  meTriggerTowerEmulError_ = 0;

  if ( meGlobalSummary_ ) dbe_->removeElement( meGlobalSummary_->getName() );
  meGlobalSummary_ = 0;

}

bool EBSummaryClient::writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov) {

  bool status = true;

//  UtilsClient::printBadChannels(qtg01_);
//  UtilsClient::printBadChannels(qtg02_);
//  UtilsClient::printBadChannels(qtg03_);
//  UtilsClient::printBadChannels(qtg04_);
//  UtilsClient::printBadChannels(qtg04PN_);
//  UtilsClient::printBadChannels(qtg05_);
//  UtilsClient::printBadChannels(qtg05PN_);
//  UtilsClient::printBadChannels(qtg06_);
//  UtilsClient::printBadChannels(qtg06PN_);

//  UtilsClient::printBadChannels(qtg07_);
//  UtilsClient::printBadChannels(qtg08_);
//  UtilsClient::printBadChannels(qtg09_);

//  UtilsClient::printBadChannels(qtg99_);

  return status;

}

void EBSummaryClient::subscribe(void){

  if ( verbose_ ) cout << "EBSummaryClient: subscribe" << endl;

  Char_t histo[200];

  sprintf(histo, "EcalBarrel/EBSummaryClient/EBIT integrity quality summary");
  if ( qtg01_ ) dbe_->useQTest(histo, qtg01_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBOT occupancy summary");
//  if ( qtg02_ ) dbe_->useQTest(histo, qtg02_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBPOT pedestal quality summary G12");
  if ( qtg03_ ) dbe_->useQTest(histo, qtg03_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBLT laser quality summary L1");
  if ( qtg04_ ) dbe_->useQTest(histo, qtg04_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBLT PN laser quality summary L1");
  if ( qtg04PN_ ) dbe_->useQTest(histo, qtg04PN_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBPT pedestal quality summary");
  if ( qtg05_ ) dbe_->useQTest(histo, qtg05_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBPT PN pedestal quality summary");
  if ( qtg05PN_ ) dbe_->useQTest(histo, qtg05PN_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBTPT test pulse quality summary");
  if ( qtg06_ ) dbe_->useQTest(histo, qtg06_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBTPT PN test pulse quality summary");
  if ( qtg06PN_ ) dbe_->useQTest(histo, qtg06PN_->getName());

  sprintf(histo, "EcalBarrel/EBSummaryClient/EBCT cosmic quality summary");
  if ( qtg07_ ) dbe_->useQTest(histo, qtg07_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBTMT timing quality summary");
  if ( qtg08_ ) dbe_->useQTest(histo, qtg08_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBTTT Et trigger tower quality summary");
//   if ( qtg09_ ) dbe_->useQTest(histo, qtg09_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBTTT emulator error quality summary");
  if ( qtg10_ ) dbe_->useQTest(histo, qtg10_->getName());

  sprintf(histo, "EcalBarrel/EBSummaryClient/EB global summary");
  if ( qtg99_ ) dbe_->useQTest(histo, qtg99_->getName());

}

void EBSummaryClient::subscribeNew(void){

}

void EBSummaryClient::unsubscribe(void){

  if ( verbose_ ) cout << "EBSummaryClient: unsubscribe" << endl;

}

void EBSummaryClient::softReset(void){

}

void EBSummaryClient::analyze(void){

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 == 0 ) {
    if ( verbose_ ) cout << "EBSummaryClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;
  }

  for ( int iex = 1; iex <= 170; iex++ ) {
    for ( int ipx = 1; ipx <= 360; ipx++ ) {

      meIntegrity_->setBinContent( ipx, iex, -1. );
      meOccupancy_->setBinContent( ipx, iex, 0. );
      mePedestalOnline_->setBinContent( ipx, iex, -1. );

      meLaserL1_->setBinContent( ipx, iex, -1. );
      mePedestal_->setBinContent( ipx, iex, -1. );
      meTestPulse_->setBinContent( ipx, iex, -1. );

      meCosmic_->setBinContent( ipx, iex, 0. );
      meTiming_->setBinContent( ipx, iex, -1. );

      meGlobalSummary_->setBinContent( ipx, iex, -1. );

    }
  }

  for ( int iex = 1; iex <= 20; iex++ ) {
    for ( int ipx = 1; ipx <= 90; ipx++ ) {
      
      meLaserL1PN_->setBinContent( ipx, iex, -1. );
      mePedestalPN_->setBinContent( ipx, iex, -1. );
      meTestPulsePN_->setBinContent( ipx, iex, -1. );

    }
  }

  for ( int iex = 1; iex <= 34; iex++ ) {
    for ( int ipx = 1; ipx <= 72; ipx++ ) {
      for ( int en = 1; en <= 256; en++ ) {
        meTriggerTowerEt_->setBinContent( ipx, iex, en, 0. );
      }
      meTriggerTowerEmulError_->setBinContent( ipx, iex, -1. ); 
    }
  }

  meIntegrity_->setEntries( 0 );
  meOccupancy_->setEntries( 0 );
  mePedestalOnline_->setEntries( 0 );

  meLaserL1_->setEntries( 0 );
  meLaserL1PN_->setEntries( 0 );
  mePedestal_->setEntries( 0 );
  mePedestalPN_->setEntries( 0 );
  meTestPulse_->setEntries( 0 );
  meTestPulsePN_->setEntries( 0 );

  meCosmic_->setEntries( 0 );
  meTiming_->setEntries( 0 );
  meTriggerTowerEt_->setEntries( 0 );
  meTriggerTowerEmulError_->setEntries( 0 );

  meGlobalSummary_->setEntries( 0 );

  for ( unsigned int i=0; i<clients_.size(); i++ ) {

    EBIntegrityClient* ebic = dynamic_cast<EBIntegrityClient*>(clients_[i]);
    EBPedestalOnlineClient* ebpoc = dynamic_cast<EBPedestalOnlineClient*>(clients_[i]);

    EBLaserClient* eblc = dynamic_cast<EBLaserClient*>(clients_[i]);
    EBPedestalClient* ebpc = dynamic_cast<EBPedestalClient*>(clients_[i]);
    EBTestPulseClient* ebtpc = dynamic_cast<EBTestPulseClient*>(clients_[i]);

    EBCosmicClient* ebcc = dynamic_cast<EBCosmicClient*>(clients_[i]);
    EBTimingClient* ebtmc = dynamic_cast<EBTimingClient*>(clients_[i]);
    EBTriggerTowerClient* ebtttc = dynamic_cast<EBTriggerTowerClient*>(clients_[i]);

    MonitorElement* me;
    MonitorElement *me_01, *me_02, *me_03;
    MonitorElement *me_04, *me_05;
    TH2F* h2;
    TProfile2D* h2d;
    TH3F* h3;

    // fill the gain value priority map<id,priority>
    std::map<float,float> priority;
    priority.insert( make_pair(0,3) );
    priority.insert( make_pair(1,1) );
    priority.insert( make_pair(2,2) );
    priority.insert( make_pair(3,2) );
    priority.insert( make_pair(4,3) );
    priority.insert( make_pair(5,1) );
          
    for ( unsigned int i=0; i<superModules_.size(); i++ ) {

      int ism = superModules_[i];

      for ( int ie = 1; ie <= 85; ie++ ) {
        for ( int ip = 1; ip <= 20; ip++ ) {

          if ( ebic ) {

            me = ebic->meg01_[ism-1];

            if ( me ) {

              float xval = me->getBinContent( ie, ip );

              int iex;
              int ipx;

              if ( ism <= 18 ) {
                iex = 1+(85-ie);
                ipx = ip+20*(ism-1);
              } else {
                iex = 85+ie;
                ipx = 1+(20-ip)+20*(ism-19);
              }

              meIntegrity_->setBinContent( ipx, iex, xval );

            }

            h2 = ebic->h_[ism-1];

            if ( h2 ) {

              float xval = h2->GetBinContent( ie, ip );

              int iex;
              int ipx;

              if ( ism <= 18 ) {
                iex = 1+(85-ie);
                ipx = ip+20*(ism-1);
              } else {
                iex = 85+ie;
                ipx = 1+(20-ip)+20*(ism-19);
              }

              meOccupancy_->setBinContent( ipx, iex, xval );

            }

          }

          if ( ebpoc ) {

            me = ebpoc->meg03_[ism-1];

            if ( me ) {

              float xval = me->getBinContent( ie, ip );

              int iex;
              int ipx;

              if ( ism <= 18 ) {
                iex = 1+(85-ie);
                ipx = ip+20*(ism-1);
              } else {
                iex = 85+ie;
                ipx = 1+(20-ip)+20*(ism-19);
              }

              mePedestalOnline_->setBinContent( ipx, iex, xval );

            }

          }

          if ( eblc ) {

            me = eblc->meg01_[ism-1];

            if ( me ) {

              float xval = me->getBinContent( ie, ip );

              int iex;
              int ipx;

              if ( ism <= 18 ) {
                iex = 1+(85-ie);
                ipx = ip+20*(ism-1);
              } else {
                iex = 85+ie;
                ipx = 1+(20-ip)+20*(ism-19);
              }

              if ( me->getEntries() != 0 ) {
                meLaserL1_->setBinContent( ipx, iex, xval );
              }

            }

          }

          if ( ebpc ) {
            
            me_01 = ebpc->meg01_[ism-1];
            me_02 = ebpc->meg02_[ism-1];
            me_03 = ebpc->meg03_[ism-1];
            
            if (me_01 && me_02 && me_03 ) {
              float xval=2;
              float val_01=me_01->getBinContent(ie,ip);
              float val_02=me_02->getBinContent(ie,ip);
              float val_03=me_03->getBinContent(ie,ip);

              std::vector<float> maskedVal, unmaskedVal;
              (val_01>2) ? maskedVal.push_back(val_01) : unmaskedVal.push_back(val_01);
              (val_02>2) ? maskedVal.push_back(val_02) : unmaskedVal.push_back(val_02);
              (val_03>2) ? maskedVal.push_back(val_03) : unmaskedVal.push_back(val_03);
              
              float brightColor=-1, darkColor=-1;
              float maxPriority=-1;
              std::vector<float>::const_iterator Val;
              for(Val=unmaskedVal.begin(); Val<unmaskedVal.end(); Val++) {
                if(priority[*Val]>maxPriority) brightColor=*Val;
              }
              maxPriority=-1;
              for(Val=maskedVal.begin(); Val<maskedVal.end(); Val++) {
                if(priority[*Val]>maxPriority) darkColor=*Val;
              }
              if(unmaskedVal.size()==3)  xval = brightColor;
              else if(maskedVal.size()==3)  xval = darkColor;
              else {
                if(brightColor==1 && darkColor==5) xval = 5;
                else xval = brightColor;
              }

              int iex;
              int ipx;
              
              if ( ism <= 18 ) {
                iex = 1+(85-ie);
                ipx = ip+20*(ism-1);
              } else {
                iex = 85+ie;
                ipx = 1+(20-ip)+20*(ism-19);
              }

              if ( me_01->getEntries() != 0 && me_02->getEntries() != 0 && me_03->getEntries() != 0 ) {
                mePedestal_->setBinContent( ipx, iex, xval );
              }

            }

          }

          if ( ebtpc ) {
            
            me_01 = ebtpc->meg01_[ism-1];
            me_02 = ebtpc->meg02_[ism-1];
            me_03 = ebtpc->meg03_[ism-1];
            
            if (me_01 && me_02 && me_03 ) {
              float xval=2;
              float val_01=me_01->getBinContent(ie,ip);
              float val_02=me_02->getBinContent(ie,ip);
              float val_03=me_03->getBinContent(ie,ip);

              std::vector<float> maskedVal, unmaskedVal;
              (val_01>2) ? maskedVal.push_back(val_01) : unmaskedVal.push_back(val_01);
              (val_02>2) ? maskedVal.push_back(val_02) : unmaskedVal.push_back(val_02);
              (val_03>2) ? maskedVal.push_back(val_03) : unmaskedVal.push_back(val_03);

              float brightColor=-1, darkColor=-1;
              float maxPriority=-1;
              std::vector<float>::const_iterator Val;
              for(Val=unmaskedVal.begin(); Val<unmaskedVal.end(); Val++) {
                if(priority[*Val]>maxPriority) brightColor=*Val;
              }
              maxPriority=-1;
              for(Val=maskedVal.begin(); Val<maskedVal.end(); Val++) {
                if(priority[*Val]>maxPriority) darkColor=*Val;
              }
              if(unmaskedVal.size()==3) xval = brightColor;
              else if(maskedVal.size()==3) xval = darkColor;
              else {
                if(brightColor==1 && darkColor==5) xval = 5;
                else xval = brightColor;
              }
              
              int iex;
              int ipx;
              
              if ( ism <= 18 ) {
                iex = 1+(85-ie);
                ipx = ip+20*(ism-1);
              } else {
                iex = 85+ie;
                ipx = 1+(20-ip)+20*(ism-19);
              }

              if ( me_01->getEntries() != 0 && me_02->getEntries() != 0 && me_03->getEntries() != 0 ) {
                meTestPulse_->setBinContent( ipx, iex, xval );
              }

            }
            
          }

          if ( ebcc ) {

            h2d = ebcc->h01_[ism-1];

            if ( h2d ) {

              float xval = h2d->GetBinContent( ie, ip );

              int iex;
              int ipx;

              if ( ism <= 18 ) {
                iex = 1+(85-ie);
                ipx = ip+20*(ism-1);
              } else {
                iex = 85+ie;
                ipx = 1+(20-ip)+20*(ism-19);
              }

              meCosmic_->setBinContent( ipx, iex, xval );

            }

          }

          if ( ebtmc ) {

            me = ebtmc->meg01_[ism-1];

            if ( me ) {

              float xval = me->getBinContent( ie, ip );

              int iex;
              int ipx;

              if ( ism <= 18 ) {
                iex = 1+(85-ie);
                ipx = ip+20*(ism-1);
              } else {
                iex = 85+ie;
                ipx = 1+(20-ip)+20*(ism-19);
              }

              meTiming_->setBinContent( ipx, iex, xval );

            }

          }

        }
      }

      for (int ie = 1; ie <= 17; ie++ ) {
        for (int ip = 1; ip <= 4; ip++ ) {
          
          if ( ebtttc ) {
              
            h3 = ebtttc->h01_[ism-1];

            bool hasRealDigi = false;

            if ( h3 ) {

              for ( int en = 1; en <= 256; en++ ) {
                
                float xval = h3->GetBinContent( ie, ip, en );
                
                if(xval!=0) hasRealDigi = true; 
                
                int iex;
                int ipx;
                
                if ( ism <= 18 ) {
                  iex = 1+(17-ie);
                  ipx = ip+4*(ism-1);
                } else {
                  iex = 17+ie;
                  ipx = 1+(4-ip)+4*(ism-19);
                }
                
                meTriggerTowerEt_->setBinContent( ipx, iex, en, xval );

              }
            }

            h2 = ebtttc->l01_[ism-1];

            if ( h2 ) {

              float xval = -1;
              float emulErrorVal = h2->GetBinContent( ie, ip );

              if(!hasRealDigi) xval = 2;
              else if(hasRealDigi && emulErrorVal!=0) xval = 0;
              else xval = 1;

              int iex;
              int ipx;

              if ( ism <= 18 ) {
                iex = 1+(17-ie);   
                ipx = ip+4*(ism-1);
              } else {
                iex = 17+ie;
                ipx = 1+(4-ip)+4*(ism-19);
              }

              meTriggerTowerEmulError_->setBinContent( ipx, iex, xval );

            }

          }
        } 
      }

      // PN's summaries
      for( int i = 1; i <= 10; i++ ) {
        for( int j = 1; j <= 5; j++ ) { 

          if ( ebpc ) {

            me_04 = ebpc->meg04_[ism-1];
            me_05 = ebpc->meg05_[ism-1];
            
            if( me_04 && me_05) {
              float xval=2;
              float val_04=me_04->getBinContent(i,1);
              float val_05=me_05->getBinContent(i,1);
              
              std::vector<float> maskedVal, unmaskedVal;
              (val_04>2) ? maskedVal.push_back(val_04) : unmaskedVal.push_back(val_04);
              (val_05>2) ? maskedVal.push_back(val_05) : unmaskedVal.push_back(val_05);
              
              float brightColor=-1, darkColor=-1;
              float maxPriority=-1;
              
              std::vector<float>::const_iterator Val;
              for(Val=unmaskedVal.begin(); Val<unmaskedVal.end(); Val++) {
                if(priority[*Val]>maxPriority) brightColor=*Val;
              }
              maxPriority=-1;
              for(Val=maskedVal.begin(); Val<maskedVal.end(); Val++) {
                if(priority[*Val]>maxPriority) darkColor=*Val;
              }
              if(unmaskedVal.size()==2)  xval = brightColor;
              else if(maskedVal.size()==2)  xval = darkColor;
              else {
                if(brightColor==1 && darkColor==5) xval = 5;
                else xval = brightColor;
              }
              
              int iex;
              int ipx;
              
              if(ism<=18) {
                iex = i;
                ipx = j+5*(ism-1);
              }
              else {
                iex = i+10;
                ipx = j+5*(ism-19);
              }

              if ( me_04->getEntries() != 0 && me_05->getEntries() != 0 ) {
                mePedestalPN_->setBinContent( ipx, iex, xval );
              }

            }

          }

          if ( ebtpc ) {

            me_04 = ebtpc->meg04_[ism-1];
            me_05 = ebtpc->meg05_[ism-1];
            
            if( me_04 && me_05) {
              float xval=2;
              float val_04=me_04->getBinContent(i,1);
              float val_05=me_05->getBinContent(i,1);
              
              std::vector<float> maskedVal, unmaskedVal;
              (val_04>2) ? maskedVal.push_back(val_04) : unmaskedVal.push_back(val_04);
              (val_05>2) ? maskedVal.push_back(val_05) : unmaskedVal.push_back(val_05);
              
              float brightColor=-1, darkColor=-1;
              float maxPriority=-1;
              
              std::vector<float>::const_iterator Val;
              for(Val=unmaskedVal.begin(); Val<unmaskedVal.end(); Val++) {
                if(priority[*Val]>maxPriority) brightColor=*Val;
              }
              maxPriority=-1;
              for(Val=maskedVal.begin(); Val<maskedVal.end(); Val++) {
                if(priority[*Val]>maxPriority) darkColor=*Val;
              }
              if(unmaskedVal.size()==2)  xval = brightColor;
              else if(maskedVal.size()==2)  xval = darkColor;
              else {
                if(brightColor==1 && darkColor==5) xval = 5;
                else xval = brightColor;
              }
              
              int iex;
              int ipx;
              
              if(ism<=18) {
                iex = i;
                ipx = j+5*(ism-1);
              }
              else {
                iex = i+10;
                ipx = j+5*(ism-19);
              }

              if ( me_04->getEntries() != 0 && me_05->getEntries() != 0 ) {
                meTestPulsePN_->setBinContent( ipx, iex, xval );
              }

            }
          }

          if ( eblc ) {

            me = eblc->meg09_[ism-1];

            if( me ) {

              float xval = me->getBinContent(i,1);
              
              int iex;
              int ipx;
              
              if(ism<=18) {
                iex = i;
                ipx = j+5*(ism-1);
              }
              else {
                iex = i+10;
                ipx = j+5*(ism-19);
              }

              if ( me->getEntries() != 0 && me->getEntries() != 0 ) {
                meLaserL1PN_->setBinContent( ipx, iex, xval );
              }

            }

          }
          
        }
      }

    } // loop on SM

  } // loop on clients

  // The global-summary
  // right now a summary of Integrity and PO
  for ( int iex = 1; iex <= 170; iex++ ) {
    for ( int ipx = 1; ipx <= 360; ipx++ ) {

      if(meIntegrity_ && mePedestalOnline_) {

        float xval = -1;
        float val_in = meIntegrity_->getBinContent(ipx,iex);
        float val_po = mePedestalOnline_->getBinContent(ipx,iex);

        // turn each dark color (masked channel) to bright green
        if(val_in>2) val_in=1;
        if(val_po>2) val_po=1;

        // -1 = unknown
        //  0 = red
        //  1 = green
        //  2 = yellow

        if(val_in==-1) xval=-1;
        else if(val_in==0) xval=0;
        else if(val_po==0) xval=0;
        else if(val_in==2) xval=2;
        else if(val_po==2) xval=2;
        else xval=1;

        meGlobalSummary_->setBinContent( ipx, iex, xval );

      }

    }
  }

}

void EBSummaryClient::htmlOutput(int run, string htmlDir, string htmlName){

  cout << "Preparing EBSummaryClient html output ..." << endl;

  ofstream htmlFile;

  htmlFile.open((htmlDir + htmlName).c_str());

  // html page header
  htmlFile << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">  " << endl;
  htmlFile << "<html>  " << endl;
  htmlFile << "<head>  " << endl;
  htmlFile << "  <meta content=\"text/html; charset=ISO-8859-1\"  " << endl;
  htmlFile << " http-equiv=\"content-type\">  " << endl;
  htmlFile << "  <title>Monitor:Summary output</title> " << endl;
  htmlFile << "</head>  " << endl;
  htmlFile << "<style type=\"text/css\"> td { font-weight: bold } </style>" << endl;
  htmlFile << "<body>  " << endl;
  //htmlFile << "<br>  " << endl;
  htmlFile << "<a name=""top""></a>" << endl;
  htmlFile << "<h2>Run:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
  htmlFile << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">" << run << "</span></h2>" << endl;
  htmlFile << "<h2>Monitoring task:&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">SUMMARY</span></h2> " << endl;
  htmlFile << "<hr>" << endl;
  htmlFile << "<table border=1><tr><td bgcolor=red>channel has problems in this task</td>" << endl;
  htmlFile << "<td bgcolor=lime>channel has NO problems</td>" << endl;
  htmlFile << "<td bgcolor=yellow>channel is missing</td></table>" << endl;
  htmlFile << "<br>" << endl;

  // Produce the plots to be shown as .png files from existing histograms

  const int csize = 400;

//  const double histMax = 1.e15;

  int pCol3[6] = { 301, 302, 303, 304, 305, 306 };
  int pCol4[10];
  for ( int i = 0; i < 10; i++ ) pCol4[i] = 401+i;

  // dummy histogram labelling the SM's
  TH2C labelGrid("labelGrid","label grid for SM", 18, 0., 360., 2, -85., 85.);
  for ( short sm=0; sm<36; sm++ ) {
    int x = 1 + sm%18;
    int y = 1 + sm/18;
    labelGrid.SetBinContent(x, y, Numbers::iEB(sm+1));
  }
  labelGrid.SetMarkerSize(2);
  labelGrid.SetMinimum(-18.01);

  TH2C labelGridPN("labelGridPN","label grid for SM", 18, 0., 90., 2, -10., 10.);
  for ( short sm=0; sm<36; sm++ ) {
    int x = 1 + sm%18;
    int y = 1 + sm/18;
    labelGridPN.SetBinContent(x, y, Numbers::iEB(sm+1));
  }
  labelGridPN.SetMarkerSize(4);
  labelGridPN.SetMinimum(-18.01);

  TH2C labelGridTT("labelGridTT","label grid for SM", 18, 0., 72., 2, -17., 17.);
  for ( short sm=0; sm<36; sm++ ) {
    int x = 1 + sm%18;
    int y = 1 + sm/18;
    labelGridTT.SetBinContent(x, y, Numbers::iEB(sm+1));
  }
  labelGridTT.SetMarkerSize(2);
  labelGridTT.SetMinimum(-18.01);

  string imgNameMapI, imgNameMapO;
  string imgNameMapPO;
  string imgNameMapLL1, imgNameMapLL1_PN;
  string imgNameMapP, imgNameMapP_PN;
  string imgNameMapTP, imgNameMapTP_PN;
  string imgNameMapC;
  string imgNameMapTM;
  string imgNameMapTTEt;
  string imgNameMapTTEmulError;
  string imgName, meName;
  string imgNameMapGS;

  TCanvas* cMap = new TCanvas("cMap", "Temp", int(360./170.*csize), csize);
  TCanvas* cMapPN = new TCanvas("cMapPN", "Temp", int(360./170.*csize), int(20./90.*360./170.*csize));

  float saveHeigth = gStyle->GetTitleH();
  gStyle->SetTitleH(0.07);
  float saveFontSize = gStyle->GetTitleFontSize();
  gStyle->SetTitleFontSize(14);
  float saveTitleOffset = gStyle->GetTitleX();

  TH2F* obj2f;
  TH3F* obj3f;
  TProfile2D* obj2p;

  imgNameMapI = "";

  gStyle->SetPaintTextFormat("+g");

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meIntegrity_ );

  if ( obj2f ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapI = meName + ".png";
    imgName = htmlDir + imgNameMapI;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapO = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meOccupancy_ );

  if ( obj2f ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapO = meName + ".png";
    imgName = htmlDir + imgNameMapO;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(10, pCol4);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(0.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->GetZaxis()->SetLabelSize(0.03);
    obj2f->Draw("colz");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapPO = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( mePedestalOnline_ );

  if ( obj2f && obj2f->GetEntries() != 0 ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapPO = meName + ".png";
    imgName = htmlDir + imgNameMapPO;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapLL1 = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meLaserL1_ );

  if ( obj2f && obj2f->GetEntries() != 0 ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapLL1 = meName + ".png";
    imgName = htmlDir + imgNameMapLL1;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapLL1_PN = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meLaserL1PN_ );

  if ( obj2f && obj2f->GetEntries() != 0 ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapLL1_PN = meName + ".png";
    imgName = htmlDir + imgNameMapLL1_PN;

    cMapPN->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2, kFALSE);
    cMapPN->SetGridx();
    cMapPN->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.07);
    obj2f->GetYaxis()->SetLabelSize(0.07);
    gStyle->SetTitleX(0.15);
    obj2f->Draw("col");
    labelGridPN.Draw("text,same");
    cMapPN->Update();
    cMapPN->SaveAs(imgName.c_str());
    gStyle->SetTitleX(saveTitleOffset);
  }

  imgNameMapP = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( mePedestal_ );

  if ( obj2f && obj2f->GetEntries() != 0 ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapP = meName + ".png";
    imgName = htmlDir + imgNameMapP;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapP_PN = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( mePedestalPN_ );

  if ( obj2f && obj2f->GetEntries() != 0 ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapP_PN = meName + ".png";
    imgName = htmlDir + imgNameMapP_PN;

    cMapPN->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2, kFALSE);
    cMapPN->SetGridx();
    cMapPN->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.07);
    obj2f->GetYaxis()->SetLabelSize(0.07);
    gStyle->SetTitleX(0.15);
    obj2f->Draw("col");
    labelGridPN.Draw("text,same");
    cMapPN->Update();
    cMapPN->SaveAs(imgName.c_str());
    gStyle->SetTitleX(saveTitleOffset);
  }


  imgNameMapTP = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meTestPulse_ );

  if ( obj2f && obj2f->GetEntries() != 0 ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapTP = meName + ".png";
    imgName = htmlDir + imgNameMapTP;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapTP_PN = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meTestPulsePN_ );

  if ( obj2f && obj2f->GetEntries() != 0 ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapTP_PN = meName + ".png";
    imgName = htmlDir + imgNameMapTP_PN;

    cMapPN->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMapPN->SetGridx();
    cMapPN->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.07);
    obj2f->GetYaxis()->SetLabelSize(0.07);
    gStyle->SetTitleX(0.15);
    obj2f->Draw("col");
    labelGridPN.Draw("text,same");
    cMapPN->Update();
    cMapPN->SaveAs(imgName.c_str());
    gStyle->SetTitleX(saveTitleOffset);
  }

  imgNameMapC = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meCosmic_ );

  if ( obj2f && obj2f->GetEntries() != 0 ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapC = meName + ".png";
    imgName = htmlDir + imgNameMapC;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(10, pCol4);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(0.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->GetZaxis()->SetLabelSize(0.03);
    obj2f->Draw("colz");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapTM = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meTiming_ );
  
  if ( obj2f && obj2f->GetEntries() != 0 ) {
  
    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" ); 
      }
    }
    imgNameMapTM = meName + ".png";
    imgName = htmlDir + imgNameMapTM;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapTTEmulError = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meTriggerTowerEmulError_ );
  
  if ( obj2f && obj2f->GetEntries() != 0 ) {
  
    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" ); 
      }
    }
    imgNameMapTTEmulError = meName + ".png";
    imgName = htmlDir + imgNameMapTTEmulError;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->Draw("col");
    labelGridTT.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapTTEt = "";

  obj3f = 0;
  obj3f = UtilsClient::getHisto<TH3F*>( meTriggerTowerEt_ );

  if ( obj3f && obj3f->GetEntries() != 0 ) {
  
    meName = obj3f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" ); 
      }
    }
    imgNameMapTTEt = meName + ".png";
    imgName = htmlDir + imgNameMapTTEt;

    obj2p = obj3f->Project3DProfile("yx");

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(10, pCol4);
    obj2p->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2p->GetYaxis()->SetNdivisions(2);

    std::string projname(obj2p->GetName());
    string::size_type loc = projname.find( "_pyx", 0 );
    projname.replace( loc, projname.length(), "");
    obj2p->SetTitle(projname.c_str());

    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(0.0);
    obj2p->GetXaxis()->SetLabelSize(0.03);
    obj2p->GetYaxis()->SetLabelSize(0.03);
    obj2p->GetZaxis()->SetLabelSize(0.03); 
    obj2p->Draw("colz");
    labelGridTT.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());
    delete obj2p;
  }

  imgNameMapGS = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( meGlobalSummary_ );

  if ( obj2f ) {

    meName = obj2f->GetName();

    for ( unsigned int i = 0; i < meName.size(); i++ ) {
      if ( meName.substr(i, 1) == " " )  {
        meName.replace(i, 1 ,"_" );
      }
    }
    imgNameMapGS = meName + ".png";
    imgName = htmlDir + imgNameMapGS;

    cMap->cd();
    gStyle->SetOptStat(" ");
    gStyle->SetPalette(6, pCol3);
    obj2f->GetXaxis()->SetNdivisions(18, kFALSE);
    obj2f->GetYaxis()->SetNdivisions(2);
    cMap->SetGridx();
    cMap->SetGridy();
    obj2f->SetMinimum(-0.00000001);
    obj2f->SetMaximum(6.0);
    obj2f->GetXaxis()->SetLabelSize(0.03);
    obj2f->GetYaxis()->SetLabelSize(0.03);
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }


  gStyle->SetPaintTextFormat();

  if ( imgNameMapI.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapI << "\" usemap=\"#Integrity\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapO.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapO << "\" usemap=\"#Occupancy\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapPO.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapPO << "\" usemap=\"#PedestalOnline\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapLL1.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapLL1 << "\" usemap=\"#LaserL1\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapLL1_PN.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapLL1_PN << "\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapP.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapP << "\" usemap=\"#Pedestal\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapP_PN.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapP_PN << "\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapTP.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapTP << "\" usemap=\"#TestPulse\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapTP_PN.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapTP_PN << "\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapC.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapC << "\" usemap=\"#Cosmic\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapTM.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapTM << "\" usemap=\"#Timing\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapTTEmulError.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapTTEmulError << "\" usemap=\"#TriggerTower\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  if ( imgNameMapTTEt.size() != 0 ) {
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;
    htmlFile << "<td><img src=\"" << imgNameMapTTEt << "\" usemap=\"#TriggerTower\" border=0></td>" << endl;
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;
  }

  delete cMap;
  delete cMapPN;

  gStyle->SetPaintTextFormat();

  if ( imgNameMapI.size() != 0 ) this->writeMap( htmlFile, "Integrity" );
  if ( imgNameMapO.size() != 0 ) this->writeMap( htmlFile, "Occupancy" );
  if ( imgNameMapPO.size() != 0 ) this->writeMap( htmlFile, "PedestalOnline" );
  if ( imgNameMapLL1.size() != 0 ) this->writeMap( htmlFile, "LaserL1" );
  if ( imgNameMapP.size() != 0 ) this->writeMap( htmlFile, "Pedestal" );
  if ( imgNameMapTP.size() != 0 ) this->writeMap( htmlFile, "TestPulse" );

  if ( imgNameMapC.size() != 0 ) this->writeMap( htmlFile, "Cosmic" );
  if ( imgNameMapTM.size() != 0 ) this->writeMap( htmlFile, "Timing" );
  if ( imgNameMapTTEt.size() != 0 ) this->writeMap( htmlFile, "TriggerTower" );
  if ( imgNameMapTTEmulError.size() != 0 ) this->writeMap( htmlFile, "TriggerTower" );

  // html page footer
  htmlFile << "</body> " << endl;
  htmlFile << "</html> " << endl;

  htmlFile.close();

  gStyle->SetTitleH( saveHeigth );
  gStyle->SetTitleFontSize( saveFontSize );

}

void EBSummaryClient::writeMap( std::ofstream& hf, std::string mapname ) {

  std::map<std::string, std::string> refhtml;
  refhtml["Integrity"] = "EBIntegrityClient.html";
  refhtml["Occupancy"] = "EBIntegrityClient.html";
  refhtml["PedestalOnline"] = "EBPedestalOnlineClient.html";
  refhtml["LaserL1"] = "EBLaserClient.html";
  refhtml["Pedestal"] = "EBPedestalClient.html";
  refhtml["TestPulse"] = "EBTestPulseClient.html";

  refhtml["Cosmic"] = "EBCosmicClient.html";
  refhtml["Timing"] = "EBTimingClient.html";
  refhtml["TriggerTower"] = "EBTriggerTowerClient.html";

  const int A0 =  85;
  const int A1 = 759;
  const int B0 =  35;
  const int B1 = 334;

  hf << "<map name=\"" << mapname << "\">" << std::endl;
  for( unsigned int sm=0; sm<superModules_.size(); sm++ ) {
    int i=(superModules_[sm]-1)/18;
    int j=(superModules_[sm]-1)%18;
    int x0 = A0 + (A1-A0)*j/18;
    int x1 = A0 + (A1-A0)*(j+1)/18;
    int y0 = B0 + (B1-B0)*(1-i)/2;
    int y1 = B0 + (B1-B0)*((1-i)+1)/2;
    hf << "<area title=\"" << Numbers::sEB(superModules_[sm]).c_str()
       << "\" shape=\"rect\" href=\"" << refhtml[mapname]
       << "#" << Numbers::sEB(superModules_[sm]).c_str()
       << "\" coords=\"" << x0 << ", " << y0 << ", "
                         << x1 << ", " << y1 << "\">"
       << std::endl;
  }
  hf << "</map>" << std::endl;

}

