all:
	waf
	$(MAKE) -C tests

run: all
	./build/debug/soma_mouse

touchport: all
	./build/debug/touchport > touchport.txt
	sed -i 's/[(),]//g' touchport.txt
	cat touchport.txt

count: all
	./build/debug/finger_counter

ids: all
	./build/debug/finger_id_tracker

classify: all
	./build/debug/hand_shape_classifier

mouse: all
	./build/debug/soma_mouse
	./build/release/soma_mouse

keyboard: all
	./build/debug/keyboard
	./build/release/keyboard

check: all
	$(MAKE) -C tests check

dump: all
	./build/debug/dump_samples 1 > /dev/null
