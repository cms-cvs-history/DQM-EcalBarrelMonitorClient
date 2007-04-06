/*
 * \file EBSummaryClient.cc
 *
 * $Date: 2007/04/06 14:23:21 $
 * $Revision: 1.12 $
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

#include "DQMServices/Core/interface/DaqMonitorBEInterface.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"
#include "DQMServices/Core/interface/QTestStatus.h"
#include "DQMServices/QualityTests/interface/QCriterionRoot.h"

#include "OnlineDB/EcalCondDB/interface/RunTag.h"
#include "OnlineDB/EcalCondDB/interface/RunIOV.h"

#include "DataFormats/EcalDetId/interface/EBDetId.h"

#include <DQM/EcalBarrelMonitorClient/interface/EBSummaryClient.h>
#include <DQM/EcalBarrelMonitorClient/interface/EBMUtilsClient.h>
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

using namespace cms;
using namespace edm;
using namespace std;

EBSummaryClient::EBSummaryClient(const ParameterSet& ps){

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

  meIntegrity_      = 0;
  mePedestalOnline_ = 0;

}

EBSummaryClient::~EBSummaryClient(){

}

void EBSummaryClient::beginJob(MonitorUserInterface* mui){

  mui_ = mui;

  if ( verbose_ ) cout << "EBSummaryClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

  if ( enableQT_ ) {

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

  mui_->setCurrentFolder( "EcalBarrel/EBSummaryClient" );
  DaqMonitorBEInterface* bei = mui_->getBEInterface();

  if ( meIntegrity_ ) bei->removeElement( meIntegrity_->getName() );
  sprintf(histo, "EBIT integrity quality summary");
  meIntegrity_ = bei->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if ( mePedestalOnline_ ) bei->removeElement( mePedestalOnline_->getName() );
  sprintf(histo, "EBPOT pedestal quality summary G12");
  mePedestalOnline_ = bei->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

}

void EBSummaryClient::cleanup(void) {

  mui_->setCurrentFolder( "EcalBarrel/EBSummaryClient" );
  DaqMonitorBEInterface* bei = mui_->getBEInterface();

  if ( meIntegrity_ ) bei->removeElement( meIntegrity_->getName() );
  meIntegrity_ = 0;

  if ( mePedestalOnline_ ) bei->removeElement( mePedestalOnline_->getName() );
  mePedestalOnline_ = 0;

}

bool EBSummaryClient::writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov, int ism) {

  bool status = true;

  return status;

}

void EBSummaryClient::subscribe(void){

  if ( verbose_ ) cout << "EBSummaryClient: subscribe" << endl;

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
      mePedestalOnline_->setBinContent( ipx, iex, -1. );

    }
  }

  for ( unsigned int i=0; i<clients_.size(); i++ ) {

    EBIntegrityClient* ebit = dynamic_cast<EBIntegrityClient*>(clients_[i]);
    if ( ebit ) {

      for ( unsigned int i=0; i<superModules_.size(); i++ ) {

        int ism = superModules_[i];

        MonitorElement* me = ebit->meg01_[ism-1];

        if ( me ) {

          for ( int ie = 1; ie <= 85; ie++ ) {
            for ( int ip = 1; ip <= 20; ip++ ) {

              float xval = me->getBinContent( ie, ip );

              int ic = (ip-1) + 20*(ie-1) + 1;

              EBDetId id(ism, ic, EBDetId::SMCRYSTALMODE);

              int iex = id.ieta();
              int ipx = id.iphi();

              if ( ism <= 18 ) {
                iex = iex + 85;
              } else {
                iex = iex + 85 + 1;
              }

              meIntegrity_->setBinContent( ipx, iex, xval );

            }
          }

        }

      }

    }

    EBPedestalOnlineClient* ebpo = dynamic_cast<EBPedestalOnlineClient*>(clients_[i]);
    if ( ebpo ) {

      for ( unsigned int i=0; i<superModules_.size(); i++ ) {

        int ism = superModules_[i];

        MonitorElement* me = ebpo->meg03_[ism-1];

        if ( me ) {

          for ( int ie = 1; ie <= 85; ie++ ) {
            for ( int ip = 1; ip <= 20; ip++ ) {

              float xval = me->getBinContent( ie, ip );

              int ic = (ip-1) + 20*(ie-1) + 1;

              EBDetId id(ism, ic, EBDetId::SMCRYSTALMODE);

              int iex = id.ieta();
              int ipx = id.iphi();

              if ( ism <= 18 ) {
                iex = iex + 85;
              } else {
                iex = iex + 85 + 1;
              }

              mePedestalOnline_->setBinContent( ipx, iex, xval );

            }
          }

        }

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

  // dummy histogram labelling the SM's
  TH2C labelGrid("labelGrid","label grid for SM", 18, 0., 360., 2, -85., 85.);
  for ( short sm=0; sm<36; sm++ ) {
    int x = 1 + sm%18;
    int y = 2 - sm/18;
    labelGrid.SetBinContent(x, y, sm+1);
  }
  labelGrid.SetMarkerSize(2);
  labelGrid.SetMinimum(0.1);

  string imgNameMapI, imgNameMapPO, imgName, meName;

  TCanvas* cMap = new TCanvas("cMap", "Temp", int(360./170.*csize), csize);

  float saveHeigth = gStyle->GetTitleH();
  gStyle->SetTitleH(0.07);
  float saveFontSize = gStyle->GetTitleFontSize();
  gStyle->SetTitleFontSize(15);

  TH2F* obj2f;

  imgNameMapI = "";

  obj2f = 0;
  obj2f = EBMUtilsClient::getHisto<TH2F*>( meIntegrity_ );

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
    obj2f->SetTitleSize(0.5);
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapPO = "";

  obj2f = 0;
  obj2f = EBMUtilsClient::getHisto<TH2F*>( mePedestalOnline_ );

  if ( obj2f ) {

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
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
  htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
  htmlFile << "<tr align=\"center\">" << endl;

  if ( imgNameMapI.size() != 0 )
    htmlFile << "<td><img src=\"" << imgNameMapI << "\" usemap=""#Int"" border=0></td>" << endl;
  else
    htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

  htmlFile << "</tr>" << endl;
  htmlFile << "</table>" << endl;
  htmlFile << "<br>" << endl;

  htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
  htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
  htmlFile << "<tr align=\"center\">" << endl;

  if ( imgNameMapPO.size() != 0 )
    htmlFile << "<td><img src=\"" << imgNameMapPO << "\" usemap=""#PeOnl"" border=0></td>" << endl;
  else
    htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

  htmlFile << "</tr>" << endl;
  htmlFile << "</table>" << endl;
  htmlFile << "<br>" << endl;

  delete cMap;

  this->writeMap( htmlFile, "Int" );
  this->writeMap( htmlFile, "PeOnl" );

  // html page footer
  htmlFile << "</body> " << endl;
  htmlFile << "</html> " << endl;

  htmlFile.close();

  gStyle->SetTitleH( saveHeigth );
  gStyle->SetTitleFontSize( saveFontSize );
}

void EBSummaryClient::writeMap( std::ofstream& hf, std::string mapname ) {

 std::map<std::string, std::string> refhtml;
 refhtml["Int"] = "EBIntegrityClient.html";
 refhtml["PeOnl"] = "EBPedestalOnlineClient.html";

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
  int y0 = B0 + (B1-B0)*i/2;
  int y1 = B0 + (B1-B0)*(i+1)/2;
  hf << "<area shape=\"rect\" href=\"" << refhtml[mapname] << "#" << (j+1)+18*(i) << "\" coords=\"";
  hf << x0+1 << ", " << y0+1 << ", " << x1 << ", " << y1 << "\">" << std::endl;
 }
 hf << "</map>" << std::endl;
}

