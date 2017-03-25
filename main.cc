#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <errno.h>
#include <string.h>

#include <iostream>

#include "GPIO.hh"
#include "SPI.hh"
#include "MAX31856.hh"

int main() {

  // first verify the cpha mode from the dev.spi.0.cpha sysctl
  {
    int value;
    size_t size = sizeof(value);
    if ( sysctlbyname("dev.spi.0.cpha", &value, &size, 0, 0)<0 ) {
      printf("Unable to fetch sysctl dev.spi.0.cpha: %s", strerror(errno));
      return 1;
    }
    if ( value != 1 ) {
      printf("Please set dev.spi.0.cpha=1 first\n");
      return 2;
    }
  }
  GPIO gpio(0);

  gpio[8].setname("cs0").output();
  gpio[7].setname("cs1").output();
  gpio["cs0"].low();
  gpio["cs1"].low();

  DirectSelect ds(gpio);
  SPI spi(ds, gpio, "/dev/spigen0");
  MAX31856 thermo1(spi, 0);

  thermo1.set50Hz(true)
    .setTCType(MAX31856::TCType::T)
    .setAvgMode(MAX31856::AvgMode::S4)
    .setConversionMode(true);

  SPI::Data cmd{0x00},data(1);
  spi.transfer(0, cmd, data);
  printf("CR0: %s / %s\n", data.hexdump().c_str(),
	 data.bindump().c_str());
  cmd[0] = 0x01;
  data[0] = 0;
  spi.transfer(0, cmd, data);
  printf("CR1: %s / %s\n", data.hexdump().c_str(),
	 data.bindump().c_str());

  double cjt, tct;
  cjt = thermo1.readCJTemp();
  printf("CJ temp: %f C\n", cjt);
  tct = thermo1.readTCTemp();
  printf("TC temp: %f C\n", tct);

  return 0;
}
