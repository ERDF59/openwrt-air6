TARGET := atcid

FLAGS += -DMTK_GPS_FEATURE -DCONFIG_QUECTEL_MAIN_UART

INCLUDES := -I. \
         -I./platform \
         -I$(TOPDIR)/../quectel/package/libs/libql_qlog/inc \


SRCS := \
           ./at_tok.c \
           ./atcid.c \
           ./atcid_serial.c \
           ./atcid_cmd_dispatch.c \
           ./atcid_cust_cmd.c \
           ./atcid_util.c \
           ./platform/atcid_mipc.c \
           ./platform/atcid_common.c \
           ./atcid_sim_cmd.c\
           ./atcid_universal_ant_cmd.c\
           ./atcid_async_notify.c\
           ./main_uart.c \
           ./platform/ql_send_URC.c

CXXSRCS := \
           ./platform/atcid_adaptation.cpp \


EXTRA_CXXFLAGS := -std=c++11 -fPIC

OBJS := ${SRCS:%.c=%.o}

CXXOBJS := ${CXXSRCS:%.cpp=%.o}

#LDFLAGS = -L. -lz -ldl -lpthread -lstdc++

.PHONY: all
all : $(TARGET)

${TARGET}: $(OBJS) $(CXXOBJS)
	$(CC) $(CFLAGS) $(OBJS) $(CXXOBJS) $(LDFLAGS) $(LIBS) -o $@

%.o: %.c
	${CC} $(CFLAGS) -c $< -o $@ $(INCLUDES) $(FLAGS)

%.o: %.cpp
	${CXX} $(CXXFLAGS) -c $< -o $@ $(INCLUDES) $(FLAGS) $(EXTRA_CXXFLAGS)

.PHONY: clean
clean:
	$(warning "makefile atcid clean")
	rm -rf $(OBJS) $(CXXOBJS) $(TARGET)