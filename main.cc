#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <memory>
#include <iostream>
#include <vector>

#include "GPIO.hh"
#include "SPI.hh"
#include "MAX31856.hh"

int main(int argc, char *argv[0]) {

  char *csv = 0;
  int nsensors = 1;
  if ( argc >= 2 ) nsensors = atoi(argv[1]);
  if ( argc >= 3 ) csv = argv[2];

  if ( nsensors < 1 || nsensors > 4 ) {
    fprintf(stderr, "Min 1 max 4 sensors\n");
    return 1;
  }

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

  gpio[7].setname("cs0").output();
  gpio[8].setname("cs1").output();
  gpio[5].setname("cs2").output();
  gpio[6].setname("cs3").output();

  std::map<int, std::string> dspins;

  DirectSelect ds(gpio, {{0,"cs0"},{1,"cs1"},{2,"cs2"},{3,"cs3"}});
  SPI spi(ds, gpio, "/dev/spigen0");
  std::vector<std::shared_ptr<MAX31856> > tcs;
  for ( int i=0; i<nsensors; ++i ) {
    auto tc = std::make_shared<MAX31856>(spi, i);
    tc->set50Hz(true)
      .setTCType(MAX31856::TCType::T)
      .setAvgMode(MAX31856::AvgMode::S4)
      .setConversionMode(true);
    tcs.push_back(tc);
  }


  sleep(1);

#if 0
  SPI::Data cmd{0x00},data(1);
  spi.transfer(0, cmd, data);
  printf("0/CR0: %s / %s\n", data.hexdump().c_str(),
	 data.bindump().c_str());
  cmd[0] = 0x01;
  data[0] = 0;
  spi.transfer(0, cmd, data);
  printf("0/CR1: %s / %s\n", data.hexdump().c_str(),
	 data.bindump().c_str());
#endif

  int kq = kqueue();
  struct kevent ke[8];
  int nchanges = 8;
  EV_SET(&ke[0], 0, EVFILT_TIMER, EV_ADD|EV_ENABLE, NOTE_SECONDS, 3, 0);
  if ( kevent(kq, ke, 1, 0, 0, 0) < 0 ) {
    printf("kevent failed: %i/%s\n", errno, strerror(errno));
    return 0;
  }

  int fd = 0;
  if ( csv ) fd = open(csv, O_WRONLY|O_CREAT|O_TRUNC|O_SYNC,
		       S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH);
  while (true) {
    if ( (nchanges = kevent(kq, 0, 0, ke, 8, 0)) > 0 ) {
      for (int i=0; i<nchanges; ++i) {
	// EVFILT_TIMER with ident=0 is our sensor timer
	if ( ke[i].filter == EVFILT_TIMER && ke[i].ident == 0 ) {
	  time_t t = time(0);
	  printf("%li", t);
	  if ( fd ) dprintf(fd, "%li", t);
	  for ( auto &it: tcs ) {
	    float temp = it->readTCTemp();
	    printf(",%i:%.2f", it->getChipID(), temp);
	    if ( fd ) dprintf(fd,",%.2f", temp);
	  } // for tcs
	  printf("\n");
	  if ( fd ) dprintf(fd, "\n");
	} // if EVFILT_TIMER
      } // for nchanges
    } // if kevent
  } // while true
  if ( fd ) close(fd);


  return 0;
}
