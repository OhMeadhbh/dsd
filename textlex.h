/* textlex.h
**
** Copyright (C) 2017 Meadhbh S. Hamrick
** Released under a BSD License; see license.txt for details.
**
** This file provides the interface to the DSD Text Lexxer implemented in
** textlex.c.
*/

/* Macro Definitions */

#ifndef _H_TEXTLEX
#define _H_TEXTLEX

/* Macro Definitions : Error Codes */

#define TEXTLEX_C_ERRORS         16
#define TEXTLEX_E_NOERR           0
#define TEXTLEX_E_ERROR           1
#define TEXTLEX_E_EXIT            2
#define TEXTLEX_E_START           3
#define TEXTLEX_E_EOLLF           4
#define TEXTLEX_E_ANNOTATE        5
#define TEXTLEX_E_LITERAL         6
#define TEXTLEX_E_NUMBER          7
#define TEXTLEX_E_FLOAT_START     8
#define TEXTLEX_E_FLOAT           9
#define TEXTLEX_E_EXPONENT_START 10
#define TEXTLEX_E_EXPONENT_NEG   11
#define TEXTLEX_E_EXPONENT       12
#define TEXTLEX_E_HEX            13
#define TEXTLEX_E_STRING_ESCAPE  14
#define TEXTLEX_E_BASE16_EOLLF   15

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

#define TEXTLEX_C_TOKENS         13
#define TEXTLEX_T_COMMENT         0
#define TEXTLEX_T_ANNOTATION      1
#define TEXTLEX_T_LITERAL         2
#define TEXTLEX_T_INTEGER         3
#define TEXTLEX_T_FLOAT           4
#define TEXTLEX_T_HEX             5
#define TEXTLEX_T_STRING          6
#define TEXTLEX_T_BASE64          7
#define TEXTLEX_T_ARRAY_OPEN      8
#define TEXTLEX_T_ARRAY_CLOSE     9
#define TEXTLEX_T_MAP_OPEN       10
#define TEXTLEX_T_MAP_CLOSE      11
#define TEXTLEX_T_EQUALS         12

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

tTextLexErr textlex_init( tTextLexContext * context, tTextLexBuffer * buffer, tTextLexCount size );
tTextLexErr textlex_update( tTextLexContext * context, tTextLexBuffer * data, tTextLexCount length );
tTextLexErr textlex_final( tTextLexContext * context );
tTextLexErr textlex_default_overflow( tTextLexContext * context );

#endif /* _H_TEXTLEX */
