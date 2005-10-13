/*
 * \file EcalBarrelMonitorIntegrityClient.cpp
 *
 *  $Date: 2005/10/13 07:21:39 $
 *  $Revision: 1.1 $
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
  cout << " *** Ecal Barrel Monitor Integrity ***" << endl;
  cout << endl;

  TApplication app("app",&argc,argv);

  // default client name
  string cfuname = "User0";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  TCanvas* c1 = new TCanvas("Ecal Barrel Integrity Monitoring 1","Ecal Barrel Integrity Monitoring 1", 10,10,550,480);
  c1->Draw();
  c1->Modified();
  c1->Update();
  TCanvas* c2 = new TCanvas("Ecal Barrel Integrity Monitoring 2","Ecal Barrel Integrity Monitoring 2",600,10,550,480);
  c2->Divide(2,2);
  c2->Draw();
  c2->Modified();
  c2->Update();

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
  mui->subscribe("EcalBarrel/RUN");
  mui->subscribe("EcalBarrel/EVT");
  mui->subscribe("EcalBarrel/EBMonitorEvent/EB*SM*");

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
      mui->subscribeNew("EcalBarrel/RUN");
      mui->subscribeNew("EcalBarrel/EVT");
      mui->subscribeNew("EcalBarrel/EBMonitorEvent/EB*SM*");

      // # of full monitoring cycles processed
      int updates = mui->getNumUpdates();

      // draw monitoring objects every 2 monitoring cycles
      if(updates % 2 == 0 && updates != last_plotting) {

          MonitorElement* me;

          me = mui->get("Collector/FU0/EcalBarrel/EVT");
          if ( me ) {
            cout << me->valueString() << endl;
          }

          me = mui->get("Collector/FU0/EcalBarrel/EBMonitorEvent/EBMM event SM01");
          if ( me ) {
            MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
            if ( ob ) {
              TH2F* h = dynamic_cast<TH2F*> (ob->operator->());
              if ( h ) {
                h->SetMaximum(4096.);
                c1->cd();
                h->Draw("box");
                c1->Modified();
                c1->Update();
              }
            }
          }

          me = mui->get("Collector/FU0/EcalBarrel/EBMonitorEvent/EBMM event SM02");
          if ( me ) {
            MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
            if ( ob ) {
              TH2F* h = dynamic_cast<TH2F*> (ob->operator->());
              if ( h ) {
                h->SetMaximum(4096.);
                c2->cd(1);
                h->Draw("box");
                c2->Modified();
                c2->Update();
              }
            }
          }

          me = mui->get("Collector/FU0/EcalBarrel/EBMonitorEvent/EBMM event SM03");
          if ( me ) {
            MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
            if ( ob ) {
              TH2F* h = dynamic_cast<TH2F*> (ob->operator->());
              if ( h ) {
                h->SetMaximum(4096.);
                c2->cd(2);
                h->Draw("box");
                c2->Modified();
                c2->Update();
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
