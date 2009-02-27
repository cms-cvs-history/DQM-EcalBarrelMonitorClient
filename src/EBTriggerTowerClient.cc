/*
 * \file EBTriggerTowerClient.cc
 *
 * $Date: 2009/02/27 12:31:29 $
 * $Revision: 1.115 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <memory>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "DQMServices/Core/interface/DQMStore.h"

#include "DQM/EcalCommon/interface/UtilsClient.h"
#include "DQM/EcalCommon/interface/Numbers.h"

#include <DataFormats/EcalDetId/interface/EBDetId.h>

#include <DQM/EcalBarrelMonitorClient/interface/EBTriggerTowerClient.h>

using namespace cms;
using namespace edm;
using namespace std;

EBTriggerTowerClient::EBTriggerTowerClient(const ParameterSet& ps) {

  // cloneME switch
  cloneME_ = ps.getUntrackedParameter<bool>("cloneME", true);

  // verbose switch
  verbose_ = ps.getUntrackedParameter<bool>("verbose", true);

  // debug switch
  debug_ = ps.getUntrackedParameter<bool>("debug", false);

  // prefixME path
  prefixME_ = ps.getUntrackedParameter<string>("prefixME", "");

  // enableCleanup_ switch
  enableCleanup_ = ps.getUntrackedParameter<bool>("enableCleanup", false);

  // vector of selected Super Modules (Defaults to all 36).
  superModules_.reserve(36);
  for ( unsigned int i = 1; i <= 36; i++ ) superModules_.push_back(i);
  superModules_ = ps.getUntrackedParameter<vector<int> >("superModules", superModules_);

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    i01_[ism-1] = 0;
    i02_[ism-1] = 0;
    j01_[ism-1] = 0;
    j02_[ism-1] = 0;

    l01_[ism-1] = 0;
    m01_[ism-1] = 0;
    n01_[ism-1] = 0;
    o01_[ism-1] = 0;

    mei01_[ism-1] = 0;
    mei02_[ism-1] = 0;
    mej01_[ism-1] = 0;
    mej02_[ism-1] = 0;

    mel01_[ism-1] = 0;
    mem01_[ism-1] = 0;
    men01_[ism-1] = 0;
    meo01_[ism-1] = 0;

//     for (int j=0; j<68; j++) {
//
//       k01_[ism-1][j] = 0;
//       k02_[ism-1][j] = 0;
//
//       mek01_[ism-1][j] = 0;
//       mek02_[ism-1][j] = 0;
//
//     }

    for (int j=0; j<2; j++) {
      me_i01_[ism-1][j] = 0;
      me_i02_[ism-1][j] = 0;
      me_n01_[ism-1][j] = 0;
    }
    for (int j=0; j<6; j++) {
      me_j01_[ism-1][j] = 0;
      me_j02_[ism-1][j] = 0;
      me_m01_[ism-1][j] = 0;
    }
    me_o01_[ism-1] = 0;

  }

}

EBTriggerTowerClient::~EBTriggerTowerClient() {

}

void EBTriggerTowerClient::beginJob(DQMStore* dqmStore) {

  dqmStore_ = dqmStore;

  if ( debug_ ) cout << "EBTriggerTowerClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

}

void EBTriggerTowerClient::beginRun(void) {

  if ( debug_ ) cout << "EBTriggerTowerClient: beginRun" << endl;

  jevt_ = 0;

  this->setup();

}

void EBTriggerTowerClient::endJob(void) {

  if ( debug_ ) cout << "EBTriggerTowerClient: endJob, ievt = " << ievt_ << endl;

  this->cleanup();

}

void EBTriggerTowerClient::endRun(void) {

  if ( debug_ ) cout << "EBTriggerTowerClient: endRun, jevt = " << jevt_ << endl;

  this->cleanup();

}

void EBTriggerTowerClient::setup(void) {

  char histo[200];

  dqmStore_->setCurrentFolder( prefixME_ + "/EBTriggerTowerClient" );

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    for (int j=0; j<2; j++) {
      if ( me_i01_[ism-1][j] ) dqmStore_->removeElement( me_i01_[ism-1][j]->getName() );
      sprintf(histo, "EBTTT FineGrainVeto Real Digis Flag %d %s", j, Numbers::sEB(ism).c_str());
      me_i01_[ism-1][j] = dqmStore_->book2D(histo, histo, 17, 0., 17., 4, 0., 4.);
      me_i01_[ism-1][j]->setAxisTitle("ieta'", 1);
      me_i01_[ism-1][j]->setAxisTitle("iphi'", 2);
      if ( me_i02_[ism-1][j] ) dqmStore_->removeElement( me_i02_[ism-1][j]->getName() );
      sprintf(histo, "EBTTT FineGrainVeto Emulated Digis Flag %d %s", j, Numbers::sEB(ism).c_str());
      me_i02_[ism-1][j] = dqmStore_->book2D(histo, histo, 17, 0., 17., 4, 0., 4.);
      me_i02_[ism-1][j]->setAxisTitle("ieta'", 1);
      me_i02_[ism-1][j]->setAxisTitle("iphi'", 2);
      if ( me_n01_[ism-1][j] ) dqmStore_->removeElement( me_n01_[ism-1][j]->getName() );
      sprintf(histo, "EBTTT EmulFineGrainVetoError Flag %d %s", j, Numbers::sEB(ism).c_str());
      me_n01_[ism-1][j] = dqmStore_->book2D(histo, histo, 17, 0., 17., 4, 0., 4.);
      me_n01_[ism-1][j]->setAxisTitle("ieta'", 1);
      me_n01_[ism-1][j]->setAxisTitle("iphi'", 2);
    }
    for (int j=0; j<6; j++) {
      string bits;
      if ( j == 0 ) bits = "Bit 000";
      if ( j == 1 ) bits = "Bit 001";
      if ( j == 2 ) bits = "Bit 011";
      if ( j == 3 ) bits = "Bit 100";
      if ( j == 4 ) bits = "Bit 101";
      if ( j == 5 ) bits = "Bits 110+111";
      if ( me_j01_[ism-1][j] ) dqmStore_->removeElement( me_j01_[ism-1][j]->getName() );
      sprintf(histo, "EBTTT Flags Real Digis %s %s", bits.c_str(), Numbers::sEB(ism).c_str());
      me_j01_[ism-1][j] = dqmStore_->book2D(histo, histo, 17, 0., 17., 4, 0., 4.);
      me_j01_[ism-1][j]->setAxisTitle("ieta'", 1);
      me_j01_[ism-1][j]->setAxisTitle("iphi'", 2);
      if ( me_j02_[ism-1][j] ) dqmStore_->removeElement( me_j02_[ism-1][j]->getName() );
      sprintf(histo, "EBTTT Flags Emulated Digis %s %s", bits.c_str(), Numbers::sEB(ism).c_str());
      me_j02_[ism-1][j] = dqmStore_->book2D(histo, histo, 17, 0., 17., 4, 0., 4.);
      me_j02_[ism-1][j]->setAxisTitle("ieta'", 1);
      me_j02_[ism-1][j]->setAxisTitle("iphi'", 2);
      if ( me_m01_[ism-1][j] ) dqmStore_->removeElement( me_m01_[ism-1][j]->getName() );
      sprintf(histo, "EBTTT EmulFlagError %s %s", bits.c_str(), Numbers::sEB(ism).c_str());
      me_m01_[ism-1][j] = dqmStore_->book2D(histo, histo, 17, 0., 17., 4, 0., 4.);
      me_m01_[ism-1][j]->setAxisTitle("ieta'", 1);
      me_m01_[ism-1][j]->setAxisTitle("iphi'", 2);
    }
    if ( me_o01_[ism-1] ) dqmStore_->removeElement( me_o01_[ism-1]->getName() );
    sprintf(histo, "EBTTT Trigger Primitives Timing %s", Numbers::sEB(ism).c_str());
    me_o01_[ism-1] = dqmStore_->book2D(histo, histo, 17, 0., 17., 4, 0., 4.);
    me_o01_[ism-1]->setAxisTitle("ieta'", 1);
    me_o01_[ism-1]->setAxisTitle("iphi'", 2);

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    for (int j=0; j<2; j++) {
      if ( me_i01_[ism-1][j] ) me_i01_[ism-1][j]->Reset();
      if ( me_i02_[ism-1][j] ) me_i02_[ism-1][j]->Reset();
      if ( me_n01_[ism-1][j] ) me_n01_[ism-1][j]->Reset();
    }
    for (int j=0; j<6; j++) {
      if ( me_j01_[ism-1][j] ) me_j01_[ism-1][j]->Reset();
      if ( me_j02_[ism-1][j] ) me_j02_[ism-1][j]->Reset();
      if ( me_m01_[ism-1][j] ) me_m01_[ism-1][j]->Reset();
    }
    if ( me_o01_[ism-1] ) me_o01_[ism-1]->Reset();

  }

}

void EBTriggerTowerClient::cleanup(void) {

  if ( ! enableCleanup_ ) return;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( cloneME_ ) {
      if ( i01_[ism-1] ) delete i01_[ism-1];
      if ( i02_[ism-1] ) delete i02_[ism-1];
      if ( j01_[ism-1] ) delete j01_[ism-1];
      if ( j02_[ism-1] ) delete j02_[ism-1];
      if ( l01_[ism-1] ) delete l01_[ism-1];
      if ( m01_[ism-1] ) delete m01_[ism-1];
      if ( n01_[ism-1] ) delete n01_[ism-1];
      if ( o01_[ism-1] ) delete o01_[ism-1];
    }

    i01_[ism-1] = 0;
    i02_[ism-1] = 0;
    j01_[ism-1] = 0;
    j02_[ism-1] = 0;

    l01_[ism-1] = 0;
    m01_[ism-1] = 0;
    n01_[ism-1] = 0;
    o01_[ism-1] = 0;

    mei01_[ism-1] = 0;
    mei02_[ism-1] = 0;
    mej01_[ism-1] = 0;
    mej02_[ism-1] = 0;

    mel01_[ism-1] = 0;
    mem01_[ism-1] = 0;
    men01_[ism-1] = 0;
    meo01_[ism-1] = 0;

//     for (int j=0; j<68; j++) {
//
//       if ( cloneME_ ) {
//         if ( k01_[ism-1][j] ) delete k01_[ism-1][j];
//         if ( k02_[ism-1][j] ) delete k02_[ism-1][j];
//       }
//
//       k01_[ism-1][j] = 0;
//       k02_[ism-1][j] = 0;
//
//       mek01_[ism-1][j] = 0;
//       mek02_[ism-1][j] = 0;
//
//     }

  }

  dqmStore_->setCurrentFolder( prefixME_ + "/EBTriggerTowerClient" );

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    for (int j=0; j<2; j++) {
      if ( me_i01_[ism-1][j] ) dqmStore_->removeElement( me_i01_[ism-1][j]->getName() );
      me_i01_[ism-1][j] = 0;
      if ( me_i02_[ism-1][j] ) dqmStore_->removeElement( me_i02_[ism-1][j]->getName() );
      me_i02_[ism-1][j] = 0;
      if ( me_n01_[ism-1][j] ) dqmStore_->removeElement( me_n01_[ism-1][j]->getName() );
      me_n01_[ism-1][j] = 0;
    }
    for (int j=0; j<6; j++) {
      if ( me_j01_[ism-1][j] ) dqmStore_->removeElement( me_j01_[ism-1][j]->getName() );
      me_j01_[ism-1][j] = 0;
      if ( me_j02_[ism-1][j] ) dqmStore_->removeElement( me_j02_[ism-1][j]->getName() );
      me_j02_[ism-1][j] = 0;
      if ( me_m01_[ism-1][j] ) dqmStore_->removeElement( me_m01_[ism-1][j]->getName() );
      me_m01_[ism-1][j] = 0;
    }
    if ( me_o01_[ism-1] ) dqmStore_->removeElement( me_o01_[ism-1]->getName() );
    me_o01_[ism-1] = 0;

  }

}

bool EBTriggerTowerClient::writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov, bool& status, bool flag) {

  status = true;

  if ( ! flag ) return false;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( verbose_ ) {
      cout << " " << Numbers::sEB(ism) << " (ism=" << ism << ")" << endl;
      cout << endl;
      UtilsClient::printBadChannels(mel01_[ism-1], UtilsClient::getHisto<TH2F*>(mel01_[ism-1]), true);
      for (int j=0; j<2; j++) {
        UtilsClient::printBadChannels(me_n01_[ism-1][j], UtilsClient::getHisto<TH2F*>(me_n01_[ism-1][j]), true);
      }
      for (int j=0; j<6; j++) {
       UtilsClient::printBadChannels(me_m01_[ism-1][j], UtilsClient::getHisto<TH2F*>(me_m01_[ism-1][j]), true);
      }
    }

  }

  return true;

}

void EBTriggerTowerClient::analyze(void) {

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 == 0 ) {
    if ( debug_ ) cout << "EBTriggerTowerClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;
  }

  analyze("Real Digis",
          "EBTriggerTowerTask", false );

  analyze("Emulated Digis",
          "EBTriggerTowerTask/Emulated", true );

}

void EBTriggerTowerClient::analyze(const char* nameext,
                                   const char* folder,
                                   bool emulated) {
  char histo[200];

  MonitorElement* me;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    sprintf(histo, (prefixME_ + "/%s/EBTTT FineGrainVeto %s %s").c_str(), folder, nameext, Numbers::sEB(ism).c_str());
    me = dqmStore_->get(histo);
    if(!emulated) {
      i01_[ism-1] = UtilsClient::getHisto<TH3F*>( me, cloneME_, i01_[ism-1] );
      mei01_[ism-1] = me;
    }
    else {
      i02_[ism-1] = UtilsClient::getHisto<TH3F*>( me, cloneME_, i02_[ism-1] );
      mei02_[ism-1] = me;
    }

    sprintf(histo, (prefixME_ + "/%s/EBTTT Flags %s %s").c_str(), folder, nameext, Numbers::sEB(ism).c_str());
    me = dqmStore_->get(histo);
    if(!emulated) {
      j01_[ism-1] = UtilsClient::getHisto<TH3F*>( me, cloneME_, j01_[ism-1] );
      mej01_[ism-1] = me;
    }
    else {
      j02_[ism-1] = UtilsClient::getHisto<TH3F*>( me, cloneME_, j02_[ism-1] );
      mej02_[ism-1] = me;
    }

    if(!emulated) {
      sprintf(histo, (prefixME_ + "/%s/EBTTT EmulError %s").c_str(), folder, Numbers::sEB(ism).c_str());
      me = dqmStore_->get(histo);
      l01_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, l01_[ism-1] );
      mel01_[ism-1] = me;

      sprintf(histo, (prefixME_ + "/%s/EBTTT EmulFlagError %s").c_str(), folder, Numbers::sEB(ism).c_str());
      me = dqmStore_->get(histo);
      m01_[ism-1] = UtilsClient::getHisto<TH3F*>( me, cloneME_, m01_[ism-1] );
      mem01_[ism-1] = me;

      sprintf(histo, (prefixME_ + "/%s/EBTTT EmulFineGrainVetoError %s").c_str(), folder, Numbers::sEB(ism).c_str());
      me = dqmStore_->get(histo);
      n01_[ism-1] = UtilsClient::getHisto<TH3F*>( me, cloneME_, n01_[ism-1] );
      men01_[ism-1] = me;

      sprintf(histo, (prefixME_ + "/%s/EBTTT EmulMatch %s").c_str(), folder, Numbers::sEB(ism).c_str());
      me = dqmStore_->get(histo);
      o01_[ism-1] = UtilsClient::getHisto<TH3F*>( me, cloneME_, o01_[ism-1] );
      meo01_[ism-1] = me;

    }

//     for (int j=0; j<68; j++) {
//
//       sprintf(histo, (prefixME_ + "/EBTriggerTowerTask/EnergyMaps/EBTTT Et T %s TT%02d").c_str(), ism, j+1);
//       me = dqmStore_->get(histo);
//       k01_[ism-1][j] = UtilsClient::getHisto<TH1F*>( me, cloneME_, k01_[ism-1][j] );
//       mek01_[ism-1][j] = me;
//
//       sprintf(histo, (prefixME_ + "/EBTriggerTowerTask/EnergyMaps/EBTTT Et R %s TT%02d").c_str(), ism, j+1);
//       me = dqmStore_->get(histo);
//       k02_[ism-1][j] = UtilsClient::getHisto<TH1F*>( me, cloneME_, k02_[ism-1][j] );
//       mek02_[ism-1][j] = me;
//
//     }

    for (int j=0; j<2; j++) {
      if ( me_i01_[ism-1][j] ) me_i01_[ism-1][j]->Reset();
      if ( me_i02_[ism-1][j] ) me_i02_[ism-1][j]->Reset();
      if ( me_n01_[ism-1][j] ) me_n01_[ism-1][j]->Reset();
    }
    for (int j=0; j<6; j++) {
      if ( me_j01_[ism-1][j] ) me_j01_[ism-1][j]->Reset();
      if ( me_j02_[ism-1][j] ) me_j02_[ism-1][j]->Reset();
      if ( me_m01_[ism-1][j] ) me_m01_[ism-1][j]->Reset();
    }
    if ( me_o01_[ism-1] ) me_o01_[ism-1]->Reset();

    for (int ie = 1; ie <= 17; ie++) {
      for (int ip = 1; ip <= 4; ip++) {

        for (int j=0; j<2; j++) {
          if ( i01_[ism-1] ) me_i01_[ism-1][j]->Fill(ie-0.5, ip-0.5, i01_[ism-1]->GetBinContent(ie, ip, j+1));
          if ( i02_[ism-1] ) me_i02_[ism-1][j]->Fill(ie-0.5, ip-0.5, i02_[ism-1]->GetBinContent(ie, ip, j+1));
          if ( n01_[ism-1] ) me_n01_[ism-1][j]->Fill(ie-0.5, ip-0.5, n01_[ism-1]->GetBinContent(ie, ip, j+1));
        }
        for (int j=0; j<6; j++) {
          if ( j == 0 ) {
            if ( j01_[ism-1] ) me_j01_[ism-1][j]->Fill(ie-0.5, ip-0.5, j01_[ism-1]->GetBinContent(ie, ip, j+1));
            if ( j02_[ism-1] ) me_j02_[ism-1][j]->Fill(ie-0.5, ip-0.5, j02_[ism-1]->GetBinContent(ie, ip, j+1));
            if ( m01_[ism-1] ) me_m01_[ism-1][j]->Fill(ie-0.5, ip-0.5, m01_[ism-1]->GetBinContent(ie, ip, j+1));
          }
          if ( j == 1 ) {
            if ( j01_[ism-1] ) me_j01_[ism-1][j]->Fill(ie-0.5, ip-0.5, j01_[ism-1]->GetBinContent(ie, ip, j+1));
            if ( j02_[ism-1] ) me_j02_[ism-1][j]->Fill(ie-0.5, ip-0.5, j02_[ism-1]->GetBinContent(ie, ip, j+1));
            if ( m01_[ism-1] ) me_m01_[ism-1][j]->Fill(ie-0.5, ip-0.5, m01_[ism-1]->GetBinContent(ie, ip, j+1));
          }
          if ( j == 2 ) {
            if ( j01_[ism-1] ) me_j01_[ism-1][j]->Fill(ie-0.5, ip-0.5, j01_[ism-1]->GetBinContent(ie, ip, j+2));
            if ( j02_[ism-1] ) me_j02_[ism-1][j]->Fill(ie-0.5, ip-0.5, j02_[ism-1]->GetBinContent(ie, ip, j+2));
            if ( m01_[ism-1] ) me_m01_[ism-1][j]->Fill(ie-0.5, ip-0.5, m01_[ism-1]->GetBinContent(ie, ip, j+2));
          }
          if ( j == 3 ) {
            if ( j01_[ism-1] ) me_j01_[ism-1][j]->Fill(ie-0.5, ip-0.5, j01_[ism-1]->GetBinContent(ie, ip, j+2));
            if ( j02_[ism-1] ) me_j02_[ism-1][j]->Fill(ie-0.5, ip-0.5, j02_[ism-1]->GetBinContent(ie, ip, j+2));
            if ( m01_[ism-1] ) me_m01_[ism-1][j]->Fill(ie-0.5, ip-0.5, m01_[ism-1]->GetBinContent(ie, ip, j+2));
          }
          if ( j == 4 ) {
            if ( j01_[ism-1] ) me_j01_[ism-1][j]->Fill(ie-0.5, ip-0.5, j01_[ism-1]->GetBinContent(ie, ip, j+2));
            if ( j02_[ism-1] ) me_j02_[ism-1][j]->Fill(ie-0.5, ip-0.5, j02_[ism-1]->GetBinContent(ie, ip, j+2));
            if ( m01_[ism-1] ) me_m01_[ism-1][j]->Fill(ie-0.5, ip-0.5, m01_[ism-1]->GetBinContent(ie, ip, j+2));
          }
          if ( j == 5 ) {
            if ( j01_[ism-1] ) {
              me_j01_[ism-1][j]->Fill(ie-0.5, ip-0.5, j01_[ism-1]->GetBinContent(ie, ip, j+2));
              me_j01_[ism-1][j]->Fill(ie-0.5, ip-0.5, j01_[ism-1]->GetBinContent(ie, ip, j+3));
            }
            if ( j02_[ism-1] ) {
              me_j02_[ism-1][j]->Fill(ie-0.5, ip-0.5, j02_[ism-1]->GetBinContent(ie, ip, j+2));
              me_j02_[ism-1][j]->Fill(ie-0.5, ip-0.5, j02_[ism-1]->GetBinContent(ie, ip, j+3));
            }
            if ( m01_[ism-1] ) {
              me_m01_[ism-1][j]->Fill(ie-0.5, ip-0.5, m01_[ism-1]->GetBinContent(ie, ip, j+2));
              me_m01_[ism-1][j]->Fill(ie-0.5, ip-0.5, m01_[ism-1]->GetBinContent(ie, ip, j+3));
            }
          }
        }
        if ( o01_[ism-1] ) {
          float index=-1;
          double max=0;
          for (int j=0; j<6; j++) {
            double sampleEntries = o01_[ism-1]->GetBinContent(ie, ip, j+1);
            if(sampleEntries > max) {
              index=j;
              max = sampleEntries;
            }
          }
          if ( max > 0 ) {
            if ( index == 0 ) me_o01_[ism-1]->setBinContent(ie, ip, -1);
            else me_o01_[ism-1]->setBinContent(ie, ip, index );
          }
        }

      }
    }

  }

}

void EBTriggerTowerClient::softReset(bool flag) {

}

