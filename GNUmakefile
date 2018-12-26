#!make -f

default:
	@echo "Either use 'build' script or 'cd build && make'" 1>&2

cleanup:
	-rm -f cmake_install.cmake CMakeCache.txt CMakeFiles Makefile

SCRIPTS := $(notdir $(wildcard bin/*))
SOURCES := $(filter-out $(realpath bin)%,$(shell findbin -a $(SCRIPTS)))

update-bin:
	for f in ${SCRIPTS}; do \
          for s in ${SOURCES}; do \
            perl -le '($$s,$$f)=@ARGV; system "rsync -auv $$s bin/" if $$s =~ m{/$$f}' $$s $$f; \
          done \
        done

MAKEFILE_RULES := $(realpath $(lastword $(MAKEFILE_LIST)))
PHONIES := $(shell perl -lane 'print $$1 if m{^([a-zA-Z][-a-zA-Z0-9_]*):[^=]*$$};' ${MAKEFILE_RULES})
.PHONY: $(PHONIES)

targets:
	@echo "Targets here are limited to: $(PHONIES)"

help:
	@echo "See README.md"
