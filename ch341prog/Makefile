Q = @
HOSTCC ?= gcc
HOSTCXX ?= g++

LIBS = -lusb-1.0

OBJS = main.o ch341.o misc.o spi_flash.o spi_ids.o stdafx.o

DEPS = $(OBJS:.o=.d)

all:	ch341prog

ch341prog: $(OBJS)
	@echo "  HOSTLD   " $@
	$(Q)$(HOSTCXX) $(LDFLAGS) $^ $(LIBS) -o $@$(EXE)

%.o: %.c
	@echo "  HOSTCC   " $@
	$(Q)$(HOSTCC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -MF "$(@:.o=.d)" -c $< -o $@

%.o: %.cpp
	@echo "  HOSTCXX  " $@
	$(Q)$(HOSTCXX) $(CPPFLAGS) $(CFLAGS) -MMD -MP -MF "$(@:.o=.d)" -c $< -o $@

clean:
	@echo "  CLEAN    "
	$(Q)rm -f $(OBJS) $(DEPS) ch341prog ch341prog.exe

sinclude $(DEPS)
