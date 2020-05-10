#include "MVMDIWorkspace.hh"
#include "MVMDIWindow.hh"

void MVMDIWorkspace::activate (MVMDIWindow & win)
{
    for (int i = 0; i < children (); i++)
    {
        MVMDIWindow * w = dynamic_cast<MVMDIWindow *> (child (i));
        if (!w)
            continue;
        if (!(w == &win))
            w->didDefocus ();
    }
    insert (win, children ());
    win.didFocus ();
    win.redraw ();
    redraw ();
}

void MVMDIWorkspace::addWindow (MVMDIWindow & win)
{
    for (int i = 0; i < children (); i++)
    {
        MVMDIWindow * w = dynamic_cast<MVMDIWindow *> (child (i));
        if (!w)
            continue;
        w->didDefocus ();
    }
    add (win);
    win.didFocus ();
}

MVMDIWindow * MVMDIWorkspace::createBigWindow ()
{
    MVMDIWindow * win =
        new MVMDIWindow (x (), y (), w () - 10, h () - 10, "TextView");
    addWindow (*win);
    win->end ();
    return win;
}

int MVMDIWorkspace::handle (int event)
{
    if (event == FL_PUSH)
        for (int i = children (); i != -1; --i)
        {
            MVMDIWindow * w = dynamic_cast<MVMDIWindow *> (child (i));
            if (!w)
                continue;
            if (Fl::event_inside (w) && !w->isFocused ())
                w->didFocus ();
        }

    return Fl_Scroll::handle (event);
}
