all:
	waf
	$(MAKE) -C tests

run: all
	./build/debug/soma_mouse

count: all
	./build/debug/finger_counter

classify: all
	./build/debug/hand_shape_classifier

mouse: all
	./build/debug/soma_mouse
	./build/release/soma_mouse

check: all
	$(MAKE) -C tests check

dump: all
	./build/debug/dump_samples 1 > /dev/null
