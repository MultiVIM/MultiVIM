#include <iostream>
#include <string>

#include <FL/fl_draw.H>

#include "LGPL/EFLTK.hh"
#include "MVMDIWindow.hh"
#include "MVMDIWorkspace.hh"

struct Point
{
    int x, y;

    static Point flEvent ()
    {
        return Point (Fl::event_x (), Fl::event_y ());
    }

    Point (int X, int Y) : x (X), y (Y)
    {
    }
    bool isInRect (Fl_Rect & rect)
    {
        return (x >= rect.x ()) && (y >= rect.y ()) &&
               (x < (rect.x () + rect.w ())) && (y < (rect.y () + rect.h ()));
    }
};

MVMDITitle::MVMDITitle (int x, int y, int w, int h, const char * l)
    : Fl_Group (x, y, w, h, l), _close (0, 0, 0, 0), _max (0, 0, 0, 0),
      _min (0, 0, 0, 0)
{
    _owner = (MVMDIWindow *)parent ();
    labelcolor (FL_WHITE);
    labelfont (FL_SCREEN_BOLD);
    align (FL_ALIGN_LEFT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

    fl_add_symbol ("xx", draw_cl, 1);
    fl_add_symbol ("mx", draw_max, 1);
    fl_add_symbol ("mi", draw_min, 1);

    //    _close.label ("c");
    _close.label ("@xx");
    //_close.callback (closeMdiWin, _owner);

    _max.label ("@mx");
    //_max.callback (maxMdiWin, _owner);

    _min.label ("@mi");
    //_min.callback (minMdiWin, _owner);

    _close.show ();
    _min.show ();
    _max.show ();

    layout ();
}

static bool flEventInsideWindow (Fl_Window * win)
{
    int eX = Fl::event_x (), eY = Fl::event_y ();
    return eX > 0 && eY > 0 && eX < win->w () && eY < win->h ();
}

int MVMDITitle::handle (int event)
{
    static bool moving = false;
    static int rx, ry;

    switch (event)
    {
    case FL_ACTIVATE:
        printf ("MDI Titlebar went active\n");
        break;
    case FL_DEACTIVATE:
        printf ("MDI Titlebar went inactive\n");
        break;
    case FL_PUSH:
        if (!Point::flEvent ().isInRect (buttonArea))
        {
            rx = Fl::event_x ();
            ry = Fl::event_y ();
            fl_cursor (FL_CURSOR_MOVE);
            moving = true;
            return 1;
        }
        break;

    case FL_DRAG:
    {
        if (moving && Fl::event_inside (_owner->parent ()))
        {
            int dX = Fl::event_x () - rx;
            int dY = Fl::event_y () - ry;
            int newX = _owner->x () + dX;
            int newY = _owner->y () + dY;
            rx = Fl::event_x ();
            ry = Fl::event_y ();

            _owner->resize (newX, newY, _owner->w (), _owner->h ());
            _owner->redraw ();
        }
        return 1;
    }

    case FL_RELEASE:
        fl_cursor (FL_CURSOR_DEFAULT);
        moving = false;
        return 1;
    }
    return Fl_Group::handle (event);
}

void MVMDITitle::layout ()
{
    int butY = y () + 2;
    int butX = x () + w () - 2 - allButtonsSize ();
    buttonArea = {butX, butY, allButtonsSize (), buttonSize ()};

    _min.resize (butX, butY, buttonSize (), buttonSize ());
    butX += buttonSize () + 1;
    _max.resize (butX, butY, buttonSize (), buttonSize ());
    butX += buttonSize () + 1;
    _close.resize (butX, butY, buttonSize (), buttonSize ());
}

void MVMDITitle::resize (int X, int Y, int W, int H)
{
    Fl_Group::resize (X, Y, W, H);
    layout ();
}

void MVMDITitle::draw ()
{
    Fl_Color colStart, colEnd;

    if (active ())
        colStart = 0x0000c800;
    else
        colStart = 0x8686c200;
    colEnd = fl_color_average (colStart, FL_WHITE, 0.2);
    drawGradient (x (), y (), w (), h (), colStart, colEnd);

    Fl_Group::draw ();
}

int MVMDITitle::allButtonsSize ()
{
    return buttonSize () * 3 + 2;
}

int MVMDITitle::minX ()
{
    int x = 0, y = 0;
    measure_label (x, y);
    x += allButtonsSize ();
    x += 8;
    return x;
}

int MVMDITitle::minY ()
{
    return 24;
}

MVMDIWindow::MVMDIWindow (int x, int y, int w, int h, const char * label)
    : Fl_Group (x, y, w, h, 0), titleBar (2, 2, w, 18, label),
      contents (nullptr)
{
    box (FL_SHADOW_BOX);
    resize (x, y, w, h);
    clip_children (1);
}

void MVMDIWindow::didDefocus ()
{
    titleBar.deactivate ();
}
void MVMDIWindow::didFocus ()
{
    titleBar.activate ();
}

static void boxInset (Fl_Boxtype box, int & x, int & y, int & w, int & h)
{
    x += Fl::box_dx (box);
    y += Fl::box_dy (box);
    w -= Fl::box_dw (box);
    h -= Fl::box_dh (box);
}

void MVMDIWindow::addFullSizeChild (Fl_Group * widget)
{
    contents = widget;
    add (widget);
    resize (x (), y (), w (), h ());
}

void MVMDIWindow::handleDragEvent (bool begin)
{
    static int origX, origY, origW, origH;
    int dX = Fl::event_x () - origX;
    int dY = Fl::event_y () - origY;
    int dW = Fl::event_x () - origW - origX;
    int dH = Fl::event_y () - origH - origY;
    int newX, newY, newW, newH;

    if (begin)
    {
        origX = x (), origY = y (), origW = w (), origH = h ();
        return;
    }

    newX = x ();
    newY = y ();
    newW = w ();
    newH = h ();

    switch (dragState)
    {
    case kLeft:
    {
        newX = origX + dX;
        newW = origW - dX;
        if (newW < minX ())
            return;
        break;
    }
    case kTop:
    {
        newY = origY + dY;
        newH = origH - dY;
        if (newH < minY ())
            return;
        break;
    }
    case kRight:
    {
        newW = origW + dW;
        if (newW < minX ())
            return;
        break;
    }
    case kBottom:
    {
        newH = origH + dH;
        if (newH < minY ())
            return;
        break;
    }
    case kTopLeft:
    {
        newX = origX + dX;
        newW = origW - dX;
        newY = origY + dY;
        newH = origH - dY;

        if (newW < minX ())
        {
            newX = x ();
            newW = w ();
        }
        if (newH < minY ())
        {
            newY = y ();
            newH = h ();
        }
        break;
    }
    case kTopRight:
    {
        newY = origY + dY;
        newH = origH - dY;
        newW = origW + dW;

        if (newW < minX ())
        {
            newX = x ();
            newW = w ();
        }
        if (newH < minY ())
        {
            newY = y ();
            newH = h ();
        }
        break;
    }
    case kBottomLeft:
    {
        newX = origX + dX;
        newW = origW - dX;
        newH = origH + dH;

        if (newW < minX ())
        {
            newX = x ();
            newW = w ();
        }
        if (newH < minY ())
        {
            newY = y ();
            newH = h ();
        }
        break;
    }
    case kBottomRight:
    {
        newW = origW + dW;
        newH = origH + dH;

        if (newW < minX ())
        {
            newX = x ();
            newW = w ();
        }
        if (newH < minY ())
        {
            newY = y ();
            newH = h ();
        }
        break;
    }
    }

    resize (newX, newY, newW, newH);
    redraw ();
}

int MVMDIWindow::handle (int event)
{
    static bool dragging = false;
    switch (event)
    {
    case FL_PUSH:
    {
        if (ws ())
            ws ()->activate (*this);

#define HandleForArea(rect, cursor, dS)                                        \
    if (Point (Fl::event_x (), Fl::event_y ()).isInRect (rect))                \
    {                                                                          \
        fl_cursor (cursor, 0, 255);                                            \
        dragState = dS;                                                        \
        handleDragEvent (true);                                                \
        return 1;                                                              \
    }
        HandleForArea (leftDragArea, FL_CURSOR_WE, kLeft);
        HandleForArea (topDragArea, FL_CURSOR_NS, kTop);
        HandleForArea (rightDragArea, FL_CURSOR_WE, kRight);
        HandleForArea (bottomDragArea, FL_CURSOR_NS, kBottom);
        HandleForArea (topLeftDragArea, FL_CURSOR_NWSE, kTopLeft);
        HandleForArea (topRightDragArea, FL_CURSOR_NESW, kTopRight);
        HandleForArea (bottomLeftDragArea, FL_CURSOR_NESW, kBottomLeft);
        HandleForArea (bottomRightDragArea, FL_CURSOR_NWSE, kBottomRight);

        break;
    }

    case FL_DRAG:
        if (Fl::event_button () != FL_LEFT_MOUSE)
            break;
        if (dragState != kNotDragging)
            handleDragEvent (false);
        return 1;

    case FL_RELEASE:
        dragState = kNotDragging;
        return 1;

    case FL_LEAVE:
        fl_cursor (FL_CURSOR_DEFAULT, 0, 255);
        break;

    case FL_MOVE:
    {
#define CursorForArea(cursor, area)                                            \
    if (Point (Fl::event_x (), Fl::event_y ()).isInRect (area))                \
    {                                                                          \
        fl_cursor (cursor);                                                    \
        return 1;                                                              \
    }
        CursorForArea (FL_CURSOR_WE, leftDragArea);
        CursorForArea (FL_CURSOR_WE, rightDragArea);
        CursorForArea (FL_CURSOR_NS, leftDragArea);
        CursorForArea (FL_CURSOR_NS, topDragArea);
        CursorForArea (FL_CURSOR_NWSE, bottomLeftDragArea);
        CursorForArea (FL_CURSOR_NESW, topRightDragArea);
        CursorForArea (FL_CURSOR_NESW, bottomLeftDragArea);
        CursorForArea (FL_CURSOR_NWSE, bottomRightDragArea);

        fl_cursor (FL_CURSOR_DEFAULT, 0, 255);
        break;
    }
    }

    if (!Fl_Group::handle (event) && !contents->handle (event) &&
        !Fl::event_inside (this))
        return 0;
    return 1;
}

void MVMDIWindow::redraw ()
{
    Fl_Group::redraw ();
    for (int i = 0; i < children (); i++)
    {
        child (i)->redraw ();
    }
    parent ()->redraw ();
}

void MVMDIWindow::draw ()
{
    Fl_Group::draw ();
    if (contents)
        ((Fl_Widget *)contents)->draw ();
}

void MVMDIWindow::resize (int X, int Y, int W, int H)
{
    const int dragW = 3, dragH = 3;
    int eX = X, eY = Y, eW = W, eH = H; /* EFFECTIVE size excluding borders */

    Fl_Group::resize (X, Y, W, H);

    boxInset (box (), eX, eY, eW, eH);
    titleBar.resize (eX + 2, eY + 2, eW - 4, 18);

    leftDragArea = {X, Y + (dragH * 2), dragW, H - (dragH * 4)};
    topDragArea = {X + (dragW * 2), Y, W - (dragW * 4), dragH};
    rightDragArea = {X + W - dragW, Y + (dragH * 2), dragW, H - (dragH * 4)};
    bottomDragArea = {X + (dragW * 2), Y + H - dragH, W - (dragW * 4), dragH};

    topLeftDragArea = {X, Y, dragW * 2, dragH * 2};
    topRightDragArea = {X + W - (dragW * 2), Y, dragW * 2, dragH * 2};
    bottomLeftDragArea = {X, Y + H - (dragH * 2), dragW * 2, dragH * 2};
    bottomRightDragArea = {
        X + W - (dragW * 2), Y + H - (dragH * 2), dragW * 2, dragH * 2};

    if (contents)
        contents->resize (
            eX + 2, eY + titleBar.h () + 2, eW - 4, eH - titleBar.h () - 4);
}

int MVMDIWindow::minX ()
{
    return titleBar.minX () + Fl::box_dx (box ()) + Fl::box_dw (box ()) + 4;
}

int MVMDIWindow::minY ()
{
    return 24;
}

MVMDIWorkspace * MVMDIWindow::ws ()
{
    return dynamic_cast<MVMDIWorkspace *> (parent ());
}
