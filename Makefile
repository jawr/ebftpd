
CXX = g++
CXXFLAGS = -Wnon-virtual-dtor -Wall -Wextra -g -ggdb -std=c++0x
LIBS = -lmongoclient -lcrypto -lcryptopp -lboost_thread -lboost_regex -lgnutls -lboost_serialization
LIBS += -lboost_iostreams -lboost_system -lpthread -lnettle -lssl -lboost_filesystem
INCLUDE = -include pch.hpp -I.

OBJECTS = \
	main.o \
	cmd/dirlist.o \
	cmd/rfc/abor.o \
	cmd/rfc/acct.o \
	cmd/rfc/adat.o \
	cmd/rfc/allo.o \
	cmd/rfc/appe.o \
	cmd/rfc/auth.o \
	cmd/rfc/ccc.o \
	cmd/rfc/cdup.o \
	cmd/rfc/conf.o \
	cmd/rfc/cwd.o \
	cmd/rfc/dele.o \
	cmd/rfc/enc.o \
	cmd/rfc/eprt.o \
	cmd/rfc/epsv.o \
	cmd/rfc/feat.o \
	cmd/rfc/help.o \
	cmd/rfc/lang.o \
	cmd/rfc/list.o \
	cmd/rfc/lprt.o \
	cmd/rfc/lpsv.o \
	cmd/rfc/mdtm.o \
	cmd/rfc/mic.o \
	cmd/rfc/mkd.o \
	cmd/rfc/mlsd.o \
	cmd/rfc/mlst.o \
	cmd/rfc/mode.o \
	cmd/rfc/nlst.o \
	cmd/rfc/noop.o \
	cmd/rfc/opts.o \
	cmd/rfc/pass.o \
	cmd/rfc/pasv.o \
	cmd/rfc/pbsz.o \
	cmd/rfc/port.o \
	cmd/rfc/prot.o \
	cmd/rfc/pwd.o \
	cmd/rfc/quit.o \
	cmd/rfc/rein.o \
	cmd/rfc/rest.o \
	cmd/rfc/retr.o \
	cmd/rfc/rmd.o \
	cmd/rfc/rnfr.o \
	cmd/rfc/rnto.o \
	cmd/rfc/site.o \
	cmd/rfc/size.o \
	cmd/rfc/smnt.o \
	cmd/rfc/stat.o \
	cmd/rfc/stor.o \
	cmd/rfc/stou.o \
	cmd/rfc/stru.o \
	cmd/rfc/syst.o \
	cmd/rfc/type.o \
	cmd/rfc/user.o \
	cmd/factory.o \
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
	if ! grep -q "\"$$VERSION\"" version.hpp; then \
    echo "const char* version = \"$$VERSION\";" > version.hpp; \
    fi; \
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
