/* textlex.h
**
** Copyright (C) 2017, 2019 Meadhbh S. Hamrick
** Released under a BSD License; see license.txt for details.
**
** This file provides the interface to the DSD Text Lexxer implemented in
** textlex.c.
*/

/* Macro Definitions */

#ifndef _H_TEXTLEX
#define _H_TEXTLEX

/* Macro Definitions : Generic Error Codes */

#define TEXTLEX_E_NOERR           0 /* No Error */
#define TEXTLEX_E_ERROR           1 /* Generic Internal Error */
#define TEXTLEX_E_MEMORY          2 /* For use by the overflow callback */

/* Macro Definitions : Error Codes Associated with Parsing States */

#define TEXTLEX_E_START          64
#define TEXTLEX_E_EOLLF          65
#define TEXTLEX_E_ANNOTATE       67
#define TEXTLEX_E_LITERAL        68
#define TEXTLEX_E_NUMBER         69
#define TEXTLEX_E_FLOAT_START    70
#define TEXTLEX_E_FLOAT          71
#define TEXTLEX_E_EXPONENT_START 72
#define TEXTLEX_E_EXPONENT_NEG   73
#define TEXTLEX_E_EXPONENT       74
#define TEXTLEX_E_HEX            75
#define TEXTLEX_E_STRING_ESCAPE  77
#define TEXTLEX_E_BASE16_EOLLF   81

/* Macro Definitions : Parser States */

#define TEXTLEX_C_STATES         18
#define TEXTLEX_S_START           0
#define TEXTLEX_S_EOLLF           1
#define TEXTLEX_S_COMMENT         2
#define TEXTLEX_S_ANNOTATE        3
#define TEXTLEX_S_LITERAL         4
#define TEXTLEX_S_NUMBER          5
#define TEXTLEX_S_FLOAT_START     6
#define TEXTLEX_S_FLOAT           7
#define TEXTLEX_S_EXPONENT_START  8
#define TEXTLEX_S_EXPONENT_NEG    9
#define TEXTLEX_S_EXPONENT       10
#define TEXTLEX_S_HEX            11
#define TEXTLEX_S_STRING         12
#define TEXTLEX_S_STRING_ESCAPE  13
#define TEXTLEX_S_BASE64         14
#define TEXTLEX_S_BASE16_START   15
#define TEXTLEX_S_BASE16_COMMENT 16
#define TEXTLEX_S_BASE16_EOLLF   17

/* Macro Definitions : Token Types */

#define TEXTLEX_C_TOKENS         14
#define TEXTLEX_T_END             0
#define TEXTLEX_T_COMMENT         1
#define TEXTLEX_T_ANNOTATION      2
#define TEXTLEX_T_LITERAL         3
#define TEXTLEX_T_INTEGER         4
#define TEXTLEX_T_FLOAT           5
#define TEXTLEX_T_HEX             6
#define TEXTLEX_T_STRING          7
#define TEXTLEX_T_BASE64          8
#define TEXTLEX_T_ARRAY_OPEN      9
#define TEXTLEX_T_ARRAY_CLOSE    10
#define TEXTLEX_T_MAP_OPEN       11
#define TEXTLEX_T_MAP_CLOSE      12
#define TEXTLEX_T_EQUALS         13

/* These are the defaults for the types used in the API. If you want to change
** them, create a header file named "tltypes.h" that defines your preferred
** type definitions and compile textlex.c (and your application) with the
** -DTEXTLEX_TYPE_OVERRIDE command line option.
*/

#ifndef TEXTLEX_TYPE_OVERRIDE
  typedef unsigned int    tTextLexErr;
  typedef unsigned int    tTextLexState;
  typedef unsigned int    tTextLexCount;
  typedef unsigned char   tTextLexBuffer;
#else
#include "tltypes.h"
#endif

/* Structs, Typedefs, Unions & Enums */

/* This is the lexxer's context structure. It gets initialized with a call to
** textlex_init() and updated with every call to textlex_update() and
** textlex_final(). You should probably treat this as an opaque data structure
** unless you're digging in and modifying the lexxer.
*/

typedef struct _text_lex_context {
  tTextLexState    state;
  tTextLexCount    size;
  tTextLexBuffer * buffer;
  tTextLexCount    index;
  tTextLexCount    line;
  tTextLexCount    octet;
  tTextLexCount    bytes_read;
  tTextLexErr    (*token)( struct _text_lex_context * context, tTextLexCount token );
  tTextLexErr    (*overflow)( struct _text_lex_context * context );
} tTextLexContext;

/* Function Prototypes */

/* textlex_init()
**
** Pass this function a pointer to a textlex context and it will initialize it
** for you. It does not allocate any memory, so you'll have to allocate space
** for the context and the buffer before calling this function.
**
** After calling this function, you will want to set the token callback (and
** probably also the overflow callback.)
*/

tTextLexErr textlex_init( tTextLexContext * context, tTextLexBuffer * buffer, tTextLexCount size );

/* textlex_update()
**
** Pass a context, a pointer to DSD text to parse and the length of the text
** and it will begin parsing, calling the token callback when a complete token
** is found.
**
** You can call this function once, passing it a pointer to a complete DSD
** "document," or you can break up the document into several pieces, calling
** this function on each piece.
**
** If the document parsed is well formed, this function *should* return a
** TEXTLEX_E_NOERR error code. If the document contains a syntax error, it
** will return an error code > 64, associated with the state the lexxer was
** in when it encountered the error.
*/

tTextLexErr textlex_update( tTextLexContext * context, tTextLexBuffer * data, tTextLexCount length );

/* textlex_final()
**
** There are some corner cases where the lexxer can't figure out when an
** input document stops. Call this function exactly once after processing all
** input to tell the lexxer you're really stopping.
*/

tTextLexErr textlex_final( tTextLexContext * context );

/* textlex_default_overflow()
**
** This is the default overflow handler that's setup with the call to
** textlex_init(). It's made public here in case you change the overflow
** handler and then want to change it back to the default.
**
** But you probably shouldn't do that.
*/

tTextLexErr textlex_default_overflow( tTextLexContext * context );

#endif /* _H_TEXTLEX */
