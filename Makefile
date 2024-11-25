# Compiler
CC = icpx
CFLAGS += -march=native -O0

# Output Directory
OUT_DIR = ./out

# Targets
INNER_PRODUCT_BIN = $(OUT_DIR)/inner-product

# Source Files
INNER_PRODUCT_SRC = inner-product.cpp

# Default Target
all: $(INNER_PRODUCT_BIN)

# Build Binary
$(INNER_PRODUCT_BIN): $(INNER_PRODUCT_SRC)
	@mkdir -p $(OUT_DIR) # Ensure output directory exists
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Phony Targets
.PHONY: clean
clean:
	rm -rf $(OUT_DIR)
