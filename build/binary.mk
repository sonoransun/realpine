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

