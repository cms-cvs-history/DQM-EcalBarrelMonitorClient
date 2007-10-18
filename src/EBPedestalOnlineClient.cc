/*
 * \file EBPedestalOnlineClient.cc
 *
 * $Date: 2007/10/17 15:58:42 $
 * $Revision: 1.99 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <memory>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "TStyle.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DQMServices/UI/interface/MonitorUIRoot.h"
#include "DQMServices/Core/interface/QTestStatus.h"
#include "DQMServices/QualityTests/interface/QCriterionRoot.h"

#include "OnlineDB/EcalCondDB/interface/RunTag.h"
#include "OnlineDB/EcalCondDB/interface/RunIOV.h"
#include "OnlineDB/EcalCondDB/interface/MonPedestalsOnlineDat.h"
#include "OnlineDB/EcalCondDB/interface/RunCrystalErrorsDat.h"

#include "CondTools/Ecal/interface/EcalErrorDictionary.h"

#include "DQM/EcalCommon/interface/EcalErrorMask.h"
#include <DQM/EcalCommon/interface/UtilsClient.h>
#include <DQM/EcalCommon/interface/LogicID.h>
#include <DQM/EcalCommon/interface/Numbers.h>

#include <DQM/EcalBarrelMonitorClient/interface/EBPedestalOnlineClient.h>

using namespace cms;
using namespace edm;
using namespace std;

EBPedestalOnlineClient::EBPedestalOnlineClient(const ParameterSet& ps){

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

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    h03_[ism-1] = 0;

    meh03_[ism-1] = 0;

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    meg03_[ism-1] = 0;

    mep03_[ism-1] = 0;

    mer03_[ism-1] = 0;

    qth03_[ism-1] = 0;

    qtg03_[ism-1] = 0;

  }

  expectedMean_ = 200.0;
  discrepancyMean_ = 25.0;
  RMSThreshold_ = 2.0;

}

EBPedestalOnlineClient::~EBPedestalOnlineClient(){

}

void EBPedestalOnlineClient::beginJob(MonitorUserInterface* mui){

  mui_ = mui;
  dbe_ = mui->getBEInterface();

  if ( verbose_ ) cout << "EBPedestalOnlineClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

  if ( enableQT_ ) {

    Char_t qtname[200];

    for ( unsigned int i=0; i<superModules_.size(); i++ ) {

      int ism = superModules_[i];

      sprintf(qtname, "EBPOT quality %s G12", Numbers::sEB(ism).c_str());
      qth03_[ism-1] = dynamic_cast<MEContentsProf2DWithinRangeROOT*> (dbe_->createQTest(ContentsProf2DWithinRangeROOT::getAlgoName(), qtname));

      qth03_[ism-1]->setMeanRange(expectedMean_ - discrepancyMean_, expectedMean_ + discrepancyMean_);

      qth03_[ism-1]->setRMSRange(0.0, RMSThreshold_);

      qth03_[ism-1]->setMinimumEntries(10*1700);

      qth03_[ism-1]->setErrorProb(1.00);

      sprintf(qtname, "EBPOT quality test %s G12", Numbers::sEB(ism).c_str());
      qtg03_[ism-1] = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (dbe_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

      qtg03_[ism-1]->setMeanRange(1., 6.);

      qtg03_[ism-1]->setErrorProb(1.00);

    }

  }

}

void EBPedestalOnlineClient::beginRun(void){

  if ( verbose_ ) cout << "EBPedestalOnlineClient: beginRun" << endl;

  jevt_ = 0;

  this->setup();

  this->subscribe();

}

void EBPedestalOnlineClient::endJob(void) {

  if ( verbose_ ) cout << "EBPedestalOnlineClient: endJob, ievt = " << ievt_ << endl;

  this->unsubscribe();

  this->cleanup();

}

void EBPedestalOnlineClient::endRun(void) {

  if ( verbose_ ) cout << "EBPedestalOnlineClient: endRun, jevt = " << jevt_ << endl;

  this->unsubscribe();

  this->cleanup();

}

void EBPedestalOnlineClient::setup(void) {

  Char_t histo[200];

  dbe_->setCurrentFolder( "EcalBarrel/EBPedestalOnlineClient" );

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg03_[ism-1] ) dbe_->removeElement( meg03_[ism-1]->getName() );
    sprintf(histo, "EBPOT pedestal quality G12 %s", Numbers::sEB(ism).c_str());
    meg03_[ism-1] = dbe_->book2D(histo, histo, 85, 0., 85., 20, 0., 20.);

    if ( mep03_[ism-1] ) dbe_->removeElement( mep03_[ism-1]->getName() );
    sprintf(histo, "EBPOT pedestal mean G12 %s", Numbers::sEB(ism).c_str());
    mep03_[ism-1] = dbe_->book1D(histo, histo, 100, 150., 250.);

    if ( mer03_[ism-1] ) dbe_->removeElement( mer03_[ism-1]->getName() );
    sprintf(histo, "EBPOT pedestal rms G12 %s", Numbers::sEB(ism).c_str());
    mer03_[ism-1] = dbe_->book1D(histo, histo, 100, 0.,  10.);

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    UtilsClient::resetHisto( meg03_[ism-1] );

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        meg03_[ism-1]->setBinContent( ie, ip, 2. );

      }
    }

    UtilsClient::resetHisto( mep03_[ism-1] );
    UtilsClient::resetHisto( mer03_[ism-1] );

  }

}

void EBPedestalOnlineClient::cleanup(void) {

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( cloneME_ ) {
      if ( h03_[ism-1] ) delete h03_[ism-1];
    }

    h03_[ism-1] = 0;

    meh03_[ism-1] = 0;

  }

  dbe_->setCurrentFolder( "EcalBarrel/EBPedestalOnlineClient" );

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg03_[ism-1] ) dbe_->removeElement( meg03_[ism-1]->getName() );
    meg03_[ism-1] = 0;

    if ( mep03_[ism-1] ) dbe_->removeElement( mep03_[ism-1]->getName() );
    mep03_[ism-1] = 0;

    if ( mer03_[ism-1] ) dbe_->removeElement( mer03_[ism-1]->getName() );
    mer03_[ism-1] = 0;

  }

}

bool EBPedestalOnlineClient::writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov) {

  bool status = true;

  EcalLogicID ecid;

  MonPedestalsOnlineDat p;
  map<EcalLogicID, MonPedestalsOnlineDat> dataset;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    cout << " SM=" << ism << endl;

    UtilsClient::printBadChannels(qth03_[ism-1]);

//    UtilsClient::printBadChannels(qtg03_[ism-1]);

    float num03;
    float mean03;
    float rms03;

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        bool update03;

        update03 = UtilsClient::getBinStats(h03_[ism-1], ie, ip, num03, mean03, rms03);

        if ( update03 ) {

          if ( ie == 1 && ip == 1 ) {

            cout << "Preparing dataset for SM=" << ism << endl;

            cout << "G12 (" << ie << "," << ip << ") " << num03  << " " << mean03 << " " << rms03  << endl;

            cout << endl;

          }

          p.setADCMeanG12(mean03);
          p.setADCRMSG12(rms03);

          if ( meg03_[ism-1] && int(meg03_[ism-1]->getBinContent( ie, ip )) % 3 == 1 ) {
            p.setTaskStatus(true);
          } else {
            p.setTaskStatus(false);
          }

          status = status && UtilsClient::getBinQual(meg03_[ism-1], ie, ip);

          int ic = Numbers::indexEB(ism, ie, ip);

          if ( econn ) {
            try {
              ecid = LogicID::getEcalLogicID("EB_crystal_number", Numbers::iSM(ism, EcalBarrel), ic);
              dataset[ecid] = p;
            } catch (runtime_error &e) {
              cerr << e.what() << endl;
            }
          }

        }

      }
    }

  }

  if ( econn ) {
    try {
      cout << "Inserting MonPedestalsOnlineDat ... " << flush;
      if ( dataset.size() != 0 ) econn->insertDataArraySet(&dataset, moniov);
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

  return status;

}

void EBPedestalOnlineClient::subscribe(void){

  if ( verbose_ ) cout << "EBPedestalOnlineClient: subscribe" << endl;

  Char_t histo[200];

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    unsigned int ism = superModules_[i];

    sprintf(histo, "*/EcalBarrel/EBPedestalOnlineTask/Gain12/EBPOT pedestal %s G12", Numbers::sEB(ism).c_str());
    mui_->subscribe(histo, ism);

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( enableMonitorDaemon_ ) {
      sprintf(histo, "*/EcalBarrel/EBPedestalOnlineTask/Gain12/EBPOT pedestal %s G12", Numbers::sEB(ism).c_str());
      if ( qth03_[ism-1] ) dbe_->useQTest(histo, qth03_[ism-1]->getName());
    } else {
      sprintf(histo, "EcalBarrel/EBPedestalOnlineTask/Gain12/EBPOT pedestal %s G12", Numbers::sEB(ism).c_str());
      if ( qth03_[ism-1] ) dbe_->useQTest(histo, qth03_[ism-1]->getName());
    }

    sprintf(histo, "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 %s", Numbers::sEB(ism).c_str());
    if ( qtg03_[ism-1] ) dbe_->useQTest(histo, qtg03_[ism-1]->getName());

  }

}

void EBPedestalOnlineClient::subscribeNew(void){

  Char_t histo[200];

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    unsigned int ism = superModules_[i];

    sprintf(histo, "*/EcalBarrel/EBPedestalOnlineTask/Gain12/EBPOT pedestal %s G12", Numbers::sEB(ism).c_str());
    mui_->subscribeNew(histo, ism);

  }

}

void EBPedestalOnlineClient::unsubscribe(void){

  if ( verbose_ ) cout << "EBPedestalOnlineClient: unsubscribe" << endl;

  Char_t histo[200];

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    unsigned int ism = superModules_[i];

    sprintf(histo, "*/EcalBarrel/EBPedestalOnlineTask/Gain12/EBPOT pedestal %s G12", Numbers::sEB(ism).c_str());
    mui_->unsubscribe(histo, ism);

  }

}

void EBPedestalOnlineClient::softReset(void){

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meh03_[ism-1] ) dbe_->softReset(meh03_[ism-1]);

  }

}

void EBPedestalOnlineClient::analyze(void){

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 == 0 ) {
    if ( verbose_ ) cout << "EBPedestalOnlineClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;
  }

  uint64_t bits03 = 0;
  bits03 |= EcalErrorDictionary::getMask("PEDESTAL_ONLINE_HIGH_GAIN_MEAN_WARNING");
  bits03 |= EcalErrorDictionary::getMask("PEDESTAL_ONLINE_HIGH_GAIN_RMS_WARNING");
  bits03 |= EcalErrorDictionary::getMask("PEDESTAL_ONLINE_HIGH_GAIN_MEAN_ERROR");
  bits03 |= EcalErrorDictionary::getMask("PEDESTAL_ONLINE_HIGH_GAIN_RMS_ERROR");

  map<EcalLogicID, RunCrystalErrorsDat> mask;

  EcalErrorMask::fetchDataSet(&mask);

  Char_t histo[200];

  MonitorElement* me;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    sprintf(histo, (prefixME_+"EcalBarrel/EBPedestalOnlineTask/Gain12/EBPOT pedestal %s G12").c_str(), Numbers::sEB(ism).c_str());
    me = dbe_->get(histo);
    h03_[ism-1] = UtilsClient::getHisto<TProfile2D*>( me, cloneME_, h03_[ism-1] );
    meh03_[ism-1] = me;

    UtilsClient::resetHisto( meg03_[ism-1] );
    UtilsClient::resetHisto( mep03_[ism-1] );
    UtilsClient::resetHisto( mer03_[ism-1] );

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        if ( meg03_[ism-1] ) meg03_[ism-1]->setBinContent(ie, ip, 2.);

        bool update03;

        float num03;
        float mean03;
        float rms03;

        update03 = UtilsClient::getBinStats(h03_[ism-1], ie, ip, num03, mean03, rms03);

        if ( update03 ) {

          float val;

          val = 1.;
          if ( fabs(mean03 - expectedMean_) > discrepancyMean_ )
            val = 0.;
          if ( rms03 > RMSThreshold_ )
            val = 0.;
          if ( meg03_[ism-1] ) meg03_[ism-1]->setBinContent(ie, ip, val);

          if ( mep03_[ism-1] ) mep03_[ism-1]->Fill(mean03);
          if ( mer03_[ism-1] ) mer03_[ism-1]->Fill(rms03);

        }

        // masking

        if ( mask.size() != 0 ) {
          map<EcalLogicID, RunCrystalErrorsDat>::const_iterator m;
          for (m = mask.begin(); m != mask.end(); m++) {

            EcalLogicID ecid = m->first;

            int ic = Numbers::indexEB(ism, ie, ip);

            if ( ecid.getID1() == Numbers::iSM(ism, EcalBarrel) && ecid.getID2() == ic ) {
              if ( (m->second).getErrorBits() & bits03 ) {
                if ( meg03_[ism-1] ) {
                  float val = int(meg03_[ism-1]->getBinContent(ie, ip)) % 3;
                  meg03_[ism-1]->setBinContent( ie, ip, val+3 );
                }
              }
            }

          }
        }

      }
    }

    vector<dqm::me_util::Channel> badChannels;

    if ( qth03_[ism-1] ) badChannels = qth03_[ism-1]->getBadChannels();

//    if ( ! badChannels.empty() ) {
//      for ( vector<dqm::me_util::Channel>::iterator it = badChannels.begin(); it != badChannels.end(); ++it ) {
//        if ( meg03_[ism-1] ) meg03_[ism-1]->setBinContent(it->getBinX(), it->getBinY(), 0.);
//      }
//    }

  }

}

void EBPedestalOnlineClient::htmlOutput(int run, string htmlDir, string htmlName){

  cout << "Preparing EBPedestalOnlineClient html output ..." << endl;

  ofstream htmlFile;

  htmlFile.open((htmlDir + htmlName).c_str());

  // html page header
  htmlFile << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">  " << endl;
  htmlFile << "<html>  " << endl;
  htmlFile << "<head>  " << endl;
  htmlFile << "  <meta content=\"text/html; charset=ISO-8859-1\"  " << endl;
  htmlFile << " http-equiv=\"content-type\">  " << endl;
  htmlFile << "  <title>Monitor:PedestalOnlineTask output</title> " << endl;
  htmlFile << "</head>  " << endl;
  htmlFile << "<style type=\"text/css\"> td { font-weight: bold } </style>" << endl;
  htmlFile << "<body>  " << endl;
  //htmlFile << "<br>  " << endl;
  htmlFile << "<a name=""top""></a>" << endl;
  htmlFile << "<h2>Run:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
  htmlFile << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">" << run << "</span></h2>" << endl;
  htmlFile << "<h2>Monitoring task:&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">PEDESTAL ONLINE</span></h2> " << endl;
  htmlFile << "<hr>" << endl;
  htmlFile << "<table border=1><tr><td bgcolor=red>channel has problems in this task</td>" << endl;
  htmlFile << "<td bgcolor=lime>channel has NO problems</td>" << endl;
  htmlFile << "<td bgcolor=yellow>channel is missing</td></table>" << endl;
  htmlFile << "<br>" << endl;
  htmlFile << "<table border=1>" << std::endl;
  for ( unsigned int i=0; i<superModules_.size(); i ++ ) {
    htmlFile << "<td bgcolor=white><a href=""#"
	     << Numbers::sEB(superModules_[i]).c_str() << ">"
	     << setfill( '0' ) << setw(2) << superModules_[i] << "</a></td>";
  }
  htmlFile << std::endl << "</table>" << std::endl;

  // Produce the plots to be shown as .png files from existing histograms

  const int csize = 250;

  const double histMax = 1.e15;

  int pCol3[6] = { 301, 302, 303, 304, 305, 306 };

  TH2C dummy( "dummy", "dummy for sm", 85, 0., 85., 20, 0., 20. );
  for ( int i = 0; i < 68; i++ ) {
    int a = 2 + ( i/4 ) * 5;
    int b = 2 + ( i%4 ) * 5;
    dummy.Fill( a, b, i+1 );
  }
  dummy.SetMarkerSize(2);
  dummy.SetMinimum(0.1);

  string imgNameQual, imgNameMean, imgNameRMS, imgName, meName;

  TCanvas* cQual = new TCanvas("cQual", "Temp", 3*csize, csize);
  TCanvas* cMean = new TCanvas("cMean", "Temp", csize, csize);
  TCanvas* cRMS = new TCanvas("cRMS", "Temp", csize, csize);

  TH2F* obj2f;
  TH1F* obj1f;

  // Loop on barrel supermodules

  for ( unsigned int i=0; i<superModules_.size(); i ++ ) {

    int ism = superModules_[i];

    // Quality plots

    imgNameQual = "";

    obj2f = 0;
    obj2f = UtilsClient::getHisto<TH2F*>( meg03_[ism-1] );

    if ( obj2f ) {

      meName = obj2f->GetName();

      for ( unsigned int i = 0; i < meName.size(); i++ ) {
        if ( meName.substr(i, 1) == " " )  {
          meName.replace(i, 1, "_");
        }
      }
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

    // Mean distributions

    imgNameMean = "";

    obj1f = 0;
    obj1f = UtilsClient::getHisto<TH1F*>( mep03_[ism-1] );

    if ( obj1f ) {

      meName = obj1f->GetName();

      for ( unsigned int i = 0; i < meName.size(); i++ ) {
        if ( meName.substr(i, 1) == " " )  {
          meName.replace(i, 1 ,"_" );
        }
      }
      imgNameMean = meName + ".png";
      imgName = htmlDir + imgNameMean;

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
      cMean->SaveAs(imgName.c_str());
      gPad->SetLogy(0);

    }

    // RMS distributions

    obj1f = 0;
    obj1f = UtilsClient::getHisto<TH1F*>( mer03_[ism-1] );

    imgNameRMS = "";

    if ( obj1f ) {

      meName = obj1f->GetName();

      for ( unsigned int i = 0; i < meName.size(); i++ ) {
        if ( meName.substr(i, 1) == " " )  {
          meName.replace(i, 1, "_");
        }
      }
      imgNameRMS = meName + ".png";
      imgName = htmlDir + imgNameRMS;

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

    if( i>0 ) htmlFile << "<a href=""#top"">Top</a>" << std::endl;
    htmlFile << "<hr>" << std::endl;
    htmlFile << "<h3><a name="""
	     << Numbers::sEB(ism).c_str() << """></a><strong>"
	     << Numbers::sEB(ism).c_str() << "</strong></h3>" << endl;
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    if ( imgNameQual.size() != 0 )
      htmlFile << "<td colspan=\"2\"><img src=\"" << imgNameQual << "\"></td>" << endl;
    else
      htmlFile << "<td colspan=\"2\"><img src=\"" << " " << "\"></td>" << endl;

    htmlFile << "</tr>" << endl;
    htmlFile << "<tr>" << endl;

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

  }

  delete cQual;
  delete cMean;
  delete cRMS;

  // html page footer
  htmlFile << "</body> " << endl;
  htmlFile << "</html> " << endl;

  htmlFile.close();

}

