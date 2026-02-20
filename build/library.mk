#//////
#//
#//  Copyright (C) 2026  sonoransun
#//
#//  Permission is hereby granted, free of charge, to any person obtaining a copy
#//  of this software and associated documentation files (the "Software"), to deal
#//  in the Software without restriction, including without limitation the rights
#//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#//  copies of the Software, and to permit persons to whom the Software is
#//  furnished to do so, subject to the following conditions:
#//
#//  The above copyright notice and this permission notice shall be included in all
#//  copies or substantial portions of the Software.
#//
#//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#//  SOFTWARE.
#//
#//////

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

