all:
	waf

run: all check

check: all
	./build/debug/test_audio verbose=true
	./build/debug/test_finger_counter verbose=true
	./build/debug/test_finger_id_tracker verbose=true
	./build/debug/test_frame_counter verbose=true
	./build/debug/test_mouse verbose=true
	./build/debug/test_options verbose=true
	./build/debug/test_sliding_window verbose=true
	./build/debug/test_stats verbose=true
	./build/release/test_audio
	./build/release/test_finger_counter
	./build/release/test_finger_id_tracker
	./build/release/test_frame_counter
	./build/release/test_mouse
	./build/release/test_options
	./build/release/test_sliding_window
	./build/release/test_stats
	@echo "Success!"
