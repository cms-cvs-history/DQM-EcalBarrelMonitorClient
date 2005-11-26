/*
 * \file EBCosmicClient.cc
 * 
 * $Date: 2005/11/25 12:57:11 $
 * $Revision: 1.6 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <DQM/EcalBarrelMonitorClient/interface/EBCosmicClient.h>

EBCosmicClient::EBCosmicClient(const edm::ParameterSet& ps, MonitorUserInterface* mui){

  mui_ = mui;

  for ( int i = 0; i < 36; i++ ) {

    h01_[i] = 0;
    h02_[i] = 0;
    h03_[i] = 0;

  }

}

EBCosmicClient::~EBCosmicClient(){

  for ( int i = 0; i < 36; i++ ) {

    if ( h01_[i] ) delete h01_[i];
    if ( h02_[i] ) delete h02_[i];
    if ( h03_[i] ) delete h03_[i];

  }

}

void EBCosmicClient::beginJob(const edm::EventSetup& c){

  cout << "EBCosmicClient: beginJob" << endl;

  ievt_ = 0;

  this->subscribe();

}

void EBCosmicClient::beginRun(const edm::EventSetup& c){

  cout << "EBCosmicClient: beginRun" << endl;

  jevt_ = 0;

  for ( int ism = 1; ism <= 36; ism++ ) {

    if ( h01_[ism-1] ) delete h01_[ism-1];
    if ( h02_[ism-1] ) delete h02_[ism-1];
    if ( h03_[ism-1] ) delete h03_[ism-1];
    h01_[ism-1] = 0;
    h02_[ism-1] = 0;
    h03_[ism-1] = 0;

  }

}

void EBCosmicClient::endJob(void) {

  cout << "EBCosmicClient: endJob, ievt = " << ievt_ << endl;

  this->unsubscribe();

}

void EBCosmicClient::endRun(EcalCondDBInterface* econn, RunIOV* runiov, RunTag* runtag) {

  cout << "EBCosmicClient: endRun, jevt = " << jevt_ << endl;

  if ( jevt_ == 0 ) return;

  EcalLogicID ecid;
//  MonPedestalsDat p;
//  map<EcalLogicID, MonPedestalsDat> dataset;

  cout << "Writing MonCosmicDatObjects to database ..." << endl;

  const float n_min_tot = 1000.;
  const float n_min_bin = 50.;

  for ( int ism = 1; ism <= 36; ism++ ) {

    float num01, num02;
    float mean01, mean02;
    float rms01, rms02;

    for ( int ie = 1; ie <= 85; ie++ ) {
      for ( int ip = 1; ip <= 20; ip++ ) {

        num01  = num02  = -1.;
        mean01 = mean02 = -1.;
        rms01  = rms02  = -1.;

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

        if ( update_channel ) {

          if ( ie == 1 && ip == 1 ) {

            cout << "Inserting dataset for SM=" << ism << endl;

            cout << "Sel (" << ie << "," << ip << ") " << num01  << " "
                                                       << mean01 << " "
                                                       << rms01  << endl;
            cout << "Cut (" << ie << "," << ip << ") " << num02  << " "
                                                       << mean02 << " "
                                                       << rms02  << endl;
          }

//          p.setPedMeanG12(mean03);
//          p.setPedRMSG12(rms03);

//          p.setTaskStatus(1);

          if ( econn ) {
            try {
              ecid = econn->getEcalLogicID("EB_crystal_index", ism, ie-1, ip-1);
//              dataset[ecid] = p;
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
      cout << "Inserting dataset ... " << flush;
//      econn->insertDataSet(&dataset, runiov, runtag );
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

}

void EBCosmicClient::subscribe(void){

  // subscribe to all monitorable matching pattern
  mui_->subscribe("*/EcalBarrel/EBCosmicTask/Sel/EBCT amplitude sel SM*");
  mui_->subscribe("*/EcalBarrel/EBCosmicTask/Cut/EBCT amplitude cut SM*");
  mui_->subscribe("*/EcalBarrel/EBCosmicTask/Spectrum/EBCT amplitude spectrum SM*");

}

void EBCosmicClient::subscribeNew(void){

  // subscribe to new monitorable matching pattern
  mui_->subscribeNew("*/EcalBarrel/EBCosmicTask/Sel/EBCT amplitude sel SM*");
  mui_->subscribeNew("*/EcalBarrel/EBCosmicTask/Cut/EBCT amplitude cut SM*");
  mui_->subscribeNew("*/EcalBarrel/EBCosmicTask/Spectrum/EBCT amplitude spectrum SM*");

}

void EBCosmicClient::unsubscribe(void){

  // unsubscribe to all monitorable matching pattern
  mui_->unsubscribe("*/EcalBarrel/EBCosmicTask/Sel/EBCT amplitude sel SM*");
  mui_->unsubscribe("*/EcalBarrel/EBCosmicTask/Cut/EBCT amplitude cut SM*");
  mui_->unsubscribe("*/EcalBarrel/EBCosmicTask/Spectrum/EBCT amplitude spectrum SM*");

}

void EBCosmicClient::analyze(const edm::Event& e, const edm::EventSetup& c){

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 == 0 )  
    cout << "EBCosmicClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;

  this->subscribeNew();

  Char_t histo[150];

  MonitorElement* me;
  MonitorElementT<TNamed>* ob;

  for ( int ism = 1; ism <= 36; ism++ ) {

    sprintf(histo, "Collector/FU0/EcalBarrel/EBCosmicTask/Sel/EBCT amplitude sel SM%02d", ism);
    me = mui_->get(histo);
    if ( me ) {
      cout << "Found '" << histo << "'" << endl;
      ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
      if ( ob ) {
        if ( h01_[ism-1] ) delete h01_[ism-1];
        h01_[ism-1] = dynamic_cast<TProfile2D*> ((ob->operator->())->Clone());
      }
    }

    sprintf(histo, "Collector/FU0/EcalBarrel/EBCosmicTask/Cut/EBCT amplitude cut SM%02d", ism);
    me = mui_->get(histo);
    if ( me ) {
      cout << "Found '" << histo << "'" << endl;
      ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
      if ( ob ) {
        if ( h02_[ism-1] ) delete h02_[ism-1];
        h02_[ism-1] = dynamic_cast<TProfile2D*> ((ob->operator->())->Clone());
      }
    }

    sprintf(histo, "Collector/FU0/EcalBarrel/EBCosmicTask/Spectrum/EBCT amplitude spectrum SM%02d", ism);
    me = mui_->get(histo);
    if ( me ) {
      cout << "Found '" << histo << "'" << endl;
      ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
      if ( ob ) {
        if ( h03_[ism-1] ) delete h03_[ism-1];
        h03_[ism-1] = dynamic_cast<TH1F*> ((ob->operator->())->Clone());
      }
    }

  }

}

void EBCosmicClient::htmlOutput(int run, string htmlDir, string htmlName){

  cout << "Preparing EBCosmicClient html output ..." << endl;

  ofstream htmlFile;

  htmlFile.open((htmlDir + htmlName).c_str());

  // html page header
  htmlFile << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">  " << endl;
  htmlFile << "<html>  " << endl;
  htmlFile << "<head>  " << endl;
  htmlFile << "  <meta content=\"text/html; charset=ISO-8859-1\"  " << endl;
  htmlFile << " http-equiv=\"content-type\">  " << endl;
  htmlFile << "  <title>Monitor:CosmicTask output</title> " << endl;
  htmlFile << "</head>  " << endl;
  htmlFile << "<style type=\"text/css\"> td { font-weight: bold } </style>" << endl;
  htmlFile << "<body>  " << endl;
  htmlFile << "<br>  " << endl;
  htmlFile << "<h2>Run:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
  htmlFile << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl; 
  htmlFile << " style=\"color: rgb(0, 0, 153);\">" << run << "</span></h2>" << endl;
  htmlFile << "<h2>Monitoring task:&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">COSMIC</span></h2> " << endl;
  htmlFile << "<hr>" << endl;
  htmlFile << "<table border=1><tr><td bgcolor=red>channel has problems in this task</td>" << endl;
  htmlFile << "<td bgcolor=lime>channel has NO problems</td>" << endl;
  htmlFile << "<td bgcolor=white>channel is missing</td></table>" << endl;
  htmlFile << "<hr>" << endl;

  // Produce the plots to be shown as .jpg files from existing histograms

  int csize = 250;

  double histMax = 1.e15;

  int pCol4[10];
  for( int i=0; i<10; i++ ) pCol4[i] = 30+i;
//  pCol4[0] = 10;


  TH2C dummy( "dummy", "dummy for sm", 85, 0., 85., 20, 0., 20. );
  for( int i = 0; i < 68; i++ ) {
    int a = 2 + ( i/4 ) * 5;
    int b = 2 + ( i%4 ) * 5;
    dummy.Fill( a, b, i+1 );
  }
  dummy.SetMarkerSize(2);

  string imgNameME[3], imgName, meName;

  // Loop on barrel supermodules

  for ( int ism = 1 ; ism <= 36 ; ism++ ) {
    
    if ( h01_[ism-1] && h02_[ism-1] && h03_[ism-1] ) {

      // Monitoring elements plots
      
      for ( int iCanvas = 1; iCanvas <= 2; iCanvas++ ) {

        TProfile2D* obj2f = 0;
      
        switch ( iCanvas ) {
        case 1:
          meName = h01_[ism-1]->GetName();
          obj2f = h01_[ism-1];
          break;
        case 2:
          meName = h02_[ism-1]->GetName();
          obj2f = h02_[ism-1];
          break;
        default:
          break;
        }
        
        TCanvas *cMe = new TCanvas("cMe" , "Temp", 2*csize , csize );
        for ( unsigned int iMe = 0 ; iMe < meName.size(); iMe++ ) {
          if ( meName.substr(iMe, 1) == " " )  {
            meName.replace(iMe, 1, "_");
          }
        }
        imgNameME[iCanvas-1] = meName + ".jpg";
        imgName = htmlDir + imgNameME[iCanvas-1];
        gStyle->SetOptStat(" ");
        gStyle->SetPalette( 10, pCol4 );
        obj2f->GetXaxis()->SetNdivisions(17);
        obj2f->GetYaxis()->SetNdivisions(4);
        cMe->SetGridx();
        cMe->SetGridy();
//        obj2f->SetMinimum(-0.00000001);
        obj2f->SetMaximum();
        obj2f->Draw("colz");
        dummy.Draw("text,same");
        cMe->Update();
        cMe->SaveAs(imgName.c_str());
        delete cMe;

      }

      // Energy spectrum distributions
      
      TH1F* obj1f = 0; 
    
      meName = h03_[ism-1]->GetName();
      obj1f = h03_[ism-1];
    
      TCanvas *cAmp = new TCanvas("cAmp" , "Temp", csize , csize );
      for ( unsigned int iAmp=0 ; iAmp < meName.size(); iAmp++ ) {
        if ( meName.substr(iAmp,1) == " " )  {
          meName.replace(iAmp, 1 ,"_" );
        }
      }
      imgNameME[2] = meName + ".jpg";
      imgName = htmlDir + imgNameME[2];
      gStyle->SetOptStat("euomr");
      obj1f->SetStats(kTRUE);
      if ( obj1f->GetMaximum(histMax) > 0. ) {
        gPad->SetLogy(1);
      } else {
        gPad->SetLogy(0);
      }
      obj1f->Draw();
      cAmp->Update();
      TPaveStats* stAmp = dynamic_cast<TPaveStats*>(obj1f->FindObject("stats"));
      if ( stAmp ) {
        stAmp->SetX1NDC(0.6);
        stAmp->SetY1NDC(0.75);
      }
      cAmp->SaveAs(imgName.c_str());
      gPad->SetLogy(0);
      delete cAmp;

    }
      
    htmlFile << "<h3><strong>Supermodule&nbsp;&nbsp;" << ism << "</strong></h3>" << endl;
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    for ( int iCanvas = 1 ; iCanvas <= 2 ; iCanvas++ ) {

      if ( imgNameME[iCanvas-1].size() != 0 ) 
        htmlFile << "<td colspan=\"2\"><img src=\"" << imgNameME[iCanvas-1] << "\"></td>" << endl;
      else
        htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

    }
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    if ( imgNameME[2].size() != 0 )
      htmlFile << "<td colspan=\"2\"><img src=\"" << imgNameME[2] << "\"></td>" << endl;
    else
      htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

  }

  // html page footer
  htmlFile << "</body> " << endl;
  htmlFile << "</html> " << endl;

  htmlFile.close();

}
