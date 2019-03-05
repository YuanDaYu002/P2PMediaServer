#=========================================
# FileName: makefile
# Version: 1.0
# LastChange: 2010-03-09 14:46:48
# Description:
# History:
#=========================================
TARGET  = url_dowload
OBJS	= url_dowload.o
CXX	= gcc
C_FLAGS = -g -c
#INC     = -I ./curl-7.61.1/include/curl
#LDFLAGS = -L./curl-7.61.1/lib/.libs/*.a 
#LIBS    = -lcurl  
#-lnsl -lssl -lcrypto

#new debug
#INC     = -I ./original_source_curl/x86_curl_lib_out/include/curl/*.h
#LDFLAGS = -L ./original_source_curl/x86_curl_lib_out/lib
LDFLAGS = -L ./libcurl/x86_curl_lib_out/lib


$(TARGET) : $(OBJS)
	$(CXX) -g -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS) $(INC) -lmy_curl -lpthread

url_dowload.o : url_dowload.c
	$(CXX) $(C_FLAGS) $<


clean:
	rm -f *.o $(TARGET)
