#
# Makefile
#

PREFIX ?= /usr/local

SRC = src
BIN = bin
BUILD = build

LEX_FILE := $(SRC)/z80.l
LEX_SOURCE := $(BUILD)/z80.yy.c
LFLAGS ?=

YACC_FILE := $(SRC)/z80.y
YACC_SOURCE := $(BUILD)/z80.tab.c
YACC_HEADER := $(BUILD)/z80.tab.h
YFLAGS += -d

MV = mv

SOURCES := $(addprefix $(SRC)/, main.c tixasm.c opcode.c expr.c \
								symbol_table.c reloc_table.c \
								vector.c hash_table.c) \
		   $(LEX_SOURCE) $(YACC_SOURCE)
OBJECTS := $(patsubst $(SRC)/%,$(BUILD)/%,$(patsubst %.c,%.o,$(SOURCES)))
DEPS := $(OBJECTS:%.o=%.d)

TARGET := $(BIN)/tixasm

CFLAGS += -g -I$(BUILD) -I$(SRC)
LDFLAGS +=

all: $(TARGET)

debug: $(TARGET)

clean:
	rm -rf $(BUILD) $(BIN)

install:
	install -m 755 $(TARGET) $(PREFIX)/bin

$(BUILD):
	@mkdir -p $@

$(BIN):
	@mkdir -p $@

$(TARGET): $(OBJECTS) | $(BIN)
	$(CC) $(LDFLAGS) -o $@ $^

-include $(DEPS)

$(LEX_SOURCE): $(LEX_FILE) | $(BUILD) $(YACC_HEADER)
	$(LEX) $(LFLAGS) $<
	@$(MV) lex.yy.c $@

$(YACC_SOURCE) $(YACC_HEADER): $(YACC_FILE) | $(BUILD)
	$(YACC) $(YFLAGS) $<
	@$(MV) y.tab.c $(YACC_SOURCE)
	@$(MV) y.tab.h $(YACC_HEADER)

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD) $(YACC_HEADER)
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

.PHONY: all debug clean install
