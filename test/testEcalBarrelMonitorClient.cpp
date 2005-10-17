/*
 * \file EcalBarrelMonitorClient.cpp
 *
 *  $Date: 2005/10/16 13:56:33 $
 *  $Revision: 1.15 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "TROOT.h"
#include "TApplication.h"
#include "TThread.h"

#include <iostream>
#include <math.h>

using namespace std;

TCanvas* c1;

MonitorUserInterface* mui;

void *mhs1(void *) {

//  TThread::Printf("Start of mhs1");

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
    mui->subscribeNew("EcalBarrel/EBMonitorEvent/EBMM event SM*");

    // # of full monitoring cycles processed
    int updates = mui->getNumUpdates();

    MonitorElement* me;

    // draw monitoring objects every 2 monitoring cycles
    if ( updates % 2 == 0 && updates != last_plotting ) {

      me = mui->get("Collector/FU0/EcalBarrel/STATUS");
      if ( me ) {
        string s = me->valueString();
        string status = "unknown";
        if ( s.substr(2,1) == "0" ) status = "start-of-run";
        if ( s.substr(2,1) == "1" ) status = "running";
        if ( s.substr(2,1) == "2" ) status = "end-of-run";
        cout << "status = " << status << endl;
        if ( status == "end-of-run" ) mui->save("EcalBarrelMonitorClient.root");
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

      me = mui->get("Collector/FU0/EcalBarrel/EBMonitorEvent/EBMM event SM01");
      if ( me ) {
        MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
        if ( ob ) {
          TH2F* h = dynamic_cast<TH2F*> (ob->operator->());
          if ( h ) {
            h->SetMaximum(4096.);
            h->Draw("box");
            c1->Modified();
            c1->Update();
          }
        }
      }

      last_plotting = updates;
    }

    // come here every 100 monitoring cycles, operate on Monitoring Elements
    if ( updates % 100 == 0 && updates != last_save ) {

      saveHistograms = true;

      last_save = updates;
    }

    // save monitoring structure in root-file
    if ( saveHistograms ) mui->save("EcalBarrelMonitorClient.root");

    gSystem->Sleep(1);
  }

  mui->save("EcalBarrelMonitorClient.root");

//  TThread::Printf("End of mhs1\n");

  c1->Modified();
  c1->Update();

  return 0;
}

int main(int argc, char** argv) {
  cout << endl;
  cout << " *** Ecal Barrel Generic Monitor Client ***" << endl;
  cout << endl;

  TApplication app("app",&argc,argv);

  // default client name
  string cfuname = "User0";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  c1 = new TCanvas("Ecal Barrel Generic Monitoring","Ecal Barrel Generic Monitoring",200,10,600,480);
  c1->Draw();
  c1->Modified();
  c1->Update();

  if(argc >= 2) cfuname = argv[1];
  if(argc >= 3) hostname = argv[2];

  cout << " Client " << cfuname
       << " begins requesting monitoring from host " << hostname << endl;

  // start user interface instance
  mui = new MonitorUIRoot(hostname,port_no,cfuname);

  mui->setVerbose(0);

  // will attempt to reconnect upon connection problems (w/ a 5-sec delay)
  mui->setReconnectDelay(5);

  // subscribe to all monitorable matching pattern
  mui->subscribe("EcalBarrel/STATUS");
  mui->subscribe("EcalBarrel/RUN");
  mui->subscribe("EcalBarrel/EVT");
  mui->subscribe("EcalBarrel/EBMonitorEvent/EBMM event SM*");

  TThread *th1 = new TThread("th1",mhs1);

  th1->Run();

  app.Run(kTRUE);

  th1->SetCancelAsynchronous();

  th1->Kill();

  delete mui;

  return 0;

}

