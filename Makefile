all: teensy32 teensy40

teensy32:
	platformio run --environment teensy32

teensy40:
	platformio run --environment teensy40

clean:
	rm -rf .pio

.PHONY: teensy32, teensy40, all, clean
