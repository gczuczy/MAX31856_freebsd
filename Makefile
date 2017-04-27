LDFLAGS=-lgpio
OUT=spitest
OBJS=main.o GPIO.o SPI.o MAX31856.o
CXXFLAGS=-std=c++11

.ifdef DEBUG
CXXFLAGS+= -DMAX31856_DEBUG
.endif

.SUFFIXES : .o .cc .hh
.PATH.cc  : $(.CURDIR)
.PATH.hh  : $(.CURDIR)

all: $(OUT)

depend: .depend

.depend: Makefile
	mkdep ./*.cc ./*.hh

clean:
	rm -f $(OUT) $(OBJS) .depend

$(OUT): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

.cc.o:
	$(CXX) -c -o $@ $< $(CXXFLAGS)

.sinclude ".depend"
