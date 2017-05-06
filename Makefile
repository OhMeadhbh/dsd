# Makefile
#
# The files in this package are licensed using a BSD style open source license.
# Please see license.txt for details.

EXES=test_textlex test_textlex_small
OBJS=test_textlex.o textlex.o textlex_small.o test_textlex_small.o

all : $(EXES)

clean :
	$(RM) $(EXES) $(OBJS)

test_textlex : test_textlex.o textlex.o

test_textlex_small : test_textlex_small.o textlex_small.o

test_textlex.o : test_textlex.c textlex.h

test_textlex_small.o : test_textlex.c textlex.h
	$(CC) $(CFLAGS) -c -DTEXTLEX_TYPE_OVERRIDE -o test_textlex_small.o \
    test_textlex.c

textlex.o : textlex.c textlex.h

textlex_small.o : textlex.c textlex.h
	$(CC) $(CFLAGS) -c -DTEXTLEX_TYPE_OVERRIDE -o $@ $<
