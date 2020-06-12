MK := $(MK_BASE)/rules.mk

$(MK)_SRCS := $(MK_BASE)/randomfile.cpp \
	      	  $(MK_BASE)/main.cpp

$(MK)_LDFLAGS := -lstdc++fs

$(MK)_CFLAGS := -g -O2 -Wall -fstack-protector-all
$(MK)_CXXFLAGS := $($(MK)_CFLAGS) -std=c++17

$(MK)_TGT1 := $(OUT_BIN_DIR)/randomfile

include common.mk

# Do NOT use $($(MK)_OBJS) in target's recipe since it will contain not expected value
RFILE_OBJS := $($(MK)_OBJS)

$($(MK)_TGT1): $(RFILE_OBJS) $(RFILE_DEP_LIBS)
	$Qecho [$(CXX)] $@
	$Qmkdir -p $(shell dirname $@)
	$Q$(CXX) -o $@ $(RFILE_OBJS)	$(TARGET_SPECIFIC_LDFLAGS)
