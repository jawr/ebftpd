CXXFLAGS := -Wnon-virtual-dtor -Wall -Wextra -g -ggdb -std=gnu++0x -pedantic 
CXXFLAGS += -Winit-self -pthread -DEXTERNAL_TOOL -I../../src

LIBS := -lboost_program_options -lboost_thread -lboost_regex 
LIBS += -lmongoclient -lboost_system -lboost_filesystem
 
SOURCES := index.cpp \
	../../src/cfg/config.cpp \
	../../src/cfg/setting.cpp \
	../../src/cfg/section.cpp \
	../../src/acl/acl.cpp \
	../../src/util/string.cpp \
	../../src/acl/permission.cpp \
	../../src/logs/logs.cpp \
	../../src/util/logger.cpp \
	../../src/util/path/path.cpp \
	../../src/util/path/status.cpp \
	../../src/util/path/diriterator.cpp \
	../../src/util/error.cpp

OBJECTS = $(join $(addsuffix .index/, $(dir $(SOURCES))), $(notdir $(SOURCES:.cpp=.o)))

.PHONY: all mkdir
all: index
index: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIBS) -o index
	
$(OBJECTS): | mkdir
	$(CXX) -c $(CXXFLAGS) -o $@ $(patsubst %.o,%.cpp,$(subst .index/,,$@))

mkdir:
	@mkdir -p $(sort $(dir $(OBJECTS)))

clean:
	-@rm -f $(OBJECTS) index
	-@for d in $(sort $(dir $(OBJECTS))); do \
		if [ -d $$d ]; then \
		rmdir $$d; \
		fi; \
		done
	
