//
// Fl_Drag_Tabs.cxx
//
// Extention to the Fl_Scroll_Tabs widget that allows browser-like dragging of
// tabs for the Fast Light Tool Kit (FLTK).
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

#include "Fl_Drag_Tabs.H"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <iostream>
char _dragged_group = 0;
Fl_Widget * _dragged_widget = NULL;
static void window_deleter (void * data)
{
    Fl::remove_idle (&window_deleter, data);
    Fl_Window * wnd = (Fl_Window *)data;
    wnd->hide ();
    delete wnd;
}
static int grabfunc (int event)
{
    if (event == FL_RELEASE)
        Fl::pushed (0);
    return 0;
}
extern int (*fl_local_grab) (int);
static int local_handle (int event, Fl_Window * window)
{
    fl_local_grab = 0;
    Fl::e_x = Fl::e_x_root - window->x ();
    Fl::e_y = Fl::e_y_root - window->y ();
    int ret = Fl::handle (event, window);
    fl_local_grab = grabfunc;
    return ret;
}

int local_dnd (Fl_Window * info = NULL)
{
    Fl_Window * source_fl_win = Fl::first_window ();
    Fl::first_window ()->cursor (FL_CURSOR_MOVE);
    fl_local_grab = grabfunc;
    Fl_Window * local_window = 0;
    int good = 0;

    while (Fl::pushed ())
    {
        // figure out what window we are pointing at:
        int x = Fl::event_x_root (), y = Fl::event_y_root ();
        if (info && ((info->x () != x + 5) || (info->y () != y + 5)))
            info->resize (x + 5, y + 5, info->w (), info->h ());
        Fl_Window * new_local_window = 0;
        for (new_local_window = Fl::first_window (); new_local_window;
             new_local_window = Fl::next_window (new_local_window))
        {
            if ((x > new_local_window->x ()) && (y > new_local_window->y ()) &&
                (x < new_local_window->x () + new_local_window->w ()) &&
                (y < new_local_window->y () + new_local_window->h ()))
                break;
        }
        if (new_local_window != local_window)
        {
            if (local_window && good)
            {
                local_handle (FL_DND_LEAVE, local_window);
            }
            local_window = new_local_window;
            if (local_window)
            {
                good = local_handle (FL_DND_ENTER, local_window);
            }
        }
        if (local_window && good)
        {
            local_handle (FL_DND_DRAG, local_window);
        }
        Fl::wait ();
    }
    if (not good)
        local_window = 0;

    if (local_window)
    {
        if (local_handle (FL_DND_RELEASE, local_window))
            Fl::paste (*local_window, 0, Fl::clipboard_plain_text);
    }

    fl_local_grab = 0;
    source_fl_win->cursor (FL_CURSOR_DEFAULT);
    return 1;
}

Fl_Drag_Tabs::Fl_Drag_Tabs (int X, int Y, int W, int H, const char * L)
    : Fl_Scroll_Tabs (X, Y, W, H, L), _group (0), _del (false),
      _creator (&Fl_Drag_Tabs::default_creator), _creator_data (NULL)
{
}

void Fl_Drag_Tabs::take_control (Fl_Widget * where = NULL)
{
    Fl_Drag_Tabs * f = (Fl_Drag_Tabs *)_dragged_widget->parent ();
    Fl_Drag_Tabs * t = this;
    if (where == _dragged_widget)
        return;
    if ((not where) && (t->find (_dragged_widget) == t->children () - 1))
        return;
    if (f->value () == _dragged_widget)
        f->value (NULL);
    f->remove (_dragged_widget);
    if ((not f->children ()) && (f->_del))
    {
        Fl::add_timeout (0., &window_deleter, f->window ());
    }
    else
    {
        f->window ()->damage (
            FL_DAMAGE_ALL, f->x (), f->y (), f->w (), f->h ());
        f->do_callback ();
    }
    int XX, YY, WW, HH;
    t->client_area (XX, YY, WW, HH, 0);
    t->insert (*_dragged_widget, where);
    _dragged_widget->resize (XX, YY, WW, HH);
    t->do_callback ();
    if (t->children () == 1)
        t->resizable (_dragged_widget);

    t->value (_dragged_widget);
    t->window ()->damage (FL_DAMAGE_ALL, t->x (), t->y (), t->w (), t->h ());
}

int Fl_Drag_Tabs::handle (int event)
{
    switch (event)
    {
    case FL_DND_ENTER:
        if (not _dragged_widget)
            return 0;
        if (_group != _dragged_group)
            return 0;
        // XXX;
        return 1;
    case FL_DND_DRAG:
        //			if (not _dragged_widget) return 1;
        //			{
        //				int X,Y,W,H;
        //				this->client_area(X,Y,W,H,0);
        //				if(Fl::event_inside(X,Y,W,H)) return 1;
        //			}
        return 1;
    case FL_DND_LEAVE:
        return 1;
    case FL_DND_RELEASE:
        if (not _dragged_widget)
            return 1;
        {
            int X, Y, W, H;
            this->client_area (X, Y, W, H, 0);
            if (Fl::event_inside (X, Y, W, H))
                return 1;
        }
        this->take_control (which (Fl::event_x (), Fl::event_y ()));
        _dragged_widget = NULL;
        return 1;
    case FL_PASTE:
        // XXX;
        return 1;
    case FL_DRAG:
        if (_dragged_widget)
            return 0;
        if (which (Fl::event_x (), Fl::event_y ()) == push ())
            return 0;
        _dragged_group = _group;
        _dragged_widget = push ();
        if (not _dragged_widget)
            return 0;
        push (NULL);

        Fl::copy ("Fl_Drag_Tabs\r\n", 14, 0, Fl::clipboard_plain_text);
        {
            int ww, hh;
            _dragged_widget->measure_label (ww, hh);
            ww += 10;
            hh += 10;
            Fl_Window * wnd = new Fl_Window (ww, hh);
            Fl_Box * box = new Fl_Box (0, 0, ww, hh, _dragged_widget->label ());
            box->box (FL_UP_BOX);
            wnd->end ();
            wnd->clear_border ();
            wnd->show ();
            local_dnd (wnd);
            wnd->hide ();
            delete wnd;
        }
        if ((_dragged_widget) && (_creator))
        {
            int XX, YY;
            Fl::get_mouse (XX, YY);
            Fl_Drag_Tabs * t = _creator (XX, YY, _creator_data);
            t->_group = this->_group;
            t->_del = true;
            t->_creator = this->_creator;
            if (t)
                t->take_control (NULL);
        }
        _dragged_widget = NULL;
        return 1;
    default:
        return Fl_Scroll_Tabs::handle (event);
    }
}

Fl_Drag_Tabs * Fl_Drag_Tabs::default_creator (int x, int y, void * d)
{
    Fl_Window * wnd = new Fl_Window (x, y, 300, 200);
    Fl_Drag_Tabs * tabs = new Fl_Drag_Tabs (0, 0, 300, 200);
    tabs->end ();
    wnd->end ();
    wnd->resizable (tabs);
    wnd->show ();
    return tabs;
}
