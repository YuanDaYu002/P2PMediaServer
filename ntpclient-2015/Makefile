
NTPLIB= ../lib/libntpclient.a

FLAGS= -lpthread 

all:
	$(CC) -c ntpclient.c $(FLAGS) -o ntpclient.o 
	$(CC) -c phaselock.c $(FLAGS) -o phaselock.o
	ar -rc $(NTPLIB) ntpclient.o  phaselock.o


clean:
	rm -rf ./*.o
	rm -rf $(NTPLIB)

	