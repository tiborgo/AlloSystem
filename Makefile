#=========================================================================
# AlloCore main makefile
#=========================================================================

include Makefile.config

# Include configuration files of AlloCore modules
# TODO: Permit selective inclusive of modules for building a library
# and doing unit tests.

MODULE_DIRS := $(GFX_DIR) $(IO_DIR) $(MATH_DIR) $(PRO_DIR) $(SND_DIR) $(SPA_DIR) $(SYS_DIR) $(TYP_DIR)

include $(SRC_DIR)$(GFX_DIR)Makefile.config
include $(SRC_DIR)$(IO_DIR)Makefile.config
include $(SRC_DIR)$(MATH_DIR)Makefile.config
include $(SRC_DIR)$(PRO_DIR)Makefile.config
include $(SRC_DIR)$(SND_DIR)Makefile.config
include $(SRC_DIR)$(SPA_DIR)Makefile.config
include $(SRC_DIR)$(SYS_DIR)Makefile.config
include $(SRC_DIR)$(TYP_DIR)Makefile.config

# Prefix full path to source files
GFX_SRC		:= $(addprefix $(SRC_DIR)$(GFX_DIR), $(GFX_SRC))
IO_SRC		:= $(addprefix $(SRC_DIR)$(IO_DIR), $(IO_SRC))
MATH_SRC	:= $(addprefix $(SRC_DIR)$(MATH_DIR), $(MATH_SRC))
PRO_SRC		:= $(addprefix $(SRC_DIR)$(PRO_DIR), $(PRO_SRC))
TYP_SRC		:= $(addprefix $(SRC_DIR)$(TYP_DIR), $(TYP_SRC))
SND_SRC		:= $(addprefix $(SRC_DIR)$(SND_DIR), $(SND_SRC))
SPA_SRC		:= $(addprefix $(SRC_DIR)$(SPA_DIR), $(SPA_SRC))
SYS_SRC		:= $(addprefix $(SRC_DIR)$(SYS_DIR), $(SYS_SRC))

# These are all the source files
SRCS		= \
		$(GFX_SRC) $(IO_SRC) $(PRO_SRC) $(MATH_SRC) \
		$(SND_SRC) $(SPA_SRC) $(SYS_SRC) $(TYP_SRC)

OBJS		= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= $(addprefix -I, $(INC_DIRS) $(RINC_DIRS) $(BUILD_DIR)/include)
LDFLAGS		:= $(addprefix -L, $(LIB_DIRS) $(BUILD_DIR)/lib) $(LDFLAGS)
LINK_LIBS_PATH	= $(wildcard $(BUILD_DIR)lib/*.a)
LINK_LIBS_FLAGS	=

#--------------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------------

help:
	@echo No rule specified.
	@echo The possible rules are:
	@echo     allocore .... build allocore
	@echo     allojit ..... build allocore JIT extension
	@echo     alloutil .... build allocore utilities extension
	@echo     gamma ....... build Gamma external
	@echo     glv ......... build GLV external

include Makefile.rules


# Compile and run source files in examples/ folder
EXEC_TARGETS = examples/%.cpp
ifeq ($(PLATFORM), linux)
	LINK_LIBS_FLAGS += $(addprefix -l :, $(notdir $(LINK_LIBS_PATH)))
endif
.PRECIOUS: $(EXEC_TARGETS)
$(EXEC_TARGETS): allocore alloutil FORCE
#	@echo $(LINK_LIBS_FLAGS)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)$(*F) $@ $(LDFLAGS) $(LINK_LIBS_FLAGS) $(LINK_LIBS_PATH)
ifneq ($(AUTORUN), 0)
	@cd $(BIN_DIR) && ./$(*F)
endif

extended: all alloni

all: extensions externals

# AlloCore extensions
extensions: allojit alloutil allocore

allocore: $(SLIB_PATH)
# 	copy header files
	@$(INSTALL) -d $(BUILD_DIR)/include/$@
	@$(INSTALL) -c -m 644 $(INC_DIR)/$@/*.h* $(BUILD_DIR)/include/$@
	@for v in `cd $(INC_DIR)/$@ && find * -type d ! -path '*.*'`; do \
		$(INSTALL) -d $(BUILD_DIR)/include/$@/$$v; \
		$(INSTALL) -c -m 644 $(INC_DIR)/$@/$$v/*.h* $(BUILD_DIR)/include/$@/$$v;\
	done
	@$(INSTALL) -C -m 644 $(DEV_LIB_DIR)/*.a $(BUILD_DIR)/lib
# 	I don't know how to do this:
#	ifeq ($(PLATFORM), macosx)
#		@$(INSTALL) -C -m 644 $(DEV_LIB_DIR)/*.dylib $(BUILD_DIR)/lib
#	endif
	
#	@$(MAKE) install DESTDIR=$(BUILD_DIR)

allojit alloutil alloni:
	@$(MAKE) -C src/$@ install BUILD_DIR=../../$(BUILD_DIR) DESTDIR=../../$(BUILD_DIR)


# AlloCore externals
externals: gamma glv

gamma glv:
	@$(MAKE) -C externals/$@ install DESTDIR=../../$(BUILD_DIR)


#$(EXTRA_MODULES):
#	@$(MAKE) -C modules/$@ external
#	@$(MAKE) -C modules/$@ install DESTDIR=`pwd`/$(BUILD_DIR)


# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/LIB_NAME and
# library files are copied to DESTDIR/lib
install: allocore

#	copy all header files from local build directory to destination
	@for v in `cd $(BUILD_DIR)/include && find * -type d ! -path '*.*'`; do \
		$(INSTALL) -d $(DESTDIR)/include/$$v; \
		$(INSTALL) -c -m 644 $(BUILD_DIR)/include/$$v/*.h* $(DESTDIR)/include/$$v;\
	done

# 	copy all library files from local build directory to destination
	@for v in `cd $(BUILD_DIR)/lib && find * -type d ! -path '*.*'` .; do \
		$(INSTALL) -d $(DESTDIR)/lib/$$v; \
		$(INSTALL) -c -m 644 $(BUILD_DIR)/lib/$$v/*.a $(DESTDIR)/lib/$$v; \
	done

#	# copy header from local include directory
#	@for v in `cd $(INC_DIR) && find * -type d ! -path '*.*'`; do \
#		$(INSTALL) -d $(DESTDIR)/include/$$v; \
#		$(INSTALL) -c -m 644 $(INC_DIR)$$v/*.h* $(DESTDIR)/include/$$v;\
#	done

#	@$(INSTALL) -d $(addprefix $(DESTDIR)/include/$(LIB_NAME)/, $(MODULE_DIRS))
#	@$(INSTALL) -C -m 644 $(SLIB_PATH) $(DESTDIR)/lib
#	@$(INSTALL) -C -m 644 $(DEV_DIR)lib/*.a $(DESTDIR)/lib
#	@$(INSTALL) -c -m 644 $(EXT_LIB_DIR)* $(DESTDIR)/lib
#	@$(RANLIB) $(DESTDIR)/lib/$(SLIB_FILE)


# Archive repository
archive:
	$(eval $@_TMP := $(shell mktemp -d tmp.XXXXXXXXXX))
	@echo Creating archive, this may take some time...
	@echo Creating temporary export...
	@svn export --force . $($@_TMP)
	@echo Compressing...
	@cd $($@_TMP) && tar -czf ../allocore.tar.gz .
	@echo Compression complete.
	@rm -R $($@_TMP)


# Remove build files
.PHONY: clean
clean: createFolders
	@$(RM) -rf $(BUILD_DIR)*
	@$(RM) -rf $(TEST_DIR)/$(BUILD_DIR)*


# Build unit tests
test: allocore external
	@$(MAKE) -C $(TEST_DIR) test
