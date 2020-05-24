#include <FL/Enumerations.H>
#include <FL/Fl_Group.H>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "MVWebView.hxx"

MVWebView::MVWebView (int x, int y, int w, int h, const char * l)
    : Fl_Html_View (x, y, w, h, l)
{
    labeltype (FL_NO_LABEL);
}

int MVWebView::handle (int event)
{
    if (event == FL_PUSH)
    {
        Fl::focus (this);
    }

    return Fl_Html_View::handle (event);
}
