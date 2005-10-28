/*
 * \file EcalBarrelMonitorLaserClient.cpp
 *
 *  $Date: 2005/10/28 13:28:14 $
 *  $Revision: 1.6 $
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

#include <signal.h>

using namespace std;

TCanvas* c1;
TCanvas* c2;

MonitorUserInterface* mui;

bool exit_now = false;
bool exit_done = false;

void ctr_c_intr(int sig) {
        
  cout << "*** Exit the program by selecting Quit from the File menu ***" << endl;
//  exit_now = true;

  return;
}

void *pth1(void *) {

  bool stay_in_loop = true;

  // last time monitoring objects were plotted
  int last_plotting = -1;

  // last time root-file was saved
  int last_save = -1;

  while ( stay_in_loop && ! exit_now ) {

    bool saveHistograms = false;
  
    // this is the "main" loop where we receive monitoring
    stay_in_loop = mui->update();

    // subscribe to new monitorable matching pattern
    mui->subscribeNew("*/EcalBarrel/STATUS");
    mui->subscribeNew("*/EcalBarrel/RUN"); 
    mui->subscribeNew("*/EcalBarrel/EVT");
    mui->subscribeNew("*/EcalBarrel/EBLaserTask/Laser1/EBLT shape SM*");
    mui->subscribeNew("*/EcalBarrel/EBLaserTask/Laser1/EBLT amplitude SM*");

    // # of full monitoring cycles processed
    int updates = mui->getNumUpdates();

    MonitorElement* me;

    // draw monitoring objects every 5 monitoring cycles
    if ( updates % 5 == 0 && updates != last_plotting ) {

      me = mui->get("Collector/FU0/EcalBarrel/STATUS");
      if ( me ) {
        string s = me->valueString();
        string status = "unknown";
        if ( s.substr(2,1) == "0" ) status = "start-of-run";
        if ( s.substr(2,1) == "1" ) status = "running";
        if ( s.substr(2,1) == "2" ) status = "end-of-run";
        cout << "status = " << status << endl;
        if ( status == "end-of-run" ) {
          TThread::Lock();      
          mui->save("EcalBarrelMonitorLaserClient.root");
          TThread::UnLock();
        }
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

      me = mui->get("Collector/FU0/EcalBarrel/EBLaserTask/Laser1/EBLT shape SM01 L1");
      if ( me ) {
        MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
        if ( ob ) {
          TProfile2D* h = dynamic_cast<TProfile2D*> (ob->operator->());
          if ( h ) {
            c1->cd();
            h->SetOption("lego");
            h->Draw();
          }
        }
      }

      me = mui->get("Collector/FU0/EcalBarrel/EBLaserTask/Laser1/EBLT amplitude SM01 L1");
      if ( me ) {
        MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
        if ( ob ) {
          TProfile2D* h = dynamic_cast<TProfile2D*> (ob->operator->());
          if ( h ) {
            c2->cd();
            h->SetOption("col");
            h->Draw();
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
    if ( saveHistograms ) {
      TThread::Lock();
      mui->save("EcalBarrelMonitorLaserClient.root");
      TThread::UnLock();
    }
  }

  exit_done = true;

  return 0;
}

int main(int argc, char** argv) {

  cout << endl;
  cout << " *** Ecal Barrel Laser Monitor Client ***" << endl;
  cout << endl;

  signal(SIGINT, ctr_c_intr);

  TApplication app("app", &argc, argv);

  // default client name
  string cfuname = "UserLaser";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  c1 = new TCanvas("Ecal Barrel Laser Monitoring Amplitude L1","Ecal Barrel Laser Monitoring Amplitude L1", 0,310,500,500);
  c1->Modified();
  c1->Update();
  c2 = new TCanvas("Ecal Barrel Laser Monitoring Shape L1","Ecal Barrel Laser Monitoring Shape L1", 0, 0,800,250);
  c2->Modified();
  c2->Update();

  if ( argc >= 2 ) cfuname = argv[1];
  if ( argc >= 3 ) hostname = argv[2];

  cout << " Client " << cfuname
       << " begins requesting monitoring from host " << hostname << endl;

  // start user interface instance
  mui = new MonitorUIRoot(hostname, port_no, cfuname);

  mui->setVerbose(1);

  // will attempt to reconnect upon connection problems (w/ a 5-sec delay)
  mui->setReconnectDelay(5);

  // subscribe to all monitorable matching pattern
  mui->subscribe("*/EcalBarrel/STATUS");
  mui->subscribe("*/EcalBarrel/RUN");
  mui->subscribe("*/EcalBarrel/EVT");
  mui->subscribe("*/EcalBarrel/EBLaserTask/Laser1/EBLT shape SM*");
  mui->subscribe("*/EcalBarrel/EBLaserTask/Laser1/EBLT amplitude SM*");

  TThread *th1 = new TThread("th1",pth1);

  th1->Run();

  try { app.Run(kTRUE); } catch (...) { throw; }

  mui->unsubscribe("*");

  exit_now = true;

  while ( ! exit_done ) { usleep(100); }

  th1->Delete();

  delete mui;

  return 0;

}

