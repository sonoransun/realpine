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

.PHONY : all default clean library binary $(DIRS)

MAKE_ARGS = -s

ifeq (true,$(PROFILE))
  MAKE_ARGS += PROFILE=true
endif
ifeq (true,$(SERIALIZE_BUILD))
  MAKE_ARGS += -j 1
endif



default all:
	@for DIR in $(DIRS); do \
            cd $$DIR; \
            echo ">>> Building in $$PWD"; \
            $(MAKE) $(MAKE_ARGS) all; \
            if [ $$? != 0 ]; then exit 1; fi; \
            cd -; \
        done



clean:
	@for DIR in $(DIRS); do \
            cd $$DIR; \
            echo ">>> Cleaning $$PWD"; \
            $(MAKE) $(MAKE_ARGS) clean; \
            cd -; \
        done

