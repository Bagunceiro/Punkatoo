import os

VERFILE='.pio/build/esp32/src/versions.cpp.o'

if os.path.isfile(VERFILE):
	os.remove(VERFILE)
