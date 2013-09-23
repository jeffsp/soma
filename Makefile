all:
	waf
	$(MAKE) -C tests

run: all
	./build/debug/soma_mouse < training.dat

train: all
	./build/debug/train > training.dat
	./build/debug/classify < training.dat

check: all
	$(MAKE) -C tests check

dump: all
	./build/debug/dump_samples 1 > /dev/null
