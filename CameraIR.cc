#include"CameraIR.hh"

CameraIR::CameraIR(std::string devpath)
  :WKserial(devpath){
  connect();
  set_endline("\n");
  set_speed(38400);
  set_hardware_flow_control(0);
  set_software_flow_control(0);
  set_raw_ouput(1);
  set_raw_intput(i);
  set_parity(8,1,'N');
  set_read_timeout_msec(500);
  set_write_interval_msec(100);  
}

double CameraIR::GetCameraTemp(){
  std::sting strcom="?camptemp";
  write_line(strcom);
  std::string str;
  read_data(str, 50);
  std::cout<<str;
  return 1;//TODO decode
}
