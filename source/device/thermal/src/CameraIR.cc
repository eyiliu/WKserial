#include"CameraIR.hh"

#include<iostream>
CameraIR::CameraIR(std::string devpath)
  :WKserial(devpath){
  connect();
  set_endline("\n");
  set_speed(38400);
  set_hardware_flow_control(0);
  set_software_flow_control(0);
  set_raw_output(1);
  set_raw_intput(1);
  set_parity(8,1,'N');
  set_read_timeout_msec(500);
  set_write_interval_msec(100);  
}

double CameraIR::GetTemp(){
  std::string cmd="?camtemp";
  write_line(cmd);
  std::string re;
  read_data(re, 50);
  std::cout<<re;
  return 1;//TODO decode
}


void CameraIR::SetAirAbsorb(double v){
  std::string cmd=":absorb ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}

void CameraIR::SetAirTransmission(double v){
  std::string cmd=":tau ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}

void CameraIR::SetDistance(double v){
  std::string cmd=":distance ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}

void CameraIR::SetPathTemp(double v){ //in K
  std::string cmd=":pathtemp ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}


void CameraIR::SetEnvTemp(double v){ //in K
  std::string cmd=":envtemp ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}

void CameraIR::SetEmissivity(double v){
  if(v<0.1 || v>1.0){
    std::cout<<"out of range\n";
    return;
  }
  std::string cmd=":emissivity ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}

void CameraIR::SetVisLevel(double v){ //in K
  std::string cmd=":level ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}

void CameraIR::SetVisRange(double v){ //in K
  std::string cmd=":range ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}



void CameraIR::SetFocusSteps(uint32_t v){
  if(v>4095){
    std::cout<<"out of range\n";
    return;
  }
  std::string cmd=":focsteps ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}

void CameraIR::SetFocus(uint32_t v){
  std::string cmd=":focus ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}


void CameraIR::StopFocus(){
  std::string cmd=":stopfocus";
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}




void CameraIR::SetShutter(bool b){
  std::string cmd;
  if(b)
    cmd = ":shutter 1";
  else
    cmd = ":shutter 0";
  write_line(cmd);
  
  std::string str;
  read_data(str, 100);
}


void CameraIR::SetAutoFocus(bool b){
  std::string cmd;
  if(b)
    cmd = ":autofocus 1";
  else
    cmd = ":autofocus 0";
  write_line(cmd);
  
  std::string str;
  read_data(str, 100);
}




void CameraIR::SetFreeze(bool b){
  std::string cmd;
  if(b)
    cmd = ":freeze on";
  else
    cmd = ":freeze off";
  write_line(cmd);
  
  std::string str;
  read_data(str, 100);
}


/*
  bit 0:2
  0 off
  1 AutoRange once
  2 AutoRange durable
  3 AutoLevel onece
  4 AutoLevel durable
  5 AutoLevel 8bit duarable

  bit 6:7
 */
void CameraIR::SetAutoImage(uint32_t v){
  std::string cmd=":autoimage ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}


void CameraIR::LoadCalib(uint32_t v){
  std::string cmd=":calib ";
  cmd+=std::to_string(v);
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}



void CameraIR::Key_left(){
  SendKey(KEY_LEFT_PRS);
  SendKey(KEY_LEFT_RLS);
}

void CameraIR::Key_right(){
  SendKey(KEY_RIGHT_PRS);
  SendKey(KEY_RIGHT_RLS);
}

void CameraIR::Key_up(){
  SendKey(KEY_UP_PRS);
  SendKey(KEY_UP_RLS);
}

void CameraIR::Key_down(){
  SendKey(KEY_DOWN_PRS);
  SendKey(KEY_DOWN_RLS);
}


void CameraIR::Key_s(){
  SendKey(KEY_S_SHORT);
}


void CameraIR::Key_a(){
  SendKey(KEY_A_SHORT);
}

void CameraIR::Key_t(){
  SendKey(KEY_T_SHORT);
}

void CameraIR::Key_s_long(){
  SendKey(KEY_S_LONG);
}


void CameraIR::Key_a_long(){
  SendKey(KEY_A_LONG);
}

void CameraIR::Key_t_long(){
  SendKey(KEY_T_LONG);
}

void CameraIR::TakePhoto(){
  Key_s_long();
}

void CameraIR::SendCmd(std::string cmd){
  write_line(cmd);
  std::string re;
  read_data(re, 150);
}


void CameraIR::SendKey(std::string k){
  std::string cmd(":key ");
  cmd += k;
  write_line(cmd);
  std::string re;
  read_data(re, 100);
}
