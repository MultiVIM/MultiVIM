#ifndef EFLTK_H_
#define EFLTK_H_

#include <FL/Enumerations.H>

void drawGradient (int x, int y, int w, int h, Fl_Color colStart,
                   Fl_Color colEnd);

void draw_min (Fl_Color col);
void draw_max (Fl_Color col);
void draw_cl (Fl_Color col);

#endif