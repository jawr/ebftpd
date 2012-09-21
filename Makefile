CXX = g++
CXXFLAGS = -Wall -Wextra -g -ggdb -DDEBUG
LIBS = -lcryptopp -lboost_thread -lboost_regex -lgnutls -lboost_serialization
LIBS += -lboost_iostreams -lboost_system -lpthread -lnettle
INCLUDE = -I.

OBJECTS = \
	main.o \
	cmd/factory.o \
	cmd/command.o \
	cfg/config.o \
	cfg/get.o \
	ftp/listener.o \
	ftp/client.o \
	fs/filelock.o \
	fs/owner.o \
	fs/status.o \
	fs/file.o \
	fs/directory.o \
	fs/path.o \
	acl/acl.o \
	acl/user.o \
	acl/permission.o \
	acl/repository.o \
	acl/handler.o \
	acl/flags.o \
	logger/logger.o \
	util/path.o \
	util/verify.o \
	util/passwd.o \
	util/descriptor.o \
	util/error.o \
	util/endpoint.o \
	util/exception.o \
	util/ip.o \
	util/logger.o \
	util/resolver.o \
	util/ssl.o \
	util/tcpclient.o \
	util/tcpserver.o \
	util/thread.o

all: 
	@if [ -f .state ] && [ `cat .state` != 'all' ]; then \
	$(MAKE) $(MAKEFILE) clean; \
	fi; \
	echo "all" > .state
	$(MAKE) $(MAKEFILE) ftpd

test: 
	@if [ -z $(TEST) ]; then \
	echo "Must specify test macro as TEST=DEFINE"; \
	exit 1; \
	fi; \
	if [ -f .state ] && [ `cat .state` != "$(TEST)" ]; then \
	$(MAKE) $(MAKEFILE) clean; \
	fi; \
	echo "$(TEST)" > .state
	$(MAKE) $(MAKEFILE) ftpd CXXFLAGS="$(CXXFLAGS) -DTEST -D$(TEST)"

ftpd: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) $(LIBS) -o ftpd
	
strip:
	@strip -s ftpd

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) -MD -o $@ $<

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)

clean:
	@rm -f *.o *.d */*.o */*.d
	@rm -f ftpd
	@rm -f .state
