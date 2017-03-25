
#include "GPIO.hh"

#include <stdlib.h>
#include <unistd.h>

/*
 GPIO::Exception
 */
GPIO::Exception::Exception(std::string _msg): c_msg(_msg) {
}

GPIO::Exception::~Exception() {
}

const char *GPIO::Exception::what() const noexcept {
  return c_msg.c_str();
}

/*
  GPIO::PIN per-pin class
 */
GPIO::PIN::PIN(GPIO &_gpio, int _pin): c_gpio(_gpio), c_pin(_pin) {
}

GPIO::PIN::~PIN() {
}

GPIO::PIN &GPIO::PIN::input() {
  gpio_pin_input(c_gpio.c_handle, c_pin);
  return *this;
}

GPIO::PIN &GPIO::PIN::output() {
  gpio_pin_output(c_gpio.c_handle, c_pin);
  return *this;
}

GPIO::PIN &GPIO::PIN::high() {
  gpio_pin_high(c_gpio.c_handle, c_pin);
  return *this;
}

GPIO::PIN &GPIO::PIN::low() {
  gpio_pin_low(c_gpio.c_handle, c_pin);
  return *this;
}

GPIO::PIN &GPIO::PIN::toggle() {
  gpio_pin_toggle(c_gpio.c_handle, c_pin);
  return *this;
}

GPIO::PIN &GPIO::PIN::pullup() {
  gpio_pin_pullup(c_gpio.c_handle, c_pin);
  return *this;
}

GPIO::PIN &GPIO::PIN::pulldown() {
  gpio_pin_pulldown(c_gpio.c_handle, c_pin);
  return *this;
}

GPIO::PIN &GPIO::PIN::opendrain() {
  gpio_pin_opendrain(c_gpio.c_handle, c_pin);
  return *this;
}

GPIO::PIN &GPIO::PIN::tristate() {
  gpio_pin_tristate(c_gpio.c_handle, c_pin);
  return *this;
}

GPIO::PIN &GPIO::PIN::setname(const std::string &_name) {
  gpio_pin_set_name(c_gpio.c_handle, c_pin, (char*)_name.c_str());
  c_gpio.setname(c_pin, _name);
  return *this;
}

int GPIO::PIN::get() {
  int value;

  return gpio_pin_get(c_gpio.c_handle, c_pin);
}

/*
  GPIO: The main class
 */
GPIO::GPIO(int _unit) {
  c_handle = gpio_open(_unit);

  if ( c_handle == GPIO_INVALID_HANDLE ) {
    throw Exception("GPIO: invalid handle");
  }

  fetchpins();
}

GPIO::GPIO(const std::string _device) {
  c_handle = gpio_open_device(_device.c_str());

  if ( c_handle == GPIO_INVALID_HANDLE ) {
    throw Exception("GPIO: invalid handle");
  }

  fetchpins();
}

GPIO::~GPIO() {
  gpio_close(c_handle);
}

void GPIO::setname(int _pin, const std::string &_name) {
  c_names[_name] = _pin;
}

void GPIO::fetchpins() {
  gpio_config_t *pinlist(0);
  int pinret;

  pinret = gpio_pin_list(c_handle, &pinlist);
  for ( int i=0; i < pinret; ++i ) {
    c_pins.insert(std::make_pair(pinlist[i].g_pin, PIN(*this, pinlist[i].g_pin)));
  }

  if ( pinlist ) free(pinlist);
}

GPIO::PIN &GPIO::operator[](const std::string &_name) {
  auto it = c_names.find(_name);

  if ( it == c_names.end() ) throw Exception("PIN not found");

  return (*this)[it->second];
}

GPIO::PIN &GPIO::operator[](const int _pin) {
  auto it = c_pins.find(_pin);

  if ( it == c_pins.end() ) throw Exception("Unable to find pin");

  return it->second;
}
