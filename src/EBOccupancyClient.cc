/*
 * \file EBOccupancyClient.cc
 *
 * $Date: 2009/02/27 13:54:06 $
 * $Revision: 1.32 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <memory>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>

#include "DQMServices/Core/interface/DQMStore.h"

#include "OnlineDB/EcalCondDB/interface/MonOccupancyDat.h"

#include "OnlineDB/EcalCondDB/interface/EcalCondDBInterface.h"

#include "DQM/EcalCommon/interface/UtilsClient.h"
#include "DQM/EcalCommon/interface/LogicID.h"
#include "DQM/EcalCommon/interface/Numbers.h"

#include <DQM/EcalBarrelMonitorClient/interface/EBOccupancyClient.h>

using namespace cms;
using namespace edm;
using namespace std;

EBOccupancyClient::EBOccupancyClient(const ParameterSet& ps) {

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
  }

  for ( int i=0; i<3; i++) {
    h01_[i] = 0;
    h01ProjEta_[i] = 0;
    h01ProjPhi_[i] = 0;
  }

  for ( int i=0; i<2; i++) {
    h02_[i] = 0;
    h02ProjEta_[i] = 0;
    h02ProjPhi_[i] = 0;
  }

}

EBOccupancyClient::~EBOccupancyClient() {

}

void EBOccupancyClient::beginJob(DQMStore* dqmStore) {

  dqmStore_ = dqmStore;

  if ( debug_ ) cout << "EBOccupancyClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

}

void EBOccupancyClient::beginRun(void) {

  if ( debug_ ) cout << "EBOccupancyClient: beginRun" << endl;

  jevt_ = 0;

  this->setup();

}

void EBOccupancyClient::endJob(void) {

  if ( debug_ ) cout << "EBOccupancyClient: endJob, ievt = " << ievt_ << endl;

  this->cleanup();

}

void EBOccupancyClient::endRun(void) {

  if ( debug_ ) cout << "EBOccupancyClient: endRun, jevt = " << jevt_ << endl;

  this->cleanup();

}

void EBOccupancyClient::setup(void) {

  dqmStore_->setCurrentFolder( prefixME_ + "/EBOccupancyClient" );

}

void EBOccupancyClient::cleanup(void) {

  if ( ! enableCleanup_ ) return;

  if ( cloneME_ ) {

    for ( unsigned int i=0; i<superModules_.size(); i++ ) {
      int ism = superModules_[i];
      if ( i01_[ism-1] ) delete i01_[ism-1];
      if ( i02_[ism-1] ) delete i02_[ism-1];
    }

    for ( int i=0; i<3; ++i ) {
      if ( h01_[i] ) delete h01_[i];
      if ( h01ProjEta_[i] ) delete h01ProjEta_[i];
      if ( h01ProjPhi_[i] ) delete h01ProjPhi_[i];
    }

    for ( int i=0; i<2; ++i ) {
      if ( h02_[i] ) delete h02_[i];
      if ( h02ProjEta_[i] ) delete h02ProjEta_[i];
      if ( h02ProjPhi_[i] ) delete h02ProjPhi_[i];
    }

  }

  for ( int i=0; i<3; ++i ) {
    h01_[i] = 0;
    h01ProjEta_[i] = 0;
    h01ProjPhi_[i] = 0;
  }

  for ( int i=0; i<2; ++i ) {
    h02_[i] = 0;
    h02ProjEta_[i] = 0;
    h02ProjPhi_[i] = 0;
  }

}

bool EBOccupancyClient::writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov, bool& status, bool flag) {

  status = true;

  if ( ! flag ) return false;

  EcalLogicID ecid;

  MonOccupancyDat o;
  map<EcalLogicID, MonOccupancyDat> dataset;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( verbose_ ) {
      cout << " " << Numbers::sEB(ism) << " (ism=" << ism << ")" << endl;
      cout << endl;
    }

    const float n_min_tot = 1000.;
    const float n_min_bin = 10.;

    float num01, num02;
    float mean01, mean02;
    float rms01, rms02;

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        num01  = num02  = -1.;
        mean01 = mean02 = -1.;
        rms01  = rms02  = -1.;

        bool update_channel = false;

        if ( i01_[ism-1] && i01_[ism-1]->GetEntries() >= n_min_tot ) {
          num01 = i01_[ism-1]->GetBinContent(ie, ip);
          if ( num01 >= n_min_bin ) update_channel = true;
        }

        if ( i02_[ism-1] && i02_[ism-1]->GetEntries() >= n_min_tot ) {
          num02 = i02_[ism-1]->GetBinEntries(i02_[ism-1]->GetBin(ie, ip));
          if ( num02 >= n_min_bin ) {
            mean02 = i02_[ism-1]->GetBinContent(ie, ip);
            rms02  = i02_[ism-1]->GetBinError(ie, ip);
          }
        }

        if ( update_channel ) {

          if ( Numbers::icEB(ism, ie, ip) == 1 ) {

            if ( verbose_ ) {
              cout << "Preparing dataset for " << Numbers::sEB(ism) << " (ism=" << ism << ")" << endl;
              cout << "Digi (" << ie << "," << ip << ") " << num01  << " " << mean01 << " " << rms01  << endl;
              cout << "RecHitThr (" << ie << "," << ip << ") " << num02  << " " << mean02 << " " << rms02  << endl;
              cout << endl;
            }

          }


          o.setEventsOverLowThreshold(int(num01));
          o.setEventsOverHighThreshold(int(num02));

          o.setAvgEnergy(mean02);

          int ic = Numbers::indexEB(ism, ie, ip);

          if ( econn ) {
            ecid = LogicID::getEcalLogicID("EB_crystal_number", Numbers::iSM(ism, EcalBarrel), ic);
            dataset[ecid] = o;
          }

        }

      }
    }

  }

  if ( econn ) {
    try {
      if ( verbose_ ) cout << "Inserting MonOccupancyDat ..." << endl;
      if ( dataset.size() != 0 ) econn->insertDataArraySet(&dataset, moniov);
      if ( verbose_ ) cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  return true;

}

void EBOccupancyClient::analyze(void) {

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 == 0 ) {
    if ( debug_ ) cout << "EBOccupancyClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;
  }

  char histo[200];

  MonitorElement* me;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT digi occupancy %s").c_str(), Numbers::sEB(ism).c_str());
    me = dqmStore_->get(histo);
    i01_[ism-1] = UtilsClient::getHisto<TH1F*>( me, cloneME_, i01_[ism-1] );

    sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT rec hit energy %s").c_str(), Numbers::sEB(ism).c_str());
    me = dqmStore_->get(histo);
    i02_[ism-1] = UtilsClient::getHisto<TProfile2D*>( me, cloneME_, i02_[ism-1] );

  }

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT digi occupancy").c_str());
  me = dqmStore_->get(histo);
  h01_[0] = UtilsClient::getHisto<TH2F*> ( me, cloneME_, h01_[0] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT digi occupancy projection eta").c_str());
  me = dqmStore_->get(histo);
  h01ProjEta_[0] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h01ProjEta_[0] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT digi occupancy projection phi").c_str());
  me = dqmStore_->get(histo);
  h01ProjPhi_[0] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h01ProjPhi_[0] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT rec hit occupancy").c_str());
  me = dqmStore_->get(histo);
  h01_[1] = UtilsClient::getHisto<TH2F*> ( me, cloneME_, h01_[1] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT rec hit occupancy projection eta").c_str());
  me = dqmStore_->get(histo);
  h01ProjEta_[1] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h01ProjEta_[1] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT rec hit occupancy projection phi").c_str());
  me = dqmStore_->get(histo);
  h01ProjPhi_[1] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h01ProjPhi_[1] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT TP digi occupancy").c_str());
  me = dqmStore_->get(histo);
  h01_[2] = UtilsClient::getHisto<TH2F*> ( me, cloneME_, h01_[2] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT TP digi occupancy projection eta").c_str());
  me = dqmStore_->get(histo);
  h01ProjEta_[2] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h01ProjEta_[2] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT TP digi occupancy projection phi").c_str());
  me = dqmStore_->get(histo);
  h01ProjPhi_[2] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h01ProjPhi_[2] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT rec hit thr occupancy").c_str());
  me = dqmStore_->get(histo);
  h02_[0] = UtilsClient::getHisto<TH2F*> ( me, cloneME_, h02_[0] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT rec hit thr occupancy projection eta").c_str());
  me = dqmStore_->get(histo);
  h02ProjEta_[0] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h02ProjEta_[0] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT rec hit thr occupancy projection phi").c_str());
  me = dqmStore_->get(histo);
  h02ProjPhi_[0] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h02ProjPhi_[0] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT TP digi thr occupancy").c_str());
  me = dqmStore_->get(histo);
  h02_[1] = UtilsClient::getHisto<TH2F*> ( me, cloneME_, h02_[1] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT TP digi thr occupancy projection eta").c_str());
  me = dqmStore_->get(histo);
  h02ProjEta_[1] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h02ProjEta_[1] );

  sprintf(histo, (prefixME_ + "/EBOccupancyTask/EBOT TP digi thr occupancy projection phi").c_str());
  me = dqmStore_->get(histo);
  h02ProjPhi_[1] = UtilsClient::getHisto<TH1F*> ( me, cloneME_, h02ProjPhi_[1] );

}

void EBOccupancyClient::softReset(bool flag) {

}

