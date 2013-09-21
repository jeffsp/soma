all:
	waf configure
	waf

run: all
	./build/debug/train > training.dat
	./build/debug/classify < training.dat

mouse: all
	./build/debug/soma_mouse

check: all
	$(MAKE) -C tests check

dump: all
	./build/debug/dump_samples 1 > /dev/null
