all:
	waf configure
	waf

run: all
	./build/debug/tactile

check: all
	$(MAKE) -C tests check
