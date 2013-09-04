all:
	waf configure
	waf

run: check

check: all
	./build/debug/test_audio
	./build/debug/test_soma verbose=true
	./build/release/test_audio
	./build/release/test_soma
