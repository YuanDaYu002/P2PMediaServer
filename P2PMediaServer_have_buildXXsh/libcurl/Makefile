#=========================================
# FileName: makefile
# Version: 1.0
# LastChange: 2010-03-09 14:46:48
# Description:
# History:
#=========================================
TARGET  = url_dowload
OBJS	= url_dowload.o

CXX	= g++
C_FLAGS = -g -c
#INC     = -I ./curl-7.61.1/include/curl
LDFLAGS = -L./curl-7.61.1/lib/.libs/*.a 
LIBS    = -lcurl  -lnsl -lssl -lcrypto

$(TARGET) : $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

url_dowload.o : url_dowload.cpp
	$(CXX) $(C_FLAGS) $<


clean:
	rm -f *.o $(TARGET)
