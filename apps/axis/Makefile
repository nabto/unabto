AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/Rules.axis

# We build a library using cmake and link it with the axis makefile.
TUNNELLIB     =  $(AXIS_BUILDTYPE)/tunnel_library/libunabto_tunnel_library.a
AXISLIB       =  $(AXIS_BUILDTYPE)/libaxistunnel.a

.PHONY: $(MYLIB)

PROGS     = axistunnel

CFLAGS   += -Wall
LDFLAGS  += -lparam -lm -lpthread -lrt


ifeq ($(AXIS_BUILDTYPE), host)
all:
	@echo "you must define AXIS_BUILDTYPE"
	exit 1

else
all:    $(PROGS)

endif

# Link and strip the binary
$(PROGS): $(AXISLIB) $(TUNNELLIB) 
	$(CC) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@
	$(STRIP) $@

clean:
	rm -f $(PROGS) *.o *.a core

