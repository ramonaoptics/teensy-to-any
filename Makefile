all: teensy40

teensy40:
	platformio run --environment teensy40

clean:
	rm -rf .pio

.PHONY: teensy40, all, clean
