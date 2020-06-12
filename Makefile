TARGETS:=
BUILD_DIR:=out/build
OUT_BIN_DIR:=out/bin
OUT_LIB_DIR:=out/lib
INSTALL_PREFIX=out/install

TARGET_SPECIFIC_CFLAGS :=
TARGET_SPECIFIC_CXXFLAGS :=
TARGET_SPECIFIC_LDFLAGS :=

CFLAGS :=
CXXFLAGS :=
LDFLAGS := -L$(OUT_LIB_DIR)

BINDIR := $(INSTALL_PATH)/usr/bin
LIBSDIR := $(INSTALL_PATH)/usr/lib

-include project.mk

ifneq ($(PLATFORMS),)
ifeq (,$(wildcard .platform))
ifeq (,$(findstring $(MAKECMDGOALS), $(PLATFORMS)))
$(info No platform configured: run 'make <platform>')
$(info platforms: $(PLATFORMS))
$(error error)
endif
endif
endif

ifneq ($(PLATFORMS),)
ifeq (,$(findstring $(MAKECMDGOALS), $(PLATFORMS)))
	include .platform
endif
endif

export AR := $(TOOLCHAIN_PREFIX)ar$(TOOLCHAIN_POSTFIX)
export CC := $(TOOLCHAIN_PREFIX)gcc$(TOOLCHAIN_POSTFIX)
export CXX := $(TOOLCHAIN_PREFIX)g++$(TOOLCHAIN_POSTFIX)

ifeq ($(V),1)
Q=
else
Q=@
endif

DEPS :=

.PHONY: all clean distclean $(PLATFORMS)

.SUFFIXES:

ifneq (,$(findstring $(MAKECMDGOALS), $(PLATFORMS) clean distclean))

clean:
ifneq (,$(wildcard $(BUILD_DIR)))
	@rm -rvf $$(find $(BUILD_DIR)/ -name *.o)
endif
	@rm -rvf $(OUT_BIN_DIR)/* $(OUT_LIB_DIR)/*

distclean:
	@rm -rvf out

else

RULES = $(shell find * -type f -name rules.mk)

.DEFAULT_GOAL := all

# include the description for each module
define INCLUDE_FILE
	MK_BASE = $(shell dirname $(1))
	include $1
endef
$(foreach r,$(RULES),$(eval $(call INCLUDE_FILE,$(r))))

all: $(TARGETS)

# include the dependencies
-include $(DEPS)

# making object files from c
$(BUILD_DIR)/%.o: %.c
	$Qecho \[$(CC)] $@
	$Q$(CC) -c -o $@ $< $(CFLAGS) $(TARGET_SPECIFIC_CFLAGS)

# making object files from cpp
$(BUILD_DIR)/%.o: %.cpp
	$Qecho \[$(CXX)] $@
	$Q$(CXX) -c -o $@ $< $(CXXFLAGS) $(TARGET_SPECIFIC_CFLAGS)

# making object files from cc
$(BUILD_DIR)/%.o: %.cc
	$Qecho \[$(CXX)] $@
	$Q$(CXX) -c -o $@ $< $(CXXFLAGS) $(TARGET_SPECIFIC_CFLAGS)

# calculate C dependencies
$(BUILD_DIR)/%.d: %.c
	$Qmkdir -p $(shell dirname $@) && echo [DEP] $@
	$Q$(CC) -MM $(CFLAGS) $(TARGET_SPECIFIC_CFLAGS) $< \
	   	| sed -e '1 s/[^:]*/$(subst /,\/,${BUILD_DIR})\/$(subst /,\/,$(<:.c=.o))/' > $@

# calculate CXX dependencies
$(BUILD_DIR)/%.d: %.cpp
	$Qmkdir -p $(shell dirname $@) && echo [DEP] $@
	$Q$(CC) -MM $(CXXFLAGS) $(TARGET_SPECIFIC_CFLAGS) $< \
		| sed -e '1 s/[^:]*/$(subst /,\/,${BUILD_DIR})\/$(subst /,\/,$(<:.cpp=.o))/' > $@

# calculate CC dependencies
$(BUILD_DIR)/%.d: %.cc
	$Qmkdir -p $(shell dirname $@) && echo [DEP] $@
	$Q$(CC) -MM $(CXXFLAGS) $(TARGET_SPECIFIC_CFLAGS) $< \
		| sed -e '1 s/[^:]*/$(subst /,\/,${BUILD_DIR})\/$(subst /,\/,$(<:.cc=.o))/' > $@

endif
