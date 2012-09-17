CXX = g++
CXXFLAGS = -Wall -Wextra -g -ggdb -DDEBUG
LIBS = -lcrypto++ -lboost_thread -lboost_regex -lgnutls
INCLUDE = -I.

OBJECTS = \
	acl/acl.o \
	acl/user.o \
	logger/logger.o \
	util/verify.o \
	util/passwd.o \
	util/descriptor.o \
	util/endpoint.o \
	util/exception.o \
	util/ip.o \
	util/resolver.o \
	util/ssl.o \
	util/tcpclient.o \
	util/tcpserver.o 

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) $(LIBS) -o ftpd

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) $*.cpp -o $*.o

clean:
	@rm -f *.o *.d */*.o */*.d
	@rm -f ftpd
