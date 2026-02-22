# Copyright (C) 2026 sonoransun — see LICENCE.txt

BINARY_TARGET = true
RULE_TARGET = binary

include $(ALPINE_ROOT)/build/rules.mk

binary: bin/$(FULL_BINARY)


bin/$(FULL_BINARY): $(TARGET_DEPS)
	@echo "";echo "Building binary target: $(FULL_BINARY)"
	@echo "$(CXX) $(CPPFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(CPPLIBS)"
	@$(CXX) $(CPPFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(CPPLIBS)
	@echo "-----";echo ""


clean:
	@echo "";echo "Cleaning dependencies for target: $(FULL_BINARY)";echo ""
	@rm -rf -v bin/$(FULL_BINARY) core $(BUILD_OBJ_DIR)
	@echo "-----";echo ""

