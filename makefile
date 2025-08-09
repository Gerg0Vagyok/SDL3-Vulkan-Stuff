CC=clang
CFLAGS=-Wall -Wextra -O2
LDFLAGS=-lSDL3 -lvulkan

SRC_DIR=src
BIN_DIR=bin

GLSL_DIR=$(SRC_DIR)/glsl
SPIRV_DIR=$(BIN_DIR)/spir-v

TARGET=$(BIN_DIR)/main
SOURCES=main.c hardware_device.c queue_families.c logical_device.c swap_chain.c shader_module_loader.c error.c
SRC_OBJECTS=$(addprefix $(SRC_DIR)/, $(SOURCES))
BIN_OBJECTS=$(addprefix $(BIN_DIR)/, $(SOURCES:.c=.o))

GLSL_FILES := $(wildcard $(GLSL_DIR)/*.vert $(GLSL_DIR)/*.frag)
SPIRV_FILES := $(patsubst $(GLSL_DIR)/%, $(SPIRV_DIR)/%.spv, $(GLSL_FILES))

# FUCK OPENGL

.PHONY: all clean

all: shaders $(TARGET)

$(TARGET): $(BIN_OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(BIN_OBJECTS) $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(SPIRV_DIR):
	mkdir -p $(SPIRV_DIR)

clean:
	rm -f $(TARGET) $(BIN_OBJECTS) $(SPIRV_FILES)
	rm -rf $(SPIRV_DIR)

$(SPIRV_DIR)/%.spv: $(GLSL_DIR)/% | $(SPIRV_DIR)
	glslc $< -o $@

shaders: $(SPIRV_FILES)
