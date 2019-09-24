#!/usr/bin/make -f
# compilation configurations
SHELL:=/bin/bash
CC=gcc -Wall
BUILD_PATH=make
BIN_BASE=/usr/local/bin

########################
#Official No Man's Land#
########################

# auto generated properties
BIN_PATH=$(BIN_BASE)/shellfront

# binary build
$(BIN_PATH): all main.c
	@$(call msg,Linking binary source,6)
	sudo $(CC) -x c -o "$@" "main.c" $$(pkg-config --cflags --libs gtk+-3.0 vte-2.91)
	@$(call msg,==> Build finished successfully,2)
all:
	@$(call msg,Checking for namespace collision,6)
	@$(call collide_check,$(BIN_PATH),Binary)
	@$(call msg,Checking for shared library,6)
	@#@$(call msg,==> None required,2)
	@$(call lib_check,gtk-3)
	@$(call lib_check,vte-2.91)

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
remove:
	@$(call msg,Removing all associated files,6)
	sudo rm "$(BIN_PATH)" 2>/dev/null;:
