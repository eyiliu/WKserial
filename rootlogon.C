{
  stringstream ss_loadlib;
  string wkfd = gSystem->WorkingDirectory();
  // wkfd.erase(wkfd.find_last_of('/'));
  ss_loadlib <<wkfd.c_str()<<"/lib/"<<"libWKthermal.so";
  gSystem->Load(ss_loadlib.str().c_str());
  std::cout<<"rootlogon.C >> Load "<<ss_loadlib.str().c_str()
	   <<std::endl<<std::endl;

  // ss_loadlib.str("");
  // ss_loadlib.clear()
  // ss_loadlib <<wkfd.c_str()<<"/lib/"<<"libWKthermal.so";
  // gSystem->Load(ss_loadlib.str().c_str());
  // std::cout<<"rootlogon.C >> Load "<<ss_loadlib.str().c_str()
  // 	   <<std::endl<<std::endl;

  // nanotec::NanotecMotor m0("/dev/ttyUSB0");
  // nanotec::NanotecMotor m1("/dev/ttyUSB1");
    
}
