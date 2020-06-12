$(MK)_C_FILES := $(filter %.c,$($(MK)_SRCS))
$(MK)_C_OBJS := $($(MK)_C_FILES:%.c=$(BUILD_DIR)/%.o)
$(MK)_C_DEPS := $($(MK)_C_FILES:%.c=$(BUILD_DIR)/%.d)

$(MK)_CC_FILES := $(filter %.cc,$($(MK)_SRCS))
$(MK)_CC_OBJS := $($(MK)_CC_FILES:%.cc=$(BUILD_DIR)/%.o)
$(MK)_CC_DEPS := $($(MK)_CC_FILES:%.cc=$(BUILD_DIR)/%.d)

$(MK)_CPP_FILES := $(filter %.cpp,$($(MK)_SRCS))
$(MK)_CPP_OBJS := $($(MK)_CPP_FILES:%.cpp=$(BUILD_DIR)/%.o)
$(MK)_CPP_DEPS := $($(MK)_CPP_FILES:%.cpp=$(BUILD_DIR)/%.d)

$($(MK)_C_OBJS) $($(MK)_C_DEPS): TARGET_SPECIFIC_CFLAGS := $(CFLAGS) $($(MK)_CFLAGS)
$($(MK)_CC_OBJS) $($(MK)_CC_DEPS) $($(MK)_CPP_OBJS) $($(MK)_CPP_DEPS): TARGET_SPECIFIC_CFLAGS := $(CXXFLAGS) $($(MK)_CXXFLAGS)
$($(MK)_TGT1) $($(MK)_TGT2): TARGET_SPECIFIC_LDFLAGS := $(LDFLAGS) $($(MK)_LDFLAGS)

$(MK)_OBJS :=  $($(MK)_C_OBJS) $($(MK)_CC_OBJS) $($(MK)_CPP_OBJS)
$(MK)_DEPS :=  $($(MK)_C_DEPS) $($(MK)_CC_DEPS) $($(MK)_CPP_DEPS)

DEPS += $($(MK)_DEPS)
TARGETS += $($(MK)_TGT1)
TARGETS += $($(MK)_TGT2)
