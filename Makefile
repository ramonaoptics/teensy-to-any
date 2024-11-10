all: teensy32 teensy40 teensy40_startup_demo

teensy32:
	platformio run --environment teensy32

teensy40:
	platformio run --environment teensy40

teensy40_startup_demo:
	platformio run --environment teensy40_startup_demo

clean:
	rm -rf .pio

.PHONY: teensy32, teensy40, teensy40_startup_demo, all, clean
