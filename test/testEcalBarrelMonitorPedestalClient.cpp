/*
 * \file EcalBarrelMonitorPedestalClient.cpp
 *
 *  $Date: 2005/10/13 14:33:16 $
 *  $Revision: 1.5 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "TROOT.h"
#include "TH2D.h"
#include "TApplication.h"

#include <iostream>
#include <math.h>

using namespace std;

int main(int argc, char** argv) {
  cout << endl;
  cout << " *** Ecal Barrel Pedestal Monitor Client ***" << endl;
  cout << endl;

  TApplication app("app",&argc,argv);

  // default client name
  string cfuname = "User0";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  TCanvas* c1 = new TCanvas("Ecal Barrel Pedestal Monitoring G01","Ecal Barrel Pedestal Monitoring G01", 70,  0,500,400);
  c1->Draw();
  c1->Modified();
  c1->Update();
  TCanvas* c2 = new TCanvas("Ecal Barrel Pedestal Monitoring G06","Ecal Barrel Pedestal Monitoring G06",600,  0,500,400);
  c2->Draw();
  c2->Modified();
  c2->Update();
  TCanvas* c3 = new TCanvas("Ecal Barrel Pedestal Monitoring G12","Ecal Barrel Pedestal Monitoring G12",600,450,500,400);
  c3->Draw();
  c3->Modified();
  c3->Update();

  if(argc >= 2) cfuname = argv[1];
  if(argc >= 3) hostname = argv[2];

  cout << " Client " << cfuname
       << " begins requesting monitoring from host " << hostname << endl;

  // start user interface instance
  MonitorUserInterface* mui = new MonitorUIRoot(hostname,port_no,cfuname);

  mui->setVerbose(0);

  // will attempt to reconnect upon connection problems (w/ a 5-sec delay)
  mui->setReconnectDelay(5);

  // subscribe to all monitorable matching pattern
  mui->subscribe("EcalBarrel/STATUS");
  mui->subscribe("EcalBarrel/RUN");
  mui->subscribe("EcalBarrel/EVT");
  mui->subscribe("EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM*");
  mui->subscribe("EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM*");
  mui->subscribe("EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM*");

  bool stay_in_loop = true;

  // last time monitoring objects were plotted
  int last_plotting = -1;

  // last time root-file was saved
  int last_save = -1;

  while ( stay_in_loop ) {

      bool saveHistograms = false;
  
      // this is the "main" loop where we receive monitoring
      stay_in_loop = mui->update();

      // subscribe to new monitorable matching pattern
      mui->subscribeNew("EcalBarrel/STATUS");
      mui->subscribeNew("EcalBarrel/RUN"); 
      mui->subscribeNew("EcalBarrel/EVT");
      mui->subscribeNew("EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM*");
      mui->subscribeNew("EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM*");
      mui->subscribeNew("EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM*");

      // # of full monitoring cycles processed
      int updates = mui->getNumUpdates();

      // draw monitoring objects every 2 monitoring cycles
      if(updates % 2 == 0 && updates != last_plotting) {

          MonitorElement* me;

          me = mui->get("Collector/FU0/EcalBarrel/STATUS");
          if ( me ) {
            string s = me->valueString();
            string status = "unknown";
            if ( s.substr(2,1) == "0" ) status = "start-of-run";
            if ( s.substr(2,1) == "1" ) status = "running";
            if ( s.substr(2,1) == "2" ) status = "end-of-run";
            cout << "status = " << status << endl;
//            if ( status == "end-of-run" ) stay_in_loop = false;
          }

          me = mui->get("Collector/FU0/EcalBarrel/RUN");
          if ( me ) {
            string s = me->valueString();
            string run = s.substr(2,s.length()-2);
            cout << "run = " << run << endl;
          }

          me = mui->get("Collector/FU0/EcalBarrel/EVT");
          if ( me ) {
            string s = me->valueString();
            string evt = s.substr(2,s.length()-2);
            cout << "event = " << evt.c_str() << endl;
          }

          me = mui->get("Collector/FU0/EcalBarrel/EBPedestalTask/Gain01/EBPT pedestal SM01 G01");
          if ( me ) {
            MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
            if ( ob ) {
              TProfile2D* h = dynamic_cast<TProfile2D*> (ob->operator->());
              if ( h ) {
                h->SetMaximum(4096.);
                c1->cd();
                h->Draw("box");
                c1->Modified();
                c1->Update();
              }
            }
          }

          me = mui->get("Collector/FU0/EcalBarrel/EBPedestalTask/Gain06/EBPT pedestal SM01 G06");
          if ( me ) {
            MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
            if ( ob ) {
              TProfile2D* h = dynamic_cast<TProfile2D*> (ob->operator->());
              if ( h ) {
                h->SetMaximum(4096.);
                c2->cd();
                h->Draw("box");
                c2->Modified();
                c2->Update();
              }
            }
          }

          me = mui->get("Collector/FU0/EcalBarrel/EBPedestalTask/Gain12/EBPT pedestal SM01 G12");
          if ( me ) {
            MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
            if ( ob ) {
              TProfile2D* h = dynamic_cast<TProfile2D*> (ob->operator->());
              if ( h ) {
                h->SetMaximum(4096.);
                c3->cd();
                h->Draw("box");
                c3->Modified();
                c3->Update();
              }
            }
          }

          last_plotting = updates;
        }

      // come here every 100 monitoring cycles, operate on Monitoring Elements
      if( updates % 100 == 0 && updates != last_save ) {

          saveHistograms = true;

          last_save = updates;
      }

      // save monitoring structure in root-file
      if ( saveHistograms ) mui->save("EcalBarrelMonitorClient.root");

    }

  // if here (ie. Collector has stopped), save into root file
  mui->save("EcalBarrelMonitorClient.root");

  delete mui;

  return 0;
}
