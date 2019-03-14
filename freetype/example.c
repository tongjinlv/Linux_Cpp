 
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#define WIDTH   120
#define HEIGHT   30
#define SIZE    HEIGHT*90/100
 
unsigned char image[HEIGHT][WIDTH];

void draw_bitmap( FT_Bitmap*  bitmap,FT_Int x,FT_Int y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;
  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= WIDTH || j >= HEIGHT )
        continue;
 
      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}
void show_image( void )
{
  int  i, j;
  for ( i = 0; i < HEIGHT; i++ )
  {
	if(i==0)for ( j = 0; j < WIDTH; j++ )putchar( '_' );
    for ( j = 0; j < WIDTH; j++ )putchar( image[i][j] == 0 ? ' ': image[i][j] < 128 ? '0'+image[i][j]/13: '*' );
    putchar( '|' );
	putchar( '\n' );
	putchar( '|' );
	if(i==HEIGHT-1)for ( j = 0; j < WIDTH; j++ )putchar( '-' );
  }
  putchar( '\n' );
}
int main( int argc,char**  argv )
{
  FT_Library    library;
  FT_Face       face;
 
  FT_GlyphSlot  slot;
  FT_Matrix     matrix;                 /* transformation matrix */
  FT_Vector     pen;                    /* untransformed origin  */
  FT_Error      error;
  char*         filename;
  char*         text;
  double        angle;
  int           target_height;
  int           n, num_chars;
  if ( argc != 3 )
  {
    fprintf ( stderr, "usage: %s font sample-text\n", argv[0] );
    exit( 1 );
  }
  filename      = argv[1];                           /* first argument     */
  text          = argv[2];                           /* second argument    */
  num_chars     = strlen( text );
  angle         = ( 0.0 / 360 ) * 3.14159 * 2;      /* use 25 degrees     */
  target_height = HEIGHT;
  error = FT_Init_FreeType( &library );              /* initialize library */
  error = FT_New_Face( library, argv[1], 0, &face ); /* create face object */
  FT_Set_Pixel_Sizes(face, SIZE, 0);
  slot = face->glyph;
  matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
  matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
  matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
  matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
  pen.x = 0;
  pen.y =20;
  for ( n = 0; n < num_chars; n++ )
  {
    FT_Set_Transform( face, &matrix, &pen );
	wchar_t *chinese_char = L"爱陈玉兰";
    error = FT_Load_Char( face, chinese_char[n], FT_LOAD_RENDER );
    if (error) continue;            
    draw_bitmap(&slot->bitmap,slot->bitmap_left,target_height-6-slot->bitmap_top );
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
  }
 
  show_image();
  FT_Done_Face( face );
  FT_Done_FreeType( library );
  return 0;
}
