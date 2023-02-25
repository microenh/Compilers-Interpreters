/****************************************************************/
/*                                                              */
/*      P A S C A L   R U N T I M E   L I B R A R Y             */
/*                                                              */
/*      Note that all formal parameters are reversed to         */
/*      accomodate the Pascal calling convention of the         */
/*      compiled code.                                          */
/*                                                              */
/*      All floating point parameters are passed in as longs    */
/*      to bypass unwanted type conversions.  Floating point    */
/*      function values are also returned as longs.             */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define MAX_SOURCE_LINE_LENGTH  256

OLEAN;

union {
  float real;
  long  dword;
} value;

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/

bool eof_flag  = false;
bool eoln_flag = false;

/*--------------------------------------------------------------*/
/*  Forwards                                                    */
/*--------------------------------------------------------------*/

int read_integer(void);
long read_real(void);
char read_char(void);
void read_line(void);
void write_integer(int field_width, int i);
void write_real(int precision, int field_width, int i);
void write_boolean(int field_width, int b);
void write_char(int field_width, int ch);
void write_string(int length, int field_width, char *value);
void write_line(void);
bool std_end_of_file(void);
bool std_end_of_line(void);
long float_negate(long i);
long float_add(long j, long i);
long float_subtract(long j, long i);
long float_multiply(long j, long i);
long float_divide(long j, long i);
long float_convert(int i);
int float_compare(long j, long i);
long std_abs(long i);
long std_arctan(long i);
long std_cos(long i);
long std_exp(long i);
long std_ln(long i);
long std_sin(long i);
long std_sqrt(long i);
int std_round(long i);
int std_trunc(long i);

/*--------------------------------------------------------------*/
/*  main                The main routine, which calls           */
/*                      pascal_main, the "main" of the compiled */
/*                      program.                                */
/*--------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  pascal_main();
  exit(0);
}

		/********************************/
		/*                              */
		/*      Read routines           */
		/*                              */
		/********************************/

/*--------------------------------------------------------------*/
/*  read_integer        Read an integer value.                  */
/*--------------------------------------------------------------*/

int read_integer(void)
{
  int i;

  scanf("%d", &i);
  return(i);
}

/*--------------------------------------------------------------*/
/*  read_real           Read a real value.                      */
/*--------------------------------------------------------------*/

long read_real(void)
{
  scanf("%g", &value.real);
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  read_char          Read a character value.                  */
/*--------------------------------------------------------------*/

char read_char(void)
{
  char ch;

  scanf("%c", &ch);
  if (eof_flag || (ch == '\n') || (ch == '\r'))
    ch = ' ';

  return(ch);
}

/*--------------------------------------------------------------*/
/*  read_line          Skip the rest of the input record.       */
/*--------------------------------------------------------------*/

void read_line(void)
{
  char ch;

  do {
  	ch = getchar();
  } while(!eof_flag && (ch != '\n'));
}

		/********************************/
		/*                              */
		/*      Write routines          */
		/*                              */
		/********************************/

/*--------------------------------------------------------------*/
/*  write_integer       Write an integer value.                 */
/*--------------------------------------------------------------*/

void write_integer(int field_width, int i)
{
  printf("%*d", field_width, i);
}

/*--------------------------------------------------------------*/
/*  write_real          Write an real value.                    */
/*--------------------------------------------------------------*/

void write_real(int precision, int field_width, int i)
{
  value.dword = i;
  printf("%*.*f", field_width, precision, value.real);
}

/*--------------------------------------------------------------*/
/*  write_boolean       Write a boolean value.                  */
/*--------------------------------------------------------------*/

void write_boolean(int field_width, int b)
{
  printf("%*s", field_width, b == 0 ? "FALSE" : "TRUE");
}

/*--------------------------------------------------------------*/
/*  write_char          Write a character value.                */
/*--------------------------------------------------------------*/

void write_char(int field_width, int ch)
{
  printf("%*c", field_width, ch);
}

/*--------------------------------------------------------------*/
/*  write_string        Write a string value.                   */
/*--------------------------------------------------------------*/

void write_string(int length, int field_width, char *value)
{
  char buffer[MAX_SOURCE_LINE_LENGTH];

  strncpy(buffer, value, length);
  buffer[length] = '\0';

  printf("%*s", -field_width, buffer);
}

/*--------------------------------------------------------------*/
/*  write_line          Write a carriage return.                */
/*--------------------------------------------------------------*/

void write_line(void)
{
  putchar('\n');
}

		/********************************/
		/*                              */
		/*      Other I/O routines      */
		/*                              */
		/********************************/

/*--------------------------------------------------------------*/
/*  std_end_of_file     Return 1 if at end of file, else 0.     */
/*--------------------------------------------------------------*/

bool std_end_of_file(void)
{
  char ch = getchar();

  if (eof_flag || feof(stdin))
    eof_flag = true;
  else 
    ungetc(ch, stdin);

    return(eof_flag);
}

/*--------------------------------------------------------------*/
/*  std_end_of_line     Return 1 if at end of line, else 0.     */
/*--------------------------------------------------------------*/

bool std_end_of_line(void)
{
  char ch = getchar();

  if (eof_flag || feof(stdin))
  	eoln_flag = eof_flag = true;
  else {
	  eoln_flag = ch == '\n';
    ungetc(ch, stdin);
  }

  return(eoln_flag);
}

	    /************************************************/
	    /*                                              */
	    /*      Floating point arithmetic routines      */
	    /*                                              */
	    /************************************************/

/*--------------------------------------------------------------*/
/*  float_negate        Return the negated value.               */
/*--------------------------------------------------------------*/

long float_negate(long i)
{
  value.dword = i;

  value.real = -value.real;
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  float_add           Return the sum x + y.                   */
/*--------------------------------------------------------------*/

long float_add(long j, long i)
{
  float x, y;

  value.dword = i;  x = value.real;
  value.dword = j;  y = value.real;

  value.real = x + y;
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  float_subtract      Return the difference x - y.            */
/*--------------------------------------------------------------*/

long float_subtract(long j, long i)
{
  float x, y;

  value.dword = i;  x = value.real;
  value.dword = j;  y = value.real;

  value.real = x - y;
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  float_multiply      Return the product x*y.                 */
/*--------------------------------------------------------------*/

long float_multiply(long j, long i)
{
  float x, y;

  value.dword = i;  x = value.real;
  value.dword = j;  y = value.real;

  value.real = x*y;
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  float_divide        Return the quotient x/y.                */
/*--------------------------------------------------------------*/

long float_divide(long j, long i)
{
  float x, y;

  value.dword = i;  x = value.real;
  value.dword = j;  y = value.real;

  value.real = x/y;
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  float_convert       Convert an integer value to real and    */
/*                      return the converted value.             */
/*--------------------------------------------------------------*/

long float_convert(int i)
{
  value.real = i;
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  float_compare       Return -1 if x <  y                     */
/*                              0 if x == y                     */
/*                             +1 if x >  y                     */
/*--------------------------------------------------------------*/

int float_compare(long j, long i)
{
  int   comp;
  float x, y;

  value.dword = i;  x = value.real;
  value.dword = j;  y = value.real;

  if (x < y)
    comp = -1;
  else if (x == y)
    comp =  0;
  else
    comp = +1;

  return(comp);
}

	    /************************************************/
	    /*                                              */
	    /*      Standard floating point functions       */
	    /*                                              */
	    /************************************************/

/*--------------------------------------------------------------*/
/*  std_abs             Return abs of parameter.                */
/*--------------------------------------------------------------*/

long std_abs(long i)
{
  value.dword = i;

  value.real = fabs(value.real);
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  std_arctan          Return arctan of parameter.             */
/*--------------------------------------------------------------*/

long std_arctan(long i)
{
  value.dword = i;

  value.real = atan(value.real);
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  std_cos             Return cos of parameter.                */
/*--------------------------------------------------------------*/

long std_cos(long i)
{
  value.dword = i;

  value.real = cos(value.real);
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  std_exp             Return exp of parameter.                */
/*--------------------------------------------------------------*/

long std_exp(long i)
{
  value.dword = i;

  value.real = exp(value.real);
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  std_ln              Return ln of parameter.                 */
/*--------------------------------------------------------------*/

long std_ln(long i)
{
  value.dword = i;

  value.real = log(value.real);
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  std_sin             Return sin of parameter.                */
/*--------------------------------------------------------------*/

long std_sin(long i)
{
  value.dword = i;

  value.real = sin(value.real);
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  std_sqrt            Return sqrt of parameter.               */
/*--------------------------------------------------------------*/

long std_sqrt(long i)
{
  value.dword = i;

  value.real = sqrt(value.real);
  return(value.dword);
}

/*--------------------------------------------------------------*/
/*  std_round           Return round of parameter.              */
/*--------------------------------------------------------------*/

int std_round(long i)
{
  value.dword = i;

  value.dword = (int) (value.real + 0.5);
  return((int) value.dword);
}

/*--------------------------------------------------------------*/
/*  std_trunc           Return trunc of parameter.              */
/*--------------------------------------------------------------*/

int std_trunc(long i)
{
  value.dword = i;

  value.dword = (int) value.real;
  return((int) value.dword);
}
