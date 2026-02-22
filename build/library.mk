# Copyright (C) 2026 sonoransun — see LICENCE.txt

LIBRARY_TARGET = true
RULE_TARGET = library

include $(ALPINE_ROOT)/build/rules.mk


LIB_DIR = $(ALPINE_ROOT)/lib
FULL_SHARED_LIBRARY = $(LIB_DIR)/lib$(FULL_LIBRARY).so
FULL_STATIC_LIBRARY = $(LIB_DIR)/lib$(FULL_LIBRARY).a



library: $(FULL_SHARED_LIBRARY) $(FULL_STATIC_LIBRARY)

$(FULL_SHARED_LIBRARY): $(TARGET_DEPS)
	@echo ""; echo "Building shared library: $(FULL_SHARED_LIBRARY)"
	@echo "$(LD) $(LDFLAGS) $(CPPLIBS) $(OBJECTS) -o $@"
	@$(LD) $(LDFLAGS) $(CPPLIBS) $(OBJECTS) -o $@
	@cp $@ lib/
	@echo "-----";echo ""

$(FULL_STATIC_LIBRARY): $(TARGET_DEPS)
	@echo ""; echo "Building static library: $(FULL_STATIC_LIBRARY)"
	@rm -f $@ lib/lib$(FULL_LIBRARY).a
	@echo "$(AR) $@ $(OBJECTS)"
	@$(AR) $@ $(OBJECTS)
	@cp $@ lib/
	@echo "-----";echo ""

clean:
	@echo "";echo "Cleaning dependencies for target: $(FULL_LIBRARY)";echo ""
	@rm -rf -v $(FULL_SHARED_LIBRARY) $(FULL_STATIC_LIBRARY) core $(BUILD_OBJ_DIR) lib/lib$(FULL_LIBRARY).so lib/lib$(FULL_LIBRARY).a $(CORBA_GEN_FILES)
	@echo "-----";echo ""

