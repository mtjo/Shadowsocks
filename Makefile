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
	mkdir -p $(BUILDDIR)bin
	mkdir -p $(BUILDDIR)dev

makebinary :
	$(CXX) $(CXXFLAGS) JSON.cpp inifile.c Shadowsocks.cpp $(LIB_DIR) $(LDFLAGS) -o $(BINARY)
	
clean:
	rm -r build

.PHONY : clean TaskWatcher

package: 
	cp start_script build/
	cp -rf bin/* build/bin/
	cp -rf dev/* build/dev/
	cp autorun.sh build/bin
	cp frpc.sh build/bin
	../plugin_packager_x64
