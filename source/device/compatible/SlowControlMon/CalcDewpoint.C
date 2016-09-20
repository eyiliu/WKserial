Double_t CalcDewpoint(Double_t Temperature, Double_t Humidity) {

  // formula from http://www.ajdesigner.com/phphumidity/dewpoint_equation_dewpoint_temperature.php

  //  return TMath::Power( Humidity/100., 1./8. ) * (112+0.9*Temperature)+0.1*Temperature-112;

  
  // Wiki:
  double betaw = 17.62;
  double lambdaw = 243.12;
  double betai = 22.46;
  double lambdai = 273.62;
  if(Temperature<0){
	return lambdai*(betai * Temperature /(lambdai + Temperature) + TMath::Log(Humidity/100))/(betai * lambdai /(lambdai + Temperature) - TMath::Log(Humidity/100));
  }
  else{
	return lambdaw*(betaw * Temperature /(lambdaw + Temperature) + TMath::Log(Humidity/100))/(betaw * lambdaw /(lambdaw + Temperature) - TMath::Log(Humidity/100));
  }
}
