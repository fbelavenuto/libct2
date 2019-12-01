# Makefile

CC = gcc
LD = gcc
AR = ar
CP = cp
RM = rm -f
MD = mkdir

SDIR = src
ODIR = obj
IDIR = inc

CFLAGS = -g -Wall -I$(IDIR)
LDFLAGS = -shared

LIBOBJ = wav.o ct2.o
OBJS = $(addprefix $(ODIR)/, $(LIBOBJ))

TARGET_LIB = libct2

all: $(ODIR) $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(LD) $(LDFLAGS) -o $@.so $^
	$(AR) r $@.a $^

$(ODIR):
	$(MD) $(ODIR)

.PHONY: clean install

clean:
	$(RM) *.exe $(TARGET_LIB).* $(ODIR)/*

install:


$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
