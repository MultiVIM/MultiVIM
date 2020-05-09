//
// "$Id: Fl_Scroll_Tabs.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// Tab widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

// This is the "file card tabs" interface to allow you to put lots and lots
// of buttons and switches in a panel, as popularized by many toolkits.

// Each child widget is a card, and its label() is printed on the card tab.
// Clicking the tab makes that card visible.

#include "Fl_Scroll_Tabs.H"
#include <FL/Fl.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>
#include <stdio.h>

#include <iostream>
#define OUTPUT2(x) #x << "= " << (x) << "; "

#define BORDER 2
#define EXTRASPACE 10
#define SELECTION_BORDER 5
#define BUTTON_WIDTH 20

// Return the left edges of each tab (plus a fake left edge for a tab
// past the right-hand one).  These positions are actually of the left
// edge of the slope.  They are either separated by the correct distance
// or by EXTRASPACE or by zero.
// These positions are updated in the private arrays tab_pos[] and
// tab_width[], resp.. If needed, these arrays are (re)allocated.
// Return value is the index of the selected item.

int Fl_Scroll_Tabs::tab_positions ()
{
    // sets:
    // bar_x - the left most location of the tab-bar
    // bar_w - the width of the entire tab-bar
    // tab_w - the width of a single tab
    // tab_skip - the index of the left-most tab
    // max_skip - the maximum value of tab_skip
    // returns:
    // true if scrolling is used, false otherwise

    int nc = children ();
    // first lets check if we need to be scrollable
    int dx = Fl::box_dx (box ());
    if (dx + minw () * nc < w ())
    {
        // No need for scrolling!
        bar_x = dx;
        bar_w = w () - dx;
        tab_w = nc ? bar_w / nc : bar_w;
        if (tab_w > maxw ())
            tab_w = maxw ();
        tab_skip = 0;
        max_skip = 0;
        return 0;
    }
    // We need to be scrollable!
    bar_x = BUTTON_WIDTH + dx;
    bar_w = w () - dx - 2 * BUTTON_WIDTH;
    //{
    //	int lw,lh;
    //	this->measure_label(lw,lh);
    //	lw+=Fl::box_dw(box())+2*BORDER;
    //	bar_w-=lw;
    //}
    tab_w = minw ();
    if (tab_w > bar_w)
        tab_w = bar_w;
    max_skip = nc - bar_w / tab_w;
    if (max_skip < 0)
        max_skip = 0;
    if (tab_skip > max_skip)
        tab_skip = max_skip;
    if (tab_skip < 0)
        tab_skip = 0;
    return 1;
}

// Returns space (height) in pixels needed for tabs. Negative to put them on the
// bottom. Returns full height, if children() = 0.
int Fl_Scroll_Tabs::tab_height ()
{
    if (children () == 0)
        return h ();
    int H = h ();
    int H2 = y ();
    Fl_Widget * const * a = array ();
    for (int i = children (); i--;)
    {
        Fl_Widget * o = *a++;
        if (o->y () < y () + H)
            H = o->y () - y ();
        if (o->y () + o->h () > H2)
            H2 = o->y () + o->h ();
    }
    H2 = y () + h () - H2;
    H -= SELECTION_BORDER;
    H2 -= SELECTION_BORDER;
    if (H2 > H)
        return (H2 <= 0) ? 0 : -H2;
    else
        return (H <= 0) ? 0 : H;
}

// This is used for event handling (clicks) and by fluid to pick tabs.
// Returns 0, if children() = 0, or if the event is outside of the tabs area.
Fl_Widget * Fl_Scroll_Tabs::which (int event_x, int event_y)
{
    int nc = children ();
    if (nc == 0)
        return 0;
    int H = tab_height ();
    if (H < 0)
    {
        if (event_y > y () + h () || event_y < y () + h () + H)
            return 0;
    }
    else
    {
        if (event_y > y () + H || event_y < y ())
            return 0;
    }
    tab_positions ();
    if (event_x < bar_x)
        return 0;
    if (event_x > bar_x + bar_w)
        return 0;
    int i = (event_x - bar_x) / tab_w + tab_skip;
    if (i >= nc)
        return 0;
    return child (i);
}

void Fl_Scroll_Tabs::redraw_tabs ()
{
    int H = tab_height ();
    if (H >= 0)
    {
        H += Fl::box_dy (box ());
        damage (FL_DAMAGE_SCROLL, x (), y (), w (), H);
    }
    else
    {
        H = Fl::box_dy (box ()) - H;
        damage (FL_DAMAGE_SCROLL, x (), y () + h () - H, w (), H);
    }
}

int Fl_Scroll_Tabs::handle (int event)
{

    Fl_Widget * o;
    int i;

    switch (event)
    {

    case FL_PUSH:
    {
        int H = tab_height ();
        if (H >= 0)
        {
            if (Fl::event_y () > y () + H)
                return Fl_Group::handle (event);
        }
        else
        {
            if (Fl::event_y () < y () + h () + H)
                return Fl_Group::handle (event);
        }
    }
        if (Fl::event_x () < bar_x)
        {
            --tab_skip;
            redraw_tabs ();
            return 1;
        }
        if (Fl::event_x () > bar_x + bar_w)
        {
            ++tab_skip;
            redraw_tabs ();
            return 1;
        }

        /* FALLTHROUGH */
    case FL_DRAG:
    case FL_RELEASE:
        o = which (Fl::event_x (), Fl::event_y ());
        if (event == FL_RELEASE)
        {
            push (0);
            if (o && Fl::visible_focus () && Fl::focus () != this)
            {
                Fl::focus (this);
                redraw_tabs ();
            }
            if (o && value (o))
            {
                Fl_Widget_Tracker wp (o);
                set_changed ();
                do_callback ();
                if (wp.deleted ())
                    return 1;
            }
            Fl_Tooltip::current (o);
        }
        else
        {
            push (o);
        }
        return 1;
    case FL_MOVE:
    {
        int ret = Fl_Group::handle (event);
        Fl_Widget *o = Fl_Tooltip::current (), *n = o;
        int H = tab_height ();
        if ((H >= 0) && (Fl::event_y () > y () + H))
            return ret;
        else if ((H < 0) && (Fl::event_y () < y () + h () + H))
            return ret;
        else
        {
            n = which (Fl::event_x (), Fl::event_y ());
            if (!n)
                n = this;
        }
        if (n != o)
            Fl_Tooltip::enter (n);
        return ret;
    }
    case FL_FOCUS:
    case FL_UNFOCUS:
        if (!Fl::visible_focus ())
            return Fl_Group::handle (event);
        if (Fl::event () == FL_RELEASE || Fl::event () == FL_SHORTCUT ||
            Fl::event () == FL_KEYBOARD || Fl::event () == FL_FOCUS ||
            Fl::event () == FL_UNFOCUS)
        {
            redraw_tabs ();
            if (Fl::event () == FL_FOCUS)
                return Fl_Group::handle (event);
            if (Fl::event () == FL_UNFOCUS)
                return 0;
            else
                return 1;
        }
        else
            return Fl_Group::handle (event);
    case FL_KEYBOARD:
        switch (Fl::event_key ())
        {
        case FL_Left:
            if (child (0)->visible ())
                return 0;
            for (i = 1; i < children (); i++)
                if (child (i)->visible ())
                    break;
            value (child (i - 1));
            set_changed ();
            do_callback ();
            return 1;
        case FL_Right:
            if (child (children () - 1)->visible ())
                return 0;
            for (i = 0; i < children (); i++)
                if (child (i)->visible ())
                    break;
            value (child (i + 1));
            set_changed ();
            do_callback ();
            return 1;
        case FL_Down:
            redraw ();
            return Fl_Group::handle (FL_FOCUS);
        default:
            break;
        }
        return Fl_Group::handle (event);
    case FL_SHORTCUT:
        for (i = 0; i < children (); ++i)
        {
            Fl_Widget * c = child (i);
            if (c->test_shortcut (c->label ()))
            {
                char sc = !c->visible ();
                value (c);
                if (sc)
                    set_changed ();
                do_callback ();
                return 1;
            }
        }
        return Fl_Group::handle (event);
    case FL_SHOW:
        value (); // update visibilities and fall through
    default:
        return Fl_Group::handle (event);
    }
}

int Fl_Scroll_Tabs::push (Fl_Widget * o)
{
    if (push_ == o)
        return 0;
    if ((push_ && !push_->visible ()) || (o && !o->visible ()))
        redraw_tabs ();
    push_ = o;
    return 1;
}

/**
    Gets the currently visible widget/tab.
    The value() is the first visible child (or the last child if none
    are visible) and this also hides any other children.
    This allows the tabs to be deleted, moved to other groups, and
    show()/hide() called without it screwing up.
 */
Fl_Widget * Fl_Scroll_Tabs::value ()
{
    Fl_Widget * v = 0;
    Fl_Widget * const * a = array ();
    for (int i = children (); i--;)
    {
        Fl_Widget * o = *a++;
        if (v)
            o->hide ();
        else if (o->visible ())
            v = o;
        else if (!i)
        {
            o->show ();
            v = o;
        }
    }
    return v;
}

/**
    Sets the widget to become the current visible widget/tab.
    Setting the value hides all other children, and makes this one
    visible, if it is really a child.
 */
int Fl_Scroll_Tabs::value (Fl_Widget * newvalue)
{
    Fl_Widget * const * a = array ();
    int ret = 0;
    int nc = children ();
    for (int i = 0; i < nc; ++i)
    {
        Fl_Widget * o = *a++;
        if (o == newvalue)
        {
            if (!o->visible ())
                ret = 1;
            o->show ();
            show_tab (i);
        }
        else
        {
            o->hide ();
        }
    }

    return ret;
}

void Fl_Scroll_Tabs::show_tab (int i)
{
    if (i < 0)
        return;
    if (i < tab_skip)
    {
        tab_skip = i;
        return;
    }
    int nc = children ();
    if (i >= nc)
        return;
    int n = bar_w / tab_w;
    if (n == 0)
        n = 1;
    if (i >= tab_skip + n)
    {
        tab_skip = i - n + 1;
        return;
    }
    return;
}

enum
{
    LEFT,
    RIGHT,
    SELECTED
};

void Fl_Scroll_Tabs::draw ()
{
    Fl_Widget * v = value ();
    int H = tab_height ();

    if (damage () & FL_DAMAGE_ALL)
    { // redraw the entire thing:
        Fl_Color c = v ? v->color () : color ();

        draw_box (box (),
                  x (),
                  y () + (H >= 0 ? H : 0),
                  w (),
                  h () - (H >= 0 ? H : -H),
                  c);

        if (v)
            draw_child (*v);
    }
    else
    { // redraw the child
        if (v)
            update_child (*v);
    }
    if (damage () & (FL_DAMAGE_SCROLL | FL_DAMAGE_ALL))
    {
        draw_box (FL_FLAT_BOX,
                  x (),
                  H >= 0 ? y () : y () + h () - H,
                  w (),
                  H >= 0 ? H : -H,
                  color ());

        int clip_y = (H >= 0) ? y () + H : y () + h () + H - SELECTION_BORDER;
        fl_push_clip (x (), clip_y, w (), SELECTION_BORDER);
        draw_box (
            box (), x (), clip_y, w (), SELECTION_BORDER, selection_color ());
        fl_pop_clip ();

        int nc = children ();
        int has_scroll = tab_positions ();
        if (has_scroll)
        {
            draw_buttons (H);
        }
        int i;
        Fl_Widget * const * a = array ();
        int X = bar_x;
        for (i = tab_skip; i < nc; i++, X += tab_w)
            draw_tab (X, tab_w - BORDER, H, a[i], a[i] == v);
    }
}

void Fl_Scroll_Tabs::draw_buttons (int H)
{
    int Y = H >= 0 ? y () : y () + h () + H;
    H = H >= 0 ? H : -H;
    // int lw,lh;
    // measure_label(lw,lh);
    // lw+=Fl::box_dw(box())+2*BORDER;

    Fl_Label l;
    l.image = 0;
    l.deimage = 0;
    l.type = labeltype ();
    l.font = labelfont ();
    l.size = labelsize ();
    l.align_ = align ();

    l.value = "@<";
    l.color = this->labelcolor ();
    if (tab_skip == 0)
        l.color = fl_inactive (l.color);
    l.draw (x (), Y, BUTTON_WIDTH, H, FL_ALIGN_CENTER);

    l.color = this->labelcolor ();
    if (tab_skip == max_skip)
        l.color = fl_inactive (l.color);
    l.value = "@>";
    l.draw (x () + w () - BUTTON_WIDTH /*-lw*/,
            Y,
            BUTTON_WIDTH,
            H,
            FL_ALIGN_CENTER);

    // fl_draw_box(box(),x()+w()-lw,Y,lw,H,color());
    // draw_label(x()+w()-lw,Y,lw,H,FL_ALIGN_CENTER);
}

void Fl_Scroll_Tabs::draw_tab (int X, int W, int H, Fl_Widget * o, int sel)
{
    int dh = Fl::box_dh (box ());
    int dy = Fl::box_dy (box ());
    char prev_draw_shortcut = fl_draw_shortcut;
    fl_draw_shortcut = 1;

    Fl_Boxtype bt = (o == push_ && !sel) ? fl_down (box ()) : box ();

    // compute offsets to make selected tab look bigger
    int yofs = sel ? 0 : BORDER;

    int clip_W = bar_w - X + bar_x;
    if (clip_W < 0)
        return;
    if (clip_W > W)
        clip_W = W;

    if (H >= 0)
    {
        if (sel)
            fl_push_clip (X, y (), clip_W, H + dh - dy);
        else
            fl_push_clip (X, y (), clip_W, H);

        H += dh;

        Fl_Color c = sel ? selection_color () : o->selection_color ();

        draw_box (bt, X, y () + yofs, W, H + 10 - yofs, c);

        // Save the previous label color
        Fl_Color oc = o->labelcolor ();

        // Draw the label using the current color...
        o->labelcolor (sel ? labelcolor () : o->labelcolor ());
        o->draw_label (X + Fl::box_dx (box ()) + BORDER,
                       y () + yofs,
                       W,
                       H - yofs,
                       FL_ALIGN_LEFT);

        // Restore the original label color...
        o->labelcolor (oc);

        if (Fl::focus () == this && o->visible ())
            draw_focus (box (), X, y (), W, H);

        fl_pop_clip ();
    }
    else
    {
        H = -H;

        if (false && sel)
            fl_push_clip (X, y () + h () - H - dy, clip_W, H + dy);
        else
            fl_push_clip (X, y () + h () - H, clip_W, H);

        H += dh;

        Fl_Color c = sel ? selection_color () : o->selection_color ();

        draw_box (bt, X, y () + h () - H - 10, W, H + 10 - yofs, c);

        // Save the previous label color
        Fl_Color oc = o->labelcolor ();

        // Draw the label using the current color...
        o->labelcolor (sel ? labelcolor () : o->labelcolor ());
        o->draw_label (X + Fl::box_dx (box ()),
                       y () + h () - H,
                       W,
                       H - yofs,
                       FL_ALIGN_LEFT);

        // Restore the original label color...
        o->labelcolor (oc);

        if (Fl::focus () == this && o->visible ())
            draw_focus (box (), X, y () + h () - H, W, H);

        fl_pop_clip ();
    }
    fl_draw_shortcut = prev_draw_shortcut;
}

/**
    Creates a new Fl_Scroll_Tabs widget using the given position, size,
    and label string. The default boxtype is FL_THIN_UP_BOX.

    Use add(Fl_Widget*) to add each child, which are usually
    Fl_Group widgets. The children should be sized to stay
    away from the top or bottom edge of the Fl_Scroll_Tabs widget,
    which is where the tabs will be drawn.

    All children of Fl_Scroll_Tabs should have the same size and exactly fit on
   top of each other. They should only leave space above or below where that
   tabs will go, but not on the sides. If the first child of Fl_Scroll_Tabs is
   set to "resizable()", the riders will not resize when the tabs are resized.

    The destructor <I>also deletes all the children</I>. This
    allows a whole tree to be deleted at once, without having to
    keep a pointer to all the children in the user code. A kludge
    has been done so the Fl_Scroll_Tabs and all of its children
    can be automatic (local) variables, but you must declare the
    Fl_Scroll_Tabs widget <I>first</I> so that it is destroyed last.
 */
Fl_Scroll_Tabs::Fl_Scroll_Tabs (int X, int Y, int W, int H, const char * l)
    : Fl_Group (X, Y, W, H, l)
{
    box (FL_THIN_UP_BOX);
    push_ = 0;
    _minw = 50;
    _maxw = 200;
    bar_x = X;
    bar_w = W;
    tab_w = _maxw;
    if (tab_w > bar_w)
        tab_w = bar_w;
    tab_skip = max_skip = 0;
}

Fl_Scroll_Tabs::~Fl_Scroll_Tabs ()
{
}

/**
    Returns the position and size available to be used by its children.

    If there isn't any child yet the \p tabh parameter will be used to
    calculate the return values. This assumes that the children's labelsize
    is the same as the Fl_Scroll_Tabs' labelsize and adds a small border.

    If there are already children, the values of child(0) are returned, and
    \p tabh is ignored.

    \note Children should always use the same positions and sizes.

    \p tabh can be one of
    \li    0: calculate label size, tabs on top
    \li   -1: calculate label size, tabs on bottom
    \li >  0: use given \p tabh value, tabs on top (height = tabh)
    \li < -1: use given \p tabh value, tabs on bottom (height = -tabh)

    \param[in]	tabh		position and optional height of tabs (see above)
    \param[out]	rx,ry,rw,rh	(x,y,w,h) of client area for children

    \since	FLTK 1.3.0
 */
void Fl_Scroll_Tabs::client_area (int & rx, int & ry, int & rw, int & rh,
                                  int tabh)
{

    if (children ())
    { // use existing values

        rx = child (0)->x ();
        ry = child (0)->y ();
        rw = child (0)->w ();
        rh = child (0)->h ();
    }
    else
    { // calculate values

        int y_offset;
        int label_height = fl_height (labelfont (), labelsize ()) + BORDER * 2;

        if (tabh == 0) // use default (at top)
            y_offset = label_height + SELECTION_BORDER;
        else if (tabh == -1) // use default (at bottom)
            y_offset = -label_height - SELECTION_BORDER;
        else
            y_offset = tabh; // user given value

        rx = x ();
        rw = w ();

        if (y_offset >= 0)
        { // labels at top
            ry = y () + y_offset;
            rh = h () - y_offset;
        }
        else
        { // labels at bottom
            ry = y ();
            rh = h () + y_offset;
        }
    }
}

//
// End of "$Id: Fl_Scroll_Tabs.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
