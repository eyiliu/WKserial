#include "compatible.hh"

#include <sstream>
#include <iostream>

TempInterlock::TempInterlock(std::string devpath)
  :WKserial(devpath), lastUpdateTime(0){

  connect();
  set_endline("\r");
  set_speed(115200);
  set_hardware_flow_confrol(0);
  set_software_flow_control(0);
  set_raw_output(1);
  set_raw_intput(1);
  set_parity(8,1,'N');
  set_read_timeout_msec(1000);
  set_write_interval_msec(100);
}


double TempInterlock::getTemp(int channel)
{
  if(time(0)-lastUpdateTime>5){
    UpdateTempVec();   //"idr, 100"
    lastUpdateTime = time(0);
  }
  if((size_t)channel < lastTempVec.size())
    return lastTempVec[channel];
  else
    return -3000; //error
}

int TempInterlock::UpdateTempVec(){
  std::string data;
  scpi_send_command("idr, 100");
  scpi_receive_data(data, 1024);  //TODO: buffer size
  if(data.size() == 0){
    fprintf(stderr,"[TempInterlock::UpdateTempVec] Error: Can not receive data!\n");
    return -1;
  }
  // since the expected structure of the answer is "\r\nReceived: <command>\r\n..."
  // the program checks if it can find the command string in the answer string.
  // the device does not return the received command but the understood command this can give a problem.
  // for example: if one tries to send the command "idr,100" to the device it will understand it correctly but the response from the device is:
  // "\r\nReceived: idr, 100\r\n..." where you have a space between idr, and 100. one has to be very careful to use the command correct.

  lastTempVec.clear();
  std::stringstream ss(data);
  std::string substr;
  std::getline(ss, substr, ':');
  std::getline(ss, substr);
  std::getline(ss, substr);

  while(std::getline(ss, substr, ',')){
    std::stringstream subss(substr);
    double val;
    subss>>val;
    lastTempVec.push_back(val);
  }

  return 0;
}


Chiller_Medingen_C20::Chiller_Medingen_C20(std::string devpath)
  :WKserial(devpath){

  connect();
  set_endline("\r\n");
  set_speed(9600);
  set_hardware_flow_confrol(0);
  set_software_flow_control(0);
  set_raw_output(1);
  set_raw_intput(1);
  set_parity(8,1,'N');
  set_read_timeout_msec(500);
  set_write_interval_msec(100);
}

double Chiller_Medingen_C20::getActualTemp( void )
{
  scpi_send_command("IN PV 1");
  std::string datain;
  return scpi_obtain_double_value();
}

double Chiller_Medingen_C20::getTargetTemp( void )
{
  scpi_send_command("IN SP 1");
  std::string datain;
  return scpi_obtain_double_value();
}

int Chiller_Medingen_C20::setTargetTemp( int temp )
{
  std::stringstream ss;
  ss<<"OUT SP 1 "<<temp;
  scpi_send_command(ss.str());
  return 0;
}

double Chiller_Medingen_C20::getStatus( void )
{
  scpi_send_command("STATUS");
  std::string datain;
  return scpi_obtain_double_value();
}



Keithley2410::Keithley2410(std::string devpath)
:WKserial(devpath){
  connect();
  set_endline("\n");
  set_speed(57600);
  set_hardware_flow_confrol(0);
  set_software_flow_control(0);
  set_raw_output(1);
  set_raw_intput(1);
  set_parity(8,1,'N');
  set_read_timeout_msec(500);
  set_write_interval_msec(100);
  // scpi_send_command(":SENS:FUNC:ON 'VOLT:DC'");
  // scpi_send_command(":SENS:FUNC:ON 'CURR:DC'");

}

double Keithley2410::getVoltage()
{
  scpi_send_command(":READ?");
  std::vector<double> vd;
  scpi_obtain_double_vector(vd);  
  if (vd.size()>2){
    return vd[0];
  }
  return -100000;
}

double Keithley2410::getCurrent()
{
  scpi_send_command(":READ?");
  std::vector<double> vd;
  scpi_obtain_double_vector(vd);  
  if (vd.size()>2){
    return vd[1];
  }
  return -100000;
}

int Keithley2410::setStatus( bool on_off ){
  if(on_off == 0){
    scpi_send_command(":OUTP:STAT 0");
  }
  else{
    scpi_send_command(":OUTP:STAT 1");
  }
  return 0;
}

int Keithley2410::setVoltage( double Voltage )
{
  scpi_send_command(":SOUR:VOLT:MODE FIX");
  std::stringstream ss0;
  ss0<<":SOUR:VOLT:RANG "<<Voltage;
  scpi_send_command(ss0.str());
  std::stringstream ss1;
  ss1<<":SOUR:VOLT:LEV "<<Voltage;
  scpi_send_command(ss1.str());
  return 0;
}

int Keithley2410::setCurrent( double current )
{
  scpi_send_command(":SOUR:CURR:MODE FIX");
  std::stringstream ss0;
  ss0<<":SOUR:CURR:RANG "<<current;
  scpi_send_command(ss0.str());
  std::stringstream ss1;
  ss1<<":SOUR:CURR:LEV "<<current;
  scpi_send_command(ss1.str());
  return 0;
}

double Keithley2410::getCurrentRange()
{
  scpi_send_command(":CURR:RANG?");
  return scpi_obtain_double_value();
}

int Keithley2410::setCurrentRange( double current )
{
  std::stringstream ss;
  ss<<":CURR:RANG "<<current;
  scpi_send_command(ss.str());
  return 0;
}


void Keithley2410::setAutoRange()
{
  scpi_send_command(":SENS:CURR:RANG:AUTO 1");
  scpi_send_command(":SENS:VOLT:RANG:AUTO 1");
}

int Keithley2410::setOverVoltageProtection( double Voltage )
{
  std::stringstream ss;
  ss<<":VOLT:PROT:LEV "<<Voltage;
  scpi_send_command(ss.str());
  return 0;
}

int Keithley2410::setOverCurrentProtection( double Current )
{
  std::stringstream ss;
  ss<<":CURR:PROT:LEV "<<Current;
  scpi_send_command(ss.str());
  return 0;
}

int Keithley2410::setVoltageSource()
{
  scpi_send_command(":SOUR:FUNC VOLT");
  return 0;
}

int Keithley2410::setCurrentSource()
{
  scpi_send_command(":SOUR:FUNC CURR");
  return 0;
}


tti_CPX400::tti_CPX400(std::string devpath)
  :WKserial(devpath){
  
  connect();
  set_endline("\n");
  set_speed(9600);
  set_hardware_flow_confrol(0);
  set_software_flow_control(0);
  set_raw_output(1);
  set_raw_intput(1);
  set_parity(8,1,'N');
  set_read_timeout_msec(500);
  set_write_interval_msec(100);
}


double tti_CPX400::getVoltage( int channel )
{
  std::stringstream ss0;
  ss0<<"V"<<channel<<"?";
  scpi_send_command(ss0.str());

  std::string data;
  scpi_receive_data(data, 50);
  data = data.substr(2); //remove first 2 chars
  std::stringstream ss1(data);
  double value;
  ss1>>value;
  return value; 
}



double tti_CPX400::getCurrent( int channel )
{
  std::stringstream ss0;
  ss0<<"I"<<channel<<"?";
  scpi_send_command(ss0.str());

  std::string data;
  scpi_receive_data(data, 50);
  data = data.substr(2); //remove first 2 chars
  std::stringstream ss1(data);
  double value;
  ss1>>value;
  return value;
}

double tti_CPX400::getActualVoltage( int channel )
{
  std::stringstream ss0;
  ss0<<"V"<<channel<<"O?";
  scpi_send_command(ss0.str());

  std::string data;
  scpi_receive_data(data, 50);
  std::stringstream ss1(data);
  double value;
  ss1>>value;
  return value; 
}



double tti_CPX400::getActualCurrent( int channel )
{
  std::stringstream ss0;
  ss0<<"I"<<channel<<"O?";
  scpi_send_command(ss0.str());

  std::string data;
  scpi_receive_data(data, 50);
  std::stringstream ss1(data);
  double value;
  ss1>>value;
  return value;
}




double tti_CPX400::getOverVoltageProtection( int channel )
{
  std::stringstream ss0;
  ss0<<"OVP"<<channel<<"?";
  scpi_send_command(ss0.str());

  std::string data;
  scpi_receive_data(data, 50);
  std::stringstream ss1(data);
  double value;
  ss1>>value;
  return value; 
}



double tti_CPX400::getOverCurrentProtection( int channel )
{
  std::stringstream ss0;
  ss0<<"OCP"<<channel<<"?";
  scpi_send_command(ss0.str());

  std::string data;
  scpi_receive_data(data, 50);
  // data = data.substr(5); //remove first 5 chars
  std::stringstream ss1(data);
  double value;
  ss1>>value;
  return value;
}


int tti_CPX400::setVoltage( int channel,double voltage )
{
  std::stringstream ss;
  ss<<"V"<<channel<<" "<<voltage;
  scpi_send_command(ss.str());
  return 0;
}


int tti_CPX400::setCurrent( int channel,double current )
{
  std::stringstream ss;
  ss<<"I"<<channel<<" "<<current;
  scpi_send_command(ss.str());
  return 0;
}


int tti_CPX400::setOverVoltageProtection( int channel,double voltage )
{
  std::stringstream ss;
  ss<<"OVP"<<channel<<" "<<voltage;
  scpi_send_command(ss.str());
  return 0;
}


int tti_CPX400::setOverCurrentProtection( int channel,double current )
{
  std::stringstream ss;
  ss<<"OCP"<<channel<<" "<<current;
  scpi_send_command(ss.str());
  return 0;
}


int tti_CPX400::setDeltaVoltage(int channel,double voltage){
  std::stringstream ss;
  ss<<"DELTAV"<<channel<<" "<<voltage;
  scpi_send_command(ss.str());
  return 0;
}

int tti_CPX400::GetNumberOfChannels(){
  if(scpi_is_device("CPX400SP"))
    return 1;
  if(scpi_is_device("CPX400DP"))
    return 2;
  return 0;
}


int tti_CPX400::setStatus( int channel, bool on_off )  //manual need
{
  std::stringstream ss;
  ss<<"OP"<<channel<<" "<<on_off;
  scpi_send_command(ss.str());
  return 0;
}

