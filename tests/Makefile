all:
	waf configure
	waf

run: check

check: all
	./build/debug/test_tactile verbose=true
	./build/release/test_tactile
