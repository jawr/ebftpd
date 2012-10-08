
CXX = g++
CXXFLAGS = -Wnon-virtual-dtor -Wall -Wextra -g -ggdb -std=c++0x
LIBS = -lmongoclient -lcrypto -lcryptopp -lboost_thread -lboost_regex -lgnutls -lboost_serialization
LIBS += -lboost_iostreams -lboost_system -lpthread -lnettle -lssl -lboost_filesystem
INCLUDE = -include pch.hpp -I.

OBJECTS = \
	main.o \
	cmd/factory.o \
	cmd/command.o \
	cmd/dirlist.o \
	cfg/config.o \
	cfg/get.o \
	cfg/setting.o \
	db/bson/user.o \
	db/bson/group.o \
  db/interface.o \
  db/pool.o \
  db/worker.o \
  db/task.o \
	ftp/listener.o \
	ftp/client.o \
	ftp/portallocator.o \
	ftp/addrallocator.o \
	ftp/data.o \
	ftp/control.o \
	fs/direnumerator.o \
	fs/filelock.o \
	fs/owner.o \
	fs/status.o \
	fs/file.o \
	fs/directory.o \
	fs/path.o \
	acl/allowfxp.o \
	acl/groupcache.o \
	acl/check.o \
	acl/acl.o \
	acl/user.o \
	acl/usercache.o \
	acl/permission.o \
	acl/handler.o \
	acl/flags.o \
	logger/logger.o \
	util/misc.o \
	util/path.o \
	util/verify.o \
	util/passwd.o \
	util/error.o \
	util/logger.o \
	util/string.o \
	util/thread.o \
	util/net/endpoint.o \
	util/net/error.o \
	util/net/ipaddress.o \
	util/net/resolver.o \
	util/net/tcplistener.o \
	util/net/tcpsocket.o \
	util/net/test.o \
	util/net/tlscontext.o \
	util/net/tlserror.o \
	util/net/tlssocket.o \
	util/net/interfaces.o \
	util/net/ftp.o 

.PHONY: test clean strip

all: 
	@if [ -f .state ] && [ `cat .state` != 'all' ]; then \
	$(MAKE) $(MAKEFILE) clean; \
	fi; \
	echo "all" > .state; \
	VERSION=`git log --decorate | grep "^commit " | grep -n "tag: " | \
		sed -r 's|^([0-9]+):.+tag: ([^),]+).+$$|\2-\1|p' | head -n1`; \
	echo "const char* version = \"$$VERSION\";" > version.hpp
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

ftpd: pch.hpp.gch $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) $(LIBS) -o ftpd

pch.hpp.gch:
	$(CXX) -c $(CXXFLAGS) pch.hpp
	
strip:
	@strip -s ftpd

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) -MD -o $@ $<

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)

clean:
	@find -iname "*.[od]" -exec rm '{}' ';'
	@find -iname "*.gch" -exec rm '{}' ';'
	@rm -f ftpd
	@rm -f .state
