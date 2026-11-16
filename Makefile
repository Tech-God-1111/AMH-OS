SHELL := /bin/bash
.PHONY: help bootstrap configure build flash serial clean
help:
	@echo "bootstrap | configure | build | flash PORT=/dev/ttyUSB0 | serial PORT=/dev/ttyUSB0 | clean"
bootstrap:
	bash ./scripts/bootstrap.sh
configure:
	bash ./scripts/configure.sh
build:
	bash ./scripts/build.sh
flash:
	@test -n "$(PORT)" || (echo "Usage: make flash PORT=/dev/ttyUSB0"; exit 2)
	bash ./scripts/flash.sh "$(PORT)"
serial:
	@test -n "$(PORT)" || (echo "Usage: make serial PORT=/dev/ttyUSB0"; exit 2)
	bash ./scripts/serial.sh "$(PORT)"
clean:
	$(MAKE) -C third_party/nuttx distclean
