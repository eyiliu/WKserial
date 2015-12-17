#ifndef COMPATIABLE_HH
#define COMPATIABLE_HH

#include <vector>
#include <string>

#include <time.h>

#include "serial.hh"

class TempInterlock:public WKserial{
public:
  TempInterlock(std::string devpath);
  double getTemp(int channel);
  int UpdateTempVec();
private:
  time_t  lastUpdateTime;
  std::vector<double> lastTempVec;

};


class Chiller_Medingen_C20:public WKserial{
public:
  Chiller_Medingen_C20(std::string devpath);
  double getStatus( void );
  double getActualTemp( void );
  double getTargetTemp( void );
  int setTargetTemp( int temp );
  
private:

 
};


class Keithley2410:public WKserial{
public:
  Keithley2410(std::string devpath);
  int  setStatus(bool on_off); //set output on/off where <on_off> has the following meaning: 0=OFF, 1=ON
  int  setVoltage(double Voltage);//Set output to <Voltage> Volts
  int  setCurrent(double current);// Set output current limit to <current> Amps
  void setAutoRange();
  int setCurrentRange( double current );
  int setOverVoltageProtection(double Voltage); // keep in mind these function are not very precise
  int setOverCurrentProtection(double Current);
  int setVoltageSource();
  int setCurrentSource();

  //get
  double getVoltage();
  double getCurrent();
  double getCurrentRange();
    
private:
  
};



class tti_CPX400:public WKserial{
public:
  tti_CPX400(std::string devpath);
  double  getVoltage(int channel);
  double  getActualVoltage(int channel);
  double  getCurrent(int channel);
  double  getActualCurrent(int channel);
  double  getOverVoltageProtection(int channel);
  double  getOverCurrentProtection(int channel);
  // bool   getStatus(int channel);//Returns output <channel> on/off
  int  setStatus(int channel, bool on_off);
  int  setVoltage(int channel,double voltage);
  int  setOverVoltageProtection(int channel, double voltage);
  int  setCurrent(int channel,double current);
  int  setOverCurrentProtection(int channel,double current);
  int  setDeltaVoltage(int channel,double voltage);
  int  GetNumberOfChannels();
 
};


#endif
