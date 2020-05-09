
#include "MVMainWindowView.hh"

MVMainWindowView::MVMainWindowView (int w, int h, __weak id presenter)
    : Fl_Cairo_Window (w, h, "MultiVIM"), presenter (presenter),
      mainArea (mainAreaX (), mainAreaY (), w, mainAreaHeight ())
{
    begin ();

    // Fl::cairo_autolink_context (true);

    mainArea.end ();

    menuBar = new Fl_Menu_Bar (0, 0, w, menuBarHeight);

    menuBar->add ("&File", 0, nullptr, nullptr, FL_SUBMENU);
    menuBar->add ("File/&New", 0, nullptr, nullptr);
    menuBar->add ("&Edit", 0, nullptr, nullptr, FL_SUBMENU);
    menuBar->add ("&Options", 0, nullptr, nullptr, FL_SUBMENU);
    menuBar->add ("&Buffers", 0, nullptr, nullptr, FL_SUBMENU);
    menuBar->add ("&Window", 0, nullptr, nullptr, FL_SUBMENU);
    menuBar->add ("&Help", 0, nullptr, nullptr, FL_SUBMENU);

    tBar = new MVToolbar (0, toolBarY (), w, toolBarHeight);
    tBar->addButton ("new", "New file");
    tBar->addButton ("open", "Open file");
    tBar->addButton ("save", "Save file");
    tBar->addSpacer ();
    tBar->addButton ("cut", "Cut");
    tBar->addButton ("copy", "Copy");
    tBar->addButton ("paste", "Paste");
    tBar->addSpacer ();
    tBar->addButton ("undo", "Undo");
    tBar->addButton ("redo", "Redo");

    // mainArea = new TabContainer (0, mainAreaY (), w, mainAreaHeight ());

    msgBar = new Fl_Box (0, msgBarY (), w, msgBarHeight);
    msgBar->box (FL_BORDER_BOX);
    msgBar->color (fl_rgb_color (0, 128, 128));
    msgBar->label ("Status");
    msgBar->align (FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    // msgBar->end ();

    Statusbar * sbar = new Statusbar (0, statusBarY (), w, statusBarHeight);
    sbar->addEntry (new StatusbarEntry (0, 0, 0, 0, "Line: 2"));
    sbar->addEntry (new StatusbarEntry (0, 0, 0, 0, "Col: 37"));
    sbar->addEntry (new StatusbarEntry (0, 0, 0, 0, "Mode: command"));
    sbar->addEntry (new StatusbarEntry (0, 0, 0, 0, "NVim"));
    resizable (mainArea);

    // set_draw_cb (draw_cb);
    end ();
}

static void cb (Fl_Widget * caller, void * v)
{
}

void MVMainWindowView::draw ()
{
    Fl_Cairo_Window::draw ();
}
