all:
	waf
	$(MAKE) -C tests

run: all
	./build/debug/test2

check: all
	$(MAKE) -C tests check

dump: all
	./build/debug/sample_dumper > dump.txt

count: all
	./build/debug/finger_counter

ids: all
	./build/debug/finger_id_tracker

classify: all
	./build/debug/hand_shape_classifier

touchport: all
	./build/debug/touch_port > touch_port.txt
	sed -i 's/[(),]//g' touch_port.txt
	cat touch_port.txt

clicker: all
	./build/debug/mouse_clicker

mouse: all
	./build/debug/soma_mouse
	./build/release/soma_mouse

keyboard: all
	./build/debug/keyboard
	./build/release/keyboard
