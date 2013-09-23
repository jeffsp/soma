all:
	waf

run: check

check: all
	./build/debug/test_audio verbose=true
	./build/debug/test_mouse verbose=true
	./build/debug/test_options verbose=true
	./build/debug/test_stats verbose=true
	./build/debug/test_utility verbose=true
	./build/release/test_audio
	./build/release/test_mouse
	./build/release/test_options
	./build/release/test_stats
	./build/debug/test_utility
	@echo "Success!"
