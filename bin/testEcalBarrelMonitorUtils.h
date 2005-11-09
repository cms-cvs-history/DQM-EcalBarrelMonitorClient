/*
 * \file EcalBarrelMonitorWriteClient.h
 *
 *  $Date: 2005/11/08 18:31:54 $
 *  $Revision: 1.9 $
 *  \author G. Della Ricca
 *
 */

#include "TROOT.h"

TProfile2D* getTProfile2D(MonitorElement* me) {

  MonitorElementT<TNamed>* ob;
  TProfile2D* h = 0;

  if ( me ) {
    ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
    if ( ob ) {
      h = dynamic_cast<TProfile2D*> (ob->operator->());
    }
  }

  return h;

}

TH1F* getTH1F(MonitorElement* me) {

  MonitorElementT<TNamed>* ob;
  TH1F* h = 0;

  if ( me ) {
    ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
    if ( ob ) {
      h = dynamic_cast<TH1F*> (ob->operator->());
    }
  }

  return h;

}

TH2F* getTH2F(MonitorElement* me) {

  MonitorElementT<TNamed>* ob;
  TH2F* h = 0;

  if ( me ) {
    ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
    if ( ob ) {
      h = dynamic_cast<TH2F*> (ob->operator->());
    }
  }

  return h;

}

