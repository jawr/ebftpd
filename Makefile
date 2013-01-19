CXX := g++
CXXFLAGS := -Wnon-virtual-dtor -Wall -Wextra -g -ggdb -std=gnu++0x -pedantic -rdynamic
CXXFLAGS += -Winit-self -pthread
LIBS := -lmongoclient -lcrypto -lboost_thread -lboost_regex -lboost_serialization
LIBS += -lboost_iostreams -lboost_system -lssl -lboost_filesystem
LIBS += -lboost_date_time -lboost_program_options
INCLUDE := -Isrc -include src/pch.hpp

UNITY := false
STATE := other

MAKECMDGOALS ?= all

ifeq ($(MAKECMDGOALS),$(filter $(MAKECMDGOALS),unity unitytest))
UNITY := true
ifndef UNITY_PARTS
$(warning UNITY_PARTS=NUM environment variable not setting, using default of a single unity file)
UNITY_PARTS := 1
endif
endif

ifeq ($(MAKECMDGOALS),$(filter $(MAKECMDGOALS),test unitytest))
STATE := test
endif

ifeq ($(MAKECMDGOALS),$(filter $(MAKECMDGOALS),all unity ebftpd))
STATE := normal
endif

ifeq ($(UNITY),false)
SOURCE := $(shell find src/ -type f -name "*.cpp" -printf "src/%P\n" | grep -v "^unity/")
else
SOURCE := $(shell ./scripts/unity.sh $(UNITY_PARTS))
endif

ifeq ($(STATE),test)
CXXFLAGS += -DTEST -D$(TEST)
ifndef TEST
$(error You must specify test macro as TEST=MACRO after make command)
endif
ifeq ($(wildcard .state),)
$(shell echo $(TEST) > .state)
else
ifneq ($(shell cat .state 2>/dev/null),$(TEST))
$(error You must run make clean before changing build states)
endif
endif
endif

ifeq ($(STATE),normal)
ifeq ($(wildcard .state),)
$(shell echo normal > .state)
else
ifneq ($(shell cat .state 2>/dev/null),normal)
$(error You must run make clean before changing build states)
endif
endif
endif

CRYPTOPP_NAME := $(shell scripts/cryptopp.sh 2>/dev/null)
ifeq ($(CRYPTOPP_NAME),cryptopp)
CXXFLAGS += -DCRYPTOPP
LIBS += -lcryptopp
else
ifeq ($(CRYPTOPP_NAME),crypto++)
LIBS += -lcrypto++
else
$(error Unable to find crypto++ header files in /usr/include or /usr/local/include)
endif
endif

VERSION := $(shell ./scripts/version.sh)

OBJECTS := $(SOURCE:.cpp=.o)

.PHONY: all unity test unitytest state strip clean

all: ebftpd

unity: ebftpd

test:	ebftpd

unitytest: ebftpd

ebftpd: src/pch.hpp.gch $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) $(LIBS) -o ebftpd

src/pch.hpp.gch:
	$(CXX) -c $(CXXFLAGS) src/pch.hpp


%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) -MD -o $@ $<

DEPS := $(OBJECTS:.o=.d)
-include $(DEPS)

strip:
	@strip -s ebftpd

clean:
	@find src/ -iname "*.[od]" -exec rm '{}' ';'
	@find src/ -iname "*.gch" -exec rm '{}' ';'
	@rm -f ebftpd
	@rm -f .state
	@rm -f unity/*
	@rm -f ebftpd.dbg

package:
	if [ ! -z `ls unity/unity* 2>/dev/null` ]; then \
	echo "Do make clean and a non-unity build before creating a package!"; \
	exit 0; \
	fi; \
	if [ ! -f ebftpd ]; then \
	echo "You must build ebftpd before creating a package!"; \
	exit 0; \
	fi; \
	echo "Separating debug symbols .."; \
	objcopy --only-keep-debug ebftpd ebftpd.dbg; \
	strip --strip-debug --strip-unneeded ebftpd; \
	objcopy --add-gnu-debuglink ebftpd.dbg ebftpd; \
	echo "Preparing package .."; \
	ARCH=$$(uname -m); \
	PACKAGE=ebftpd-$(VERSION)-$$ARCH; \
	ARCHIVE=$$PACKAGE.tar.bz2; \
	mkdir -p $$PACKAGE/bin/lib $$PACKAGE/bin/libexec; \
	mkdir -p $$PACKAGE/etc; \
	mkdir -p $$PACKAGE/site; \
	mkdir -p $$PACKAGE/data/logs; \
	mkdir -p $$PACKAGE/data/text; \
	cp INSTALL COPYING $$PACKAGE; \
	cp README.md $$PACKAGE/README; \
	cp ebftpd.conf.example $$PACKAGE/etc; \
	cp ebftpd ebftpd.dbg $$PACKAGE/bin/libexec; \
	cp scripts/ebftpd.sh $$PACKAGE/bin/ebftpd; \
	cp data/text/* $$PACKAGE/data/text; \
	echo "Copying dependencies .."; \
	cp -p $$(ldd ebftpd | awk '/^\tlib/ {print $$3}') $$PACKAGE/bin/lib; \
	LD_LINUX=$$(ldd ebftpd | awk '/ld-linux/ {print	$$1}'); \
	cp -p $$LD_LINUX $$PACKAGE/bin/lib; \
	ln -s $$(basename $$LD_LINUX) $$PACKAGE/bin/lib/ld-linux.so; \
	echo "Setting permissions .."; \
	find $$PACKAGE/ -type d -exec chmod 755 '{}' ';'; \
	find $$PACKAGE/ -type f -exec chmod 644 '{}' ';'; \
	chmod 755 $$PACKAGE/bin/lib/*; \
	chmod 755 $$PACKAGE/bin/ebftpd; \
	chmod 755 $$PACKAGE/bin/libexec/ebftpd; \
	echo "Creating archive .."; \
	tar cf - $$PACKAGE | bzip2 --best > $$ARCHIVE; \
	rm -rf $$PACKAGE; \
	echo "Package $$ARCHIVE created.";
	
