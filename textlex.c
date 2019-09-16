/* textlex.c
**
** Copyright (C) 2017, 2019 Meadhbh S. Hamrick
** Released under a BSD License; see license.txt for details.
**
** This file implements a DSD Text Lexxer. Please see textlex.h or README.md
** for more info.
*/

/* Macro Definitions */

#define SET_STATE( x ) context->state = x
#define COPY_TO_BUFFER context->buffer[ context->index++ ] = current; if( context->index >= context->size ) { err = context->overflow( context ); }
#define TOKEN( x ) if( NULL != context->token ) { err = context->token( context, x ); } context->index = 0

#define WS ' ': \
  case '\t'
#define LF '\n'
#define CR '\r'

#define DIGIT '0': \
  case '1' : \
  case '2' : \
  case '3' : \
  case '4' : \
  case '5' : \
  case '6' : \
  case '7' : \
  case '8' : \
  case '9'

#define HEXALPHA 'A' : \
  case 'a' : \
  case 'B' : \
  case 'b' : \
  case 'C' : \
  case 'c' : \
  case 'd' : \
  case 'D' : \
  case 'E' : \
  case 'e' : \
  case 'F' : \
  case 'f'

#define HEXDIGIT DIGIT : \
  case HEXALPHA

#define NONHEXALPHA 'G' : \
  case 'g' : \
  case 'H' : \
  case 'h' : \
  case 'i' : \
  case 'I' : \
  case 'j' : \
  case 'J' : \
  case 'k' : \
  case 'K' : \
  case 'l' : \
  case 'L' : \
  case 'm' : \
  case 'M' : \
  case 'n' : \
  case 'N' : \
  case 'o' : \
  case 'O' : \
  case 'p' : \
  case 'P' : \
  case 'q' : \
  case 'Q' : \
  case 'r' : \
  case 'R' : \
  case 's' : \
  case 'S' : \
  case 't' : \
  case 'T' : \
  case 'u' : \
  case 'U' : \
  case 'v' : \
  case 'V' : \
  case 'w' : \
  case 'W' : \
  case 'x' : \
  case 'X' : \
  case 'y' : \
  case 'Y' : \
  case 'z' : \
  case 'Z'

#define ALPHA HEXALPHA : \
  case NONHEXALPHA

#define NONDEL( t ) case WS: \
  case LF: \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    SET_STATE( TEXTLEX_S_START ); \
    break; \
\
  case CR: \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    SET_STATE( TEXTLEX_S_EOLLF ); \
    break; \
\
  case '#': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    SET_STATE( TEXTLEX_S_COMMENT ); \
    break; \
\
  case '@': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    SET_STATE( TEXTLEX_S_ANNOTATE ); \
    break; \
\
  case '*': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    SET_STATE( TEXTLEX_S_LITERAL ); \
    break; \
\
 case '$':      \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    SET_STATE( TEXTLEX_S_HEX ); \
    break; \
\
  case '"': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    SET_STATE( TEXTLEX_S_STRING ); \
    break; \
\
  case '\'': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    SET_STATE( TEXTLEX_S_BASE64 ); \
    break; \
\
  case '(': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    SET_STATE( TEXTLEX_S_BASE16_START ); \
    break; \
\
  case '[': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    TOKEN( TEXTLEX_T_ARRAY_OPEN ); \
    SET_STATE( TEXTLEX_S_START ); \
    break; \
\
  case ']': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    TOKEN( TEXTLEX_T_ARRAY_CLOSE ); \
    SET_STATE( TEXTLEX_S_START ); \
    break; \
\
  case '{': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    TOKEN( TEXTLEX_T_MAP_OPEN ); \
    SET_STATE( TEXTLEX_S_START ); \
    break; \
\
  case '}': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    TOKEN( TEXTLEX_T_MAP_CLOSE ); \
    SET_STATE( TEXTLEX_S_START ); \
    break; \
\
  case '=': \
    TOKEN( t ); \
    TOKEN( TEXTLEX_T_END ); \
    TOKEN( TEXTLEX_T_EQUALS ); \
    SET_STATE( TEXTLEX_S_START ); \
    break;

/* File Includes */

#include "textlex.h"
#include <string.h>

/* Function Definitions */

tTextLexErr textlex_init( tTextLexContext * context, tTextLexBuffer * buffer, tTextLexCount size ) {
  memset( context, 0, sizeof( tTextLexContext ) );
  context->buffer = buffer;
  context->size = size;
  context->overflow = textlex_default_overflow;
  
  return( TEXTLEX_E_NOERR );
}

tTextLexErr textlex_update( tTextLexContext * context, tTextLexBuffer * data, tTextLexCount length ) {
  tTextLexErr err = TEXTLEX_E_NOERR;
  unsigned int i;
  unsigned char current;

  context->bytes_read = 0;
  
  for( i = 0; i < length; i++ ) {
    current = data[ i ];
    context->bytes_read += 1;
    
    if( '\n' == current ) {
      context->line++;
      context->octet = 0;
    }

    switch( context->state ) {
    case TEXTLEX_S_START:
      switch( current ) {
      case WS:
      case LF:
        SET_STATE( TEXTLEX_S_START );
        break;

      case CR:
        SET_STATE( TEXTLEX_S_EOLLF );
        break;

      case '#':
        SET_STATE( TEXTLEX_S_COMMENT );
        break;

      case '@':
        SET_STATE( TEXTLEX_S_ANNOTATE );
        break;

      case '*':
        SET_STATE( TEXTLEX_S_LITERAL );
        break;

      case DIGIT:
      case '-':
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_NUMBER );
        break;
        
      case '$':
        SET_STATE( TEXTLEX_S_HEX );
        break;

      case '"':
        SET_STATE( TEXTLEX_S_STRING );
        break;

      case '\'':
        SET_STATE( TEXTLEX_S_BASE64 );
        break;

      case '(':
        SET_STATE( TEXTLEX_S_BASE16_START );
        break;

      case '[':
        TOKEN( TEXTLEX_T_ARRAY_OPEN );
        SET_STATE( TEXTLEX_S_START );
        break;

      case ']':
        TOKEN( TEXTLEX_T_ARRAY_CLOSE );
        SET_STATE( TEXTLEX_S_START );
        break;

      case '{':
        TOKEN( TEXTLEX_T_MAP_OPEN );
        SET_STATE( TEXTLEX_S_START );
        break;

      case '}':
        TOKEN( TEXTLEX_T_MAP_CLOSE );
        SET_STATE( TEXTLEX_S_START );
        break;

      case '=':
        TOKEN( TEXTLEX_T_EQUALS );
        SET_STATE( TEXTLEX_S_START );
        break;
        
      default:
        err = TEXTLEX_E_START;
        break;
      }
      break;
      
    case TEXTLEX_S_EOLLF:
      switch( current ) {
      case LF:
        SET_STATE( TEXTLEX_S_START );
        break;
        
      default:
        err = TEXTLEX_E_START;
        break;
      }
      break;
      
    case TEXTLEX_S_COMMENT:
      switch( current ) {
      case LF:
        TOKEN( TEXTLEX_T_COMMENT );
        TOKEN( TEXTLEX_T_END );
        SET_STATE( TEXTLEX_S_START );
        break;
        
      case CR:
        TOKEN( TEXTLEX_T_COMMENT );
        TOKEN( TEXTLEX_T_END );
        SET_STATE( TEXTLEX_S_EOLLF );
        break;

      default:
        COPY_TO_BUFFER;
        break;
      }
      break;
      
    case TEXTLEX_S_ANNOTATE:
      switch( current ) {
      case ALPHA:
      case DIGIT:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_ANNOTATE );
        break;

      NONDEL( TEXTLEX_T_ANNOTATION );

      default:
        err = TEXTLEX_E_ANNOTATE;
        break;
      }
      break;
      
    case TEXTLEX_S_LITERAL:
      switch( current ) {
      case ALPHA:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_LITERAL );
        break;

      NONDEL( TEXTLEX_T_LITERAL );

      default:
        err = TEXTLEX_E_LITERAL;
        break;
      }
      break;
      
    case TEXTLEX_S_NUMBER:
      switch( current ) {
      case DIGIT:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_NUMBER );
        break;

      case '.':
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_FLOAT_START );
        break;
        
      NONDEL( TEXTLEX_T_INTEGER );

      default:
        err = TEXTLEX_E_NUMBER;
        break;
      }
      break;
      
    case TEXTLEX_S_FLOAT_START:
      switch( current ) {
      case DIGIT:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_FLOAT );
        break;
        
      default:
        err = TEXTLEX_E_FLOAT_START;
        break;
      }
      break;
      
    case TEXTLEX_S_FLOAT:
      switch( current ) {
      case DIGIT:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_FLOAT );
        break;

      case 'E':
      case 'e':
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_EXPONENT_START );
        break;
        
      NONDEL( TEXTLEX_T_FLOAT );

      default:
        err = TEXTLEX_E_FLOAT;
        break;
      }
      break;
      
    case TEXTLEX_S_EXPONENT_START:
      switch( current ) {
      case '-':
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_EXPONENT_NEG );
        break;

      case DIGIT:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_EXPONENT );
        break;
        
      default:
        err = TEXTLEX_E_EXPONENT_START;
        break;
      }
      break;
      
    case TEXTLEX_S_EXPONENT_NEG:
      switch( current ) {
      case DIGIT:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_EXPONENT );
        break;

      default:
        err = TEXTLEX_E_EXPONENT_NEG;
        break;
      }
      break;
      
    case TEXTLEX_S_EXPONENT:
      switch( current ) {
      case DIGIT:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_EXPONENT );
        break;

      NONDEL( TEXTLEX_T_FLOAT );

      default:
        err = TEXTLEX_E_EXPONENT;
        break;
      }
      break;
      
    case TEXTLEX_S_HEX:
      switch( current ) {
      case HEXDIGIT:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_HEX );
        break;

      NONDEL( TEXTLEX_T_HEX );

      default:
        err = TEXTLEX_E_HEX;
        break;
      }
      break;
      
    case TEXTLEX_S_STRING:
      switch( current ) {
      case '\\':
        SET_STATE( TEXTLEX_S_STRING_ESCAPE );
        break;

      case '"':
        TOKEN( TEXTLEX_T_STRING );
        TOKEN( TEXTLEX_T_END );
        SET_STATE( TEXTLEX_S_START );
        break;
        
      default:
        COPY_TO_BUFFER;
        break;
      }
      break;
      
    case TEXTLEX_S_STRING_ESCAPE:
      switch( current ) {
      case '\\':
      case '"':
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_STRING );
        break;
        
      default:
        err = TEXTLEX_E_STRING_ESCAPE;
        break;
      }
      break;
      
    case TEXTLEX_S_BASE64:
      switch( current ) {
      case ALPHA:
      case DIGIT:
      case '+':
      case '/':
      case '=':
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_BASE64 );
        break;

      case '\'':
        TOKEN( TEXTLEX_T_BASE64 );
        TOKEN( TEXTLEX_T_END );
        SET_STATE( TEXTLEX_S_START );
        break;
      }
      break;
      
    case TEXTLEX_S_BASE16_START:
      switch( current ) {
      case HEXDIGIT:
        COPY_TO_BUFFER;
        SET_STATE( TEXTLEX_S_BASE16_START );
        break;

      case '#':
        TOKEN( TEXTLEX_T_HEX );
        TOKEN( TEXTLEX_T_END );
        SET_STATE( TEXTLEX_S_BASE16_COMMENT );
        break;

      case ')':
        TOKEN( TEXTLEX_T_HEX );
        TOKEN( TEXTLEX_T_END );
        SET_STATE( TEXTLEX_S_START );
        break;
      }
      break;
      
    case TEXTLEX_S_BASE16_COMMENT:
      switch( current ) {
      case LF:
        TOKEN( TEXTLEX_T_COMMENT );
        TOKEN( TEXTLEX_T_END );
        SET_STATE( TEXTLEX_S_BASE16_START );
        break;
        
      case CR:
        TOKEN( TEXTLEX_T_COMMENT );
        TOKEN( TEXTLEX_T_END );
        SET_STATE( TEXTLEX_S_BASE16_EOLLF );
        break;

      default:
        COPY_TO_BUFFER;
        break;
      }
      break;
      
    case TEXTLEX_S_BASE16_EOLLF:
      switch( current ) {
      case CR:
        SET_STATE( TEXTLEX_S_BASE16_START );
        break;
        
      default:
        err = TEXTLEX_E_BASE16_EOLLF;
        break;
      }
      break;
    }

    if( TEXTLEX_E_NOERR != err ) {
      break;
    }

    context->octet++;
  }
  
  return( err );
}

tTextLexErr textlex_final( tTextLexContext * context ) {
  tTextLexErr err = TEXTLEX_E_NOERR;

  switch( context->state ) {
  case TEXTLEX_S_COMMENT:
    TOKEN( TEXTLEX_T_COMMENT );
    TOKEN( TEXTLEX_T_END );
    break;
    
  case TEXTLEX_S_ANNOTATE:
    TOKEN( TEXTLEX_T_ANNOTATION );
    TOKEN( TEXTLEX_T_END );
    break;
    
  case TEXTLEX_S_LITERAL:
    TOKEN( TEXTLEX_T_LITERAL );
    TOKEN( TEXTLEX_T_END );
    break;
    
  case TEXTLEX_S_NUMBER:
    TOKEN( TEXTLEX_T_INTEGER );
    TOKEN( TEXTLEX_T_END );
    break;
    
  case TEXTLEX_S_FLOAT:
  case TEXTLEX_S_EXPONENT:
    TOKEN( TEXTLEX_T_FLOAT );
    TOKEN( TEXTLEX_T_END );
    break;
    
  case TEXTLEX_S_HEX:
    TOKEN( TEXTLEX_T_HEX );
    TOKEN( TEXTLEX_T_END );
    break;
  }
  
  return( err );
}

tTextLexErr textlex_default_overflow( tTextLexContext * context ) {
  tTextLexErr err = TEXTLEX_E_NOERR;

  switch( context->state ) {
  case TEXTLEX_S_COMMENT:
    TOKEN( TEXTLEX_T_COMMENT );
    break;
    
  case TEXTLEX_S_ANNOTATE:
    TOKEN( TEXTLEX_T_ANNOTATION );
    break;
    
  case TEXTLEX_S_LITERAL:
    TOKEN( TEXTLEX_T_LITERAL );
    break;
    
  case TEXTLEX_S_NUMBER:
    TOKEN( TEXTLEX_T_INTEGER );
    break;
    
  case TEXTLEX_S_FLOAT:
  case TEXTLEX_S_EXPONENT:
    TOKEN( TEXTLEX_T_FLOAT );
    break;
    
  case TEXTLEX_S_HEX:
    TOKEN( TEXTLEX_T_HEX );
    break;

  case TEXTLEX_S_STRING:
  case TEXTLEX_S_STRING_ESCAPE:
    TOKEN( TEXTLEX_T_STRING);
    break;
    
  case TEXTLEX_S_BASE64:
    TOKEN( TEXTLEX_T_BASE64 );
    break;

  case TEXTLEX_S_BASE16_START:
    TOKEN( TEXTLEX_T_HEX );
    break;
    
  case TEXTLEX_S_BASE16_COMMENT:
    TOKEN( TEXTLEX_T_COMMENT );
    break;
  }

  context->index = 0;
  
  return( err );
}
