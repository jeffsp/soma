all:
	waf configure
	waf

run: measure

mouse: all
	./build/debug/soma_mouse

check: all
	$(MAKE) -C tests check

dump: all
	./build/debug/dump_samples 1 > /dev/null

measure: all
	./build/debug/measure_samples
