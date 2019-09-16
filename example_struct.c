/* example_struct.c
**
** Copyright (C) 2019 Meadhbh S. Hamrick
** Released under a BSD License; see license.txt for details.
** 
** This file implements a simple parser which uses the text lexxer to fill in
** values in a sample c data structure.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "textlex.h"

typedef struct {
  unsigned char secret[20];
  unsigned char salt[8];
  unsigned int iterations;
} tTestStruct;

unsigned char * input_fixtures [] = {
  "@m{\"iterations\"=12 \"salt\"=\"01234567\" \"secret\"=(8a 4d 21 92 03 82 3c f8 6a 05 c5 ea 0c 40 be f4 c8 02 76 33)}",
  "@m\n{\n  \"iterations\"=12\n  \"salt\"=\"01234567\"\n  \"secret\"=(8a 4d 21 92 03 82 3c f8 6a 05 c5 ea 0c 40 be f4 c8 02 76 33)\n}",
  "@m\n{\n  \"iterations\"=12\n  \"salt\"=(37 36 35 34 33 32 31 30)\n  \"secret\"=(8a 4d 21 92 03 82 3c f8 6a 05 c5 ea 0c 40 be f4 c8 02 76 33)\n}",
  (unsigned char *) NULL
};

tTestStruct output_fixtures [] = {
  { {0x8a, 0x4d, 0x21, 0x92, 0x03, 0x82, 0x3c, 0xf8, 0x6a, 0x05, 0xc5, 0xea, 0x0c, 0x40, 0xbe, 0xf4, 0xc8, 0x02, 0x76, 0x33}, "01234567", 12},
  { {0x8a, 0x4d, 0x21, 0x92, 0x03, 0x82, 0x3c, 0xf8, 0x6a, 0x05, 0xc5, 0xea, 0x0c, 0x40, 0xbe, 0xf4, 0xc8, 0x02, 0x76, 0x33}, "01234567", 12},
  { {0x8a, 0x4d, 0x21, 0x92, 0x03, 0x82, 0x3c, 0xf8, 0x6a, 0x05, 0xc5, 0xea, 0x0c, 0x40, 0xbe, 0xf4, 0xc8, 0x02, 0x76, 0x33}, "76543210", 12}
};

void hexdump( unsigned char * data, unsigned int length ) {
  unsigned int index;
  unsigned char buffer[16];
  
  for( index = 0; index < length; index++ ) {
    if( 0 == (index % 16) ) {
      printf( "%04X: ", index );
    }

    printf( "%02X ", ( (unsigned int) data[ index ] ) & 0xFF );
    if( ( data[ index ] >= 32 ) && ( data[ index ] < 127 ) ) {
      buffer[ index % 16 ] = data[ index ];
    } else {
      buffer[ index % 16 ] = '.';
    }
    
    if( 15 == (index % 16) ) {
      printf( "[%16.16s]\n", buffer );
    }
  }

  if( 0 != (index % 16) ) {
    for( ; 0 != ( index % 16 ); index++ ) {
      printf( "   " );
      buffer[ index % 16 ] = ' ';
    }
    printf( "[%16.16s]\n", buffer );
  }    
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

unsigned int hex_to_int( char c ) {
  static char *hex = "00112233445566778899AaBbCcDdEeFf";
  unsigned int index = 0;

  for( ; index < 32; index++ ) {
    if( c == hex[ index ] ) {
      break;
    }
  }

  if( 32 == index ) {
    index = 0;
  } else {
    index = index / 2;
  }

  return( index );
}

void hex_to_bytes( void * destination, unsigned char * source, unsigned int count ) {
  unsigned int index = 0;
  unsigned int half_index = 0;
  unsigned int accumulator = 0;
  unsigned char * bar = (unsigned char *) destination;

  for( ; index < count; index++ ) {
    if( 0 == index % 2 ) {
      accumulator = hex_to_int( source[ index ] ) * 16;
      half_index = index / 2;
    } else {
      accumulator += hex_to_int( source[ index ] );
      bar[ half_index ] = accumulator;
    }
  }
}

unsigned int parse_this( unsigned char *input, tTestStruct *output ) {
  tTextLexContext context;
  unsigned char * buffer = NULL;
  unsigned int buffer_size = 80;
  unsigned int error = 0;

#define PARSE_S_START    0
#define PARSE_S_INMAP    1
#define PARSE_S_EEQUALS  2
#define PARSE_S_EVALUE   3
#define PARSE_S_END      4

  unsigned int state = PARSE_S_START;
  unsigned char key[ 20 ];
  
  tTextLexErr overflow_handler( tTextLexContext *ncontext ) {
    unsigned int error = TEXTLEX_E_NOERR;
    unsigned int new_size = ncontext->size * 2;
    unsigned char * new_buffer = realloc( buffer, new_size );
    if( NULL == new_buffer ) {
      error = TEXTLEX_E_ERROR;
    } else {
      ncontext->buffer = new_buffer;
      ncontext->size = new_size;
    }
    return( error );
  }
  
  tTextLexErr token_handler( tTextLexContext * ncontext, tTextLexCount token ) {
    switch( state ) {
    case PARSE_S_START:
      if( TEXTLEX_T_MAP_OPEN == token ) {
        state = PARSE_S_INMAP;
      }
      break;

    case PARSE_S_INMAP:
      if( TEXTLEX_T_MAP_CLOSE == token ) {
        state = PARSE_S_END;
      } else if( TEXTLEX_T_STRING == token ) {
        memcpy( key, ncontext->buffer, ncontext->index );
        key[ ncontext->index ] = '\0';
        state = PARSE_S_EEQUALS;
      }
      break;

    case PARSE_S_EEQUALS:
      if( TEXTLEX_T_EQUALS == token ) {
        state = PARSE_S_EVALUE;
      }
      break;

    case PARSE_S_EVALUE:
      switch( token ) {
      case TEXTLEX_T_STRING:
        if( ! strcmp( "iterations", key ) ) {
          output->iterations = (unsigned int) atoi( ncontext->buffer );
        } else if( ! strcmp( "salt", key ) ) {
          strncpy( output->salt, ncontext->buffer, ( 8 <= ncontext->index ) ? ncontext->index : 8 );
        } else if( ! strcmp( "secret", key ) ) {
          strncpy( output->secret, ncontext->buffer, ( 20 <= ncontext->index ) ? ncontext->index : 20);
        }
        break;
      case TEXTLEX_T_INTEGER:
        if( ! strcmp( "iterations", key ) ) {
          output->iterations = (unsigned int) atoi( ncontext->buffer );
        }
        break;
      case TEXTLEX_T_HEX:
        if( ! strcmp( "iterations", key ) ) {
          output->iterations = (unsigned int) strtol( ncontext->buffer, NULL, 16 );
        } else if( ! strcmp( "salt", key ) ) {
          hex_to_bytes( output->salt, ncontext->buffer, ncontext->index );
        } else if( ! strcmp( "secret", key ) ) {
          hex_to_bytes( output->secret, ncontext->buffer, ncontext->index );
        }
        break;
      }
      state = PARSE_S_INMAP;
      break;

    case PARSE_S_END:
      break;
    }
    return( TEXTLEX_E_NOERR );
  }

  do {
    if( NULL == ( buffer = malloc( buffer_size ) ) ) {
      error = 1;
      break;
    }

    if( TEXTLEX_E_NOERR != ( error = textlex_init( & context, buffer, buffer_size ) ) ) {
      error = ( error << 3 ) + 2;
      break;
    }

    context.token = token_handler;
    context.overflow = overflow_handler;
    
    if( TEXTLEX_E_NOERR != ( error = textlex_update( & context, input, strlen( input ) ) ) ) {
      error = ( error << 3 ) + 3;
      break;
    }

    if( TEXTLEX_E_NOERR != ( error = textlex_final( & context ) ) ) {
      error = ( error << 3 ) + 4;
      break;
    }
  } while( 0 );

  if( NULL != buffer ) {
    free( buffer );
  }
  
  return(error);
}

int main() {

  tTestStruct test_struct;
  unsigned int index = 0;
  unsigned int error = 0;
  
  unsigned char *current_input = input_fixtures[ index ];
  tTestStruct *current_output = & output_fixtures[ index ];

  while( (unsigned char *) NULL != current_input ) {

    printf( "fixture %d: %s\n", index, current_input );

    memset( &test_struct, 0, sizeof( tTestStruct ) );
    error = parse_this( current_input, & test_struct );

    if( 0 != error ) {
      printf( "Error %d on test fixture %d\n", error, index );
    } else {
      printf( "Expected\n" );
      hexdump( (unsigned char *) current_output, sizeof( tTestStruct ) );
      printf( "Actual\n" );
      hexdump( (unsigned char *) & test_struct, sizeof( tTestStruct ) );

      if( ! memcmp( (void *) & test_struct, (void *) current_output, sizeof( tTestStruct ) ) ) {
        printf( "Success!\n" );
      }else {
        printf( "Fail!\n" );
        break;
      }

      printf( "\n" );
    }
    
    index++;
    current_input = input_fixtures[ index ];
    current_output = & output_fixtures[ index ];
  }
  
  return( 0 );
}
