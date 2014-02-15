.PHONY: all clean

all:
	$(MAKE) -C c
	$(MAKE) -C cpp

clean:
	$(MAKE) clean -C cpp
	$(MAKE) clean -C c
