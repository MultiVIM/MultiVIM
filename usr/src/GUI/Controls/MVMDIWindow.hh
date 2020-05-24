#ifndef MDIWINDOW_HH_
#define MDIWINDOW_HH_

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Rect.H>
#include <FL/fl_draw.H>

class MVMDIWorkspace;
class MVMDIWindow;

class MVWExtras
{
  public:
    /* Minimum size of this widget. */
    virtual int minX ()
    {
        return 1;
    }
    /* Minimum height of this widget. */
    virtual int minY ()
    {
        return 1;
    }
};

class MVMDITitle : public Fl_Group, public MVWExtras
{
    friend class MVMDIWindow;

    /* Width and height of button */
    int buttonSize ()
    {
        return h () - 4;
    }
    /* Width and height of all visible buttons */
    int allButtonsSize ();

    /* Area used by the buttons . */
    Fl_Rect buttonArea;

  public:
    MVMDITitle (int x, int y, int w, int h, const char * l = 0);

    virtual int handle (int event);
    virtual void layout ();
    virtual void resize (int X, int Y, int W, int H);
    virtual void draw ();

    virtual int minX ();
    virtual int minY ();

    Fl_Button _close, _max, _min;
    MVMDIWindow * _owner;
};

class MVMDIWindow : public Fl_Group, public MVWExtras
{
    friend class MVMDITitle;
    friend class MVMDIWorkspace;

    enum
    {
        kNotDragging = 0,
        kLeft,
        kTop,
        kRight,
        kBottom,
        kTopLeft,
        kTopRight,
        kBottomLeft,
        kBottomRight,
    } dragState;

    void handleDragEvent (bool reset);

  protected:
    void didDefocus ();
    void didFocus ();
    bool isFocused ()
    {
        return titleBar.active ();
    }
    void setCursorForDragState ();

  public:
    MVMDIWindow (int x, int y, int w, int h, const char * l = 0);

    void addFullSizeChild (Fl_Group * w);

    virtual int handle (int event);
    void redraw ();
    void draw ();
    void resize (int X, int Y, int W, int H);

    virtual int minX ();
    virtual int minY ();

    MVMDIWorkspace * ws (); /* return parent workspace */

    Fl_Rect leftDragArea;
    Fl_Rect topDragArea;
    Fl_Rect rightDragArea;
    Fl_Rect bottomDragArea;

    Fl_Rect topLeftDragArea;
    Fl_Rect topRightDragArea;
    Fl_Rect bottomLeftDragArea;
    Fl_Rect bottomRightDragArea;

    MVMDITitle titleBar;
    Fl_Group * contents;
};

#endif