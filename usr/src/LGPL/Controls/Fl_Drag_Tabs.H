//
// Fl_Drag_Tabs.H
//
// Extention to the Fl_Tabs widget that allows browser-like dragging of tabs for
// the Fast Light Tool Kit (FLTK).
//
// Copyright by Guy Bensky
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#ifndef FL_DRAG_TABS_H
#define FL_DRAG_TABS_H

#include "Fl_Scroll_Tabs.H"
//#include <FL/Fl_Tabs.H>

//#define Fl_Scroll_Tabs Fl_Tabs

class Fl_Drag_Tabs : public Fl_Scroll_Tabs
{
  public:
    Fl_Drag_Tabs (int X, int Y, int W, int H, const char * L = 0);
    ~Fl_Drag_Tabs ()
    {
    }
    //   void draw(){
    //     cout << "DRAW\n";
    //     if (damage()&FL_DAMAGE_ALL)
    //       draw_box();
    //     Fl_Tabs::draw();
    //   }
    static Fl_Window * ttt_wnd;
    int handle (int event);
    void group (char grp)
    {
        _group = grp;
    } // the tabs group. You can only DND tabs inside a common group
    char group () const
    {
        return _group;
    }
    static Fl_Drag_Tabs * default_creator (int X, int Y, void * data);
    typedef Fl_Drag_Tabs * (*creator_f) (int X, int Y, void * data);
    creator_f creator () const
    {
        return _creator;
    }
    void creator (creator_f ct, void * data)
    {
        _creator = ct;
        _creator_data = data;
    }
    void deletable (bool d)
    {
        _del = d;
    } // true: delete parent window if last tab closes. False: keep window
    bool deletable () const
    {
        return _del;
    }

  private:
    void take_control (Fl_Widget * where);
    char _group;
    bool _del;
    creator_f _creator;
    void * _creator_data;
};

#endif
