
#include "SPI.hh"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/spigenio.h>

#include <exception>

/*
 * DirectSelet
 */

DirectSelect::DirectSelect(GPIO &_gpio, std::map<int, std::string> _idpins): c_gpio(_gpio), c_idpins(_idpins) {
  for ( auto &it: c_idpins ) {
    c_gpio[it.second].high();
  }
}

void DirectSelect::high(int _id) {
  auto it = c_idpins.find(_id);

  if ( it == c_idpins.end() ) {
    printf("Unknown CS id: %i\n", _id);
    throw std::exception();
  }

  c_gpio[it->second].high();
}

void DirectSelect::low(int _id) {
  auto it = c_idpins.find(_id);

  if ( it == c_idpins.end() ) {
    printf("Unknown CS id: %i\n", _id);
    throw std::exception();
  }
  c_gpio[it->second].low();
}

DirectSelect::~DirectSelect() {
  for ( auto &it: c_idpins ) {
    c_gpio[it.second].high();
  }
}

/*
 * ChipSelector
 */

ChipSelector::~ChipSelector() = default;

/*
 * CSGuard
 */

CSGuard::CSGuard(ChipSelector &_cs, int _id): c_cs(_cs), c_id(_id) {
  c_cs.low(c_id);
}

CSGuard::~CSGuard() {
  c_cs.high(c_id);
}

/*
 * SPI::Data
 */

SPI::Data::Data(int _size): c_size(_size), c_data(0) {
  if ( _size > 0 ) {
    c_data = new uint8_t[c_size];
    for (int i=0; i<c_size; ++i) c_data[i] = 0;
  } else {
    c_size = 0;
  }
}

SPI::Data::Data(std::initializer_list<uint8_t> _il) {
  c_size = _il.size();
  c_data = new uint8_t[c_size];
  int i=0;
  for ( auto &it: _il ) {
    c_data[i] = it;
    ++i;
  }
}

SPI::Data::~Data() {
  delete[] c_data;
}

uint8_t &SPI::Data::operator[](const int _idx) {
  if ( _idx < 0 || _idx >= c_size ) {
    throw std::exception();
  }
  return c_data[_idx];
}

std::string SPI::Data::hexdump() const {
  std::string hex;

  char buffer[8];
  int len;
  for (int i=0; i<c_size; ++i) {
    len = snprintf(buffer, 7, "%02x", c_data[i]);
    hex += std::string(buffer, len);
    if ( i != c_size-1 ) hex += " ";
  }

  return hex;
}

std::string SPI::Data::bindump() const {
  std::string bin;

  for (int i=0; i<c_size; ++i ) {
    // c_data[i] is a uint8_t, 8 bits
    for (uint8_t b=128; b>0; b >>= 1) {
      bin += (c_data[i]&b)?'1':'0';
    }
    if ( i != c_size-1 ) bin += " ";
  }
  return bin;
}

/*
 * SPI
 */

SPI::SPI(ChipSelector &_cs, GPIO &_gpio, const std::string &_spidev): c_cs(_cs), c_gpio(_gpio), c_spifd(-1) {
  if ((c_spifd = open(_spidev.c_str(), O_RDWR)) < 0) {
    printf("Unable to open spidev %s: %s\n", _spidev.c_str(), strerror(errno));
    throw std::exception();
  }
}

SPI::~SPI() {
  if ( c_spifd >= 0 ) close(c_spifd);
}

void SPI::transfer(int _chipid, Data &_cmd, Data &_data) {
  struct spigen_transfer tx;
  int err;

  tx.st_command.iov_base = _cmd.data();
  tx.st_command.iov_len = _cmd.size();
  if ( _data.size() ) {
    tx.st_data.iov_base = _data.data();
    tx.st_data.iov_len = _data.size();
  } else {
    tx.st_data.iov_base = 0;
    tx.st_data.iov_len = 0;
  }
#ifdef SPI_DEBUG
  {
    std::string hex;
    char buffer[8];
    int len;
    for (int i=0; i<tx.st_command.iov_len; ++i) {
      len = snprintf(buffer, 7, "%02x", ((uint8_t*)tx.st_command.iov_base)[i]);
      hex += std::string(buffer, len);
      if ( i != tx.st_command.iov_len-1 ) hex += " ";
    }
    printf("Pre-transfer CMD:%s ", hex.c_str());
    hex = "";
    if ( _data.size() ) {
      for (int i=0; i<tx.st_data.iov_len; ++i) {
	len = snprintf(buffer, 7, "%02x", ((uint8_t*)tx.st_data.iov_base)[i]);
	hex += std::string(buffer, len);
      if ( i != tx.st_data.iov_len-1 ) hex += " ";
      }
      printf(" D:%s", hex.c_str());
    }
    printf("\n");
  }
#endif
  CSGuard g(c_cs, _chipid);
  if ( (err = ioctl(c_spifd, SPIGENIOC_TRANSFER, &tx)) < 0 ) {
    printf("Error during transfer: %i\n", err);
    return;
  }
#ifdef SPI_DEBUG
  {
    std::string hex;
    char buffer[8];
    int len;
    printf("Post-transfer data(%zu): ", tx.st_data.iov_len);
    if ( tx.st_data.iov_len>0 ) {
      for (int i=0; i<tx.st_data.iov_len; ++i) {
	len = snprintf(buffer, 7, "%02x", ((uint8_t*)tx.st_data.iov_base)[i]);
	hex += std::string(buffer, len);
      if ( i != tx.st_data.iov_len-1 ) hex += " ";
      }
      printf("D:%s\n", hex.c_str());
    } else {
      printf("zero size\n");
    }
  }
#endif
}
