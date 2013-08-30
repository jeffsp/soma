all:
	waf configure
	waf

run: check

check: all
	./build/debug/test_soma verbose=true
	./build/release/test_soma
