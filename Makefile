# Makefile
#
# The files in this package are licensed using a BSD style open source license.
# Please see license.txt for details.

EXES=test_textlex test_textlex_small test_textlex_buffer example_simple example_struct
OBJS=test_textlex.o textlex.o textlex_small.o test_textlex_small.o \
     test_textlex_buffer.o example_simple.o example_struct.o

all : $(EXES)

clean :
	$(RM) $(EXES) $(OBJS)

test_textlex : test_textlex.o textlex.o

test_textlex_small : test_textlex_small.o textlex_small.o

test_textlex_buffer : test_textlex_buffer.o textlex.o

example_simple : textlex.o example_simple.o

example_struct : textlex.o example_struct.o

test_textlex.o : test_textlex.c textlex.h

test_textlex_small.o : test_textlex.c textlex.h
	$(CC) $(CFLAGS) -c -DTEXTLEX_TYPE_OVERRIDE -o test_textlex_small.o \
    test_textlex.c

test_textlex_buffer.o : test_textlex.c textlex.h
	$(CC) $(CFLAGS) -c -D_BUFFER_SIZE=20 -o test_textlex_buffer.o \
    test_textlex.c

textlex.o : textlex.c textlex.h

textlex_small.o : textlex.c textlex.h
	$(CC) $(CFLAGS) -c -DTEXTLEX_TYPE_OVERRIDE -o $@ $<

example_simple.o : textlex.c example_simple.c textlex.h

example_struct.o : textlex.c example_struct.c textlex.h
