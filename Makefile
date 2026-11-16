SHELL := /bin/bash
.PHONY: help bootstrap configure build flash serial test clean
help:
	@echo "bootstrap | configure | build | test | flash PORT=/dev/ttyUSB0 AMH_BOARD_CONFIRMED=YES | serial PORT=/dev/ttyUSB0"
bootstrap:
	bash ./scripts/bootstrap.sh
configure:
	bash ./scripts/configure.sh
build:
	bash ./scripts/build.sh
test:
	python3 ./tests/check_project.py
flash:
	@test -n "$(PORT)" || (echo "Usage: make flash PORT=/dev/ttyUSB0 AMH_BOARD_CONFIRMED=YES"; exit 2)
	@test "$(AMH_BOARD_CONFIRMED)" = "YES" || (echo "Set AMH_BOARD_CONFIRMED=YES only after verifying the exact board."; exit 2)
	AMH_BOARD_CONFIRMED="$(AMH_BOARD_CONFIRMED)" bash ./scripts/flash.sh "$(PORT)"
serial:
	@test -n "$(PORT)" || (echo "Usage: make serial PORT=/dev/ttyUSB0"; exit 2)
	bash ./scripts/serial.sh "$(PORT)"
clean:
	@test ! -d third_party/nuttx || $(MAKE) -C third_party/nuttx distclean
