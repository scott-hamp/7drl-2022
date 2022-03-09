BUILD = linux
# BUILD = { linux, win64 }
# BUILDINDEX = { 0: linux, 1: win64 }
CURSES = pdcurses
# CURSES = { ncurses, ncursesw, pdcurses }
RELEASE = 0

WIN64 = x86_64-w64-mingw32

ifeq ($(CURSES),ncursesw)
	CURSESINDEX = 0
else
	CURSESINDEX = 1
endif

ifeq ($(BUILD),linux)
	CC = gcc
else
	CC = $(WIN64)-gcc
endif

ifeq ($(BUILD),linux)
	CXXFLAGS = -std=c17 -w -DBUILDINDEX=0 -DRELEASE=${RELEASE}
else
	INCLUDELINK = -I$(WIN64)/$(CURSES)/include -L$(WIN64)/$(CURSES)/lib
	ifeq ($(CURSES),ncursesw)
		CXXFLAGS = -std=c17 -w -DBUILDINDEX=1 -DCURSESINDEX=0 -DRELEASE=${RELEASE} ${INCLUDELINK}
	else
		CXXFLAGS = -std=c17 -w -mwindows -DBUILDINDEX=1 -DCURSESINDEX=1 -DRELEASE=${RELEASE} ${INCLUDELINK}
	endif
endif

ifeq ($(BUILD),linux)
	LDFLAGS = -lncursesw -lm
else
	LDFLAGS = -l$(CURSES) -lm
endif

PROJECTNAME = from-the-depths
VERSION = 7drl
APPNAMESELF = $(PROJECTNAME)_v$(VERSION)

ifeq ($(RELEASE),0)
	APPNAME = debug/app
else
	ifeq ($(BUILD),linux)
		APPNAME = release/linux/$(APPNAMESELF)_linux
	else
		APPNAME = release/win64/$(APPNAMESELF)_win64-${CURSES}
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