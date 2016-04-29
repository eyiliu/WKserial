{
  // our current library works for root 6! Compile code in "serial_yi" for your favourite root version first, if root 6 is not it ;)
  // set root with  source /<PATHROROOTVERSION>/bin/thisroot.sh
  // start:
  //root
  // load library with:
  //  gSystem->Load("../libwkserial.so");
  // execue this macro with:
  // .x UseTTi_example.C
  // adjust to correct serial port number in line below:
  tti_CPX400 * LV = new tti_CPX400("/dev/ttyS2");

  std::cout<<  LV->getCurrent( 1 )<<std::endl; // get current limit for channel 1 (there is 1 and 2 potentially)
  std::cout<<  LV->getVoltage( 1 )<<std::endl; // get voltage limit
  std::cout<<  LV->getActualCurrent( 1 )<<std::endl; // get measured current
  
  //  LV->setStatus(1,1); // turn output on
  //  LV->setStatus(1,0); // turn output off

  std::cout<<  LV->setCurrent(1, 0.234)<<std::endl; // set current limit for chan 1 to 0.999
  std::cout<<  LV->setVoltage(1, 0.123)<<std::endl; // set current limit for chan 1 to 0.999

  std::cout<<"Did it work?"<<std::endl;
  
}
