# Root Makefile - Delegates to legacy_lib and demo_app

.PHONY: all clean legacy_lib demo_app

all: legacy_lib demo_app

legacy_lib:
	$(MAKE) -C legacy_lib $(MAKECMDGOALS)

demo_app:
	$(MAKE) -C demo_app $(MAKECMDGOALS)

clean:
	$(MAKE) -C legacy_lib clean
	$(MAKE) -C demo_app clean
