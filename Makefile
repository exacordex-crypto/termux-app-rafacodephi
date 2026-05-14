CC ?= clang
CFLAGS_BASE ?= -O2 -fno-strict-aliasing -Wall -Wextra -Werror=implicit-function-declaration
ARCH ?= host
EXTRA_CFLAGS ?=

.PHONY: all clean diagnose selftest extract-abi-bootstrap

all: diagnose

diagnose:
	$(MAKE) -C bootstrap_rafaelia clean
	$(MAKE) -C bootstrap_rafaelia CC=$(CC) ARCH=$(ARCH) CFLAGS_COMMON="$(CFLAGS_BASE) $(EXTRA_CFLAGS) -I." selftest

selftest: diagnose

clean:
	$(MAKE) -C bootstrap_rafaelia clean || true
	rm -rf build
	rm -f bootstrap_rafaelia/selftest.log
	@echo "Cleaned top-level build artifacts."

extract-abi-bootstrap:
	python3 tools/bootstrap/extract_abi_bootstrap.py
