# Copyright (C) 2026 sonoransun — see LICENCE.txt

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

