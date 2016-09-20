#include"CameraIR.hh"

#include<iostream>
CameraIR::CameraIR(std::string devpath)
  :WKserial(devpath){
  connect();
  set_endline("\r");
  set_speed(38400);
  set_hardware_flow_control(0);
  set_software_flow_control(0);
  set_raw_output(1);
  set_raw_intput(1);
  set_parity(8,1,'N');
  set_read_timeout_msec(500);
  set_write_interval_msec(100);  
}

double CameraIR::GetCameraTemp(){
  std::string strcom="?camtemp";
  write_line(strcom);
  std::string str;
  read_data(str, 50);
  std::cout<<str;
  return 1;//TODO decode
}
