#ifndef MVMDIWORKSPACE_HH_
#define MVMDIWORKSPACE_HH_

#include <FL/Fl_Scroll.H>

#include "MVMDIWindow.hh"

class MVMDIWorkspace : public Fl_Scroll, public MVWExtras
{
    friend class MVMDIWindow;
    friend class MVMDITitle;

  public:
    MVMDIWorkspace (int x, int y, int w, int h, const char * l = 0)
        : Fl_Scroll (x, y, w, h, l)
    {
    }

    void activate (MVMDIWindow & win);
    void addWindow (MVMDIWindow & win);
    MVMDIWindow * createBigWindow (const char * l = "Untitled");
    int handle (int event);
};

#endif