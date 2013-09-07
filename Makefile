all:
	waf configure
	waf

run: all
	./build/debug/soma_mouse

check: all
	$(MAKE) -C tests check

dump: all
	./build/debug/dump_samples > samples.txt
