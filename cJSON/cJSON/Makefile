#=========================================
# FileName: makefile
# Version: 1.0
# LastChange: 2010-03-09 14:46:48
# Description:
# History:
#=========================================
TARGET  = cjson_test
SRC	= cJSON.c test.c
CXX	= gcc
LIBS    = -lm

$(TARGET) : 
	$(CXX) $(SRC) -o $(TARGET)  $(LIBS)

clean:
	rm -f *.o $(TARGET)
