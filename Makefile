CXX := g++
CXXFLAGS := -Wnon-virtual-dtor -Wall -Wextra -g -ggdb -std=gnu++0x -pedantic -rdynamic
CXXFLAGS += -Winit-self -Winline -pthread
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

$(shell ./scripts/version.sh >/dev/null)

OBJECTS := $(SOURCE:.cpp=.o)

.PHONY: all unity test unitytest state strip clean

all: ebftpd

unity: ebftpd

test:  ebftpd

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

