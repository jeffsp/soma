all:
	waf configure
	waf

run: all
	./build/debug/soma
	sleep 1
	./build/debug/soma_mouse

check: all
	$(MAKE) -C tests check

play: all
	./build/debug/play
	./build/debug/play
	./build/debug/play
