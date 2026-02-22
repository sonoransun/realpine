# Copyright (C) 2026 sonoransun — see LICENCE.txt

include $(ALPINE_ROOT)/build/config.mk


.PHONY : $(SYS_PHONY) $(ALPINE_PHONY) pre_build all default clean library binary internal-dep $(RULE_TARGET)




# rule for building dependency files and first pass dependencies
# (hack for ensuring dep file creation first)
#
ifneq (true,$(_SECOND_PASS_))

default all: pre_build corba-dep internal-dep
	@$(MAKE) -s _SECOND_PASS_=true

internal-dep: $(addprefix $(BUILD_OBJ_DIR)/, $(CPP_FILES:.$(CPP_EXT)=.dep))

corba-dep: $(CORBA_DEPS)



#
# IDL compiliation
ifeq (true,$(USE_TAO_ORB))
%S.cpp: %.idl 
	@echo "Compiling IDL file: $<"
	@echo "$(IDLCOMPILE) $(IDLFLAGS) $<"
	@$(IDLCOMPILE) $(IDLFLAGS) $<
endif

ifeq (true,$(USE_ORBIT_ORB))
%-cpp.cc: %.idl
	@echo "Compiling IDL file: $<"
	@echo "$(IDLCOMPILE) $(IDLFLAGS) $<"
	@$(IDLCOMPILE) $(IDLFLAGS) $<
endif



# Build automatic dependency file...
#
# if verbose output is desired add the following line to build rule
#
$(BUILD_OBJ_DIR)/%.dep: %.$(CPP_EXT)
	@if [ ! -d $(BUILD_OBJ_DIR) ]; then sleep 1; fi
	@echo "--- Automatic dependency file update: $@"
	@TEMP="$@.tmp";rm -f $@ $$TEMP 2>/dev/null; \
	 $(CXX) -M $(CPPFLAGS) $< 1>/tmp/$<.tmp_out 2>&1; \
          if [ $$? != 0 ]; then \
	     echo "ERROR: dependency file creation failed for $@"; \
	     echo "Dependency build command:"; \
	     echo "$(CXX) -M $(CPPFLAGS) $<"; \
	     echo "-------------------------"; echo ""; \
	     echo "Failed output: "; cat /tmp/$<.tmp_out; \
	     echo "-------------------------"; echo ""; \
	     rm -f /tmp/$<.tmp_out 2>/dev/null; exit 1; \
	  fi; \
	 rm -f /tmp/$<.tmp_out 2>/dev/null; \
	 $(SHELL) -ec 'TEMP="$@.tmp";TARGET=$$(echo $@ | sed '\''s/\//\\\//g'\'');$(CXX) -M $(CPPFLAGS) $< | sed '\''s/\($*\)\.o[ :]*/\1.o : /g'\'' > $$TEMP;'
	@TEMP="$@.tmp";$(ALPINE_ROOT)/helper/bin/util_addprefix $(BUILD_OBJ_DIR)/ $$TEMP
	@TEMP="$@.tmp";$(ALPINE_ROOT)/helper/bin/util_addrule '@echo "$$(CXX) $$(CPPFLAGS) -c $$< -o $$@"' $$TEMP
	@TEMP="$@.tmp";$(ALPINE_ROOT)/helper/bin/util_addrule '@$$(CXX) $$(CPPFLAGS) -c $$< -o $$@' $$TEMP
	@TEMP="$@.tmp";mv $$TEMP $@







else
# Second pass, actual build process...
#


# create directories for binary or library target.
ifeq (binary,$(RULE_TARGET))
    TARGET_DIR_RULE = if [ ! -d bin ]; then mkdir bin; touch bin/.NO_HTML_GEN; fi
else
    TARGET_DIR_RULE = if [ ! -d lib ]; then mkdir lib; touch lib/.NO_HTML_GEN; fi
endif



default all: pre_build $(RULE_TARGET)

# Include our dependency files
#
include $(addprefix $(BUILD_OBJ_DIR)/, $(CPP_FILES:.$(CPP_EXT)=.dep))


endif

# Rule for creating object directory if it does not exist.
#
pre_build:
	@if [ ! -d $(BUILD_OBJ_DIR) ]; then mkdir $(BUILD_OBJ_DIR); touch $(BUILD_OBJ_DIR)/.NO_HTML_GEN; fi
	@$(TARGET_DIR_RULE)


