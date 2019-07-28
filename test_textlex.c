#include <stdio.h>
#include <string.h>
#include "textlex.h"

static void pretty_print( tTextLexBuffer * string );
static tTextLexErr parse_this( tTextLexBuffer * string );
static tTextLexErr _token( tTextLexContext * context, tTextLexCount token );
static unsigned char * token_name( unsigned int token );

tTextLexBuffer * fixtures [] = {
  "",
  " ",
  "#Comment 00",
  "\n",
  "\r\n",
  "#Comment 01\n",
  "#Comment 02\r\n",
  "@t",
  "@t\n",
  "@t\r\n",
  "@t #Comment 03",
  "@t #Comment 04\n",
  "@t #Comment 05\n\r",
  "*nil",
  "*false #COMMENT XX",
  "*true #Comment YY\n",
  "@m 90125",
  "3.14 #Hey! I'm a floating point value!",
  "$CAFEB0EF #Comment 06",
  "\"This is a \\\"string\\\"\" #Comment 07",
  "'OTyqgu7Aca5sDCBzEoR23A=='",
  "( 39 30 31 32 35 )",
  "(\n 41 42 43 44 # ABCD\n 45 46 47 48 # EFGH\n)",
  "[ 3.14 *nil ]",
  "[ 3.14 *nil ]",
  "[ 1 -1 2.3 -2.3 4.56 -4.56 78.9 -78.9 ]",
  "[ 12.34 -12.34 5.6e7 -5.6e7 89.0e1 -89.0e1 ]",
  "[ 12.34e5 -12.34e5 6.78e90 -6.78e90 ]",
  "[ 12.34e-5 -12.34e-5 12.34e-56 -12.34e-56 ]",
  "@s { \"one\"=1 }",
  "{ \"two\" = 2 }",
  "{ \"3\" = \"three\"}",
  "@t{\"3\"=\"three\"}",
  "*true@u@t$ABBA 12$CD",
  "*undefined@t$ABBA 12$CD#01234567890123456789",
  "*undefined@t$ABBA 12$CD#012345678901234567890",
  "*undefined@t$ABBA 12$CD#0123456789012345678901",
  "@m\n{\n\"one\" = \"two\"}",
  "@thisisaverylongannotation @thisistwentycharsabc",
  "*thisisaverylongliteral*thisistwentycharsabc",
  "#this comment should be longer than 20 chars\n#thisistwentycharsabc",
  "01234567890123456789 # Numbers can be as long as you need",
  "-12.0123012301230123 # Floats can be pretty long too",
  "-3.1345324532453E568 # Hey look, a big exponent!",
  "$0123456789ABCDEF0123 $0123456789ABCDEF012345",
  "'OTyqgu7Aca5sDCBzEoR23A==' 'gu7Aca5sDCBzEoR23A=='",
  "\"ABCDEFGHIJKLMNOPQRSTUVWXYZ\" \"ABCDEFGHIJKLMNOPQRS\\\"\" \"ABCDEFGHIJKLMNOPQRST\\\"\"",
  "(00112233445566778899) ( AABBCCDDEEFF00112233 #COMMENTCOMMENTCOMETT\n 12 12) \"test\"",
  (unsigned char *) NULL
};

int main( int argc, char * argv [] ) {
  tTextLexErr err = TEXTLEX_E_NOERR;
  unsigned int i = 0;
  tTextLexBuffer * ptr;
  
  printf( "; BEGIN TESTS\n" );

  printf( "; SIZE OF CONTEXT: %zu\n", sizeof( tTextLexContext ) );
  
  for( i = 0, ptr = fixtures[ i ]; (tTextLexBuffer *) NULL != ptr; i++, ptr = fixtures[ i ] ) {
    printf( "; TEST %3d (%03zu) ", i, strlen( ptr ) );
    pretty_print( ptr );
    printf( "\n" );

    if( TEXTLEX_E_NOERR != ( err = parse_this( ptr ) ) ) {
      break;
    }
  }
  
  printf( "; END TESTS\n" );
  
  return( ( TEXTLEX_E_NOERR == err ) ? 0 : 2 );
}

static void pretty_print( unsigned char * string ) {
  unsigned int i, il;
  unsigned char x;
  
  il = strlen( string );

  for( i = 0 ; i < il ; i++ ) {
    x = string[ i ];
    printf( "%c", ( ( x >= 32 ) && ( x <= 127 ) ) ? x : '~' );
  }
}

#ifndef _BUFFER_SIZE
#define _BUFFER_SIZE 80
#endif
static tTextLexErr parse_this( unsigned char * string ) {
  tTextLexErr err;
  tTextLexBuffer buffer[ _BUFFER_SIZE ];
  tTextLexContext context;
  
  do {
    if( TEXTLEX_E_NOERR != ( err = textlex_init( & context, buffer, _BUFFER_SIZE ) ) ) {
      break;
    }

    context.token = _token;
    
    if( TEXTLEX_E_NOERR != ( err = textlex_update( & context, string, strlen( string ) ) ) ) {
      break;
    }

    if( TEXTLEX_E_NOERR != ( err = textlex_final( & context ) ) ) {
      break;
    }
  } while( 0 );

  if( TEXTLEX_E_NOERR != err ) {
    printf( ";  ERROR %d LINE %d OCTET %d\n", err, context.line, context.octet );
  }
  
  return( err );
}

static tTextLexErr _token( tTextLexContext * context, tTextLexCount token ) {
  printf( ";  TOKEN %2d %15.15s (%2d) %1s %*.*s\n", token, token_name( token ),
          context->index, ( ( context->index == context->size ) ? "O" : " " ),
          context->index, context->index, context->buffer );
  return( TEXTLEX_E_NOERR );
}

static unsigned char * token_name( unsigned int token ) {
  static unsigned char * tokens [] = {
    "END",
    "COMMENT",
    "ANNOTATION",
    "LITERAL",
    "INTEGER",
    "FLOAT",
    "HEX",
    "STRING",
    "BASE64",
    "ARRAY_OPEN",
    "ARRAY_CLOSE",
    "MAP_OPEN",
    "MAP_CLOSE",
    "EQUALS",
    "--UNDEFINED--"
  };

  return( ( token < TEXTLEX_C_TOKENS ) ? tokens[ token ] : tokens[ TEXTLEX_C_TOKENS ] );
}
