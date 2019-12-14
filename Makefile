LIBNAME                     ?= scope
TESTAPP                     ?= test_$(LIBNAME)

MKDIR_P                     ?= mkdir -p
RM_RF                       ?= rm -rf
FIND                        ?= find

SRC_DIR                     ?= 
INCLUDE_DIR                 ?= include
OUT_DIR                     ?= _out
TEST_DIR                    ?= test
MAKEFILES_DIR               ?= build/makefiles
CATCH_DIR                   ?= $(TEST_DIR)/external/catch2

DEBUG                       ?= 0
ARCH                        ?= x86_64-linux-gnu
STDC                        ?= c99
STDCXX                      ?= c++17

TARGET_SRC_DIRS             ?= $(SRC_DIR) $(TEST_DIR)
TARGET_SRC_DIRS_EXCLUDE     ?= $(CATCH_DIR)
TARGET_INC_DIRS             ?= $(INCLUDE_DIR) $(CATCH_DIR)/single_include $(TARGET_SRC_DIRS)

ifeq ($(DEBUG),1)
  TARGET_OUT_DIR            = $(OUT_DIR)/$(ARCH)-debug
else
  TARGET_OUT_DIR            = $(OUT_DIR)/$(ARCH)
endif

WARN_FLAGS                  ?= -Wall -Wextra -Werror
WARN_CFLAGS                 ?= $(WARN_FLAGS)
WARN_CXXFLAGS               ?= $(WARN_FLAGS) -Wno-noexcept-type

DEBUGFLAGS                  ?= -g
RELEASEFLAGS                ?= -O2
ifeq ($(DEBUG),1)
  DRFLAGS                   = $(DEBUGFLAGS)
else
  DRFLAGS                   = $(RELEASEFLAGS)
endif

CPPFLAGS                    = -MMD -MP
CFLAGS                      = $(DRFLAGS) $(WARN_CFLAGS) -std=$(STDC)
CXXFLAGS                    = $(DRFLAGS) $(WARN_CXXFLAGS) -std=$(STDCXX)
INCFLAGS                    = $(addprefix -I,$(TARGET_INC_DIRS))

FIND_EXPR_BASE              = \( -name \*.cpp -or -name \*.c \) -and -print
FIND_EXPR_SUB1              = $(and $(TARGET_SRC_DIRS_EXCLUDE),$(patsubst %,-path % -or,$(TARGET_SRC_DIRS_EXCLUDE)))
FIND_EXPR_SUB2              = $(and $(FIND_EXPR_SUB1),\( -type d -and \( $(FIND_EXPR_SUB1) ! -type d \) -and -prune \) -or )
FIND_EXPR                   = $(FIND_EXPR_SUB2) $(FIND_EXPR_BASE)

SRCS                        = $(shell $(FIND) $(TARGET_SRC_DIRS) $(FIND_EXPR))
OBJS                        = $(SRCS:%=$(TARGET_OUT_DIR)/%.o)
DEPS                        = $(OBJS:.o=.d)

-include $(MAKEFILES_DIR)/$(ARCH).mk

-include $(DEPS)


.DEFAULT_GOAL := all

.PHONY: all
all: runtest

.PHONY: test
test: $(TARGET_OUT_DIR)/$(TESTAPP)

.PHONY: runtest
runtest: test
	$(TARGET_OUT_DIR)/$(TESTAPP)

.PHONY: clean
clean:
	$(RM_RF) $(OUT_DIR)

# C Sources
$(TARGET_OUT_DIR)/%.c.o: %.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCFLAGS) -c $< -o $@
 
# C++ Sources
$(TARGET_OUT_DIR)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@

# Test application
$(TARGET_OUT_DIR)/$(TESTAPP): $(OBJS)
	@$(MKDIR_P) $(dir $@)
	$(CXX) -o $@ $(OBJS)
