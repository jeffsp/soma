all:
	waf configure
	waf

run: train

mouse: all
	./build/debug/soma_mouse

check: all
	$(MAKE) -C tests check

dump: all
	./build/debug/dump_samples 1 > /dev/null

train: all
	./build/debug/train
