all: teensy32 teensy40 arduino_uno

teensy32:
	platformio run --environment teensy32

teensy40:
	platformio run --environment teensy40

arduino_uno:
	platformio run --environment arduino_uno

clean:
	rm -rf .pio

.PHONY: teensy32, teensy40, arduino_uno, all, clean
