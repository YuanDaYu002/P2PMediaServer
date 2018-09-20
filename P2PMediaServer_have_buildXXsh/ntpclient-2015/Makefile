
NTPLIB= ../lib/libntpclient.a
all:
	$(CC) -c ntpclient.c -o ntpclient.o
	$(CC) -c phaselock.c -o phaselock.o
	ar -rc $(NTPLIB) ntpclient.o  phaselock.o


clean:
	rm -rf ./*.o
	rm -rf $(NTPLIB)

	