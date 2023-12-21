CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lcurl

SRCDIR = src
SRC = $(SRCDIR)/main.c

OBJDIR = obj
OBJ = $(OBJDIR)/main.o

BINDIR = bin
TARGET = $(BINDIR)/haru

# Emojis
SUCCESS_EMOJI = ✅
ERROR_EMOJI = ❌
INFO_EMOJI = 🌸
DEBUG_EMOJI = 🐛

# Colors
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
NC = \033[0m

.PHONY: all clean

all: $(TARGET)
	@echo "$(INFO_EMOJI) • $(GREEN)Build complete!$(NC)"

$(TARGET): $(OBJ)
	@mkdir -p $(BINDIR)
	@echo "$(INFO_EMOJI) • $(YELLOW)Linking objects...$(NC)"
	@$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "$(SUCCESS_EMOJI) • $(GREEN)Executable created: $(TARGET)$(NC)"

$(OBJ): $(SRC)
	@mkdir -p $(OBJDIR)
	@echo "$(INFO_EMOJI) • $(YELLOW)Compiling source file: $<$(NC)"
	@$(CC) $(CFLAGS) -c -o $@ $<
	@echo "$(SUCCESS_EMOJI) • $(GREEN)Object file created: $@$(NC)"

run: $(TARGET)
	@echo "$(INFO_EMOJI) • $(YELLOW)Running $(TARGET)...$(NC)"
	@./$(TARGET)

clean:
	@echo "$(INFO_EMOJI) • $(YELLOW)Cleaning up...$(NC)"
	@rm -rf $(OBJDIR) $(BINDIR)
	@echo "$(SUCCESS_EMOJI) • $(GREEN)Cleanup complete!$(NC)"

debug:
	@echo "$(DEBUG_EMOJI) • $(YELLOW)Debug mode enabled:$(NC)"
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "LIBS = $(LIBS)"
	@echo "SRC = $(SRC)"
	@echo "OBJ = $(OBJ)"
	@echo "BINDIR = $(BINDIR)"
	@echo "TARGET = $(TARGET)"
	@echo "CONFIG = $(CONFIG)"

