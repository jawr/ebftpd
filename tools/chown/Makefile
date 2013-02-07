CXXFLAGS := -Wnon-virtual-dtor -Wall -Wextra -g -ggdb -std=gnu++0x -pedantic
CXXFLAGS += -Winit-self	-I../../src -pthread -DEXTERNAL_TOOL

LIBS := -lboost_program_options -lboost_thread -lboost_regex 
LIBS += -lmongoclient -lboost_system -lboost_filesystem
 
OBJECTS := \
	chown.o \
	../../src/cfg/config.o \
	../../src/cfg/setting.o \
  ../../src/cfg/section.o \
  ../../src/logs/logs.o \
	../../src/util/logger.o \
	../../src/util/path.o \
  ../../src/acl/acl.o \
  ../../src/acl/permission.o \
  ../../src/util/string.o \
  ../../src/acl/flags.o \
  ../../src/fs/owner.o \
  ../../src/util/error.o \
  ../../src/cfg/get.o \
  ../../src/util/status.o \
  ../../src/util/diriterator.o

all: chown
chown: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIBS) -o chown

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -MD -o $@ $<

-include $(OBJECTS:.o=.d)

clean:
	@rm -f *.o *.d chown
	
