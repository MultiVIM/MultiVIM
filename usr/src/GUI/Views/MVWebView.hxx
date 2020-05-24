#ifndef MVWEBVIEW_H_
#define MVWEBVIEW_H_

#include <FL/Fl.H>
#include <FL/Fl_Group.H>

#include "LGPL/Controls/Fl_Html_View.H"

class MVWebView : public Fl_Html_View
{
  public:
    MVWebView (int x, int y, int w, int h, const char * l = nullptr);

    int handle (int);
};

#endif