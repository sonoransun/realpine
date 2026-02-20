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



DIRS = \
    base \
    model \
    transport \
    protocols \
    applcore \
    interfaces \
    corba \
    gui \
    test \
    AlpineServer \
    AlpineCmdIf \
    AlpineRestBridge \
    modules \


ALL_SOURCE_DIRS = \
    base \
    model \
    transport \
    protocols \
    applcore \
    interfaces \
    corba \
    test \
    AlpineServer \
    AlpineCmdIf \
    AlpineRestBridge \
    modules/GenFile \



include $(ALPINE_ROOT)/build/recurse.mk



#
# Custom Targets
#

incomplete-list:
	@echo "";echo "Checking for incomplete tags in sources...";echo "------------------------"
	@rm -f /tmp/codefilelist
	@find $(ALL_SOURCE_DIRS) -name *.cpp >> /tmp/codefilelist 2>/dev/null
	@find $(ALL_SOURCE_DIRS) -name *.c >> /tmp/codefilelist 2>/dev/null
	@find $(ALL_SOURCE_DIRS) -name *.h >> /tmp/codefilelist 2>/dev/null
	@find $(ALL_SOURCE_DIRS) -name *.idl >> /tmp/codefilelist 2>/dev/null
	@rm -f $(ALPINE_ROOT)/incomplete.list
	@for FILE in `cat /tmp/codefilelist`; do \
	    grep MRP_TEMP $$FILE 1>/dev/null 2>&1; \
	    if [ $$? = 0 ]; then \
	        echo $$FILE >> $(ALPINE_ROOT)/incomplete.list; \
	    fi; \
	 done;
	@echo "";echo "Incomplete files:";cat $(ALPINE_ROOT)/incomplete.list
	@echo "";echo "------------------------"
	@NUMFILES=`wc -l $(ALPINE_ROOT)/incomplete.list | sed 's/[^0-9]*//g'`;echo "Total Files: $$NUMFILES"
	@echo "see $(ALPINE_ROOT)/incomplete.list";echo ""


linecount:
	@echo "Cleaning auto generated sources for accurate count"
	@cd /projects/alpine/corba/idl;make clean
	@echo "";echo "Code line count total:";echo "------------------------"
	@rm -f /tmp/codefilelist
	@find $(ALL_SOURCE_DIRS) -name *.cpp >> /tmp/codefilelist 2>/dev/null
	@find $(ALL_SOURCE_DIRS) -name *.c >> /tmp/codefilelist 2>/dev/null
	@find $(ALL_SOURCE_DIRS) -name *.h >> /tmp/codefilelist 2>/dev/null
	@find $(ALL_SOURCE_DIRS) -name *.idl >> /tmp/codefilelist 2>/dev/null
	@wc `cat /tmp/codefilelist`
	@TOTAL_FILES=`wc -l /tmp/codefilelist | sed 's/[^0-9]//g'`;echo "";echo "Total Files: $$TOTAL_FILES";
	@rm -f /tmp/codefilelist
	@echo "";echo "-----";echo ""


gen-html:
	@echo "";echo "Generating / updating HTML code conversion...";echo ""
	@$(ALPINE_ROOT)/clean_all.sh
	@rm -rf $(ALPINE_ROOT)/gen_html/*
	@$(ALPINE_ROOT)/helper/bin/buildHtmlCodeTree
	@echo "";echo "Finished.";echo ""


site-update: gen-html
	@echo "";echo "Synchronizing ALPINE site...";echo ""
	@$(ALPINE_ROOT)/helper/bin/ftpSourceTree
	@echo "";echo "Finished.";echo ""


DEVEL_FILES = \
    Contributors.txt \
    Makefile \
    VERSION \
    GNU-LGPL.txt \
    AlpineCmdIf \
    AlpineServer \
    applcore \
    base \
    bin \
    build \
    build_all.sh \
    clean_all.sh \
    corba \
    helper \
    interfaces \
    lib \
    model \
    protocols \
    quickbuild.sh \
    quickclean.sh \
    test \
    transport \
    gui \
    modules \


devel-snapshot:
	@echo "";echo "Cleaning project tree...";
	@./clean_all.sh 1>/dev/null 2>/dev/null
	@echo "";echo "Packaging development sources/files...";echo ""
	@rm -f $(ALPINE_ROOT)/release/devel/alpine-devel.tar.gz 2>/dev/null;echo ""
	@rm -f `find ./test/ -name \*.log` 2>/dev/null;echo ""
	@cd ..;tar -cvf $(ALPINE_ROOT)/release/devel/alpine-devel.tar $(addprefix alpine/,$(DEVEL_FILES))
	@gzip -9 $(ALPINE_ROOT)/release/devel/alpine-devel.tar
	@echo "";echo "Transferring development packages/sources...";echo `ls -l $(ALPINE_ROOT)/release/devel/alpine-devel.tar.gz`
	@$(ALPINE_ROOT)/helper/bin/ftpPackages


project: devel-snapshot gen-html site-update
	@echo "";echo "-- Project Build/Update Complete --"; echo ""



