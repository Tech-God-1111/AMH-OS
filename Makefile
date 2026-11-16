SHELL := /bin/bash
.PHONY: help bootstrap configure build flash serial clean
help:
	@echo "bootstrap | configure | build | flash PORT=/dev/ttyUSB0 | serial PORT=/dev/ttyUSB0 | clean"
bootstrap:
	./scripts/bootstrap.sh
configure:
	./scripts/configure.sh
build:
	./scripts/build.sh
flash:
	@test -n "$(PORT)" || (echo "Usage: make flash PORT=/dev/ttyUSB0"; exit 2)
	./scripts/flash.sh "$(PORT)"
serial:
	@test -n "$(PORT)" || (echo "Usage: make serial PORT=/dev/ttyUSB0"; exit 2)
	./scripts/serial.sh "$(PORT)"
clean:
	$(MAKE) -C third_party/nuttx distclean
