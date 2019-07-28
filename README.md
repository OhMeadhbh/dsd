# Dynamic Structured Data (DSD)

Dynamic Structured Data (DSD) is an meta-transfer syntax intended to
facilitate the transfer of information between loosely coupled
systems. DSD defines three concrete transfer syntaxes (Text, Binary
and XML), an Abstract Type System and three concrete type systems. The
DSD Abstract Type System defines type semantics for five atomic types
and two collection types.

DSD is intended to decouple type semantics from transfer syntax. It
is, by itself type-system neutral (DSD does not define how many bits
encode an integer or how floating point values are laid out in
memory.) However, DSD can carry an annotation that the creator of a
message assumes type details useful for 8-bit sensors, 32-bit
micro-controllers and 64-bit server systems.

This package provides a Lexxer and example parser for the Text and
Binary transfer syntaxes. (Users of the XML transfer syntax can simply
use their favourite XML parser.)

DSD is defined independently of this package, but a copy of it's
official definition is included in the file
[dsd-info.txt](dsd-info.txt). For a more literate introduction to the
problem(s) DSD was trying to solve, see [Goldilocks and the 11
Parsers](goldilocks.txt). DSD is based on the Virtual Worlds Region
Agent Protocol Abstract (VWRAP) Type System defined in
https://tools.ietf.org/html/draft-ietf-vwrap-type-system-00. For the
truly masochistic, details of the VWRAP effort are available at
http://meadhbh.hamrick.rocks/home/technical-sh-t/what-is-vwrap.

## DSD In 120 Seconds

DSD defines a syntax for protocol messages between systems. In this
sense it's sort of like JSON or XML (though there is a Binary transfer
syntax that's a little more efficient to parse.) Here's an example of a
typical DSD message:

    # Login Authentication w/ No Iteration Count or Salt
    @t
    {
      "username" = "OhMeadhbh"
      "secret" = (
        b6:07:54:c4:ea:1a:fa:24:20:1e
        af:02:4c:13:0d:94:cc:58:01:65
      )
      "algorithm" = "sha1"
      "version" = 2
    }

From this example you can probably deduce a couple things:

* The Hash mark starts a comment

* Text strings are delimited by double quotes (")

* Maps are bounded by the '{' and '}' characters

* Map Keys and Map Values are linked with the equals character (=)

* You don't need to use commas (because DSD/Text is "self-delimiting")

* Annotations begin with the at symbol (@)

If you walk away from this document remembering "DSD Text is sort of
like JSON with comments and no commas" and "The OhMeadhbh/DSD package
provides a lexxer for the DSD Text transfer syntax" then you've
learned enough for practical use of DSD as a syntax and this
software.

Since you're still reading this section, it probably means you're
interested in more details. Here's an interesting one: the message
above can be encoded using DSD/Binary into these 78 hexadecimal
octets:

    02 68 42 47 75 73 65 72 6e 61 6d 65 48 00 4f 68
    4d 65 61 64 68 62 68 45 73 65 63 72 65 74 48 0B
    b6 07 54 c4 ea 1a fa 24 20 1e af 02 4c 13 0d 94
    cc 58 01 65 48 00 61 6c 67 6f 72 69 74 68 6d 43
    73 68 61 31 46 76 65 72 73 69 6f 6e 10 02

It's interesting to note that the Binary encoding seldom makes
messages much shorter, but it often makes them easier to parse.

If you're in an environment which uses XML extensively, you can use
the XML transfer syntax:

    <!-- Login Authentication w/ No Iteration Count or Salt
    <type>tiny</type>
    <map>
      <key>username</key>
      <string>OhMeadhbh</string>

      <key>secret</key>
      <base64>MDEyMzQ1Njc4OTAxMjM0NTY3ODk=</base64>

      <key>algorithm</key>
      <string>sha1</string>

      <key>version</key>
      <integer>2</integer>
    </map>

If you want to see a beefier example of a DSD file (that includes
comments, see [example.dsd](example.dsd).

If you're wondering why the world needed another format that looks
like JSON, the answer is obviously "all the other ones are wrong."
Really what happened is at Linden Lab there were significant
disagreements over whether protocol should be carried over ProtoBufs,
JSON, XML or a custom Binary format. The one abstract type system /
multiple concrete transfer syntaxes separation was created so our
distributed application could be transfer-syntax agnostic. It doesn't
matter which transfer syntax you use (Text, Binary or XML), they can
all be de-serialized into byte identical data structures.

The DSD Text format itself is slightly easier to parse on 8-bit
micro-controllers used in related projects and it natively supports
comments.

## DSD/Text Lexxer Theory of Operation

The DSD/Text Lexxer is implemented in a single C99 source file:
[textlex.c](textlex.c). I tried to make the code easy to follow, but to
use the lexxer all you should have to look at is the header file:
[textlex.h](textlex.h). To compile the file, all you should need to do
is this: 

    cc -o textlex.o textlex.c

You don't need to specify an include directory with the -I command line
option (unless you move the header file.) The textlex.c file isn't
dependent on external libraries, so you shouldn't need to reference any
additional libraries with -l.

This file only includes the code for the textlex_init, textlex_update,
textlex_final and textlex_default_overflow functions. It does not
compile to an executable program.

To compile an example program, use the Make utility to make the
[test_textlex.c](test_textlex.c) program. (See the [Makefile](Makefile)
for details.)

An instance of the lexxer is initialized with the textlex_init()
function call. This call initializes the state of the lexxer instance
(which is stored in a tTextLexContext data structure.) You'll also need
a fixed length buffer to hold parsed values. The textlex_init() function
returns a success code of tTextLexErr type. If everything is initialized
correctly, it will return a TEXTLEX_E_NOERR.

Here is an example of initializing a lexxer instance:

    #define BUFFER_SIZE 80

    static tTextLexContext lexxer;
    static unsigned char buffer[ BUFFER_SIZE ];
    tTextLexErr error = TEXTLEX_E_NOERR;
    
    error = textlex_init( &lexxer, buffer, BUFFER_SIZE );

    if( TEXTLEX_E_NOERR != error ) {
        printf( "Got Error %d\n", error );
    }

Now you're ready to start processing some input. The lexxer scans input,
copying characters into the buffer (potentially unescaping string
values.) When it reads a complete lexeme, it calls the token callback
which is referenced in the context data structure.

You need to provide your own token callback. And you should set it
*AFTER* calling textlex_init() but before calling textlex_update() for
the first time.

    tTextLexErr _token_callback( tTextLexContext * context, tTextLexCount token ) {
        printf( "TOKEN %2d %*.*s", token, context->index, context->index, context->buffer );
        return( TEXTLEX_E_NOERR );
    }
    
    lexxer.token = _token_callback;

The callback defined here does nothing other than print out a number
associated with the type of item found and its value.

Now let's provide some input:

    unsigned char *input = "@t { \"username\" = \"foo\" \"password\" = \"bar\" }";

    error = textlex_update( &lexxer, input, strlen( input ) );

    if( TEXTLEX_E_NOERR != error ) {
        printf( "Error %d while calling textlex_update()\n", error );
    }

In theory, this should cause the _token_callback() function to be called
14 times with the following token types and values:

    TEXTLEX_T_ANNOTATION  t
    TEXTLEX_T_END
    TEXTLEX_T_MAP_OPEN
    TEXTLEX_T_STRING      username
    TEXTLEX_T_END
    TEXTLEX_T_EQUALS
    TEXTLEX_T_STRING      foo
    TEXTLEX_T_END
    TEXTLEX_T_STRING      password
    TEXTLEX_T_END
    TEXTLEX_T_EQUALS
    TEXTLEX_T_STRING      bar
    TEXTLEX_T_END
    TEXTLEX_T_MAP_CLOSE

From this you should probably figure out that this is not a
full-featured parser. Your code will need to maintain enough state to
figure out if the next item you encounter is intended to be a map key or
a map value. You should also be able to figure out what to do if a map
key isn't followed by an equals.

But there are some corner cases where the textlex_update() call won't
be able to tell whether or not you're done with a token. So before you
finish, call textlex_final().

    error = textlex_final( &lexxer );

If the buffer overflows, the lexxer will call the overflow callback. The
default callback calls the token callback with a buffer's full of data
and then continues lexxing. So if you have a buffer that's 16 bytes long
and the lexxer is trying to parse a string that's 32 bytes long, the
token will be called twice. First with the first half of the string and
then with the second half of the string.

This could be bad if we didn't call the token callback with the
TEXTLEX_T_END token specified. It tells the callback that a variable
length data unit is complete. It's used for strings, comments, numbers,
literals, hex numbers and annotations. You won't get an TEXTLEX_T_END
call for equals, open and close maps and open and close arrays. These
tokens are fixed length and a length of 1 is implied.

Another way to avoid worrying about the buffer overflow is to dynamically
grow the buffer size:

    unsigned int buffer_size = 16;
    unsigned char * buffer = malloc( buffer_size );

And then when you get an overflow call, you use realloc() to allocate a
buffer of twice the size:

    tTextLexErr _overflow_callback( tTextLexContext * context ) {
        buffer_size *= 2;
        buffer = realloc( buffer, buffer_size );
        context->size = buffer_size;
    }

But... it's up to you. Depending on the specific type semantics, you
might simply want to reject large data units.
