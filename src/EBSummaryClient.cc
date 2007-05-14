/*
 * \file EBSummaryClient.cc
 *
 * $Date: 2007/05/13 15:02:29 $
 * $Revision: 1.24 $
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
  meOccupancy_      = 0;
  mePedestalOnline_ = 0;
  meLaserL1_        = 0;

  qtg01_ = 0;
  qtg02_ = 0;
  qtg03_ = 0;
  qtg04_ = 0;

}

EBSummaryClient::~EBSummaryClient(){

}

void EBSummaryClient::beginJob(MonitorUserInterface* mui){

  mui_ = mui;

  if ( verbose_ ) cout << "EBSummaryClient: beginJob" << endl;

  ievt_ = 0;
  jevt_ = 0;

  if ( enableQT_ ) {

    Char_t qtname[200];

    sprintf(qtname, "EBIT summary quality test");
    qtg01_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (mui_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBOT summary quality test");
    qtg02_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (mui_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBPOT summary quality test");
    qtg03_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (mui_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    sprintf(qtname, "EBLT summary quality test L1");
    qtg04_ = dynamic_cast<MEContentsTH2FWithinRangeROOT*> (mui_->createQTest(ContentsTH2FWithinRangeROOT::getAlgoName(), qtname));

    qtg01_->setMeanRange(1., 6.);
    qtg02_->setMeanRange(1., 6.);
    qtg03_->setMeanRange(1., 6.);
    qtg04_->setMeanRange(1., 6.);

    qtg01_->setErrorProb(1.00);
    qtg02_->setErrorProb(1.00);
    qtg03_->setErrorProb(1.00);
    qtg04_->setErrorProb(1.00);

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

  if ( meOccupancy_ ) bei->removeElement( meOccupancy_->getName() );
  sprintf(histo, "EBOT occupancy summary");
  meOccupancy_ = bei->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if ( mePedestalOnline_ ) bei->removeElement( mePedestalOnline_->getName() );
  sprintf(histo, "EBPOT pedestal quality summary G12");
  mePedestalOnline_ = bei->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

  if ( meLaserL1_ ) bei->removeElement( meLaserL1_->getName() );
  sprintf(histo, "EBLT laser quality summary L1");
  meLaserL1_ = bei->book2D(histo, histo, 360, 0., 360., 170, -85., 85.);

}

void EBSummaryClient::cleanup(void) {

  mui_->setCurrentFolder( "EcalBarrel/EBSummaryClient" );
  DaqMonitorBEInterface* bei = mui_->getBEInterface();

  if ( meIntegrity_ ) bei->removeElement( meIntegrity_->getName() );
  meIntegrity_ = 0;

  if ( meOccupancy_ ) bei->removeElement( meOccupancy_->getName() );
  meOccupancy_ = 0;

  if ( mePedestalOnline_ ) bei->removeElement( mePedestalOnline_->getName() );
  mePedestalOnline_ = 0;

  if ( meLaserL1_ ) bei->removeElement( meLaserL1_->getName() );
  meLaserL1_ = 0;

}

bool EBSummaryClient::writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov, int ism) {

  bool status = true;

//  UtilsClient::printBadChannels(qtg01_);
//  UtilsClient::printBadChannels(qtg02_);
//  UtilsClient::printBadChannels(qtg03_);
//  UtilsClient::printBadChannels(qtg04_);

  return status;

}

void EBSummaryClient::subscribe(void){

  if ( verbose_ ) cout << "EBSummaryClient: subscribe" << endl;

  Char_t histo[200];

  sprintf(histo, "EcalBarrel/EBSummaryClient/EBIT integrity quality summary");
  if ( qtg01_ ) mui_->useQTest(histo, qtg01_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBOT occupancy summary");
  if ( qtg02_ ) mui_->useQTest(histo, qtg02_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBPOT pedestal quality summary G12");
  if ( qtg03_ ) mui_->useQTest(histo, qtg03_->getName());
  sprintf(histo, "EcalBarrel/EBSummaryClient/EBLT laser quality summary L1");
  if ( qtg04_ ) mui_->useQTest(histo, qtg04_->getName());

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
      meOccupancy_->setBinContent( ipx, iex, -1. );
      mePedestalOnline_->setBinContent( ipx, iex, -1. );

      meLaserL1_->setBinContent( ipx, iex, -1. );

    }
  }

  meLaserL1_->setEntries( 0 );

  for ( unsigned int i=0; i<clients_.size(); i++ ) {

    EBIntegrityClient* ebic = dynamic_cast<EBIntegrityClient*>(clients_[i]);
    EBPedestalOnlineClient* ebpoc = dynamic_cast<EBPedestalOnlineClient*>(clients_[i]);

    EBLaserClient* eblc = dynamic_cast<EBLaserClient*>(clients_[i]);

    MonitorElement* me;
    TH2F* h2;

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

  string imgNameMapI, imgNameMapO, imgNameMapPO, imgNameMapLL1, imgName, meName;

  TCanvas* cMap = new TCanvas("cMap", "Temp", int(360./170.*csize), csize);

  float saveHeigth = gStyle->GetTitleH();
  gStyle->SetTitleH(0.07);
  float saveFontSize = gStyle->GetTitleFontSize();
  gStyle->SetTitleFontSize(15);

  TH2F* obj2f;

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
    obj2f->SetTitleSize(0.5);
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
    obj2f->SetTitleSize(0.5);
    obj2f->Draw("colz");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  imgNameMapPO = "";

  obj2f = 0;
  obj2f = UtilsClient::getHisto<TH2F*>( mePedestalOnline_ );

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
    obj2f->Draw("col");
    labelGrid.Draw("text,same");
    cMap->Update();
    cMap->SaveAs(imgName.c_str());

  }

  gStyle->SetPaintTextFormat();

  htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
  htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
  htmlFile << "<tr align=\"center\">" << endl;

  if ( imgNameMapI.size() != 0 )
    htmlFile << "<td><img src=\"" << imgNameMapI << "\" usemap=""#Integrity"" border=0></td>" << endl;
  else
    htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

  htmlFile << "</tr>" << endl;
  htmlFile << "</table>" << endl;
  htmlFile << "<br>" << endl;

  htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
  htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
  htmlFile << "<tr align=\"center\">" << endl;

  if ( imgNameMapO.size() != 0 )
    htmlFile << "<td><img src=\"" << imgNameMapO << "\" usemap=""#Occupancy"" border=0></td>" << endl;
  else
    htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

  htmlFile << "</tr>" << endl;
  htmlFile << "</table>" << endl;
  htmlFile << "<br>" << endl;

  htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
  htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
  htmlFile << "<tr align=\"center\">" << endl;

  if ( imgNameMapPO.size() != 0 )
    htmlFile << "<td><img src=\"" << imgNameMapPO << "\" usemap=""#PedestalOnline"" border=0></td>" << endl;
  else
    htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

  htmlFile << "</tr>" << endl;
  htmlFile << "</table>" << endl;
  htmlFile << "<br>" << endl;

  htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
  htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
  htmlFile << "<tr align=\"center\">" << endl;

  if ( imgNameMapLL1.size() != 0 )
    htmlFile << "<td><img src=\"" << imgNameMapLL1 << "\" usemap=""#LaserL1"" border=0></td>" << endl;

  htmlFile << "</tr>" << endl;
  htmlFile << "</table>" << endl;
  htmlFile << "<br>" << endl;

  delete cMap;

  if ( imgNameMapI.size() != 0 ) this->writeMap( htmlFile, "Integrity" );
  if ( imgNameMapO.size() != 0 ) this->writeMap( htmlFile, "Occupancy" );
  if ( imgNameMapPO.size() != 0 ) this->writeMap( htmlFile, "PedestalOnline" );
  if ( imgNameMapLL1.size() != 0 ) this->writeMap( htmlFile, "LaserL1" );

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
  hf << "<area shape=\"rect\" href=\"" << refhtml[mapname] << "#" << (j+1)+18*i << "\" coords=\"";
  hf << x0 << ", " << y0 << ", " << x1 << ", " << y1 << "\">" << std::endl;
 }
 hf << "</map>" << std::endl;

}

