
OS := $(shell uname)

ifeq ($(OS),Darwin)
	OPENGL = -framework OpenGL
else
	OPENGL = -lGL
endif

CC = gcc
CFLAGS = -Wall -Wno-missing-braces -std=gnu99 -Isrc/bundle `pkg-config --cflags vips` `pkg-config --cflags glfw3` `pkg-config --cflags libcurl`
LDFLAGS = $(OPENGL) -Wall -std=gnu99 -ldl -lm `pkg-config --libs vips` `pkg-config --libs glfw3` `pkg-config --libs libcurl`
OBJECTS = main.o file.o shaders.o input.o
DEPS = file.h

MATH_OBJECTS = src/math/intersect.o src/math/camera.c src/math/math.o
BUNDLE_OBJECTS = src/bundle/glad/glad.o

megagraph: $(OBJECTS) $(MATH_OBJECTS) $(BUNDLE_OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(MATH_OBJECTS) $(BUNDLE_OBJECTS) -o megagraph

%.o: src/%.c
	$(CC) $(CFLAGS) -c $<

src/math/%.o: src/math/%.c
	$(CC) $(CFLAGS) -std=gnu99 -DTMS_FAST_MATH -c $< -o $@

src/bundle/%.o: src/bundle/%.c
	$(CC) $(CFLAGS) -std=gnu99 -c $< -o $@

src/bundle/glad/%.o: src/bundle/glad/%.c
	$(CC) $(CFLAGS) -std=gnu99 -c $< -o $@

clean:
	rm *.o 2>/dev/null || true
	rm src/bundle/*.o 2>/dev/null || true
	rm src/math/*.o 2>/dev/null || true
	rm megagraph 2>/dev/null || true
