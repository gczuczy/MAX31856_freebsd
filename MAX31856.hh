
#ifndef MAX31856_H
#define MAX31856_H

#include "SPI.hh"

#include <cstdint>

class MAX31856 {
public:
  enum class Register:uint8_t {
    CR0= 0x00,
      CR1 = 0x01,
      MASK = 0x02,
      CJHF = 0x03,
      CJLF = 0x04,
      LTHFTH = 0x05,
      LTHFTL = 0x06,
      LTLFTH = 0x07,
      LTLFTL = 0x08,
      CJTO = 0x09,
      CJTH = 0x0a,
      CJTL = 0x0b,
      LTCBH = 0x0c,
      LTCBM = 0x0d,
      LTCBL = 0x0e,
      SR = 0x0f
  };
  enum class TCType:uint8_t {
    B = 0b0000,
      E = 0b0001,
      J = 0b0010,
      K = 0b0011,
      N = 0b0100,
      R = 0b0101,
      S = 0b0110,
      T = 0b0111
  };
  enum class AvgMode:uint8_t {
    S1 = 0b000,
      S2 = 0b001,
      S4 = 0b010,
      S8 = 0b011,
      S16 = 0b100
  };
public:
  MAX31856() = delete;
  MAX31856(MAX31856&&) = delete;
  MAX31856(const MAX31856 &) = delete;
  MAX31856 &operator=(MAX31856 &&) = delete;
  MAX31856 &operator=(const MAX31856 &) = delete;

  MAX31856(SPI &_spi, int _chipid);
  ~MAX31856();

  MAX31856 &setConversionMode(bool _cmode);
  MAX31856 &set50Hz(bool _50hz);
  MAX31856 &setAvgMode(AvgMode _mode);
  MAX31856 &setTCType(TCType _type);

  //temperature readings
  double readCJTemp();
  double readTCTemp();

private:
  SPI &c_spi;
  int c_chipid;
  bool c_convmode;

private:
  void xfer(SPI::Data &_cmd, SPI::Data &_data);
  void clearOCFault();
};

#endif
