BUILDDIR := ./build/
BINARY = $(BUILDDIR)Shadowsocks

all : dir makebinary package

ROOTDIR = $(CURDIR)/../
toolchain := $(ROOTDIR)toolchain/
CXX = $(toolchain)/bin/arm-xiaomi-linux-uclibcgnueabi-g++
CC =$(toolchain)/bin/arm-xiaomi-linux-uclibcgnueabi-gcc
LIB_DIR = -L$(ROOTDIR)/lib/
CXXFLAGS += -I$(ROOTDIR)/include/
LDFLAGS = -Wall -O2 -lxmrouter -lthrift -lssl -lcrypto -lconfig++ -ljson-c \
 -lboost_system -lboost_filesystem -lthriftnb -levent -lcurl -lz -lboost_thread \
 -lroutermain -std=c++11

dir : 
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)ss
	mkdir -p $(BUILDDIR)bin

makebinary :
	$(CXX) $(CXXFLAGS) JSON.cpp Tools.cpp base64.c inifile.c Shadowsocks.cpp $(LIB_DIR) $(LDFLAGS) -o $(BINARY)
	
clean:
	rm -r build

.PHONY : clean TaskWatcher

package: 
	cp start_script build/
	cp -rf ss/* build/ss/
	cp -rf bin/* build/bin/
	../plugin_packager_x64
