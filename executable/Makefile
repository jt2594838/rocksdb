include ../make_config.mk

ifndef DISABLE_JEMALLOC
	ifdef JEMALLOC
		PLATFORM_CXXFLAGS += -DROCKSDB_JEMALLOC -DJEMALLOC_NO_DEMANGLE
	endif
	EXEC_LDFLAGS := $(JEMALLOC_LIB) $(EXEC_LDFLAGS) -lpthread
	PLATFORM_CXXFLAGS += $(JEMALLOC_INCLUDE)
endif

#ifneq ($(USE_RTTI), 1)
#	CXXFLAGS += -fno-rtti
#endif

CXXFLAGS += -I../include -I../
EXEC_LDFLAGS += -L/usr/local/lib -lthrift -lthriftnb -lboost_system -lboost_filesystem

.PHONY: clean librocksdb

all: backend

backend: librocksdb backend.cc
	$(CXX) $(CXXFLAGS) $@.cc -o$@ ../librocksdb.a -O2 -std=c++11 $(PLATFORM_LDFLAGS) $(PLATFORM_CXXFLAGS) $(EXEC_LDFLAGS)

broker: librocksdb broker.cc
	$(CXX) $(CXXFLAGS) $@.cc -o$@ ../librocksdb.a -O2 -std=c++11 $(PLATFORM_LDFLAGS) $(PLATFORM_CXXFLAGS) $(EXEC_LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf ./backend

librocksdb:
	cd .. && $(MAKE) static_lib
