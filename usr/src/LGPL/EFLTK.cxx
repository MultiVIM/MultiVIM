#include <FL/fl_draw.H>

#include "EFLTK.hh"

#define vv(x, y) fl_vertex (x, y)
#define CL_OF .39f
void draw_cl (Fl_Color col)
{
    fl_color (col);

    fl_begin_line ();
    fl_vertex (-CL_OF, -CL_OF);
    fl_vertex (CL_OF, CL_OF);
    fl_end_line ();

    fl_begin_line ();
    fl_vertex (CL_OF, -CL_OF);
    fl_vertex (-CL_OF, CL_OF);
    fl_end_line ();
}

#define MAX_OF .4f
void draw_max (Fl_Color col)
{
    fl_color (col);

    fl_begin_polygon ();
    vv (-MAX_OF, -MAX_OF);
    vv (MAX_OF, -MAX_OF);
    vv (MAX_OF, -MAX_OF + 0.25f);
    vv (-MAX_OF, -MAX_OF + 0.25f);
    fl_end_polygon ();

    /* fl_begin_polygon ();
     vv (-MAX_OF, -MAX_OF + 0.2f);
     vv (MAX_OF, -MAX_OF + 0.2f);
     fl_end_polygon ();

     fl_begin_line ();
     vv (-MAX_OF, -MAX_OF);
     vv (MAX_OF, -MAX_OF);
     fl_end_line ();*/

    fl_begin_line ();
    vv (MAX_OF, -MAX_OF);
    vv (MAX_OF, MAX_OF);
    vv (-MAX_OF, MAX_OF);
    vv (-MAX_OF, -MAX_OF);
    fl_end_line ();
}

#define MIN_OF .45f
#define MIN_OF2 .45f
void draw_min (Fl_Color col)
{
    fl_color (col);

    fl_begin_line ();
    vv (MIN_OF2, MIN_OF);
    vv (-MIN_OF2, MIN_OF);
    fl_end_line ();

    fl_begin_line ();
    vv (MIN_OF2, MIN_OF - 0.09f);
    vv (-MIN_OF2, MIN_OF - 0.09f);
    fl_end_line ();
}

void drawGradient (int x, int y, int w, int h, Fl_Color colStart,
                   Fl_Color colEnd)
{
    uchar ar, ag, ab, br, bg, bb;         // RGB values for both colors
    Fl::get_color (colStart, ar, ag, ab); // from
    Fl::get_color (colEnd, br, bg, bb);   // to

#define COLOR_W 4
    int steps = (w / COLOR_W) - 1;
    float rstep = (float)(br - ar) / steps;
    float gstep = (float)(bg - ag) / steps;
    float bstep = (float)(bb - ab) / steps;

    int X = x;
    float r = ar, g = ag, b = ab;
    for (; steps >= 0; steps--, X += COLOR_W)
    {
        fl_color ((uchar)r, (uchar)g, (uchar)b);
        r += rstep;
        g += gstep;
        b += bstep;
        fl_rectf (X, y, COLOR_W, h);
    }
    if (w % COLOR_W)
    {
        fl_rectf (X, y, w % COLOR_W, h);
    }
}