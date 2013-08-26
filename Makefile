all:
	waf configure
	waf

run: all
	./build/debug/tactile
