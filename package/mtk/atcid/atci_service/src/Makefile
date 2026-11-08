TARGET := atci_service

FLAGS += -DENABLE_GPS_AT_CMD

INCLUDES := -I. \
            -I$(TOPDIR)/../quectel/package/libs/libql_qlog/inc \


SRCS := \
           ./at_tok.c \
           ./atci_generic_cmd_dispatch.c \
           ./atci_gps_cmd.c \
           ./atci_gpio_cmd.c \
           ./atci_lcdbacklight_vibrator_cmd.c \
           ./atci_led_test_cmd.c \
           ./atci_lcd_test_cmd.c \
           ./atci_service.c \
           ./atci_system_cmd.c \
           ./atci_touch_cmd.c \
           ./atci_kpd_cmd.c	\
           ./atci_util.c \
           ./atci_bts_cmd.c \
           ./atci_suspend_cmd.c \
           ./atci_sgmii_cmd.c \
           ./atci_rgmii_cmd.c \
           ./atci_typec_cmd.c \
           ./atci_pcie_cmd.c \
           ./atci_mmc_cmd.c \
           ./atci_msdc_cmd.c \
           ./atci_nand_cmd.c \
           ./atci_efuse_cmd.c

CXXSRCS := \
           ./atci_cct_cmd.cpp \
           ./atci_pq_cmd.cpp \


EXTRA_CXXFLAGS := -std=c++11

OBJS := ${SRCS:%.c=%.o}

CXXOBJS := ${CXXSRCS:%.cpp=%.o}

#LDFLAGS = -L. -ldl -lpthread -lstdc++

.PHONY: all
all : $(TARGET)

${TARGET}: $(OBJS) $(CXXOBJS)
	$(CC) $(OBJS) $(CXXOBJS) $(LDFLAGS) $(LIBS) -o $@

%.o: %.c
	${CC} $(CFLAGS) -c $< -o $@ $(INCLUDES) $(FLAGS)
	
%.o: %.cpp
	${CXX} $(CXXFLAGS) -c $< -o $@ $(INCLUDES) $(FLAGS) $(EXTRA_CXXFLAGS)

.PHONY: clean
clean:
	$(warning "makefile atci_service clean")
	rm -rf $(OBJS) $(CXXOBJS) $(TARGET)
