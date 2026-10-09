CC      ?= gcc
AR      ?= ar
RM      ?= rm -f

CFLAGS  ?= -O2 -Wall -Wextra -I.
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS  ?= -lcrypto

PREFIX      ?= /usr/local
LIBNAME     := base58
STATIC_LIB   := lib$(LIBNAME).a
SHARED_LIB   := lib$(LIBNAME).so
EXE          := base58

LIB_OBJS := base58lib.o ttyhelper.o vector64.o vector.o
EXE_OBJS := main.o cmd.o

.PHONY: all clean static shared exe lib exe-static exe-dynamic install

all: exe

lib: static

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

exe-static: $(EXE)

exe-dynamic: $(EXE)

exe: $(EXE)

$(STATIC_LIB): $(LIB_OBJS)
	$(AR) rcs $@ $^

$(SHARED_LIB): CFLAGS += -fPIC
$(SHARED_LIB): $(LIB_OBJS)
	$(CC) -shared -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(EXE): $(EXE_OBJS) $(STATIC_LIB)
	$(CC) -o $@ $(EXE_OBJS) $(STATIC_LIB) $(LDFLAGS) $(LDLIBS)

# If you want a fully static executable:
# make exe-static LDFLAGS=-static
# This links everything statically, including OpenSSL if available as static libs.

%.o: %.c base58.h cmd.h ttyhelper.h vector.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(LIB_OBJS) $(EXE_OBJS) $(STATIC_LIB) $(SHARED_LIB) $(EXE)
