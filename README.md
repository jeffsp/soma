#Building

##Linux
* Leap_Developer_Kit_0.8.0_5300_Linux : libLeap.so is installed in /usr/bin/Leap/, so it cannot be found by the runtime
  linker.

  	$ sudo ln -s /usr/lib/Leap/libLeap.so /usr/lib/
