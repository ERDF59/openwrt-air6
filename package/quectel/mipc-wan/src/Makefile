TARGET = mipc_wan_cli

$(info $(_VOICE))
ifeq ($(_VOICE),YES)
SRCS := \
	mipc-wan-cli.c \
	mipc-wan-netlink.c \
	call_dial.c
else
SRCS := \
	mipc-wan-cli.c \
	mipc-wan-netlink.c
endif
$(info $(SRCS))

.PHONY: all
all : $(TARGET)

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(SRCS) -o $(TARGET) 

.PHONY : clean
clean:
	rm -f $(TARGET)