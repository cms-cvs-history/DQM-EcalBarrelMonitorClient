/*
 * \file EBTimingClient.cc
 *
 * $Date: 2008/04/08 18:04:49 $
 * $Revision: 1.84 $
 * \author G. Della Ricca
 *
*/

#include <memory>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>

#include "TCanvas.h"
#include "TStyle.h"

#include "DQMServices/Core/interface/DQMStore.h"

#include "OnlineDB/EcalCondDB/interface/MonTimingCrystalDat.h"
#include "OnlineDB/EcalCondDB/interface/RunCrystalErrorsDat.h"
#include "OnlineDB/EcalCondDB/interface/RunTTErrorsDat.h"

#include "OnlineDB/EcalCondDB/interface/EcalCondDBInterface.h"

#include "CondTools/Ecal/interface/EcalErrorDictionary.h"

#include "DQM/EcalCommon/interface/EcalErrorMask.h"
#include "DQM/EcalCommon/interface/UtilsClient.h"
#include "DQM/EcalCommon/interface/LogicID.h"
#include "DQM/EcalCommon/interface/Numbers.h"

#include <DQM/EcalBarrelMonitorClient/interface/EBTimingClient.h>

using namespace cms;
using namespace edm;
using namespace std;

EBTimingClient::EBTimingClient(const ParameterSet& ps){

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

    h01_[ism-1] = 0;
    h02_[ism-1] = 0;

    meh01_[ism-1] = 0;
    meh02_[ism-1] = 0;

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    meg01_[ism-1] = 0;

    mea01_[ism-1] = 0;

    mep01_[ism-1] = 0;

    mer01_[ism-1] = 0;

  }

  expectedMean_ = 5.0;
  discrepancyMean_ = 0.5;
  RMSThreshold_ = 2.5;

}

EBTimingClient::~EBTimingClient(){

}

void EBTimingClient::beginJob(DQMStore* dqmStore){

  dqmStore_ = dqmStore;

  if ( debug_ ) cout << "EBTimingClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

}

void EBTimingClient::beginRun(void){

  if ( debug_ ) cout << "EBTimingClient: beginRun" << endl;

  jevt_ = 0;

  this->setup();

}

void EBTimingClient::endJob(void) {

  if ( debug_ ) cout << "EBTimingClient: endJob, ievt = " << ievt_ << endl;

  this->cleanup();

}

void EBTimingClient::endRun(void) {

  if ( debug_ ) cout << "EBTimingClient: endRun, jevt = " << jevt_ << endl;

  this->cleanup();

}

void EBTimingClient::setup(void) {

  char histo[200];

  dqmStore_->setCurrentFolder( prefixME_ + "/EBTimingClient" );

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg01_[ism-1] ) dqmStore_->removeElement( meg01_[ism-1]->getName() );
    sprintf(histo, "EBTMT timing quality %s", Numbers::sEB(ism).c_str());
    meg01_[ism-1] = dqmStore_->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);
    meg01_[ism-1]->setAxisTitle("ieta", 1);
    meg01_[ism-1]->setAxisTitle("iphi", 2);

    if ( mea01_[ism-1] ) dqmStore_->removeElement( mea01_[ism-1]->getName() );
    sprintf(histo, "EBTMT timing %s", Numbers::sEB(ism).c_str());
    mea01_[ism-1] = dqmStore_->book1D(histo, histo, 1700, 0., 1700.);
    mea01_[ism-1]->setAxisTitle("channel", 1);
    mea01_[ism-1]->setAxisTitle("jitter", 2);

    if ( mep01_[ism-1] ) dqmStore_->removeElement( mep01_[ism-1]->getName() );
    sprintf(histo, "EBTMT timing mean %s", Numbers::sEB(ism).c_str());
    mep01_[ism-1] = dqmStore_->book1D(histo, histo, 100, 0.0, 10.0);
    mep01_[ism-1]->setAxisTitle("mean", 1);

    if ( mer01_[ism-1] ) dqmStore_->removeElement( mer01_[ism-1]->getName() );
    sprintf(histo, "EBTMT timing rms %s", Numbers::sEB(ism).c_str());
    mer01_[ism-1] = dqmStore_->book1D(histo, histo, 100, 0.0, 6.0);
    mer01_[ism-1]->setAxisTitle("rms", 1);

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg01_[ism-1] ) meg01_[ism-1]->Reset();

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent( ie, ip, 2. );

      }
    }

    if ( mea01_[ism-1] ) mea01_[ism-1]->Reset();
    if ( mep01_[ism-1] ) mep01_[ism-1]->Reset();
    if ( mer01_[ism-1] ) mer01_[ism-1]->Reset();

  }

}

void EBTimingClient::cleanup(void) {

  if ( ! enableCleanup_ ) return;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( cloneME_ ) {
      if ( h01_[ism-1] ) delete h01_[ism-1];
      if ( h02_[ism-1] ) delete h02_[ism-1];
    }

    h01_[ism-1] = 0;
    h02_[ism-1] = 0;

    meh01_[ism-1] = 0;
    meh02_[ism-1] = 0;

  }

  dqmStore_->setCurrentFolder( prefixME_ + "/EBTimingClient" );

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg01_[ism-1] ) dqmStore_->removeElement( meg01_[ism-1]->getName() );
    meg01_[ism-1] = 0;

    if ( mea01_[ism-1] ) dqmStore_->removeElement( mea01_[ism-1]->getName() );
    mea01_[ism-1] = 0;

    if ( mep01_[ism-1] ) dqmStore_->removeElement( mep01_[ism-1]->getName() );
    mep01_[ism-1] = 0;

    if ( mer01_[ism-1] ) dqmStore_->removeElement( mer01_[ism-1]->getName() );
    mer01_[ism-1] = 0;

  }

}

bool EBTimingClient::writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov) {

  bool status = true;

  EcalLogicID ecid;

  MonTimingCrystalDat t;
  map<EcalLogicID, MonTimingCrystalDat> dataset;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( verbose_ ) {
      cout << " " << Numbers::sEB(ism) << " (ism=" << ism << ")" << endl;
      cout << endl;
      UtilsClient::printBadChannels(meg01_[ism-1], h01_[ism-1]);
    }

    float num01;
    float mean01;
    float rms01;

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        bool update01;

        update01 = UtilsClient::getBinStats(h01_[ism-1], ie, ip, num01, mean01, rms01);

        if ( update01 ) {

          if ( Numbers::icEB(ism, ie, ip) == 1 ) {

            if ( verbose_ ) {
              cout << "Preparing dataset for " << Numbers::sEB(ism) << " (ism=" << ism << ")" << endl;
              cout << "crystal (" << ie << "," << ip << ") " << num01  << " " << mean01 << " " << rms01  << endl;
              cout << endl;
            }

          }

          t.setTimingMean(mean01);
          t.setTimingRMS(rms01);

          if ( meg01_[ism-1] && int(meg01_[ism-1]->getBinContent( ie, ip )) % 3 == 1 ) {
            t.setTaskStatus(true);
          } else {
            t.setTaskStatus(false);
          }

          status = status && UtilsClient::getBinQual(meg01_[ism-1], ie, ip);

          int ic = Numbers::indexEB(ism, ie, ip);

          if ( econn ) {
            ecid = LogicID::getEcalLogicID("EB_crystal_number", Numbers::iSM(ism, EcalBarrel), ic);
            dataset[ecid] = t;
          }

        }

      }
    }

  }

  if ( econn ) {
    try {
      if ( verbose_ ) cout << "Inserting MonTimingCrystalDat ..." << endl;
      if ( dataset.size() != 0 ) econn->insertDataArraySet(&dataset, moniov);
      if ( verbose_ ) cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  return status;

}

void EBTimingClient::analyze(void){

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 == 0 ) {
    if ( debug_ ) cout << "EBTimingClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;
  }

  uint64_t bits01 = 0;
  bits01 |= EcalErrorDictionary::getMask("TIMING_MEAN_WARNING");
  bits01 |= EcalErrorDictionary::getMask("TIMING_RMS_WARNING");
  bits01 |= EcalErrorDictionary::getMask("TIMING_MEAN_ERROR");
  bits01 |= EcalErrorDictionary::getMask("TIMING_RMS_ERROR");

  map<EcalLogicID, RunCrystalErrorsDat> mask1;
  map<EcalLogicID, RunTTErrorsDat> mask2;

  EcalErrorMask::fetchDataSet(&mask1);
  EcalErrorMask::fetchDataSet(&mask2);

  char histo[200];

  MonitorElement* me;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    sprintf(histo, (prefixME_ + "/EBTimingTask/EBTMT timing %s").c_str(), Numbers::sEB(ism).c_str());
    me = dqmStore_->get(histo);
    h01_[ism-1] = UtilsClient::getHisto<TProfile2D*>( me, cloneME_, h01_[ism-1] );
    meh01_[ism-1] = me;

    sprintf(histo, (prefixME_ + "/EBTimingTask/EBTMT timing vs amplitude %s").c_str(), Numbers::sEB(ism).c_str());
    me = dqmStore_->get(histo);
    h02_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h02_[ism-1] );
    meh02_[ism-1] = me;

    if ( meg01_[ism-1] ) meg01_[ism-1]->Reset();
    if ( mea01_[ism-1] ) mea01_[ism-1]->Reset();
    if ( mep01_[ism-1] ) mep01_[ism-1]->Reset();
    if ( mer01_[ism-1] ) mer01_[ism-1]->Reset();

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent(ie, ip, 2.);

        bool update01;

        float num01;
        float mean01;
        float rms01;

        update01 = UtilsClient::getBinStats(h01_[ism-1], ie, ip, num01, mean01, rms01);

        if ( update01 ) {

          float val;

          val = 1.;
          if ( fabs(mean01 - expectedMean_) > discrepancyMean_ )
            val = 0.;
          if ( rms01 > RMSThreshold_ )
            val = 0.;
          if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent(ie, ip, val);

          int ic = Numbers::icEB(ism, ie, ip);

          if ( mea01_[ism-1] ) {
            if ( mean01 > 0. ) {
              mea01_[ism-1]->setBinContent(ic, mean01);
              mea01_[ism-1]->setBinError(ic, rms01);
            } else {
              mea01_[ism-1]->setEntries(1.+mea01_[ism-1]->getEntries());
            }
          }
          if ( mep01_[ism-1] ) mep01_[ism-1]->Fill(mean01);
          if ( mer01_[ism-1] ) mer01_[ism-1]->Fill(rms01);

        }

        // masking

        if ( mask1.size() != 0 ) {
          map<EcalLogicID, RunCrystalErrorsDat>::const_iterator m;
          for (m = mask1.begin(); m != mask1.end(); m++) {

            EcalLogicID ecid = m->first;

            int ic = Numbers::indexEB(ism, ie, ip);

            if ( ecid.getLogicID() == LogicID::getEcalLogicID("EB_crystal_number", Numbers::iSM(ism, EcalBarrel), ic).getLogicID() ) {
              if ( (m->second).getErrorBits() & bits01 ) {
                if ( meg01_[ism-1] ) {
                  float val = int(meg01_[ism-1]->getBinContent(ie, ip)) % 3;
                  meg01_[ism-1]->setBinContent( ie, ip, val+3 );
                }
              }
            }

          }
        }

	// TT masking

	if ( mask2.size() != 0 ) {
          map<EcalLogicID, RunTTErrorsDat>::const_iterator m;
          for (m = mask2.begin(); m != mask2.end(); m++) {

            EcalLogicID ecid = m->first;

            int itt = Numbers::iTT(ism, EcalBarrel, ie, ip);

            if ( ecid.getLogicID() == LogicID::getEcalLogicID("EB_trigger_tower", Numbers::iSM(ism, EcalBarrel), itt).getLogicID() ) {
	      if ( meg01_[ism-1] ) {
		float val = int(meg01_[ism-1]->getBinContent(ie, ip)) % 3;
		meg01_[ism-1]->setBinContent( ie, ip, val+3 );
	      }
            }

          }
        }

      }
    }

  }

}

void EBTimingClient::htmlOutput(int run, string& htmlDir, string& htmlName){

  if ( verbose_ ) cout << "Preparing EBTimingClient html output ..." << endl;

  ofstream htmlFile;

  htmlFile.open((htmlDir + htmlName).c_str());

  // html page header
  htmlFile << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">  " << endl;
  htmlFile << "<html>  " << endl;
  htmlFile << "<head>  " << endl;
  htmlFile << "  <meta content=\"text/html; charset=ISO-8859-1\"  " << endl;
  htmlFile << " http-equiv=\"content-type\">  " << endl;
  htmlFile << "  <title>Monitor:TimingTask output</title> " << endl;
  htmlFile << "</head>  " << endl;
  htmlFile << "<style type=\"text/css\"> td { font-weight: bold } </style>" << endl;
  htmlFile << "<body>  " << endl;
  //htmlFile << "<br>  " << endl;
  htmlFile << "<a name=""top""></a>" << endl;
  htmlFile << "<h2>Run:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
  htmlFile << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">" << run << "</span></h2>" << endl;
  htmlFile << "<h2>Monitoring task:&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">TIMING</span></h2> " << endl;
  htmlFile << "<hr>" << endl;
  htmlFile << "<table border=1><tr><td bgcolor=red>channel has problems in this task</td>" << endl;
  htmlFile << "<td bgcolor=lime>channel has NO problems</td>" << endl;
  htmlFile << "<td bgcolor=yellow>channel is missing</td></table>" << endl;
  htmlFile << "<br>" << endl;
  htmlFile << "<table border=1>" << std::endl;
  for ( unsigned int i=0; i<superModules_.size(); i ++ ) {
    htmlFile << "<td bgcolor=white><a href=""#"
             << Numbers::sEB(superModules_[i]) << ">"
             << setfill( '0' ) << setw(2) << superModules_[i] << "</a></td>";
  }
  htmlFile << std::endl << "</table>" << std::endl;

  // Produce the plots to be shown as .png files from existing histograms

  const int csize = 250;

  const double histMax = 1.e15;

  int pCol3[6] = { 301, 302, 303, 304, 305, 306 };
  int pCol4[10];
  for ( int i = 0; i < 10; i++ ) pCol4[i] = 401+i;

  TH2C dummy( "dummy", "dummy for sm", 85, 0., 85., 20, 0., 20. );
  for ( int i = 0; i < 68; i++ ) {
    int a = 2 + ( i/4 ) * 5;
    int b = 2 + ( i%4 ) * 5;
    dummy.Fill( a, b, i+1 );
  }
  dummy.SetMarkerSize(2);
  dummy.SetMinimum(0.1);

  string imgNameQual, imgNameTim, imgNameMean, imgNameRMS, imgNameAmpl, imgName, meName;

  TCanvas* cQual = new TCanvas("cQual", "Temp", 0, 0, 3*csize, csize);
  TCanvas* cTim = new TCanvas("cTim", "Temp", 0, 0, csize, csize);
  TCanvas* cMean = new TCanvas("cMean", "Temp", 0, 0, csize, csize);
  TCanvas* cRMS = new TCanvas("cRMS", "Temp", 0, 0, csize, csize);
  TCanvas* cAmpl = new TCanvas("cAmpl", "Temp", 0, 0, 2*csize, csize);

  TH2F* obj2f;
  TH1F* obj1f;

  // Loop on barrel supermodules

  for ( unsigned int i=0; i<superModules_.size(); i ++ ) {

    int ism = superModules_[i];

    // Quality plots

    imgNameQual = "";

    obj2f = 0;
    obj2f = UtilsClient::getHisto<TH2F*>( meg01_[ism-1] );

    if ( obj2f ) {

      meName = obj2f->GetName();

      replace(meName.begin(), meName.end(), ' ', '_');
      imgNameQual = meName + ".png";
      imgName = htmlDir + imgNameQual;

      cQual->cd();
      gStyle->SetOptStat(" ");
      gStyle->SetPalette(6, pCol3);
      obj2f->GetXaxis()->SetNdivisions(17);
      obj2f->GetYaxis()->SetNdivisions(4);
      cQual->SetGridx();
      cQual->SetGridy();
      obj2f->SetMinimum(-0.00000001);
      obj2f->SetMaximum(6.0);
      obj2f->Draw("col");
      dummy.Draw("text,same");
      cQual->Update();
      cQual->SaveAs(imgName.c_str());

    }

    // Timing distributions

    imgNameTim = "";

    obj1f = 0;
    obj1f = UtilsClient::getHisto<TH1F*>( mea01_[ism-1] );

    if ( obj1f ) {

      meName = obj1f->GetName();

      replace(meName.begin(), meName.end(), ' ', '_');
      imgNameTim = meName + ".png";
      imgName = htmlDir + imgNameTim;

      cTim->cd();
      gStyle->SetOptStat("euo");
      obj1f->SetStats(kTRUE);
//      if ( obj1f->GetMaximum(histMax) > 0. ) {
//        gPad->SetLogy(kTRUE);
//      } else {
//        gPad->SetLogy(kFALSE);
//      }
      obj1f->SetMinimum(0.0);
      obj1f->SetMaximum(10.0);
      obj1f->Draw();
      cTim->Update();
      cTim->SaveAs(imgName.c_str());
      gPad->SetLogy(kFALSE);

    }

    // Mean distributions

    imgNameMean = "";

    obj1f = 0;
    obj1f = UtilsClient::getHisto<TH1F*>( mep01_[ism-1] );

    if ( obj1f ) {

      meName = obj1f->GetName();

      replace(meName.begin(), meName.end(), ' ', '_');
      imgNameMean = meName + ".png";
      imgName = htmlDir + imgNameMean;

      cMean->cd();
      gStyle->SetOptStat("euomr");
      obj1f->SetStats(kTRUE);
      if ( obj1f->GetMaximum(histMax) > 0. ) {
        gPad->SetLogy(kTRUE);
      } else {
        gPad->SetLogy(kFALSE);
      }
      obj1f->Draw();
      cMean->Update();
      cMean->SaveAs(imgName.c_str());
      gPad->SetLogy(kFALSE);

    }

    // RMS distributions

    obj1f = 0;
    obj1f = UtilsClient::getHisto<TH1F*>( mer01_[ism-1] );

    imgNameRMS = "";

    if ( obj1f ) {

      meName = obj1f->GetName();

      replace(meName.begin(), meName.end(), ' ', '_');
      imgNameRMS = meName + ".png";
      imgName = htmlDir + imgNameRMS;

      cRMS->cd();
      gStyle->SetOptStat("euomr");
      obj1f->SetStats(kTRUE);
      if ( obj1f->GetMaximum(histMax) > 0. ) {
        gPad->SetLogy(kTRUE);
      } else {
        gPad->SetLogy(kFALSE);
      }
      obj1f->Draw();
      cRMS->Update();
      cRMS->SaveAs(imgName.c_str());
      gPad->SetLogy(kFALSE);

    }

    // Amplitude vs. Time distributions

    obj2f = h02_[ism-1];

    imgNameAmpl = "";

    if ( obj2f ) {

      meName = obj2f->GetName();

      replace(meName.begin(), meName.end(), ' ', '_');
      imgNameAmpl = meName + ".png";
      imgName = htmlDir + imgNameAmpl;

      cAmpl->cd();
      gStyle->SetOptStat(" ");
      gStyle->SetPalette(10, pCol4);
      obj2f->SetMinimum(0.0);
      if ( obj2f->GetMaximum(histMax) > 0. ) {
        gPad->SetLogz(kTRUE);
      } else {
        gPad->SetLogz(kFALSE);
      }
      obj2f->Draw("colz");
      cAmpl->Update();
      cAmpl->SaveAs(imgName.c_str());
      gPad->SetLogz(kFALSE);

    }

    if( i>0 ) htmlFile << "<a href=""#top"">Top</a>" << std::endl;
    htmlFile << "<hr>" << std::endl;
    htmlFile << "<h3><a name="""
             << Numbers::sEB(ism) << """></a><strong>"
             << Numbers::sEB(ism) << "</strong></h3>" << endl;
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    if ( imgNameQual.size() != 0 )
      htmlFile << "<td colspan=\"3\"><img src=\"" << imgNameQual << "\"></td>" << endl;
    else
      htmlFile << "<td colspan=\"3\"><img src=\"" << " " << "\"></td>" << endl;

    htmlFile << "</tr>" << endl;
    htmlFile << "<tr>" << endl;

    if ( imgNameTim.size() != 0 )
      htmlFile << "<td><img src=\"" << imgNameTim << "\"></td>" << endl;
    else
      htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

    if ( imgNameMean.size() != 0 )
      htmlFile << "<td><img src=\"" << imgNameMean << "\"></td>" << endl;
    else
      htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

    if ( imgNameRMS.size() != 0 )
      htmlFile << "<td><img src=\"" << imgNameRMS << "\"></td>" << endl;
    else
      htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

    htmlFile << "</tr>" << endl;

    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    if ( imgNameAmpl.size() != 0 )
      htmlFile << "<td colspan=\"3\"><img src=\"" << imgNameAmpl << "\"></td>" << endl;
    else
      htmlFile << "<td colspan=\"3\"><img src=\"" << " " << "\"></td>" << endl;

    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

  }

  delete cQual;
  delete cTim;
  delete cMean;
  delete cRMS;
  delete cAmpl;

  // html page footer
  htmlFile << "</body> " << endl;
  htmlFile << "</html> " << endl;

  htmlFile.close();

}

