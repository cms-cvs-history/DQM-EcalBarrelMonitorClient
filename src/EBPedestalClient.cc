/*
 * \file EBPedestalClient.cc
 *
 * $Date: 2006/07/05 07:52:38 $
 * $Revision: 1.86 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <memory>
#include <iostream>
#include <fstream>

#include "TStyle.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DQMServices/Core/interface/DaqMonitorBEInterface.h"

#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "OnlineDB/EcalCondDB/interface/RunTag.h"
#include "OnlineDB/EcalCondDB/interface/RunIOV.h"

#include "OnlineDB/EcalCondDB/interface/MonPedestalsDat.h"

#include "OnlineDB/EcalCondDB/interface/MonPNPedDat.h"

#include <DQM/EcalBarrelMonitorClient/interface/EBPedestalClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBMUtilsClient.h>

EBPedestalClient::EBPedestalClient(const ParameterSet& ps){

  // collateSources switch
  collateSources_ = ps.getUntrackedParameter<bool>("collateSources", false);

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
  for ( unsigned int i = 1; i < 37; i++ ) superModules_.push_back(i);
  superModules_ = ps.getUntrackedParameter<vector<int> >("superModules", superModules_);

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    h01_[ism-1] = 0;
    h02_[ism-1] = 0;
    h03_[ism-1] = 0;

    j01_[ism-1] = 0;
    j02_[ism-1] = 0;
    j03_[ism-1] = 0;

    k01_[ism-1] = 0;
    k02_[ism-1] = 0;
    k03_[ism-1] = 0;

    i01_[ism-1] = 0;
    i02_[ism-1] = 0;

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    meg01_[ism-1] = 0;
    meg02_[ism-1] = 0;
    meg03_[ism-1] = 0;

    mep01_[ism-1] = 0;
    mep02_[ism-1] = 0;
    mep03_[ism-1] = 0;

    mer01_[ism-1] = 0;
    mer02_[ism-1] = 0;
    mer03_[ism-1] = 0;

    mes01_[ism-1] = 0;
    mes02_[ism-1] = 0;
    mes03_[ism-1] = 0;

    met01_[ism-1] = 0;
    met02_[ism-1] = 0;
    met03_[ism-1] = 0;

    qth01_[ism-1] = 0;
    qth02_[ism-1] = 0;
    qth03_[ism-1] = 0;

  }

  expectedMean_[0] = 200.0;
  expectedMean_[1] = 200.0;
  expectedMean_[2] = 200.0;

  discrepancyMean_[0] = 25.0;
  discrepancyMean_[1] = 25.0;
  discrepancyMean_[2] = 25.0;

  RMSThreshold_[0] = 1.0;
  RMSThreshold_[1] = 1.2;
  RMSThreshold_[2] = 2.0;

}

EBPedestalClient::~EBPedestalClient(){

}

void EBPedestalClient::beginJob(MonitorUserInterface* mui){

  mui_ = mui;

  if ( verbose_ ) cout << "EBPedestalClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

  if ( enableQT_ ) {

    Char_t qtname[200];

    for ( unsigned int i=0; i<superModules_.size(); i++ ) {

      int ism = superModules_[i];

      sprintf(qtname, "EBPT quality SM%02d G01", ism);
      qth01_[ism-1] = dynamic_cast<MEContentsProf2DWithinRangeROOT*> (mui_->createQTest(ContentsProf2DWithinRangeROOT::getAlgoName(), qtname));

      sprintf(qtname, "EBPT quality SM%02d G06", ism);
      qth02_[ism-1] = dynamic_cast<MEContentsProf2DWithinRangeROOT*> (mui_->createQTest(ContentsProf2DWithinRangeROOT::getAlgoName(), qtname));

      sprintf(qtname, "EBPT quality SM%02d G12", ism);
      qth03_[ism-1] = dynamic_cast<MEContentsProf2DWithinRangeROOT*> (mui_->createQTest(ContentsProf2DWithinRangeROOT::getAlgoName(), qtname));
  
      qth01_[ism-1]->setMeanRange(expectedMean_[0] - discrepancyMean_[0], expectedMean_[0] + discrepancyMean_[0]);
      qth02_[ism-1]->setMeanRange(expectedMean_[1] - discrepancyMean_[1], expectedMean_[1] + discrepancyMean_[1]);
      qth03_[ism-1]->setMeanRange(expectedMean_[2] - discrepancyMean_[2], expectedMean_[2] + discrepancyMean_[2]);

      qth01_[ism-1]->setRMSRange(0.0, RMSThreshold_[0]);
      qth02_[ism-1]->setRMSRange(0.0, RMSThreshold_[1]);
      qth03_[ism-1]->setRMSRange(0.0, RMSThreshold_[2]);

      qth01_[ism-1]->setMinimumEntries(10*1700);
      qth02_[ism-1]->setMinimumEntries(10*1700);
      qth03_[ism-1]->setMinimumEntries(10*1700);

      qth01_[ism-1]->setErrorProb(1.00);
      qth02_[ism-1]->setErrorProb(1.00);
      qth03_[ism-1]->setErrorProb(1.00);

    }

  }

}

void EBPedestalClient::beginRun(void){

  if ( verbose_ ) cout << "EBPedestalClient: beginRun" << endl;

  jevt_ = 0;

  this->setup();

  this->subscribe();

}

void EBPedestalClient::endJob(void) {

  if ( verbose_ ) cout << "EBPedestalClient: endJob, ievt = " << ievt_ << endl;

  this->unsubscribe();

  this->cleanup();

}

void EBPedestalClient::endRun(void) {

  if ( verbose_ ) cout << "EBPedestalClient: endRun, jevt = " << jevt_ << endl;

  this->unsubscribe();

  this->cleanup();

}

void EBPedestalClient::setup(void) {

  Char_t histo[200];

  mui_->setCurrentFolder( "EcalBarrel/EBPedestalClient" );
  DaqMonitorBEInterface* bei = mui_->getBEInterface();

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg01_[ism-1] ) bei->removeElement( meg01_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal quality G01 SM%02d", ism);
    meg01_[ism-1] = bei->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);
    if ( meg02_[ism-1] ) bei->removeElement( meg02_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal quality G06 SM%02d", ism);
    meg02_[ism-1] = bei->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);
    if ( meg03_[ism-1] ) bei->removeElement( meg03_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal quality G12 SM%02d", ism);
    meg03_[ism-1] = bei->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);

    if ( mep01_[ism-1] ) bei->removeElement( mep01_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal mean G01 SM%02d", ism);
    mep01_[ism-1] = bei->book1D(histo, histo, 100, 150., 250.);
    if ( mep02_[ism-1] ) bei->removeElement( mep02_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal mean G06 SM%02d", ism);
    mep02_[ism-1] = bei->book1D(histo, histo, 100, 150., 250.);
    if ( mep03_[ism-1] ) bei->removeElement( mep03_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal mean G12 SM%02d", ism);
    mep03_[ism-1] = bei->book1D(histo, histo, 100, 150., 250.);

    if ( mer01_[ism-1] ) bei->removeElement( mer01_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal rms G01 SM%02d", ism);
    mer01_[ism-1] = bei->book1D(histo, histo, 100, 0., 10.);
    if ( mer02_[ism-1] ) bei->removeElement( mer02_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal rms G06 SM%02d", ism);
    mer02_[ism-1] = bei->book1D(histo, histo, 100, 0., 10.);
    if ( mer03_[ism-1] ) bei->removeElement( mer03_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal rms G12 SM%02d", ism);
    mer03_[ism-1] = bei->book1D(histo, histo, 100, 0., 10.);

    if ( mes01_[ism-1] ) bei->removeElement( mes01_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal 3sum G01 SM%02d", ism);
    mes01_[ism-1] = bei->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);
    if ( mes02_[ism-1] ) bei->removeElement( mes02_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal 3sum G06 SM%02d", ism);
    mes02_[ism-1] = bei->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);
    if ( mes03_[ism-1] ) bei->removeElement( mes03_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal 3sum G12 SM%02d", ism);
    mes03_[ism-1] = bei->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);

    if ( met01_[ism-1] ) bei->removeElement( met01_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal 5sum G01 SM%02d", ism);
    met01_[ism-1] = bei->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);
    if ( met02_[ism-1] ) bei->removeElement( met02_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal 5sum G06 SM%02d", ism);
    met02_[ism-1] = bei->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);
    if ( met03_[ism-1] ) bei->removeElement( met03_[ism-1]->getName() );
    sprintf(histo, "EBPT pedestal 5sum G12 SM%02d", ism);
    met03_[ism-1] = bei->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    EBMUtilsClient::resetHisto( met01_[ism-1] );
    EBMUtilsClient::resetHisto( met02_[ism-1] );
    EBMUtilsClient::resetHisto( met03_[ism-1] );

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        meg01_[ism-1]->setBinContent( ie, ip, 2. );
        meg02_[ism-1]->setBinContent( ie, ip, 2. );
        meg03_[ism-1]->setBinContent( ie, ip, 2. );

      }
    }

    EBMUtilsClient::resetHisto( mep01_[ism-1] );
    EBMUtilsClient::resetHisto( mep02_[ism-1] );
    EBMUtilsClient::resetHisto( mep03_[ism-1] );

    EBMUtilsClient::resetHisto( mer01_[ism-1] );
    EBMUtilsClient::resetHisto( mer02_[ism-1] );
    EBMUtilsClient::resetHisto( mer03_[ism-1] );

    EBMUtilsClient::resetHisto( mes01_[ism-1] );
    EBMUtilsClient::resetHisto( mes02_[ism-1] );
    EBMUtilsClient::resetHisto( mes03_[ism-1] );

    EBMUtilsClient::resetHisto( met01_[ism-1] );
    EBMUtilsClient::resetHisto( met02_[ism-1] );
    EBMUtilsClient::resetHisto( met03_[ism-1] );

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        mes01_[ism-1]->setBinContent( ie, ip, -999. );
        mes02_[ism-1]->setBinContent( ie, ip, -999. );
        mes03_[ism-1]->setBinContent( ie, ip, -999. );

        met01_[ism-1]->setBinContent( ie, ip, -999. );
        met02_[ism-1]->setBinContent( ie, ip, -999. );
        met03_[ism-1]->setBinContent( ie, ip, -999. );

      }
    }

  }

}

void EBPedestalClient::cleanup(void) {

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( cloneME_ ) {
      if ( h01_[ism-1] ) delete h01_[ism-1];
      if ( h02_[ism-1] ) delete h02_[ism-1];
      if ( h03_[ism-1] ) delete h03_[ism-1];

      if ( j01_[ism-1] ) delete j01_[ism-1];
      if ( j02_[ism-1] ) delete j02_[ism-1];
      if ( j03_[ism-1] ) delete j03_[ism-1];

      if ( k01_[ism-1] ) delete k01_[ism-1];
      if ( k02_[ism-1] ) delete k02_[ism-1];
      if ( k03_[ism-1] ) delete k03_[ism-1];

      if ( i01_[ism-1] ) delete i01_[ism-1];
      if ( i02_[ism-1] ) delete i02_[ism-1];
    }

    h01_[ism-1] = 0;
    h02_[ism-1] = 0;
    h03_[ism-1] = 0;

    j01_[ism-1] = 0;
    j02_[ism-1] = 0;
    j03_[ism-1] = 0;

    k01_[ism-1] = 0;
    k02_[ism-1] = 0;
    k03_[ism-1] = 0;

    i01_[ism-1] = 0;
    i02_[ism-1] = 0;

  }

  mui_->setCurrentFolder( "EcalBarrel/EBPedestalClient" );
  DaqMonitorBEInterface* bei = mui_->getBEInterface();

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg01_[ism-1] ) bei->removeElement( meg01_[ism-1]->getName() );
    meg01_[ism-1] = 0;
    if ( meg02_[ism-1] ) bei->removeElement( meg02_[ism-1]->getName() );
    meg02_[ism-1] = 0;
    if ( meg03_[ism-1] ) bei->removeElement( meg03_[ism-1]->getName() );
    meg03_[ism-1] = 0;

    if ( mep01_[ism-1] ) bei->removeElement( mep01_[ism-1]->getName() );
    mep01_[ism-1] = 0;
    if ( mep02_[ism-1] ) bei->removeElement( mep02_[ism-1]->getName() );
    mep02_[ism-1] = 0;
    if ( mep03_[ism-1] ) bei->removeElement( mep03_[ism-1]->getName() );
    mep03_[ism-1] = 0;

    if ( mer01_[ism-1] ) bei->removeElement( mer01_[ism-1]->getName() );
    mer01_[ism-1] = 0;
    if ( mer02_[ism-1] ) bei->removeElement( mer02_[ism-1]->getName() );
    mer02_[ism-1] = 0;
    if ( mer03_[ism-1] ) bei->removeElement( mer03_[ism-1]->getName() );
    mer03_[ism-1] = 0;

    if ( mes01_[ism-1] ) bei->removeElement( mes01_[ism-1]->getName() );
    mes01_[ism-1] = 0;
    if ( mes02_[ism-1] ) bei->removeElement( mes02_[ism-1]->getName() );
    mes02_[ism-1] = 0;
    if ( mes03_[ism-1] ) bei->removeElement( mes03_[ism-1]->getName() );
    mes03_[ism-1] = 0;

    if ( met01_[ism-1] ) bei->removeElement( met01_[ism-1]->getName() );
    met01_[ism-1] = 0;
    if ( met02_[ism-1] ) bei->removeElement( met02_[ism-1]->getName() );
    met02_[ism-1] = 0;
    if ( met03_[ism-1] ) bei->removeElement( met03_[ism-1]->getName() );
    met03_[ism-1] = 0;

  }

}

void EBPedestalClient::writeDb(EcalCondDBInterface* econn, MonRunIOV* moniov, int ism) {

  vector<dqm::me_util::Channel> badChannels;

  if ( qth01_[ism-1] ) badChannels = qth01_[ism-1]->getBadChannels();

  if ( ! badChannels.empty() ) {

    cout << endl;
    cout << " Channels that failed \""
         << qth01_[ism-1]->getName() << "\" "
         << "(Algorithm: "
         << qth01_[ism-1]->getAlgoName()
         << ")" << endl;

    cout << endl;
    for ( vector<dqm::me_util::Channel>::iterator it = badChannels.begin(); it != badChannels.end(); ++it ) {
      cout << " (" << it->getBinX()
           << ", " << it->getBinY()
           << ", " << it->getBinZ()
           << ") = " << it->getContents()
           << endl;
    }
    cout << endl;

  }

  if ( qth02_[ism-1] ) badChannels = qth02_[ism-1]->getBadChannels();
  
  if ( ! badChannels.empty() ) {
  
    cout << endl;
    cout << " Channels that failed \""
         << qth02_[ism-1]->getName() << "\" "
         << "(Algorithm: "
         << qth02_[ism-1]->getAlgoName()
         << ")" << endl;
  
    cout << endl;
    for ( vector<dqm::me_util::Channel>::iterator it = badChannels.begin(); it != badChannels.end(); ++it ) {  
      cout << " (" << it->getBinX()
           << ", " << it->getBinY()
           << ", " << it->getBinZ()
           << ") = " << it->getContents()
           << endl;
    }
    cout << endl;

  }

  if ( qth03_[ism-1] ) badChannels = qth03_[ism-1]->getBadChannels();
  
  if ( ! badChannels.empty() ) {
  
    cout << endl;
    cout << " Channels that failed \""
         << qth03_[ism-1]->getName() << "\" "
         << "(Algorithm: "
         << qth03_[ism-1]->getAlgoName()
         << ")" << endl;
  
    cout << endl;
    for ( vector<dqm::me_util::Channel>::iterator it = badChannels.begin(); it != badChannels.end(); ++it ) {  
      cout << " (" << it->getBinX()
           << ", " << it->getBinY()
           << ", " << it->getBinZ()
           << ") = " << it->getContents()
           << endl;
    }
    cout << endl;

  }

  EcalLogicID ecid;
  MonPedestalsDat p;
  map<EcalLogicID, MonPedestalsDat> dataset1;

  const float n_min_tot = 1000.;
  const float n_min_bin = 50.;

  float num01, num02, num03;
  float mean01, mean02, mean03;
  float rms01, rms02, rms03;

  for ( int ie = 1; ie <= 85; ie++ ) {
    for ( int ip = 1; ip <= 20; ip++ ) {

      num01  = num02  = num03  = -1.;
      mean01 = mean02 = mean03 = -1.;
      rms01  = rms02  = rms03  = -1.;

      bool update_channel = false;

      if ( h01_[ism-1] && h01_[ism-1]->GetEntries() >= n_min_tot ) {
        num01 = h01_[ism-1]->GetBinEntries(h01_[ism-1]->GetBin(ie, ip));
        if ( num01 >= n_min_bin ) {
          mean01 = h01_[ism-1]->GetBinContent(h01_[ism-1]->GetBin(ie, ip));
          rms01  = h01_[ism-1]->GetBinError(h01_[ism-1]->GetBin(ie, ip));
          update_channel = true;
        }
      }

      if ( h02_[ism-1] && h02_[ism-1]->GetEntries() >= n_min_tot ) {
        num02 = h02_[ism-1]->GetBinEntries(h02_[ism-1]->GetBin(ie, ip));
        if ( num02 >= n_min_bin ) {
          mean02 = h02_[ism-1]->GetBinContent(h02_[ism-1]->GetBin(ie, ip));
          rms02  = h02_[ism-1]->GetBinError(h02_[ism-1]->GetBin(ie, ip));
          update_channel = true;
        }
      }

      if ( h03_[ism-1] && h03_[ism-1]->GetEntries() >= n_min_tot ) {
        num03 = h03_[ism-1]->GetBinEntries(h03_[ism-1]->GetBin(ie, ip));
        if ( num03 >= n_min_bin ) {
          mean03 = h03_[ism-1]->GetBinContent(h03_[ism-1]->GetBin(ie, ip));
          rms03  = h03_[ism-1]->GetBinError(h03_[ism-1]->GetBin(ie, ip));
          update_channel = true;
        }
      }

      if ( update_channel ) {

        if ( ie == 1 && ip == 1 ) {

          cout << "Preparing dataset for SM=" << ism << endl;

          cout << "G01 (" << ie << "," << ip << ") " << num01  << " " << mean01 << " " << rms01  << endl;
          cout << "G06 (" << ie << "," << ip << ") " << num02  << " " << mean02 << " " << rms02  << endl;
          cout << "G12 (" << ie << "," << ip << ") " << num03  << " " << mean03 << " " << rms03  << endl;

        }

        p.setPedMeanG1(mean01);
        p.setPedRMSG1(rms01);

        p.setPedMeanG6(mean02);
        p.setPedRMSG6(rms02);

        p.setPedMeanG12(mean03);
        p.setPedRMSG12(rms03);

        if ( meg01_[ism-1] && meg01_[ism-1]->getBinContent( ie, ip ) == 1. &&
             meg02_[ism-1] && meg02_[ism-1]->getBinContent( ie, ip ) == 1. &&
             meg03_[ism-1] && meg03_[ism-1]->getBinContent( ie, ip ) == 1. ) {
          p.setTaskStatus(true);
        } else {
          p.setTaskStatus(false);
        }

        int ic = (ip-1) + 20*(ie-1) + 1;

        if ( econn ) {
          try {
            ecid = econn->getEcalLogicID("EB_crystal_number", ism, ic);
            dataset1[ecid] = p;
          } catch (runtime_error &e) {
            cerr << e.what() << endl;
          }
        }

      }

    }
  }

  if ( econn ) {
    try {
      cout << "Inserting MonPedestalsDat ... " << flush;
      if ( dataset1.size() != 0 ) econn->insertDataSet(&dataset1, moniov);
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  MonPNPedDat pn;
  map<EcalLogicID, MonPNPedDat> dataset2;

  const float m_min_tot = 1000.;
  const float m_min_bin = 50.;

//  float num01, num02;
//  float mean01, mean02;
//  float rms01, rms02;

  for ( int i = 1; i <= 10; i++ ) {

    num01  = num02  = -1.;
    mean01 = mean02 = -1.;
    rms01  = rms02  = -1.;

    bool update_channel = false;

    if ( i01_[ism-1] && i01_[ism-1]->GetEntries() >= m_min_tot ) {
      num01 = i01_[ism-1]->GetBinEntries(i01_[ism-1]->GetBin(1, i));
      if ( num01 >= m_min_bin ) {
        mean01 = i01_[ism-1]->GetBinContent(i01_[ism-1]->GetBin(1, i));
        rms01  = i01_[ism-1]->GetBinError(i01_[ism-1]->GetBin(1, i));
        update_channel = true;
      }
    }

    if ( i02_[ism-1] && i02_[ism-1]->GetEntries() >= m_min_tot ) {
      num02 = i02_[ism-1]->GetBinEntries(i02_[ism-1]->GetBin(1, i));
      if ( num02 >= m_min_bin ) {
        mean02 = i02_[ism-1]->GetBinContent(i02_[ism-1]->GetBin(1, i));
        rms02  = i02_[ism-1]->GetBinError(i02_[ism-1]->GetBin(1, i));
        update_channel = true;
      }
    }

    if ( update_channel ) {

      if ( i == 1 ) {

        cout << "Preparing dataset for SM=" << ism << endl;

        cout << "PNs (" << i << ") G01 " << num01  << " " << mean01 << " " << rms01  << endl;
        cout << "PNs (" << i << ") G16 " << num01  << " " << mean01 << " " << rms01  << endl;

      }

      pn.setPedMeanG1(mean01);
      pn.setPedRMSG1(rms01);

      pn.setPedMeanG16(mean02);
      pn.setPedRMSG16(rms02);

      if ( mean01 > 200. ) {
        pn.setTaskStatus(true);
      } else {
        pn.setTaskStatus(false);
      }

      if ( econn ) {
        try {
          ecid = econn->getEcalLogicID("EB_LM_PN", ism, i-1);
          dataset2[ecid] = pn;
        } catch (runtime_error &e) {
          cerr << e.what() << endl;
        }
      }

    }

  }

  if ( econn ) {
    try {
      cout << "Inserting MonPNPedDat ... " << flush;
      if ( dataset2.size() != 0 ) econn->insertDataSet(&dataset2, moniov);
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

}

void EBPedestalClient::subscribe(void){

  if ( verbose_ ) cout << "EBPedestalClient: subscribe" << endl;

  Char_t histo[200];

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01", ism);
    mui_->subscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06", ism);
    mui_->subscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12", ism);
    mui_->subscribe(histo);

    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 3sum SM%02d G01", ism);
    mui_->subscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 3sum SM%02d G06", ism);
    mui_->subscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 3sum SM%02d G12", ism);
    mui_->subscribe(histo);

    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 5sum SM%02d G01", ism);
    mui_->subscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 5sum SM%02d G06", ism);
    mui_->subscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 5sum SM%02d G12", ism);
    mui_->subscribe(histo);

    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Gain01/EBPDT PNs pedestal SM%02d G01", ism);
    mui_->subscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Gain16/EBPDT PNs pedestal SM%02d G16", ism);
    mui_->subscribe(histo);

  }

  if ( collateSources_ ) {

    if ( verbose_ ) cout << "EBPedestalClient: collate" << endl;

    for ( unsigned int i=0; i<superModules_.size(); i++ ) {

      int ism = superModules_[i];

      sprintf(histo, "EBPT pedestal SM%02d G01", ism);
      me_h01_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPedestalTask/Gain01");
      sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01", ism);
      mui_->add(me_h01_[ism-1], histo);

      sprintf(histo, "EBPT pedestal SM%02d G06", ism);
      me_h02_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPedestalTask/Gain06");
      sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06", ism);
      mui_->add(me_h02_[ism-1], histo);

      sprintf(histo, "EBPT pedestal SM%02d G12", ism);
      me_h03_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPedestalTask/Gain12");
      sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12", ism);
      mui_->add(me_h03_[ism-1], histo);

      sprintf(histo, "EBPT pedestal 3sum SM%02d G01", ism);
      me_j01_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPedestalTask/Gain01");
      sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 3sum SM%02d G01", ism);
      mui_->add(me_j01_[ism-1], histo);

      sprintf(histo, "EBPT pedestal 3sum SM%02d G06", ism);
      me_j02_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPedestalTask/Gain06");
      sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 3sum SM%02d G06", ism);
      mui_->add(me_j02_[ism-1], histo);

      sprintf(histo, "EBPT pedestal 3sum SM%02d G12", ism);
      me_j03_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPedestalTask/Gain12");
      sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 3sum SM%02d G12", ism);
      mui_->add(me_j03_[ism-1], histo);

      sprintf(histo, "EBPT pedestal 5sum SM%02d G01", ism);
      me_k01_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPedestalTask/Gain01");
      sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 5sum SM%02d G01", ism);
      mui_->add(me_k01_[ism-1], histo);

      sprintf(histo, "EBPT pedestal 5sum SM%02d G06", ism);
      me_k02_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPedestalTask/Gain06");
      sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 5sum SM%02d G06", ism);
      mui_->add(me_k02_[ism-1], histo);

      sprintf(histo, "EBPT pedestal 5sum SM%02d G12", ism);
      me_k03_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPedestalTask/Gain12");
      sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 5sum SM%02d G12", ism);
      mui_->add(me_k03_[ism-1], histo);

      sprintf(histo, "EBPDT PNs pedestal SM%02d G01", ism);
      me_i01_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPnDiodeTask/Gain01");
      sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Gain01/EBPDT PNs pedestal SM%02d G01", ism);
      mui_->add(me_i01_[ism-1], histo);

      sprintf(histo, "EBPDT PNs pedestal SM%02d G16", ism);
      me_i02_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPnDiodeTask/Gain16");
      sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Gain16/EBPDT PNs pedestal SM%02d G16", ism);
      mui_->add(me_i02_[ism-1], histo);

    }

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01", ism);
      if ( qth01_[ism-1] ) mui_->useQTest(histo, qth01_[ism-1]->getName());
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06", ism);
      if ( qth02_[ism-1] ) mui_->useQTest(histo, qth02_[ism-1]->getName());
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12", ism);
      if ( qth03_[ism-1] ) mui_->useQTest(histo, qth03_[ism-1]->getName());
    } else {
      if ( enableMonitorDaemon_ ) {
        sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01", ism);
        if ( qth01_[ism-1] ) mui_->useQTest(histo, qth01_[ism-1]->getName());
        sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06", ism);
        if ( qth02_[ism-1] ) mui_->useQTest(histo, qth02_[ism-1]->getName());
        sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12", ism);
        if ( qth03_[ism-1] ) mui_->useQTest(histo, qth03_[ism-1]->getName());
      } else {
        sprintf(histo, "EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01", ism);
        if ( qth01_[ism-1] ) mui_->useQTest(histo, qth01_[ism-1]->getName()); 
        sprintf(histo, "EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06", ism);
        if ( qth02_[ism-1] ) mui_->useQTest(histo, qth02_[ism-1]->getName()); 
        sprintf(histo, "EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12", ism);
        if ( qth03_[ism-1] ) mui_->useQTest(histo, qth03_[ism-1]->getName()); 
      }
    }

  }

}

void EBPedestalClient::subscribeNew(void){

  Char_t histo[200];

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01", ism);
    mui_->subscribeNew(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06", ism);
    mui_->subscribeNew(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12", ism);
    mui_->subscribeNew(histo);

    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 3sum SM%02d G01", ism);
    mui_->subscribeNew(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 3sum SM%02d G06", ism);
    mui_->subscribeNew(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 3sum SM%02d G12", ism);
    mui_->subscribeNew(histo);

    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 5sum SM%02d G01", ism);
    mui_->subscribeNew(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 5sum SM%02d G06", ism);
    mui_->subscribeNew(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 5sum SM%02d G12", ism);
    mui_->subscribeNew(histo);

    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Gain01/EBPDT PNs pedestal SM%02d G01", ism);
    mui_->subscribeNew(histo);
    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Gain16/EBPDT PNs pedestal SM%02d G16", ism);
    mui_->subscribeNew(histo);

  }

}

void EBPedestalClient::unsubscribe(void){

  if ( verbose_ ) cout << "EBPedestalClient: unsubscribe" << endl;

  if ( collateSources_ ) {

    if ( verbose_ ) cout << "EBPedestalClient: uncollate" << endl;

    if ( mui_ ) {

    for ( unsigned int i=0; i<superModules_.size(); i++ ) {

        int ism = superModules_[i];

        mui_->removeCollate(me_h01_[ism-1]);
        mui_->removeCollate(me_h02_[ism-1]);
        mui_->removeCollate(me_h03_[ism-1]);

        mui_->removeCollate(me_j01_[ism-1]);
        mui_->removeCollate(me_j02_[ism-1]);
        mui_->removeCollate(me_j03_[ism-1]);

        mui_->removeCollate(me_k01_[ism-1]);
        mui_->removeCollate(me_k02_[ism-1]);
        mui_->removeCollate(me_k03_[ism-1]);

        mui_->removeCollate(me_i01_[ism-1]);
        mui_->removeCollate(me_i02_[ism-1]);

      }

    }

  }

  Char_t histo[200];

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01", ism);
    mui_->unsubscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06", ism);
    mui_->unsubscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12", ism);
    mui_->unsubscribe(histo);

    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 3sum SM%02d G01", ism);
    mui_->unsubscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 3sum SM%02d G06", ism);
    mui_->unsubscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 3sum SM%02d G12", ism);
    mui_->unsubscribe(histo);

    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 5sum SM%02d G01", ism);
    mui_->unsubscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 5sum SM%02d G06", ism);
    mui_->unsubscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 5sum SM%02d G12", ism);
    mui_->unsubscribe(histo);

    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Gain01/EBPDT PNs pedestal SM%02d G01", ism);
    mui_->unsubscribe(histo);
    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Gain16/EBPDT PNs pedestal SM%02d G16", ism);
    mui_->unsubscribe(histo);

  }

}

void EBPedestalClient::analyze(void){

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 == 0 ) {
    if ( verbose_ ) cout << "EBPedestalClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;
  }

  Char_t histo[200];

  MonitorElement* me;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM%02d G01").c_str(), ism);
    }
    me = mui_->get(histo);
    h01_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, h01_[ism-1] );

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM%02d G06").c_str(), ism);
    }
    me = mui_->get(histo);
    h02_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, h02_[ism-1] );

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM%02d G12").c_str(), ism);
    }
    me = mui_->get(histo);
    h03_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, h03_[ism-1] );

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain01/EBPT pedestal 3sum SM%02d G01", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 3sum SM%02d G01").c_str(), ism);
    }
    me = mui_->get(histo);
    j01_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, j01_[ism-1] );

    if ( collateSources_ ) { 
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain06/EBPT pedestal 3sum SM%02d G06", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 3sum SM%02d G06").c_str(), ism);
    }
    me = mui_->get(histo);
    j02_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, j02_[ism-1] );

    if ( collateSources_ ) { 
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain12/EBPT pedestal 3sum SM%02d G12", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 3sum SM%02d G12").c_str(), ism);
    }
    me = mui_->get(histo);
    j03_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, j03_[ism-1] );

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain01/EBPT pedestal 5sum SM%02d G01", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal 5sum SM%02d G01").c_str(), ism);
    }
    me = mui_->get(histo);
    k01_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, k01_[ism-1] );

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain06/EBPT pedestal 5sum SM%02d G06", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal 5sum SM%02d G06").c_str(), ism);
    }
    me = mui_->get(histo);
    k02_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, k02_[ism-1] );

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPedestalTask/Gain12/EBPT pedestal 5sum SM%02d G12", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal 5sum SM%02d G12").c_str(), ism);
    }
    me = mui_->get(histo);
    k03_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, k03_[ism-1] );

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPnDiodeTask/Gain01/EBPDT PNs pedestal SM%02d G01", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPnDiodeTask/Gain01/EBPDT PNs pedestal SM%02d G01").c_str(), ism);
    }
    me = mui_->get(histo);
    i01_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, i01_[ism-1] );

    if ( collateSources_ ) {
      sprintf(histo, "EcalBarrel/Sums/EBPnDiodeTask/Gain16/EBPDT PNs pedestal SM%02d G16", ism);
    } else {
      sprintf(histo, (prefixME_+"EcalBarrel/EBPnDiodeTask/Gain16/EBPDT PNs pedestal SM%02d G16").c_str(), ism);
    }
    me = mui_->get(histo);
    i02_[ism-1] = EBMUtilsClient::getHisto<TProfile2D*>( me, cloneME_, i02_[ism-1] );

    const float n_min_tot = 1000.;
    const float n_min_bin = 50.;

    float num01, num02, num03;
    float mean01, mean02, mean03;
    float rms01, rms02, rms03;

    EBMUtilsClient::resetHisto( meg01_[ism-1] );
    EBMUtilsClient::resetHisto( meg02_[ism-1] );
    EBMUtilsClient::resetHisto( meg03_[ism-1] );

    EBMUtilsClient::resetHisto( mep01_[ism-1] );
    EBMUtilsClient::resetHisto( mep02_[ism-1] );
    EBMUtilsClient::resetHisto( mep03_[ism-1] );

    EBMUtilsClient::resetHisto( mer01_[ism-1] );
    EBMUtilsClient::resetHisto( mer02_[ism-1] );
    EBMUtilsClient::resetHisto( mer03_[ism-1] );

    EBMUtilsClient::resetHisto( mes01_[ism-1] );
    EBMUtilsClient::resetHisto( mes02_[ism-1] );
    EBMUtilsClient::resetHisto( mes03_[ism-1] );

    EBMUtilsClient::resetHisto( met01_[ism-1] );
    EBMUtilsClient::resetHisto( met02_[ism-1] );
    EBMUtilsClient::resetHisto( met03_[ism-1] );

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        num01  = num02  = num03  = -1.;
        mean01 = mean02 = mean03 = -1.;
        rms01  = rms02  = rms03  = -1.;

        if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent(ie, ip, 2.);
        if ( meg02_[ism-1] ) meg02_[ism-1]->setBinContent(ie, ip, 2.);
        if ( meg03_[ism-1] ) meg03_[ism-1]->setBinContent(ie, ip, 2.);

        bool update_channel = false;

        if ( h01_[ism-1] && h01_[ism-1]->GetEntries() >= n_min_tot ) {
          num01 = h01_[ism-1]->GetBinEntries(h01_[ism-1]->GetBin(ie, ip));
          if ( num01 >= n_min_bin ) {
            mean01 = h01_[ism-1]->GetBinContent(h01_[ism-1]->GetBin(ie, ip));
            rms01  = h01_[ism-1]->GetBinError(h01_[ism-1]->GetBin(ie, ip));
            update_channel = true;
          }
        }

        if ( h02_[ism-1] && h02_[ism-1]->GetEntries() >= n_min_tot ) {
          num02 = h02_[ism-1]->GetBinEntries(h02_[ism-1]->GetBin(ie, ip));
          if ( num02 >= n_min_bin ) {
            mean02 = h02_[ism-1]->GetBinContent(h02_[ism-1]->GetBin(ie, ip));
            rms02  = h02_[ism-1]->GetBinError(h02_[ism-1]->GetBin(ie, ip));
            update_channel = true;
          }
        }

        if ( h03_[ism-1] && h03_[ism-1]->GetEntries() >= n_min_tot ) {
          num03 = h03_[ism-1]->GetBinEntries(h03_[ism-1]->GetBin(ie, ip));
          if ( num03 >= n_min_bin ) {
            mean03 = h03_[ism-1]->GetBinContent(h03_[ism-1]->GetBin(ie, ip));
            rms03  = h03_[ism-1]->GetBinError(h03_[ism-1]->GetBin(ie, ip));
            update_channel = true;
          }
        }

        if ( update_channel ) {

          float val;

          val = 1.;
          if ( abs(mean01 - expectedMean_[0]) > discrepancyMean_[0] )
            val = 0.;
          if ( rms01 > RMSThreshold_[0] )
            val = 0.;
          if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent(ie, ip, val);

          if ( mep01_[ism-1] ) mep01_[ism-1]->Fill(mean01);
          if ( mer01_[ism-1] ) mer01_[ism-1]->Fill(rms01);

          val = 1.;
          if ( abs(mean02 - expectedMean_[1]) > discrepancyMean_[1] )
            val = 0.;
          if ( rms02 > RMSThreshold_[1] )
            val = 0.;
          if ( meg02_[ism-1] ) meg02_[ism-1]->setBinContent(ie, ip, val);

          if ( mep02_[ism-1] ) mep02_[ism-1]->Fill(mean02);
          if ( mer02_[ism-1] ) mer02_[ism-1]->Fill(rms02);

          val = 1.;
          if ( abs(mean03 - expectedMean_[2]) > discrepancyMean_[2] )
            val = 0.;
          if ( rms03 > RMSThreshold_[2] )
            val = 0.;
          if ( meg03_[ism-1] ) meg03_[ism-1]->setBinContent(ie, ip, val);

          if ( mep03_[ism-1] ) mep03_[ism-1]->Fill(mean03);
          if ( mer03_[ism-1] ) mer03_[ism-1]->Fill(rms03);

        }

      } 
    }

    vector<dqm::me_util::Channel> badChannels;

    if ( qth01_[ism-1] ) badChannels = qth01_[ism-1]->getBadChannels();

//    if ( ! badChannels.empty() ) {
//      for ( vector<dqm::me_util::Channel>::iterator it = badChannels.begin(); it != badChannels.end(); ++it ) {
//        if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent(it->getBinX(), it->getBinY(), 0.);
//      }
//    }

    if ( qth02_[ism-1] ) badChannels = qth02_[ism-1]->getBadChannels();

//    if ( ! badChannels.empty() ) {
//      for ( vector<dqm::me_util::Channel>::iterator it = badChannels.begin(); it != badChannels.end(); ++it ) {
//        if ( meg02_[ism-1] ) meg02_[ism-1]->setBinContent(it->getBinX(), it->getBinY(), 0.);
//      }
//    }
    
    if ( qth03_[ism-1] ) badChannels = qth03_[ism-1]->getBadChannels();

//    if ( ! badChannels.empty() ) {
//      for ( vector<dqm::me_util::Channel>::iterator it = badChannels.begin(); it != badChannels.end(); ++it ) {
//        if ( meg03_[ism-1] ) meg03_[ism-1]->setBinContent(it->getBinX(), it->getBinY(), 0.);
//      }
//    }

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        float x3val01;
        float x3val02;
        float x3val03;

        float y3val01;
        float y3val02;
        float y3val03;

        float z3val01;
        float z3val02;
        float z3val03;

        if ( mes01_[ism-1] ) mes01_[ism-1]->setBinContent(ie, ip, -999.);
        if ( mes02_[ism-1] ) mes02_[ism-1]->setBinContent(ie, ip, -999.);
        if ( mes03_[ism-1] ) mes03_[ism-1]->setBinContent(ie, ip, -999.);

        if ( ie >= 2 && ie <= 84 && ip >= 2 && ip <= 19 ) {

          x3val01 = 0.;
          x3val02 = 0.; 
          x3val03 = 0.; 
          for ( int i = -1; i <= +1; i++ ) {
            for ( int j = -1; j <= +1; j++ ) {

              if ( h01_[ism-1] ) x3val01 = x3val01 + h01_[ism-1]->GetBinError(h01_[ism-1]->GetBin(ie+i, ip+j)) *
                                                     h01_[ism-1]->GetBinError(h01_[ism-1]->GetBin(ie+i, ip+j));

              if ( h02_[ism-1] ) x3val02 = x3val02 + h02_[ism-1]->GetBinError(h02_[ism-1]->GetBin(ie+i, ip+j)) *
                                                     h02_[ism-1]->GetBinError(h02_[ism-1]->GetBin(ie+i, ip+j));
 
              if ( h03_[ism-1] ) x3val03 = x3val03 + h03_[ism-1]->GetBinError(h03_[ism-1]->GetBin(ie+i, ip+j)) *
                                                     h03_[ism-1]->GetBinError(h03_[ism-1]->GetBin(ie+i, ip+j));
 
            }
          }
          x3val01 = x3val01 / (9.*9.);
          x3val02 = x3val02 / (9.*9.);
          x3val03 = x3val03 / (9.*9.);

          y3val01 = 0.;
          if ( j01_[ism-1] ) y3val01 = j01_[ism-1]->GetBinError(j01_[ism-1]->GetBin(ie, ip)) *
                                       j01_[ism-1]->GetBinError(j01_[ism-1]->GetBin(ie, ip));

          y3val02 = 0.;
          if ( j02_[ism-1] ) y3val02 = j02_[ism-1]->GetBinError(j02_[ism-1]->GetBin(ie, ip)) *
                                       j02_[ism-1]->GetBinError(j02_[ism-1]->GetBin(ie, ip));

          y3val03 = 0.;
          if ( j03_[ism-1] ) y3val03 = j03_[ism-1]->GetBinError(j03_[ism-1]->GetBin(ie, ip)) *
                                       j03_[ism-1]->GetBinError(j03_[ism-1]->GetBin(ie, ip));

          z3val01 = -999.;
          if ( x3val01 != 0 && y3val01 != 0 ) z3val01 = sqrt(abs(x3val01 - y3val01));
          if ( (x3val01 - y3val01) < 0 ) z3val01 = -z3val01;

          if ( mes01_[ism-1] ) mes01_[ism-1]->setBinContent(ie, ip, z3val01);

          z3val02 = -999.;
          if ( x3val02 != 0 && y3val02 != 0 ) z3val02 = sqrt(abs(x3val02 - y3val02));
          if ( (x3val02 - y3val02) < 0 ) z3val02 = -z3val02;
          
          if ( mes02_[ism-1] ) mes02_[ism-1]->setBinContent(ie, ip, z3val02);

          z3val03 = -999.;
          if ( x3val03 != 0 && y3val03 != 0 ) z3val03 = sqrt(abs(x3val03 - y3val03));
          if ( (x3val03 - y3val03) < 0 ) z3val03 = -z3val03;
          
          if ( mes03_[ism-1] ) mes03_[ism-1]->setBinContent(ie, ip, z3val03);

        }

        float x5val01;
        float x5val02;
        float x5val03;

        float y5val01;
        float y5val02;
        float y5val03;

        float z5val01;
        float z5val02;
        float z5val03;

        if ( met01_[ism-1] ) met01_[ism-1]->setBinContent(ie, ip, -999.);
        if ( met02_[ism-1] ) met02_[ism-1]->setBinContent(ie, ip, -999.);
        if ( met03_[ism-1] ) met03_[ism-1]->setBinContent(ie, ip, -999.);

        if ( ie >= 3 && ie <= 83 && ip >= 3 && ip <= 18 ) {

          x5val01 = 0.;
          x5val02 = 0.; 
          x5val03 = 0.; 
          for ( int i = -2; i <= +2; i++ ) {
            for ( int j = -2; j <= +2; j++ ) {

              if ( h01_[ism-1] ) x5val01 = x5val01 + h01_[ism-1]->GetBinError(h01_[ism-1]->GetBin(ie+i, ip+j)) *
                                                     h01_[ism-1]->GetBinError(h01_[ism-1]->GetBin(ie+i, ip+j));

              if ( h02_[ism-1] ) x5val02 = x5val02 + h02_[ism-1]->GetBinError(h02_[ism-1]->GetBin(ie+i, ip+j)) *
                                                     h02_[ism-1]->GetBinError(h02_[ism-1]->GetBin(ie+i, ip+j));
 
              if ( h03_[ism-1] ) x5val03 = x5val03 + h03_[ism-1]->GetBinError(h03_[ism-1]->GetBin(ie+i, ip+j)) *
                                                     h03_[ism-1]->GetBinError(h03_[ism-1]->GetBin(ie+i, ip+j));
 
            }
          }
          x5val01 = x5val01 / (25.*25.);
          x5val02 = x5val02 / (25.*25.);
          x5val03 = x5val03 / (25.*25.);

          y5val01 = 0.;
          if ( k01_[ism-1] ) y5val01 = k01_[ism-1]->GetBinError(k01_[ism-1]->GetBin(ie, ip)) *
                                       k01_[ism-1]->GetBinError(k01_[ism-1]->GetBin(ie, ip));

          y5val02 = 0.;
          if ( k02_[ism-1] ) y5val02 = k02_[ism-1]->GetBinError(k02_[ism-1]->GetBin(ie, ip)) *
                                       k02_[ism-1]->GetBinError(k02_[ism-1]->GetBin(ie, ip));

          y5val03 = 0.;
          if ( k03_[ism-1] ) y5val03 = k03_[ism-1]->GetBinError(k03_[ism-1]->GetBin(ie, ip)) *
                                       k03_[ism-1]->GetBinError(k03_[ism-1]->GetBin(ie, ip));

          z5val01 = -999.;
          if ( x5val01 != 0 && y5val01 != 0 ) z5val01 = sqrt(abs(x5val01 - y5val01));
          if ( (x5val01 - y5val01) < 0 ) z5val01 = -z5val01;

          if ( met01_[ism-1] ) met01_[ism-1]->setBinContent(ie, ip, z5val01);

          z5val02 = -999.;
          if ( x5val02 != 0 && y5val02 != 0 ) z5val02 = sqrt(abs(x5val02 - y5val02));
          if ( (x5val02 - y5val02) < 0 ) z5val02 = -z5val02;
          
          if ( met02_[ism-1] ) met02_[ism-1]->setBinContent(ie, ip, z5val02);

          z5val03 = -999.;
          if ( x5val03 != 0 && y5val03 != 0 ) z5val03 = sqrt(abs(x5val03 - y5val03));
          if ( (x5val03 - y5val03) < 0 ) z5val03 = -z5val03;
          
          if ( met03_[ism-1] ) met03_[ism-1]->setBinContent(ie, ip, z5val03);

        }

      }
    }

  }

}

void EBPedestalClient::htmlOutput(int run, string htmlDir, string htmlName){

  cout << "Preparing EBPedestalClient html output ..." << endl;

  ofstream htmlFile;

  htmlFile.open((htmlDir + htmlName).c_str());

  // html page header
  htmlFile << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">  " << endl;
  htmlFile << "<html>  " << endl;
  htmlFile << "<head>  " << endl;
  htmlFile << "  <meta content=\"text/html; charset=ISO-8859-1\"  " << endl;
  htmlFile << " http-equiv=\"content-type\">  " << endl;
  htmlFile << "  <title>Monitor:PedestalTask output</title> " << endl;
  htmlFile << "</head>  " << endl;
  htmlFile << "<style type=\"text/css\"> td { font-weight: bold } </style>" << endl;
  htmlFile << "<body>  " << endl;
  htmlFile << "<br>  " << endl;
  htmlFile << "<h2>Run:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
  htmlFile << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">" << run << "</span></h2>" << endl;
  htmlFile << "<h2>Monitoring task:&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">PEDESTAL</span></h2> " << endl;
  htmlFile << "<hr>" << endl;
  htmlFile << "<table border=1><tr><td bgcolor=red>channel has problems in this task</td>" << endl;
  htmlFile << "<td bgcolor=lime>channel has NO problems</td>" << endl;
  htmlFile << "<td bgcolor=yellow>channel is missing</td></table>" << endl;
  htmlFile << "<hr>" << endl;

  // Produce the plots to be shown as .png files from existing histograms

  const int csize = 250;

  const double histMax = 1.e15;

  int pCol3[3] = { 2, 3, 5 };

  int pCol4[10];
  for ( int i = 0; i < 10; i++ ) pCol4[i] = 30+i;

  TH2C dummy( "dummy", "dummy for sm", 85, 0., 85., 20, 0., 20. );
  for ( int i = 0; i < 68; i++ ) {
    int a = 2 + ( i/4 ) * 5;
    int b = 2 + ( i%4 ) * 5;
    dummy.Fill( a, b, i+1 );
  }
  dummy.SetMarkerSize(2);
  dummy.SetMinimum(0.1);

  string imgNameQual[3], imgNameMean[3], imgNameRMS[3], imgName3Sum[3], imgName5Sum[3], imgNameMEPnPed[2], imgName, meName;

  TCanvas* cQual = new TCanvas("cQual", "Temp", 2*csize, csize);
  TCanvas* cMean = new TCanvas("cMean", "Temp", csize, csize);
  TCanvas* cRMS = new TCanvas("cRMS", "Temp", csize, csize);
  TCanvas* c3Sum = new TCanvas("c3Sum", "Temp", 2*csize, csize);
  TCanvas* c5Sum = new TCanvas("c5Sum", "Temp", 2*csize, csize);
  TCanvas* cPed = new TCanvas("cPed", "Temp", csize, csize);

  TH2F* obj2f;
  TH1F* obj1f;
  TH1D* obj1d;

  // Loop on barrel supermodules

  for ( unsigned int i=0; i<superModules_.size(); i ++ ) {

    int ism = superModules_[i];

    // Loop on gains

    for ( int iCanvas = 1 ; iCanvas <= 3 ; iCanvas++ ) {

      // Quality plots

      imgNameQual[iCanvas-1] = "";

      obj2f = 0;
      switch ( iCanvas ) {
        case 1:
          obj2f = EBMUtilsClient::getHisto<TH2F*>( meg01_[ism-1] );
          break;
        case 2:
          obj2f = EBMUtilsClient::getHisto<TH2F*>( meg02_[ism-1] );
          break;
        case 3:
          obj2f = EBMUtilsClient::getHisto<TH2F*>( meg03_[ism-1] );
          break;
        default:
          break;
      }

      if ( obj2f ) {

        meName = obj2f->GetName();

        for ( unsigned int i = 0; i < meName.size(); i++ ) {
          if ( meName.substr(i, 1) == " " )  {
            meName.replace(i, 1, "_");
          }
        }
        imgNameQual[iCanvas-1] = meName + ".png";
        imgName = htmlDir + imgNameQual[iCanvas-1];

        cQual->cd();
        gStyle->SetOptStat(" ");
        gStyle->SetPalette(3, pCol3);
        obj2f->GetXaxis()->SetNdivisions(17);
        obj2f->GetYaxis()->SetNdivisions(4);
        cQual->SetGridx();
        cQual->SetGridy();
        obj2f->SetMinimum(-0.00000001);
        obj2f->SetMaximum(2.0);
        obj2f->Draw("col");
        dummy.Draw("text,same");
        cQual->Update();
        cQual->SaveAs(imgName.c_str());

      }

      // Mean distributions

      imgNameMean[iCanvas-1] = "";

      obj1f = 0;
      switch ( iCanvas ) {
        case 1:
          obj1f = EBMUtilsClient::getHisto<TH1F*>( mep01_[ism-1] );
          break;
        case 2:
          obj1f = EBMUtilsClient::getHisto<TH1F*>( mep02_[ism-1] );
          break;
        case 3:
          obj1f = EBMUtilsClient::getHisto<TH1F*>( mep03_[ism-1] );
          break;
        default:
            break;
      }

      if ( obj1f ) {

        meName = obj1f->GetName();

        for ( unsigned int i = 0; i < meName.size(); i++ ) {
          if ( meName.substr(i, 1) == " " )  {
            meName.replace(i, 1 ,"_" );
          }
        }
        imgNameMean[iCanvas-1] = meName + ".png";
        imgName = htmlDir + imgNameMean[iCanvas-1];

        cMean->cd();
        gStyle->SetOptStat("euomr");
        obj1f->SetStats(kTRUE);
        if ( obj1f->GetMaximum(histMax) > 0. ) {
          gPad->SetLogy(1);
        } else {
          gPad->SetLogy(0);
        }
        obj1f->Draw();
        cMean->Update();
        gPad->SetLogy(0);
        cMean->SaveAs(imgName.c_str());
        gPad->SetLogy(0);

      }

      // RMS distributions

      imgNameRMS[iCanvas-1] = "";

      obj1f = 0;
      switch ( iCanvas ) {
        case 1:
          obj1f = EBMUtilsClient::getHisto<TH1F*>( mer01_[ism-1] );
          break;
        case 2:
          obj1f = EBMUtilsClient::getHisto<TH1F*>( mer02_[ism-1] );
          break;
        case 3:
          obj1f = EBMUtilsClient::getHisto<TH1F*>( mer03_[ism-1] );
          break;
        default:
          break;
      }

      if ( obj1f ) {

        meName = obj1f->GetName();

        for ( unsigned int i = 0; i < meName.size(); i++ ) {
          if ( meName.substr(i, 1) == " " )  {
            meName.replace(i, 1, "_");
          }
        }
        imgNameRMS[iCanvas-1] = meName + ".png";
        imgName = htmlDir + imgNameRMS[iCanvas-1];

        cRMS->cd();
        gStyle->SetOptStat("euomr");
        obj1f->SetStats(kTRUE);
        if ( obj1f->GetMaximum(histMax) > 0. ) {
          gPad->SetLogy(1);
        } else {
          gPad->SetLogy(0);
        }
        obj1f->Draw();
        cRMS->Update();
        cRMS->SaveAs(imgName.c_str());
        gPad->SetLogy(0);

      }

      // 3Sum distributions

      imgName3Sum[iCanvas-1] = "";

      obj2f = 0;
      switch ( iCanvas ) {
        case 1:
          obj2f = EBMUtilsClient::getHisto<TH2F*>( mes01_[ism-1] );
          break;
        case 2:
          obj2f = EBMUtilsClient::getHisto<TH2F*>( mes02_[ism-1] );
          break;
        case 3:
          obj2f = EBMUtilsClient::getHisto<TH2F*>( mes03_[ism-1] );
          break;
        default:
          break;
      }

      if ( obj2f ) {

        meName = obj2f->GetName();

        for ( unsigned int i = 0; i < meName.size(); i++ ) {
          if ( meName.substr(i, 1) == " " )  {
            meName.replace(i, 1, "_");
          }
        }
        imgName3Sum[iCanvas-1] = meName + ".png";
        imgName = htmlDir + imgName3Sum[iCanvas-1];

        c3Sum->cd();
        gStyle->SetOptStat(" ");
        gStyle->SetPalette(10, pCol4);
        obj2f->GetXaxis()->SetNdivisions(17);
        obj2f->GetYaxis()->SetNdivisions(4);
        c3Sum->SetGridx();
        c3Sum->SetGridy();
        obj2f->SetMinimum(-0.5);
        obj2f->SetMaximum(+0.5);
        obj2f->Draw("colz");
        dummy.Draw("text,same");
        c3Sum->Update();
        c3Sum->SaveAs(imgName.c_str());

      }

      // 5Sum distributions

      imgName5Sum[iCanvas-1] = "";

      obj2f = 0;
      switch ( iCanvas ) {
        case 1:
          obj2f = EBMUtilsClient::getHisto<TH2F*>( met01_[ism-1] );
          break;
        case 2:
          obj2f = EBMUtilsClient::getHisto<TH2F*>( met02_[ism-1] );
          break;
        case 3:
          obj2f = EBMUtilsClient::getHisto<TH2F*>( met03_[ism-1] );
          break;
        default:
          break;
      }

      if ( obj2f ) {

        meName = obj2f->GetName();

        for ( unsigned int i = 0; i < meName.size(); i++ ) {
          if ( meName.substr(i, 1) == " " )  {
            meName.replace(i, 1, "_");
          }
        }
        imgName5Sum[iCanvas-1] = meName + ".png";
        imgName = htmlDir + imgName5Sum[iCanvas-1];

        c5Sum->cd();
        gStyle->SetOptStat(" ");
        gStyle->SetPalette(10, pCol4);
        obj2f->GetXaxis()->SetNdivisions(17);
        obj2f->GetYaxis()->SetNdivisions(4);
        c5Sum->SetGridx();
        c5Sum->SetGridy();
        obj2f->SetMinimum(-0.5);
        obj2f->SetMaximum(+0.5);
        obj2f->Draw("colz");
        dummy.Draw("text,same");
        c5Sum->Update();
        c5Sum->SaveAs(imgName.c_str());

      }

    }

    // Loop on gains

    for ( int iCanvas = 1 ; iCanvas <= 2 ; iCanvas++ ) {

      // Monitoring elements plots

      imgNameMEPnPed[iCanvas-1] = "";

      obj1d = 0;
      switch ( iCanvas ) {
        case 1:
          if ( i01_[ism-1] ) obj1d = i01_[ism-1]->ProjectionY("_py", 1, 1, "e");
          break;
        case 2:
          if ( i02_[ism-1] ) obj1d = i02_[ism-1]->ProjectionY("_py", 1, 1, "e");
          break;
        default:
          break;
      }

      if ( obj1d ) {

        meName = obj1d->GetName();

        for ( unsigned int i = 0; i < meName.size(); i++ ) {
          if ( meName.substr(i, 1) == " " )  {
            meName.replace(i, 1 ,"_" );
          }
        }
        imgNameMEPnPed[iCanvas-1] = meName + ".png";
        imgName = htmlDir + imgNameMEPnPed[iCanvas-1];

        cPed->cd();
        gStyle->SetOptStat("euomr");
        obj1d->SetStats(kTRUE);
//        if ( obj1d->GetMaximum(histMax) > 0. ) {
//          gPad->SetLogy(1);
//        } else {
//          gPad->SetLogy(0);
//        }
        obj1d->SetMinimum(0.);
        obj1d->Draw();
        cPed->Update();
        cPed->SaveAs(imgName.c_str());
        gPad->SetLogy(0);

        delete obj1d;

      }

    }

    htmlFile << "<h3><strong>Supermodule&nbsp;&nbsp;" << ism << "</strong></h3>" << endl;
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    for ( int iCanvas = 1 ; iCanvas <= 3 ; iCanvas++ ) {

      if ( imgNameQual[iCanvas-1].size() != 0 )
        htmlFile << "<td colspan=\"2\"><img src=\"" << imgNameQual[iCanvas-1] << "\"></td>" << endl;
      else
        htmlFile << "<td colspan=\"2\"><img src=\"" << " " << "\"></td>" << endl;

    }

    htmlFile << "</tr>" << endl;
    htmlFile << "<tr>" << endl;

    for ( int iCanvas = 1 ; iCanvas <= 3 ; iCanvas++ ) {

      if ( imgNameMean[iCanvas-1].size() != 0 )
        htmlFile << "<td><img src=\"" << imgNameMean[iCanvas-1] << "\"></td>" << endl;
      else
        htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

      if ( imgNameRMS[iCanvas-1].size() != 0 )
        htmlFile << "<td><img src=\"" << imgNameRMS[iCanvas-1] << "\"></td>" << endl;
      else
        htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

    }

    htmlFile << "</tr>" << endl;

    htmlFile << "<tr align=\"center\"><td colspan=\"2\">Gain 1</td><td colspan=\"2\">Gain 6</td><td colspan=\"2\">Gain 12</td></tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    for ( int iCanvas = 1 ; iCanvas <= 3 ; iCanvas++ ) {

      if ( imgName3Sum[iCanvas-1].size() != 0 )
        htmlFile << "<td><img src=\"" << imgName3Sum[iCanvas-1] << "\"></td>" << endl;
      else
        htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

    }

    htmlFile << "</tr>" << endl;

    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    for ( int iCanvas = 1 ; iCanvas <= 3 ; iCanvas++ ) {

      if ( imgName5Sum[iCanvas-1].size() != 0 )
        htmlFile << "<td><img src=\"" << imgName5Sum[iCanvas-1] << "\"></td>" << endl;
      else
        htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

    }

    htmlFile << "</tr>" << endl;

    htmlFile << "<tr align=\"center\"><td>Gain 1</td><td>Gain 6</td><td>Gain 12</td></tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    for ( int iCanvas = 1 ; iCanvas <= 2 ; iCanvas++ ) {

      if ( imgNameMEPnPed[iCanvas-1].size() != 0 )
        htmlFile << "<td colspan=\"2\"><img src=\"" << imgNameMEPnPed[iCanvas-1] << "\"></td>" << endl;
      else
        htmlFile << "<td colspan=\"2\"><img src=\"" << " " << "\"></td>" << endl;

    }

    htmlFile << "</tr>" << endl;

    htmlFile << "<tr align=\"center\"><td colspan=\"2\">Gain 1</td><td colspan=\"2\">Gain 16</td></tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

  }

  delete cQual;
  delete cMean;
  delete cRMS;
  delete c3Sum;
  delete c5Sum;
  delete cPed;

  // html page footer
  htmlFile << "</body> " << endl;
  htmlFile << "</html> " << endl;

  htmlFile.close();

}

