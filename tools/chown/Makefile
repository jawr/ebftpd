CXXFLAGS := -Wnon-virtual-dtor -Wall -Wextra -g -ggdb -std=gnu++0x -pedantic 
CXXFLAGS += -Winit-self -pthread -DEXTERNAL_TOOL -I../../src

LIBS := -lboost_program_options -lboost_thread -lboost_regex 
LIBS += -lmongoclient -lboost_system -lboost_filesystem
 
SOURCES := chown.cpp \
	../../src/cfg/config.cpp \
	../../src/cfg/setting.cpp \
	../../src/cfg/section.cpp \
	../../src/logs/logs.cpp \
	../../src/util/logger.cpp \
	../../src/util/path.cpp \
	../../src/acl/acl.cpp \
	../../src/acl/permission.cpp \
	../../src/util/string.cpp \
	../../src/acl/flags.cpp \
	../../src/fs/owner.cpp \
	../../src/util/error.cpp \
	../../src/cfg/get.cpp \
	../../src/util/status.cpp \
	../../src/util/diriterator.cpp

OBJECTS = $(join $(addsuffix .chown/, $(dir $(SOURCES))), $(notdir $(SOURCES:.cpp=.o)))

.PHONY: all mkdir
all: chown
chown: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIBS) -o chown
	
$(OBJECTS): | mkdir
	$(CXX) -c $(CXXFLAGS) -o $@ $(patsubst %.o,%.cpp,$(subst .chown/,,$@))

mkdir:
	@mkdir -p $(sort $(dir $(OBJECTS)))

clean:
	-@rm -f $(OBJECTS) chown
	-@rmdir $(sort $(dir $(OBJECTS))) 2>/dev/null
	
