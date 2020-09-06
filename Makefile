CC=gcc
CFLAGS=-I/opt/vc/include
LDFLAGS=-L/opt/vc/lib -lbcm_host -lvcos -lvchiq_arm

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rpi-fan: rpi_fan.o 
	$(CC) $(LDFLAGS) -o rpi-fan rpi_fan.o

clean:
	rm -f *.o rpi-fan
