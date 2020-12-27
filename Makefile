CC = gcc
LD = $(CC)
CFLAGS += -Wall -O3 -std=c99
LDFLAGS += -lXNVCtrl -lX11 -lXext -lGL -lGLU -lglut -lGLEW -lm

TARGETS = gl-gsync-demo

.PHONY: default
default: $(TARGETS)

.PHONY: clean
clean:
	-rm -rf *.o core.* *~ $(TARGETS)

gl-gsync-demo: main.o gsync.o vsync.o
	$(LD) $^ $(LDFLAGS) -o $@

main.o: main.c gsync.h vsync.h
gsync.o: gsync.c gsync.h
vsync.o: vsync.c vsync.h
