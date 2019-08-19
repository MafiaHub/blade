# ------------------------------------------------------ #
# Makefile for the "Blade engine Client"                 #
#                                                        #
# Just type "make" to compile the                        #
#  - SDL Client (blade)                                  #
#  - Server (bded)                                       #
#  - Runblader Game (baseb)                              #
#                                                        #
# Base dependencies:                                     #
#  - SDL 2.0 			                                 #
#  - libGL                                               #
#                                                        #
# Optional dependencies:                                 #
#  - OpenAL                                              #
#                                                        #
# Platforms:                                             #
#  - FreeBSD                                             #
#  - Linux                                               #
#  - OpenBSD                                             #
#  - OS X                                                #
#  - Windows (MinGW)                                     #
# ------------------------------------------------------ #

# User configurable options
# -------------------------

# Enables HTTP support through cURL. Used for
# HTTP download.
WITH_CURL:=yes

# Enables the optional OpenAL sound system.
# To use it your system needs libopenal.so.1
# or openal32.dll (we recommend openal-soft)
# installed
WITH_OPENAL:=yes

# Enable systemwide installation of game assets
WITH_SYSTEMWIDE:=no

# This will set the default SYSTEMDIR, a non-empty string
# would actually be used. On Windows normals slashes (/)
# instead of backslashed (\) should be used! The string
# MUST NOT be surrounded by quotation marks!
WITH_SYSTEMDIR:=""

# This will set the architectures of the OSX-binaries.
# You have to make sure your libs/frameworks supports
# these architectures! To build an universal ppc-compatible
# one would add -arch ppc for example.
OSX_ARCH:=-arch $(shell uname -m | sed -e s/i.86/i386/)

# This will set the build options to create an MacOS .app-bundle.
# The app-bundle itself will not be created, but the runtime paths
# will be set to expect the game-data in *.app/
# Contents/Resources
OSX_APP:=yes

# This is an optional configuration file, it'll be used in
# case of presence.
CONFIG_FILE := config.mk

# ----------

# In case a of a configuration file being present, we'll just use it
ifeq ($(wildcard $(CONFIG_FILE)), $(CONFIG_FILE))
include $(CONFIG_FILE)
endif

# Detect the OS
ifdef SystemRoot
BDE_OSTYPE := Windows
else
BDE_OSTYPE ?= $(shell uname -s)
endif

# Special case for MinGW
ifneq (,$(findstring MINGW,$(BDE_OSTYPE)))
BDE_OSTYPE := Windows
endif

# On Windows / MinGW $(CC) is undefined by default.
ifeq ($(BDE_OSTYPE),Windows)
CC := gcc
endif


# Detect the architecture
ifeq ($(BDE_OSTYPE), Windows)
ifdef PROCESSOR_ARCHITEW6432
# 64 bit Windows
BDE_ARCH ?= $(PROCESSOR_ARCHITEW6432)
else
# 32 bit Windows
BDE_ARCH ?= $(PROCESSOR_ARCHITECTURE)
endif
else
# Normalize some abiguous BDE_ARCH strings
BDE_ARCH ?= $(shell uname -m | sed -e 's/i.86/i386/' -e 's/amd64/x86_64/' -e 's/^arm.*/arm/')
endif

# Detect the compiler
ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
COMPILER := clang
COMPILERVER := $(shell $(CC)  -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
else ifeq ($(shell $(CC) -v 2>&1 | grep -c -E "(gcc version|gcc-Version)"), 1)
COMPILER := gcc
COMPILERVER := $(shell $(CC)  -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
else
COMPILER := unknown
endif

# Used to detect libraries. Override to foobar-linux-gnu-pkg-config when
# cross-compiling.
PKG_CONFIG ?= pkg-config

# ----------

# Base CFLAGS.
#
# -O2 are enough optimizations.
#
# -fno-strict-aliasing since the source doesn't comply
#  with strict aliasing rules and it's next to impossible
#  to get it there...
#
# -fomit-frame-pointer since the framepointer is mostly
#  useless for debugging Quake II and slows things down.
#
# -g to build always with debug symbols. Please DO NOT
#  CHANGE THIS, since it's our only chance to debug this
#  crap when random crashes happen!
#
# -MMD to generate header dependencies. (They cannot be
#  generated if building universal binaries on OSX)
#
# -fwrapv for defined integer wrapping. MSVC6 did this
#  and the game code requires it.
ifeq ($(BDE_OSTYPE), Darwin)
CFLAGS := -O2 -fno-strict-aliasing -fomit-frame-pointer \
		  -Wall -pipe -g -fwrapv
CFLAGS += $(OSX_ARCH)
else
CFLAGS := -std=gnu99 -O2 -fno-strict-aliasing \
		  -Wall -pipe -g -ggdb -MMD -fwrapv
endif

# ----------

# Switch of some annoying warnings.
ifeq ($(COMPILER), clang)
	# -Wno-missing-braces because otherwise clang complains
	#  about totally valid 'vec3_t bla = {0}' constructs.
	CFLAGS += -Wno-missing-braces
else ifeq ($(COMPILER), gcc)
	# GCC 8.0 or higher.
	ifeq ($(shell test $(COMPILERVER) -ge 80000; echo $$?),0)
	    # -Wno-format-truncation and -Wno-format-overflow
		# because GCC spams about 50 false positives.
    	CFLAGS += -Wno-format-truncation -Wno-format-overflow
	endif
endif

# ----------

# Defines the operating system and architecture
CFLAGS += -DBDEOSTYPE=\"$(BDE_OSTYPE)\" -DBDEARCH=\"$(BDE_ARCH)\"

# ----------

# https://reproducible-builds.org/specs/source-date-epoch/
ifdef SOURCE_DATE_EPOCH
CFLAGS += -DBUILD_DATE=\"$(shell date --utc --date="@${SOURCE_DATE_EPOCH}" +"%b %_d %Y" | sed -e 's/ /\\ /g')\"
endif

# ----------

# If we're building with gcc for i386 let's define -ffloat-store.
# This helps the old and crappy x87 FPU to produce correct values.
# Would be nice if Clang had something comparable.
ifeq ($(BDE_ARCH), i386)
ifeq ($(COMPILER), gcc)
CFLAGS += -ffloat-store
endif
endif

# Force SSE math on x86_64. All sane compilers should do this
# anyway, just to protect us from broken Linux distros.
ifeq ($(BDE_ARCH), x86_64)
CFLAGS += -mfpmath=sse
endif

# ----------

# Systemwide installation
ifeq ($(WITH_SYSTEMWIDE),yes)
CFLAGS += -DSYSTEMWIDE
ifneq ($(WITH_SYSTEMDIR),"")
CFLAGS += -DSYSTEMDIR=\"$(WITH_SYSTEMDIR)\"
endif
endif

# ----------

# Just set IOAPI_NO_64 on everything that's not Linux or Windows,
# otherwise minizip will use fopen64(), fseek64() and friends that
# may be unavailable. This is - of course - not really correct, in
# a better world we would set -DIOAPI_NO_64 to force everything to
# fopen(), fseek() and so on and -D_FILE_OFFSET_BITS=64 to let the
# libc headers do their work. Currently we can't do that because
# Quake II uses nearly everywere int instead of off_t...
#
# This may have the side effect that ZIP files larger than 2GB are
# unsupported. But I doubt that anyone has such large files, they
# would likely hit other internal limits.
ifneq ($(BDE_OSTYPE),Windows)
ifneq ($(BDE_OSTYPE),Linux)
ZIPCFLAGS += -DIOAPI_NO_64
endif
endif

# ----------

# Extra CFLAGS for SDL
SDLCFLAGS := $(shell sdl2-config --cflags)

# ----------

# Base include path.
ifeq ($(BDE_OSTYPE),Linux)
INCLUDE := -I/usr/include
else ifeq ($(BDE_OSTYPE),FreeBSD)
INCLUDE := -I/usr/local/include
else ifeq ($(BDE_OSTYPE),OpenBSD)
INCLUDE := -I/usr/local/include
else ifeq ($(BDE_OSTYPE),Windows)
INCLUDE := -I/usr/include
endif

# ----------

# Extra includes for GLAD
GLAD_INCLUDE = -Isrc/client/refresh/gl3/glad/include

# ----------

# Base LDFLAGS.
ifeq ($(BDE_OSTYPE),Linux)
LDFLAGS := -L/usr/lib -lm -ldl -rdynamic
else ifeq ($(BDE_OSTYPE),FreeBSD)
LDFLAGS := -L/usr/local/lib -lm
else ifeq ($(BDE_OSTYPE),OpenBSD)
LDFLAGS := -L/usr/local/lib -lm
else ifeq ($(BDE_OSTYPE),Windows)
LDFLAGS := -L/usr/lib -lws2_32 -lwinmm -static-libgcc
else ifeq ($(BDE_OSTYPE), Darwin)
LDFLAGS := $(OSX_ARCH) -lm
endif

# Keep symbols hidden.
CFLAGS += -fvisibility=hidden
LDFLAGS += -fvisibility=hidden

ifneq ($(BDE_OSTYPE), Darwin)
# for some reason the OSX & OpenBSD linker doesn't support this...
LDFLAGS += -Wl,--no-undefined
endif

# ----------

# We don't support encrypted ZIP files.
ZIPCFLAGS += -DNOUNCRYPT

# ----------

# Extra LDFLAGS for SDL
ifeq ($(BDE_OSTYPE), Darwin)
SDLLDFLAGS := -lSDL2
else # not Darwin
SDLLDFLAGS := $(shell sdl2-config --libs)
endif # Darwin

# The renderer libs don't need libSDL2main, libmingw32 or -mwindows.
ifeq ($(BDE_OSTYPE), Windows)
DLL_SDLLDFLAGS = $(subst -mwindows,,$(subst -lmingw32,,$(subst -lSDL2main,,$(SDLLDFLAGS))))
endif

# ----------

# When make is invoked by "make VERBOSE=1" print
# the compiler and linker commands.

ifdef VERBOSE
Q :=
else
Q := @
endif

# ----------

# Phony targets
.PHONY : all client game icon server ref_gl3

# ----------

# Builds everything
all: config client server game ref_gl3

# ----------

# Print config values
config:
	@echo "Build configuration"
	@echo "============================"
	@echo "WITH_OPENAL = $(WITH_OPENAL)"
	@echo "WITH_CURL = $(WITH_CURL)"
	@echo "WITH_SYSTEMWIDE = $(WITH_SYSTEMWIDE)"
	@echo "WITH_SYSTEMDIR = $(WITH_SYSTEMDIR)"
	@echo "============================"
	@echo ""

# ----------

# Special target to compile
# the icon on Windows
ifeq ($(BDE_OSTYPE), Windows)
icon:
	@echo "===> WR build/icon/icon.res"
	${Q}mkdir -p build/icon
	${Q}windres src/backends/windows/icon.rc -O COFF -o build/icon/icon.res
endif

release/blade.exe : LDFLAGS += -mwindows

# ----------

# Cleanup
clean:
	@echo "===> CLEAN"
	${Q}rm -Rf build release/*

cleanall:
	@echo "===> CLEAN"
	${Q}rm -Rf build release

# ----------

# The client
ifeq ($(BDE_OSTYPE), Windows)
client:
	@echo "===> Building blade.exe"
	${Q}mkdir -p release
	$(MAKE) release/blade.exe
	@echo "===> Building quake2.exe Wrapper"
	$(MAKE) release/quake2.exe

build/client/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<

ifeq ($(WITH_CURL),yes)
release/blade.exe : CFLAGS += -DUSE_CURL
endif

ifeq ($(WITH_OPENAL),yes)
release/blade.exe : CFLAGS += -DUSE_OPENAL -DDEFAULT_OPENAL_DRIVER='"openal32.dll"'
endif

ifeq ($(WITH_SDL2),yes)
release/blade.exe : CFLAGS += -DSDL2
endif

else # not Windows

client:
	@echo "===> Building quake2"
	${Q}mkdir -p release
	$(MAKE) release/quake2

ifeq ($(BDE_OSTYPE), Darwin)
build/client/%.o : %.m
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) $(OSX_ARCH) -x objective-c -c $< -o $@
else
build/client/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<
endif

release/quake2 : CFLAGS += -Wno-unused-result

ifeq ($(WITH_CDA),yes)
release/quake2 : CFLAGS += -DCDA
endif

ifeq ($(WITH_OPENAL),yes)
ifeq ($(BDE_OSTYPE), OpenBSD)
release/quake2 : CFLAGS += -DUSE_OPENAL -DDEFAULT_OPENAL_DRIVER='"libopenal.so"'
else ifeq ($(BDE_OSTYPE), Darwin)
release/quake2 : CFLAGS += -DUSE_OPENAL -DDEFAULT_OPENAL_DRIVER='"libopenal.dylib"' -I/usr/local/opt/openal-soft/include -DDLOPEN_OPENAL
release/quake2 : LDFLAGS += -L/usr/local/opt/openal-soft/lib -rpath /usr/local/opt/openal-soft/lib
else
release/quake2 : CFLAGS += -DUSE_OPENAL -DDEFAULT_OPENAL_DRIVER='"libopenal.so.1"'
endif
endif # WITH_OPENAL

ifeq ($(WITH_CURL),yes)
release/quake2 : CFLAGS += -DUSE_CURL
endif

ifneq ($(BDE_OSTYPE), Darwin)
release/ref_gl1.so : LDFLAGS += -lGL
endif

ifeq ($(BDE_OSTYPE), FreeBSD)
release/quake2 : LDFLAGS += -Wl,-z,origin,-rpath='$$ORIGIN/lib'
else ifeq ($(BDE_OSTYPE), Linux)
release/quake2 : LDFLAGS += -Wl,-z,origin,-rpath='$$ORIGIN/lib'
endif

ifeq ($(WITH_SYSTEMWIDE),yes)
ifneq ($(WITH_SYSTEMDIR),"")
ifeq ($(BDE_OSTYPE), FreeBSD)
release/quake2 : LDFLAGS += -Wl,-z,origin,-rpath='$(WITH_SYSTEMDIR)/lib'
else ifeq ($(BDE_OSTYPE), Linux)
release/quake2 : LDFLAGS += -Wl,-z,origin,-rpath='$(WITH_SYSTEMDIR)/lib'
endif
else
ifeq ($(BDE_OSTYPE), FreeBSD)
release/quake2 : LDFLAGS += -Wl,-z,origin,-rpath='/usr/share/games/quake2/lib'
else ifeq ($(BDE_OSTYPE), Linux)
release/quake2 : LDFLAGS += -Wl,-z,origin,-rpath='/usr/share/games/quake2/lib'
endif
endif
endif
endif

# ----------

# The server
ifeq ($(BDE_OSTYPE), Windows)
server:
	@echo "===> Building bded"
	${Q}mkdir -p release
	$(MAKE) release/bded.exe

build/server/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<

release/bded.exe : CFLAGS += -DDEDICATED_ONLY

else # not Windows
server:
	@echo "===> Building bded"
	${Q}mkdir -p release
	$(MAKE) release/bded

build/server/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/bded : CFLAGS += -DDEDICATED_ONLY -Wno-unused-result

ifeq ($(BDE_OSTYPE), FreeBSD)
release/bded : LDFLAGS += -lexecinfo
endif
endif

# ----------

# The OpenGL 3.x renderer lib

ifeq ($(BDE_OSTYPE), Windows)

ref_gl3:
	@echo "===> Building ref_gl3.dll"
	$(MAKE) release/ref_gl3.dll

release/ref_gl3.dll : LDFLAGS += -shared

else ifeq ($(BDE_OSTYPE), Darwin)

ref_gl3:
	@echo "===> Building ref_gl3.dylib"
	$(MAKE) release/ref_gl3.dylib


release/ref_gl3.dylib : LDFLAGS += -shared

else # not Windows or Darwin

ref_gl3:
	@echo "===> Building ref_gl3.so"
	$(MAKE) release/ref_gl3.so


release/ref_gl3.so : CFLAGS += -fPIC
release/ref_gl3.so : LDFLAGS += -shared

endif # OS specific ref_gl3

build/ref_gl3/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(INCLUDE) $(GLAD_INCLUDE) -o $@ $<

# ----------

# The baseb game
ifeq ($(BDE_OSTYPE), Windows)
game:
	@echo "===> Building baseb/game.dll"
	${Q}mkdir -p release/baseb
	$(MAKE) release/baseb/game.dll

build/baseb/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/baseb/game.dll : LDFLAGS += -shared

else ifeq ($(BDE_OSTYPE), Darwin)
game:
	@echo "===> Building baseb/game.dylib"
	${Q}mkdir -p release/baseb
	$(MAKE) release/baseb/game.dylib

build/baseb/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/baseb/game.dylib : CFLAGS += -fPIC
release/baseb/game.dylib : LDFLAGS += -shared
else # not Windows or Darwin
game:
	@echo "===> Building baseb/game.so"
	${Q}mkdir -p release/baseb
	$(MAKE) release/baseb/game.so

build/baseb/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/baseb/game.so : CFLAGS += -fPIC -Wno-unused-result
release/baseb/game.so : LDFLAGS += -shared
endif

# ----------

# Used by the game
GAME_OBJS_ = \
	src/common/shared/flash.o \
	src/common/shared/rand.o \
	src/common/shared/shared.o \
	src/game/g_nav.o \
	src/game/g_ai.o \
	src/game/g_chase.o \
	src/game/g_cmds.o \
	src/game/g_combat.o \
	src/game/g_func.o \
	src/game/g_items.o \
	src/game/g_main.o \
	src/game/g_misc.o \
	src/game/g_monster.o \
	src/game/g_phys.o \
	src/game/g_spawn.o \
	src/game/g_svcmds.o \
	src/game/g_target.o \
	src/game/g_trigger.o \
	src/game/g_utils.o \
	src/game/g_weapon.o \
	src/game/monster/actor/bartender.o \
	src/game/monster/misc/move.o \
	src/game/g_quest.o \
	src/game/player/client.o \
	src/game/player/hud.o \
	src/game/player/trail.o \
	src/game/player/view.o \
	src/game/player/weapon.o \
	src/game/player/light.o \
	src/game/savegame/savegame.o

# ----------

# Used by the client
CLIENT_OBJS_ := \
	src/backends/generic/misc.o \
	src/client/cl_cin.o \
	src/client/cl_console.o \
	src/client/cl_download.o \
	src/client/cl_effects.o \
	src/client/cl_entities.o \
	src/client/cl_input.o \
	src/client/cl_inventory.o \
	src/client/cl_quest.o \
	src/client/cl_keyboard.o \
	src/client/cl_lights.o \
	src/client/cl_main.o \
	src/client/cl_network.o \
	src/client/cl_parse.o \
	src/client/cl_particles.o \
	src/client/cl_prediction.o \
	src/client/cl_screen.o \
	src/client/cl_tempentities.o \
	src/client/cl_view.o \
	src/client/input/sdl.o \
	src/client/menu/menu.o \
	src/client/menu/qmenu.o \
	src/client/menu/videomenu.o \
	src/client/curl/qcurl.o \
	src/client/curl/download.o \
	src/client/sound/sdl.o \
	src/client/sound/ogg.o \
	src/client/sound/openal.o \
	src/client/sound/sound.o \
	src/client/sound/wave.o \
	src/client/sound/qal.o \
	src/client/vid/glimp_sdl.o \
	src/client/vid/vid.o \
	src/common/argproc.o \
	src/common/clientserver.o \
	src/common/collision.o \
	src/common/crc.o \
	src/common/cmdparser.o \
	src/common/cvar.o \
	src/common/filesystem.o \
	src/common/glob.o \
	src/common/md4.o \
	src/common/movemsg.o \
	src/common/frame.o \
	src/common/netchan.o \
	src/common/pmove.o \
	src/common/szone.o \
	src/common/zone.o \
	src/common/shared/flash.o \
	src/common/shared/rand.o \
	src/common/shared/shared.o \
	src/common/shared/zpl.o \
	src/common/unzip/ioapi.o \
	src/common/unzip/unzip.o \
	src/common/unzip/miniz.o \
	src/server/sv_cmd.o \
	src/server/sv_conless.o \
	src/server/sv_entities.o \
	src/server/sv_game.o \
	src/server/sv_init.o \
	src/server/sv_main.o \
	src/server/sv_save.o \
	src/server/sv_send.o \
	src/server/sv_user.o \
	src/server/sv_world.o

ifeq ($(BDE_OSTYPE), Windows)
CLIENT_OBJS_ += \
	src/backends/windows/main.o \
	src/backends/windows/network.o \
	src/backends/windows/system.o \
	src/backends/windows/shared/hunk.o
else
CLIENT_OBJS_ += \
	src/backends/unix/main.o \
	src/backends/unix/network.o \
	src/backends/unix/signalhandler.o \
	src/backends/unix/system.o \
	src/backends/unix/shared/hunk.o
endif

# ----------

REFGL3_OBJS_ := \
	src/client/refresh/gl3/gl3_draw.o \
	src/client/refresh/gl3/gl3_image.o \
	src/client/refresh/gl3/gl3_light.o \
	src/client/refresh/gl3/gl3_lightmap.o \
	src/client/refresh/gl3/gl3_main.o \
	src/client/refresh/gl3/gl3_mesh.o \
	src/client/refresh/gl3/gl3_misc.o \
	src/client/refresh/gl3/gl3_model.o \
	src/client/refresh/gl3/gl3_sdl.o \
	src/client/refresh/gl3/gl3_surf.o \
	src/client/refresh/gl3/gl3_warp.o \
	src/client/refresh/gl3/gl3_shaders.o \
	src/client/refresh/gl3/gl3_md2.o \
	src/client/refresh/gl3/gl3_sp2.o \
	src/client/refresh/gl3/glad/src/glad.o \
	src/client/refresh/files/pcx.o \
	src/client/refresh/files/stb.o \
	src/client/refresh/files/pvs.o \
	src/client/refresh/files/wal.o \
	src/common/shared/shared.o \
	src/common/md4.o

ifeq ($(BDE_OSTYPE), Windows)
REFGL3_OBJS_ += \
	src/backends/windows/shared/hunk.o
else # not Windows
REFGL3_OBJS_ += \
	src/backends/unix/shared/hunk.o
endif

# ----------

# Used by the server
SERVER_OBJS_ := \
	src/backends/generic/misc.o \
	src/common/argproc.o \
	src/common/clientserver.o \
	src/common/collision.o \
	src/common/crc.o \
	src/common/cmdparser.o \
	src/common/cvar.o \
	src/common/filesystem.o \
	src/common/glob.o \
	src/common/md4.o \
	src/common/frame.o \
	src/common/movemsg.o \
	src/common/netchan.o \
	src/common/pmove.o \
	src/common/szone.o \
	src/common/zone.o \
	src/common/shared/rand.o \
	src/common/shared/shared.o \
	src/common/unzip/ioapi.o \
	src/common/unzip/unzip.o \
	src/common/unzip/miniz.o \
	src/server/sv_cmd.o \
	src/server/sv_conless.o \
	src/server/sv_entities.o \
	src/server/sv_game.o \
	src/server/sv_init.o \
	src/server/sv_main.o \
	src/server/sv_save.o \
	src/server/sv_send.o \
	src/server/sv_user.o \
	src/server/sv_world.o

ifeq ($(BDE_OSTYPE), Windows)
SERVER_OBJS_ += \
	src/backends/windows/main.o \
	src/backends/windows/network.o \
	src/backends/windows/system.o \
	src/backends/windows/shared/hunk.o
else # not Windows
SERVER_OBJS_ += \
	src/backends/unix/main.o \
	src/backends/unix/network.o \
	src/backends/unix/signalhandler.o \
	src/backends/unix/system.o \
	src/backends/unix/shared/hunk.o
endif

# ----------

# Rewrite pathes to our object directory
CLIENT_OBJS = $(patsubst %,build/client/%,$(CLIENT_OBJS_))
REFGL1_OBJS = $(patsubst %,build/ref_gl1/%,$(REFGL1_OBJS_))
REFGL3_OBJS = $(patsubst %,build/ref_gl3/%,$(REFGL3_OBJS_))
REFSOFT_OBJS = $(patsubst %,build/ref_soft/%,$(REFSOFT_OBJS_))
SERVER_OBJS = $(patsubst %,build/server/%,$(SERVER_OBJS_))
GAME_OBJS = $(patsubst %,build/baseb/%,$(GAME_OBJS_))

# ----------

# Generate header dependencies
CLIENT_DEPS= $(CLIENT_OBJS:.o=.d)
REFGL1_DEPS= $(REFGL1_OBJS:.o=.d)
REFGL3_DEPS= $(REFGL3_OBJS:.o=.d)
REFSOFT_DEPS= $(REFSOFT_OBJS:.o=.d)
SERVER_DEPS= $(SERVER_OBJS:.o=.d)
GAME_DEPS= $(GAME_OBJS:.o=.d)

# ----------

# Suck header dependencies in
-include $(CLIENT_DEPS)
-include $(REFGL1_DEPS)
-include $(REFGL3_DEPS)
-include $(SERVER_DEPS)
-include $(GAME_DEPS)

# ----------

# release/quake2
ifeq ($(BDE_OSTYPE), Windows)
release/blade.exe : $(CLIENT_OBJS) icon
	@echo "===> LD $@"
	${Q}$(CC) build/icon/icon.res $(CLIENT_OBJS) $(LDFLAGS) $(SDLLDFLAGS) -o $@
	# $(Q)strip $@
# the wrappper, quick'n'dirty
release/quake2.exe : src/win-wrapper/wrapper.c icon
	$(Q)$(CC) -Wall -mwindows build/icon/icon.res src/win-wrapper/wrapper.c -o $@
	# $(Q)strip $@
else
release/quake2 : $(CLIENT_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(CLIENT_OBJS) $(LDFLAGS) $(SDLLDFLAGS) $(X11LDFLAGS) -o $@
endif

# release/bded
ifeq ($(BDE_OSTYPE), Windows)
release/bded.exe : $(SERVER_OBJS) icon
	@echo "===> LD $@.exe"
	${Q}$(CC) build/icon/icon.res $(SERVER_OBJS) $(LDFLAGS) $(SDLLDFLAGS) -o $@
	# $(Q)strip $@
else
release/bded : $(SERVER_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(SERVER_OBJS) $(LDFLAGS) -o $@
endif

# release/ref_gl3.so
ifeq ($(BDE_OSTYPE), Windows)
release/ref_gl3.dll : $(REFGL3_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(REFGL3_OBJS) $(LDFLAGS) $(DLL_SDLLDFLAGS) -o $@
	# $(Q)strip $@
else ifeq ($(BDE_OSTYPE), Darwin)
release/ref_gl3.dylib : $(REFGL3_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(REFGL3_OBJS) $(LDFLAGS) $(SDLLDFLAGS) -o $@
else
release/ref_gl3.so : $(REFGL3_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(REFGL3_OBJS) $(LDFLAGS) $(SDLLDFLAGS) -o $@
endif

# release/baseb/game.so
ifeq ($(BDE_OSTYPE), Windows)
release/baseb/game.dll : $(GAME_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(GAME_OBJS) $(LDFLAGS) -o $@
	# $(Q)strip $@
else ifeq ($(BDE_OSTYPE), Darwin)
release/baseb/game.dylib : $(GAME_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(GAME_OBJS) $(LDFLAGS) -o $@
else
release/baseb/game.so : $(GAME_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(GAME_OBJS) $(LDFLAGS) -o $@
endif

# ----------
