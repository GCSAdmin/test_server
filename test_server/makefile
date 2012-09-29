#test_server
CC=gcc
CPP=g++
srcpath=.
includepath=$(srcpath)
LIB_PATH=$(srcpath)/libs/glib-2.0/lib
#glib_include=$(srcpath)/libs/glib-2.0/include/glib-2.0
glib_cflags=-I$(srcpath)/libs/glib-2.0/include/glib-2.0 -I$(srcpath)/libs/glib-2.0/lib/glib-2.0/include
GLIBS=-lglib-2.0 -lgthread-2.0

MYSQL_LIBS=		`mysql_config --libs_r` -lrt

MYSQL_INC=		-I. `mysql_config --include`

ifeq ($(MAKE_FLAG), DEBUG)
  dbg_option=-g -O0 -D_DEBUG
else
  ifeq ($(MAKE_FLAG), TEST)
    dbg_option=-O2 -D_LINUX_TEST
  else
    dbg_option=-O2
  endif
endif
PlatformLdOption=-Wl,-rpath,./,-rpath,$(LIB_PATH)
PlatformLibs=-lc -lm -lrt -lpthread -ldl $(PlatformLdOption)

CFLAGS=-fpic $(dbg_option) -c -I$(includepath) $(MYSQL_INC) $(glib_cflags) 

ALL_OBJECTS=test_server.o 

final_objects=test_server

all : $(final_objects)

.PHONY : all clean rebuild
	
test_server : $(ALL_OBJECTS)
	$(CC) $(PlatformLibs) -o $@ $(ALL_OBJECTS) -L$(LIB_PATH) $(MYSQL_LIBS) $(GLIBS)

clean:
	@rm -rf $(ALL_OBJECTS)
	@rm -rf $(final_objects)	

rebuild : clean all

#inference rules
%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@
	
%.o:%.cpp
	$(CPP) -c $(CFLAGS) $< -o $@
