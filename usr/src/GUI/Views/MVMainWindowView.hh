#ifndef MVMAINWINDOWVIEW_H_
#define MVMAINWINDOWVIEW_H_

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Cairo_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>

#include "../Controls/MVMDIWindow.hh"
#include "../Controls/MVMDIWorkspace.hh"
#include "../Controls/MVStatusBar.h"
#include "../Controls/MVTabContainer.h"
#include "../Controls/MVToolbar.h"
#include "LGPL/Controls/Fl_Drag_Tabs.H"
#include "LGPL/Controls/Flw_Split.H"
#include "MVTextView.hh"
#include "MVWebView.hxx"

class MVNewWebView;

class MVMainWindowView : public Fl_Cairo_Window
{
    const int menuBarHeight = 20;
    const int toolBarHeight = 30;
    const int msgBarHeight = 24;
    const int statusBarHeight = 24;

    int toolBarY ()
    {
        return menuBarHeight;
    }

    int msgBarY ()
    {
        return h () - (msgBarHeight + statusBarHeight);
    }

    int statusBarY ()
    {
        return h () - statusBarHeight;
    }

  public:
    MVMainWindowView (int w, int h, id presenter);

    void draw ();
    inline void textViewCairoDraw (cairo_t * cr)
    {
        return textView->cairoDraw (cr);
    }

    int mainAreaX ()
    {
        return 0;
    }

    int mainAreaY ()
    {
        return menuBarHeight + toolBarHeight;
    }

    int mainAreaHeight ()
    {
        return h () - mainAreaY () - (msgBarHeight + statusBarHeight);
    }

    void addTopLevelWebView (MVWebView * aView)
    {
        MVMDIWindow * win = mainArea.createBigWindow ("Welcome");
        win->addFullSizeChild (aView);
    }

    void addTopLevelNewWebView (MVNewWebView * aView);

    void addTopLevelTextView (TextView * aView)
    {
        MVMDIWindow * win = mainArea.createBigWindow ("Buffer 1");
        win->addFullSizeChild (aView);
        /// extView = aView;
        // mainArea->addTab (textView);
    }

    id presenter;
    MVMDIWorkspace mainArea;
    // TabContainer * mainArea;
    Fl_Menu_Bar * menuBar;
    TextView * textView;
    MVToolbar * tBar;
    Fl_Box * msgBar;
    Flw_Split * layout1;
    Flw_Split * layout2;
    Flw_Split * vertSplit;
};

#endif