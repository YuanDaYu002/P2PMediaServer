MY_PATH=$(shell pwd)
shangyun_SDK_PATH=$(MY_PATH)/shangyun/Release_PPCS_3.1.0

SRC=$(MY_PATH)/src
#LDFLAG+=-l${P2P}_API
LDFLAG =-O2 -g -Wall -DLINUX -lpthread  -lstdc++ -lPPCS_API
LDFLAG+=-I $(shangyun_SDK_PATH)/Include/PPCS 
LDFLAG+=-I $(MY_PATH)/include
#LDFLAG+=-L $(shangyun_SDK_PATH)/Lib/${OS}/${BOARD}/libPPCS_API.a
LDFLAG+=-L $(shangyun_SDK_PATH)/Lib/Linux/x64

LDFLAG+= -DP2P_SUPORT_WAKEUP


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



all: clean
	$(CXX) -static $(SRC)/* -o $(MY_PATH)/out/main_$(exe) $(LDFLAG) -s
	#-s: Remove all symbol table and relocation information from the executable.



clean:
	rm -rf $(MY_PATH)/out/*  
	rm -rf $(MY_PATH)/obj/*

