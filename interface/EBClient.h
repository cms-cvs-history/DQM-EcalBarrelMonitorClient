// $Id: EBClient.h,v 1.1 2006/05/26 07:28:11 benigno Exp $

/*!
  \file EBClient.h
  \brief Ecal Barrel Monitor Client mom class
  \author B. Gobbo 
  \version $Revision: 1.1 $
  \date $Date: 2006/05/26 07:28:11 $
*/


#ifndef EBClient_H
#define EBClient_H

#include <vector>
#include <string>

class EcalCondDBInterface; 
class MonRunIOV;

class EBClient {
  
 public:

  /*! \fn virtual void subscribe(void) 
    \brief Subscribe to Monitoring Elements 
  */
  virtual void subscribe(void)    = 0;

  /*! \fn virtual void subscribeNew(void) 
    \brief Subscribe to Monitoring Elements 
  */
  virtual void subscribeNew(void) = 0;

  /*! \fn virtual void unsubscribe(void) 
    \brief Unsubscribe to Monitoring Elements 
  */
  virtual void unsubscribe(void)  = 0;

  /*! \fn virtual void analyze(void) 
    \brief analyze method
  */
  virtual void analyze(void)      = 0;

  /*! \fn virtual void beginJob(void) 
    \brief Begin of job method
  */
  virtual void beginJob(void)     = 0;

  /*! \fn virtual void endJob(void) 
    \brief End of Job method
  */
  virtual void endJob(void)       = 0;

  /*! \fn virtual void beginRun(void) 
    \brief Begin of Run method
  */
  virtual void beginRun(void)     = 0;

  /*! \fn virtual void endRun(void) 
    \brief End of Run method
  */
  virtual void endRun(void)       = 0;

  /*! \fn virtual void setup(void) 
    \brief setup method
  */
  virtual void setup(void)        = 0;

  /*! \fn virtual void cleanup(void) 
    \brief Clean up method
  */
  virtual void cleanup(void)      = 0;

  /*! \fn virtual void htmlOutput(int run, string htmlDir, string htmlName);
    \brief create HTML page
    \param run run number
    \param htmlDir path to HTML file
    \param htmlName HTML file name

  */
  virtual void htmlOutput(int run, string htmlDir, string htmlName) = 0;

  /*! \fn virtual void writeDb(EcalCondDBInterface* econn, MonRunIOV* moniov, int ism);
    \brief Write data to DataBase
    \param econn DB interface
    \param moniov IOV interface
    \param ism Supermodule id
  */
  virtual void writeDb(EcalCondDBInterface* econn, MonRunIOV* moniov, int ism) = 0;

  virtual ~EBClient(void) {}

};

#endif // EBClient_H

