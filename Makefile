BUILD = linux
# BUILD = { linux, win64 }
# BUILDINDEX = { 0: linux, 1: win64 }
RELEASE = 0

WIN64 = x86_64-w64-mingw32

ifeq ($(BUILD),linux)
	CC = gcc
else
	CC = $(WIN64)-gcc
endif

ifeq ($(BUILD),linux)
	CXXFLAGS = -std=c17 -w -DBUILDINDEX=0
else
	CXXFLAGS = -std=c17 -w -DBUILDINDEX=1 -mwindows -I$(WIN64)/include -L$(WIN64)/lib
endif

ifeq ($(BUILD),linux)
	LDFLAGS = -lncursesw
else
	LDFLAGS = -lpdcurses
endif

PROJECTNAME = 7drl-2022
VERSION = 1
APPNAMESELF = $(PROJECTNAME)_v$(VERSION)

ifeq ($(RELEASE),0)
	APPNAME = debug/app
else
	ifeq ($(BUILD),linux)
		APPNAME = release/linux/$(APPNAMESELF)_linux
	else
		APPNAME = release/win64/$(APPNAMESELF)_win64
	endif
endif

EXT = .c
SRCDIR = src
OBJDIR = obj
RESOURCES = resources.res
# windres resources.rc -O coff -o resources.res


SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
DEP = $(OBJ:$(OBJDIR)/%.o=%.d)

RM = rm
DELOBJ = $(OBJ)

DEL = del
EXE = .exe
WDELOBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)\\%.o)

all: $(APPNAME)

$(APPNAME): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(RESOURCES) $(LDFLAGS)

# Creates the dependecy rules
%.d: $(SRCDIR)/%$(EXT)
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:%.d=$(OBJDIR)/%.o) >$@

# Includes all .h files
-include $(DEP)

# Building rule for .o files and its .c/.cpp in combination with all .h
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT)
	$(CC) $(CXXFLAGS) -o $@ -c $<

################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	$(RM) $(DELOBJ) $(DEP) debug/app debug/app.exe

# Cleans only all files with the extension .d
.PHONY: cleandep
cleandep:
	$(RM) $(DEP)