# MAX31856_freebsd
Adafruit MAX31856 userspace driver example for FreeBSD

# Requirements
This userspace driver is utilizing the spigen device driver, which provides userspace access to SPI devices over a generic interface. However, the ability to build the driver as a loadable module was committed with r314933 to HEAD, therefore if you are using an earlier revision, please add it to your kernel configuration.

The CPHA mode has to be turned on, and this is off by default in the freebsd kernel SPI interface. This can be fixed by setting the dev.spi.0.cpha sysctl to 1. The example code kindly reminds the user to this.

# Chip Select
This driver is using a generic interface for ChipSelection, therefore if there are more devices than CS pinss on the SPI pins, this allows the user to implement his or her own method to do this. This way, such extreme cases like using a mux/demux chip to handle 8 or so chips is possible.

# MAX31856 settings
I've tried to add an interface for the most common features of the chip, however a couple of things might be missing, that someone needs. Feel free to open an issue and/or submit a pull request along with it. As it can be seen from the code, handling the chip is not rocket science.

# Credits
I would like to say special thanks to gonzo@, who helped figuring out the SPI interface under FreeBSD, and also helped with pointing out the CPHA mode issue.
