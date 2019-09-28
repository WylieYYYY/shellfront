#!/usr/bin/make -f
# compilation configurations
SHELL:=/bin/bash
CC=gcc -Wall
BUILD_PATH=make
BIN_BASE=/usr/local/bin
HEADER_BASE=/usr/local/include
LIB_BASE=/usr/local/lib

########################
#Official No Man's Land#
########################

# auto generated properties
NAME=$(notdir $(CURDIR))
BIN_PATH=$(BIN_BASE)/$(NAME)
HEADER_PATH=$(HEADER_BASE)/$(NAME).h
LIB_PATH=$(LIB_BASE)/lib$(NAME).so
OBJECTS=$(patsubst src/%.c,$(BUILD_PATH)/%.o,$(wildcard src/*.c))

# binary build
$(BIN_PATH): all $(LIB_PATH) main.c
	@$(call msg,Linking binary source,6)
	sudo $(CC) -Wl,-rpath=$(LIB_BASE) -o "$@" "main.c" -l$(NAME)
	@$(call msg,==> Build finished successfully,2)
all:
	@$(call msg,Checking for namespace collision,6)
	@$(call collide_check,$(BIN_PATH),Binary)
	@$(call collide_check,$(HEADER_PATH),Header)
	@$(call collide_check,$(LIB_PATH),Library)
	@$(call msg,Checking for shared library,6)
	@#@$(call msg,==> None required,2)
	@$(call lib_check,gtk-3)
	@$(call lib_check,vte-2.91)
	@$(call msg,Creating make directory,6)
	mkdir -p "$(BUILD_PATH)"

# dynamic library build
$(HEADER_PATH):
	@$(call msg,Copying header,6)
	sudo cp "src/"*".h" "$(HEADER_BASE)"
$(LIB_PATH): $(HEADER_PATH) $(OBJECTS)
	@$(call msg,Compiling shared library,6)
	sudo $(CC) -shared -fPIC -Wl,-soname,"lib$(NAME).so" -o "$(LIB_PATH)" $(filter-out $<,$^) $$(pkg-config --cflags --libs gtk+-3.0 vte-2.91)
	@$(call msg,Updating linker,6)
	sudo ldconfig
$(BUILD_PATH)/%.o: src/%.c
	@$(call msg,Compiling $<,6)
	$(CC) -fPIC -o "$@" -c "$<" $$(pkg-config --cflags --libs gtk+-3.0 vte-2.91)

# utility
define msg
tput setaf $(2); tput bold; echo "$(1)"; tput sgr0
endef
define collide_check
if [ -e "$(1)" ]; then $(call msg,==> $(2) collision at $(1),1); exit 1; else $(call msg,==> Checked $(1),2); fi
endef
define lib_check
if [ -z "$$(ldconfig -p | grep $(1))" ]; then $(call msg,==> $(1) is not installed,1); exit 1; else $(call msg,==> Checked $(1),2); fi
endef

# transaction hooks
install: $(BIN_PATH)
clean:
	@$(call msg,Cleaning built objects,6)
	rm -r "$(BUILD_PATH)/"* 2>/dev/null;:
remove:
	@$(call msg,Removing all associated files,6)
	sudo rm "$(BIN_PATH)" 2>/dev/null;:
	sudo rm -r "$(HEADER_PATH)" 2>/dev/null;:
	sudo rm -r "$(LIB_PATH)" 2>/dev/null;:
	@$(call msg,Updating linker,6)
	sudo ldconfig
