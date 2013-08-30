all:
	waf configure
	waf

run: all
	./build/debug/soma

check: all
	$(MAKE) -C tests check
