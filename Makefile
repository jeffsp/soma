all:
	waf configure
	waf

run: all
	./build/debug/soma
	sleep 5
	./build/debug/soma_pointer

check: all
	$(MAKE) -C tests check
