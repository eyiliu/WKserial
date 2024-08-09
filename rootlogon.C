{
  stringstream ss_loadlib;
  string wkfd = gSystem->WorkingDirectory();
  ss_loadlib <<wkfd.c_str()<<"/lib/"<<"libWKthermal.so";
  gSystem->Load(ss_loadlib.str().c_str());
  std::cout<<"rootlogon.C >> Load "<<ss_loadlib.str().c_str()
	   <<std::endl<<std::endl;
    
}
