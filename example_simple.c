/* example_simple.c
**
** Copyright (C) 2019 Meadhbh S. Hamrick
** Released under a BSD License; see license.txt for details.
** 
** This program reads standard input, passing it to the textlex lexxer,
** printing information about the data and the parsing process.
** 
** It demonstrates how to call textlex_init(), textlex_update() and
** textlex_final() as well as how to set the overflow and token callbacks.
** 
*/

/* Macro Definitions */

#define BUFFER_SIZE 80

/* File Includes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "textlex.h"

/* Function Prototypes */
static tTextLexErr token_handler( tTextLexContext * context, tTextLexCount token );
static tTextLexErr overflow_handler( tTextLexContext * context );
static unsigned char * token_name( unsigned int token );
static void hexdump( unsigned char * data, unsigned int length );

/* Global Variables */

static unsigned int    lexxer_buffer_size = BUFFER_SIZE;
static unsigned char * lexxer_buffer      = NULL;

int main( ) {
  tTextLexErr error = TEXTLEX_E_NOERR;
  tTextLexContext context;
  unsigned char input_buffer[ BUFFER_SIZE ];
  size_t bytes_read;
  
  do {
    /* First, let's allocate the lexxer buffer. As the lexxer steps through the
    ** input, it copies the "significant" portions of the input into a buffer.
    ** When it determines it has a lexical unit, it calls the token callback
    ** which pulls data from the lexxer buffer.
    */
    
    if( NULL == ( lexxer_buffer = malloc( lexxer_buffer_size ) ) ) {
      fprintf( stderr, "%%SIMPLE-F-MEMORY; Failure allocating lexxer buffer.\n" );
      error = TEXTLEX_E_MEMORY;
      break;
    }

    /* Now call textlex_init(). This zeroizes memory in the context, sets
    ** the lexxer_buffer's location and the default overflow handler.
    */

    if( TEXTLEX_E_NOERR != ( error = textlex_init( & context, lexxer_buffer, lexxer_buffer_size ) ) ) {
      fprintf( stderr, "%%SIMPLE-F-INIT; Error %d calling textlex_init().\n", error );
      break;
    }

    /* Set the overflow and token callbacks. You *MUST* set the token callback
    ** or textlex_update() will eventually try to branch to location zero (and
    ** dump core. hopefully.)
    */

    context.overflow = overflow_handler;
    context.token    = token_handler;

    do {
      bytes_read = fread( input_buffer, 1, BUFFER_SIZE, stdin );
      if( 0 != bytes_read ) {
        if( TEXTLEX_E_NOERR != ( error = textlex_update( & context, input_buffer, bytes_read ) ) ) {
          fprintf( stderr, "%%SIMPLE-F-UPDATE; Failure calling textlex_update()\n" );
          fprintf( stderr, "; line = %d, octet = %d\n", context.line, context.octet );
          break;
        }
      }
    } while( !feof( stdin ) );

    if( TEXTLEX_E_NOERR != ( error = textlex_final( & context ) ) ) {
      fprintf( stderr, "%%SIMPLE-F-FINAL; Failure calling textlex_final().\n" );
      break;
    }
    
  } while( 0 );

  /* If the lexxer_buffer is non-null, that means we should free it. */
  if( NULL != lexxer_buffer ) {
    free( lexxer_buffer );
  }
  
  return( error & 255 );
}

static tTextLexErr token_handler( tTextLexContext * context, tTextLexCount token ) {
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

static tTextLexErr overflow_handler( tTextLexContext * context ) {
  unsigned int error = TEXTLEX_E_NOERR;
  unsigned int new_size = context->size * 2;
  unsigned char * new_buffer = realloc( lexxer_buffer, new_size );
  if( NULL == new_buffer ) {
    error = TEXTLEX_E_ERROR;
  } else {
    context->buffer = new_buffer;
    context->size = new_size;
  }
  return( error );
}

static void hexdump( unsigned char * data, unsigned int length ) {
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
