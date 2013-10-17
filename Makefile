#auto-increment after every compile
BUILD_NUMBER_FILE = buildnum.txt

CFLAGS = -D__BUILDNUM__=$$(cat $(BUILD_NUMBER_FILE)) -Wno-deprecated -Wno-unused-result -ffast-math -O2
LDLIBS = -lagl -laldmb -ldumb  -lGL -lGLU `allegro-config --libs`
CC=gcc
OUTPUT=GeraldinesJellybeanMachine.elf

all:
	@if ! test -f $(BUILD_NUMBER_FILE); then echo 0 > $(BUILD_NUMBER_FILE); fi
	@echo $$(($$(cat $(BUILD_NUMBER_FILE)) + 1)) > $(BUILD_NUMBER_FILE)
	$(CC) $(CFLAGS) src/*.c $(LDLIBS) -o $(OUTPUT)
	@echo "Done! :o)\n"

clean:
	@rm -f $(OUTPUT)
