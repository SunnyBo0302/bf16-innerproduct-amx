CC = g++
CFLAGS = -O0 -Wall -g
#CFLAGS = -O2 -fassociative-math -fno-signed-zeros -fno-trapping-math
#CFLAGS += -ftree-vectorize -fopt-info-vec
CFLAGS += -march=native

#LDFLAGS = -lm -fopenmp
#LDFLAGS = -lm

# Output Directory and Target
OUT_DIR = ./out
TARGET = $(OUT_DIR)/simdtest

# Source File
SRC = simdtest.cpp

# Default Target
all: $(TARGET)

# Build Target
$(TARGET): $(SRC)
	@mkdir -p $(OUT_DIR) # Ensure output directory exists
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Phony Targets
.PHONY: clean
clean:
	rm -rf $(OUT_DIR)