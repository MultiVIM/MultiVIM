// Copyright 2016 - 2019 gnuwimp@gmail.com
// Released under the GNU Lesser General Public License (LGPL)

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include "Flw_Split.H"

//|--------------------------- Flw_Split::Flw_Split
//| Create Flw_Split widget
//| <int>           X pos in pixels, default 0
//| <int>           Y pos in pixels, default 0
//| <int>           Width in pixels, default 0
//| <int>           Height in pixels, default 0
//| <int>           Label, default NULL
//|
Flw_Split::Flw_Split (int X, int Y, int W, int H, const char * l)
    : Fl_Group (X, Y, W, H, l)
{
    end ();
    clip_children (1);
    clear ();
}

//|--------------------------- Flw_Split::add1
//| Add left/top widget
//| <Fl_Widget*>    Widget, if NULL then exisitng widget will be deleted
//|
void Flw_Split::add1 (Fl_Widget * widget)
{
    if (_widgets[0])
    {
        remove (_widgets[0]);
        delete _widgets[0];
    }

    _widgets[0] = widget;

    if (widget)
        Fl_Group::add (widget);
}

//|--------------------------- Flw_Split::add2
//| Add right/bottom widget
//| <Fl_Widget*>    Widget, if NULL then exisitng widget will be deleted
//|
void Flw_Split::add2 (Fl_Widget * widget)
{
    if (_widgets[1])
    {
        remove (_widgets[1]);
        delete _widgets[1];
    }

    _widgets[1] = widget;

    if (widget)
        Fl_Group::add (widget);
}

//|--------------------------- Flw_Split::clear
//| Delete all widgets and reset internal data
//|
void Flw_Split::clear ()
{
    Fl_Group::clear ();
    _widgets[0] = NULL;
    _widgets[1] = NULL;
    _type = FLW_SPLIT_TYPE_HORIZONTAL;
    _border_space = 0;
    _widget_space = 0;
    _drag = false;
    _split_pos = w () / 2;
    _w = -1;
    _h = -1;
}

//|--------------------------- Flw_Split::get_border_space
//| Get border space
//| :int:           Space in pixels
//|
int Flw_Split::get_border_space () const
{
    return _border_space;
}

//|--------------------------- Flw_Split::get_split_pos
//| Get current split pos
//| :int:           Pos in pixels
//|
int Flw_Split::get_split_pos () const
{
    return _split_pos;
}

//|--------------------------- Flw_Split::get_type
//| Get layout type
//| :FLW_SPLIT_TYPE:    Layout type
//|
FLW_SPLIT_TYPE Flw_Split::get_type () const
{
    return _type;
}

//|--------------------------- Flw_Split::get_widget_space
//| Get space between widgets
//| :int:           Space in pixels
//|
int Flw_Split::get_widget_space () const
{
    return _widget_space;
}

//|--------------------------- Flw_Split::get_widget1
//| Get left/top widget
//| :Fl_Widget*:    Widget, can be NULL
//|
Fl_Widget * Flw_Split::get_widget1 ()
{
    return _widgets[0];
}

//|--------------------------- Flw_Split::get_widget2
//| Get right/bottom widget
//| :Fl_Widget*:    Widget, can be NULL
//|
Fl_Widget * Flw_Split::get_widget2 ()
{
    return _widgets[1];
}

//|--------------------------- Flw_Split::handle
//| Handle events
//| <int>           Event number
//|
int Flw_Split::handle (int event)
{
    if (_type == FLW_SPLIT_TYPE_HORIZONTAL)
    {
        switch (event)
        {
        case FL_DRAG:
            if (_drag)
            {
                int min = x () + _border_space + _widget_space;
                int max = (x () + w ()) - (_border_space + _widget_space);
                int pos = Fl::event_x ();

                if (pos < min)
                    pos = min;
                else if (pos > max)
                    pos = max;

                pos = pos - x ();

                if (pos != _split_pos)
                {
                    _split_pos = pos;
                    resize ();
                }

                return 1;
            }

            break;

        case FL_LEAVE:
            if (Fl::event_y () < y () || Fl::event_y () > y () + h () ||
                Fl::event_x () < x () || Fl::event_x () > x () + w ())
            {
                _drag = false;
                fl_cursor (FL_CURSOR_DEFAULT);
            }
            break;

        case FL_MOVE:
            if (_widgets[0] && _widgets[1] && _widgets[0]->visible () &&
                _widgets[1]->visible ())
            {
                int hs = _widget_space / 2 + 4;
                int pos = x () + _split_pos;

                if (Fl::event_x () > (pos - hs) && Fl::event_x () <= (pos + hs))
                {
                    if (_drag == false)
                    {
                        _drag = true;
                        fl_cursor (FL_CURSOR_WE);
                    }

                    return 1;
                }
            }

            if (_drag)
            {
                _drag = false;
                fl_cursor (FL_CURSOR_DEFAULT);
            }

            break;

        case FL_PUSH:
            if (_drag)
                return 1;
            break;

        default:
            break;
        }
    }
    else if (_type == FLW_SPLIT_TYPE_VERTICAL)
    {
        switch (event)
        {
        case FL_DRAG:
            if (_drag)
            {
                int min = y () + _border_space + _widget_space;
                int max = (y () + h ()) - (_border_space + _widget_space);
                int pos = Fl::event_y ();

                if (pos < min)
                    pos = min;
                else if (pos > max)
                    pos = max;

                pos = pos - y ();

                if (pos != _split_pos)
                {
                    _split_pos = pos;
                    resize ();
                }

                return 1;
            }

            break;

        case FL_LEAVE:
            if (Fl::event_y () < y () || Fl::event_y () > y () + h () ||
                Fl::event_x () < x () || Fl::event_x () > x () + w ())
            {
                _drag = false;
                fl_cursor (FL_CURSOR_DEFAULT);
            }
            break;

        case FL_MOVE:
            if (_widgets[0] && _widgets[1] && _widgets[0]->visible () &&
                _widgets[1]->visible ())
            {
                int hs = _widget_space / 2 + 4;
                int pos = y () + _split_pos;

                if (Fl::event_y () > (pos - hs) && Fl::event_y () <= (pos + hs))
                {
                    if (_drag == false)
                    {
                        _drag = true;
                        fl_cursor (FL_CURSOR_NS);
                    }

                    return 1;
                }
            }

            if (_drag)
            {
                _drag = false;
                fl_cursor (FL_CURSOR_DEFAULT);
            }

            break;

        case FL_PUSH:
            if (_drag)
                return 1;
            break;

        default:
            break;
        }
    }

    return Fl_Group::handle (event);
}

//|--------------------------- Flw_Split::resize()
//| Resize all child widgets using current widget size
//|
void Flw_Split::resize ()
{
    _w = -1;
    _h = -1;
    Flw_Split::resize (x (), y (), w (), h ());
    Fl::redraw ();
}

//|--------------------------- Flw_Split::resize(int X, int Y, int W, int H)
//| Resize all child widgets
//| <int>           X pos in pixels
//| <int>           Y pos in pixels
//| <int>           Width in pixels
//| <int>           Height in pixels
//|
void Flw_Split::resize (int X, int Y, int W, int H)
{
    Fl_Widget::resize (X, Y, W, H);

    if (W == _w && H == _h)
        return;

    int currx = X + _border_space;
    int curry = Y + _border_space;
    int currh = H - (_border_space * 2);
    int currw = W - (_border_space * 2);

    if (_type == FLW_SPLIT_TYPE_HORIZONTAL)
    {
        if (currw > 0 && currh > 0)
        {
            if (_widgets[0] && _widgets[1] && _widgets[0]->visible () &&
                _widgets[1]->visible ())
            {
                int max = (X + W) - (_border_space + _widget_space);
                int pos = _split_pos + X;

                if (pos < (X + _border_space))
                    pos = X;
                else if (pos > max)
                    pos = max;

                int hs = _widget_space / 2;
                int w1 = pos - (X + _border_space + hs);
                int w2 = (X + W) - (pos + _border_space + hs);

                _widgets[0]->resize (currx, curry, w1, currh);
                _widgets[1]->resize (
                    currx + w1 + _widget_space, curry, w2, currh);
            }
            else if (_widgets[0] && _widgets[0]->visible ())
                _widgets[0]->resize (currx, curry, currw, currh);
            else if (_widgets[1] && _widgets[1]->visible ())
                _widgets[1]->resize (currx, curry, currw, currh);
        }
    }
    else if (_type == FLW_SPLIT_TYPE_VERTICAL)
    {
        if (currw > 0 && currh > 0)
        {
            if (_widgets[0] && _widgets[1] && _widgets[0]->visible () &&
                _widgets[1]->visible ())
            {
                int max = (Y + H) - (_border_space + _widget_space);
                int pos = _split_pos + Y;

                if (pos < (Y + _border_space))
                    pos = Y;
                else if (pos > max)
                    pos = max;

                int hs = _widget_space / 2;
                int h1 = pos - (Y + _border_space + hs);
                int h2 = (Y + H) - (pos + _border_space + hs);

                _widgets[0]->resize (currx, curry, currw, h1);
                _widgets[1]->resize (
                    currx, curry + h1 + _widget_space, currw, h2);
            }
            else if (_widgets[0] && _widgets[0]->visible ())
                _widgets[0]->resize (currx, curry, currw, currh);
            else if (_widgets[1] && _widgets[1]->visible ())
                _widgets[1]->resize (currx, curry, currw, currh);
        }
    }

    _w = W;
    _h = H;
}

//|--------------------------- Flw_Split::set_border_space
//| Set border space
//| <int>           Space in pixels
//|
void Flw_Split::set_border_space (int border_space)
{
    _border_space = border_space;
    Fl::redraw ();
}

//|--------------------------- Flw_Split::set_type
//| Set layout type
//| <FLW_SPLIT_TYPE>    Layout type
//|
void Flw_Split::set_type (FLW_SPLIT_TYPE type)
{
    _type = type;
    Fl::redraw ();
}

//|--------------------------- Flw_Split::set_split_pos
//| Set current split pos
//| <int>           Pos in pixels
//|
void Flw_Split::set_split_pos (int split_pos)
{
    _split_pos = split_pos;
    Fl::redraw ();
}

//|--------------------------- Flw_Split::set_widget_space
//| Set space between widgets
//| <int>           Space in pixels
//|
void Flw_Split::set_widget_space (int widget_space)
{
    _widget_space = widget_space;
    Fl::redraw ();
}
