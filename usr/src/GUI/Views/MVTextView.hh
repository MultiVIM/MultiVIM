#ifndef TEXTVIEW_H_
#define TEXTVIEW_H_

#include <GL/gl.h>
#include <math.h>
#include <string>
#include <vector>

#include <cairo.h>
#include <pango/pangocairo.h>

#include <FL/Fl.H>
#include <FL/Fl_Cairo_Window.H>
#include <FL/Fl_Widget.H>

class MVGridPresenter;

class TextView : public Fl_Cairo_Window
{
    int rowToY (int row)
    {
        return charHeight * row;
    }
    int colToX (int col)
    {
        return charWidth * col;
    }

  public:
    /* The previous location of the cursor. */
    double cursorRowOld, cursorColumnOld;
    /* The point the cursor is either:
     * A) moving to; or
     * B) already at.
     */
    int cursorRow, cursorColumn;

    double factorX, factorY;
    int animCount;
    bool midAnimation;

    /*
     * Total number of rows and columns.
     */
    int rowCount, columnCount;
    /*
     * Size of a single (monospaced) character.
     */
    float charHeight, charWidth;
    int defaultFg, defaultBg;
    std::vector<std::string> rowText;
    std::vector<PangoLayout *> pangoLayouts;
    std::vector<cairo_t *> layoutContexts;
    PangoFontDescription * pangoFontDesc;
    MVGridPresenter * presenter;

    TextView (int x, int y, int w, int h, MVGridPresenter * presenter,
              const char * l = nullptr);

    void clearRowText (int row);
    void addRowText (int row, const char * txt, int fgColour, int bgColour);
    void renderRowText (int row);

    void moveCursorToRowColumn (int row, int col);

    void cairoDraw (cairo_t * cr);
    void draw ();
    void redraw ();
    int handle (int event);

    void resize (int x, int y, int w, int h);
};

#endif