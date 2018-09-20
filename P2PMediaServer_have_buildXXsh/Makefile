########################################################
OS=Linux
BOARD=x64
P2P=PPCS
#CC=arm-fullhan-linux-uclibcgnueabi-gcc
#CXX=arm-fullhan-linux-uclibcgnueabi-g++
#STRIP=arm-fullhan-linux-uclibcgnueabi-strip
CC=gcc
CXX=g++
STRIP=strip


###Export################################################
export CC
export CXX
export OS
export BOARD
###PATH###################################################

MY_PATH=$(shell pwd)
NTP_PATH=./ntpclient-2015
shangyun_SDK_PATH=$(MY_PATH)/shangyun/Release_PPCS_3.1.0

CPP_SRC=$(MY_PATH)/src

####Flag##################################################
LDFLAG = -O2 -g -Wall -DLINUX -lpthread  -lstdc++ 
LDFLAG+= -lPPCS_API -lntpclient
LDFLAG+= -DLINUX -DDEBUG
#LDFLAG+= -DP2P_SUPORT_WAKEUP

###Include################################################
INCLUDE =-I $(shangyun_SDK_PATH)/Include/PPCS 
INCLUDE+=-I $(MY_PATH)/include


###Lib#####################################################
#换到板子上边要改变库，编译工具不一样
LIB =-L $(shangyun_SDK_PATH)/Lib/Linux/x64
LIB+=-L $(MY_PATH)/lib



ifeq ($(BOARD), x64)
LDFLAG+=-m64 
exe=64
else
	ifeq ($(BOARD), x86)
		LDFLAG+=-m32 
		exe=32
	endif
endif

ifeq ($(OS), Linux)
LDFLAG+=-Wl,-rpath=./
dylib=so
endif


all: clean NTPLIB
	$(CXX) -static $(CPP_SRC)/*.cpp -o $(MY_PATH)/out/main_$(exe) $(LDFLAG) $(INCLUDE) $(LIB) -s
	#-s: Remove all symbol table and relocation information from the executable.


#libntpclient.a生成：
NTPLIB:
	cd $(NTP_PATH) && make





clean:NTPClean
	rm -rf $(MY_PATH)/out/*  
	rm -rf $(MY_PATH)/obj/*
	rm -rf $(MY_PATH)/ntpclient-2015/*.o

NTPClean:
	cd $(NTP_PATH) && make clean

