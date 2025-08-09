CC=clang
CFLAGS=-Wall -Wextra -O2
LDFLAGS=-lSDL3 -lvulkan

TARGET=main
SOURCES=main.c hardware_device.c queue_families.c logical_device.c swap_chain.c shader_module_loader.c error.c
OBJECTS=$(SOURCES:.c=.o)

GLSL_FILES := $(wildcard glsl/*.vert glsl/*.frag)
SPIRV_FILES := $(patsubst glsl/%, spir-v/%.spv, $(GLSL_FILES))

# FUCK OPENGL

.PHONY: all clean

all: shaders $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) $(SPIRV_FILES)

spir-v/%.spv: glsl/%
	@mkdir -p spir-v
	glslc $< -o $@

shaders: $(SPIRV_FILES)
