#=========================================
# FileName: makefile
# Version: 1.0
# LastChange: 2010-03-09 14:46:48
# Description:
# History:
#=========================================
CXX=g++
TARGET= test_download test_post
C_FLAGS+=-g -Wall
INC+=-I./curl-7.61.1/include/curl 
LIB+=-L./curl-7.61.1/lib/.libs/*.a 
LIB+=-lcurl
#LIB+=-lz -lrt -lcrypto -lssl
#LIB_FLAGS = -pthread

# all: $(TARGET)


all:
	$(CXX) test_url_dowload_01.cpp -o test_url_dowload_01 $(INC) $(LIB)

# test_download: test_download.o
# 	$(CXX) -o $@ $(LIB_FLAGS) $(LIB) $(C_FLAGS)



# test_post: test_post.o
# 	$(CXX) -o $@  $(LIB_FLAGS) $(LIB) $(C_FLAGS)

# test_download.o: test_download.cpp
# 	$(CXX) -c -o $*.o $(INC) $(C_FLAGS) $*.cpp

# test_post.o: test_post.cpp
# 	$(CXX) -c -o $*.o $(INC) $(C_FLAGS) $*.cpp

clean:
	rm -f *.o $(TARGET) test_url_dowload_01

