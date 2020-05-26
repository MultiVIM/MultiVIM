#include <FL/Enumerations.H>
#include <FL/Fl_Group.H>
#include <iomanip>
#include <iostream>
#include <sstream>

#import "../Presenters/MVGridPresenter.hh"

#include "MVTextView.hh"

static cairo_t * createLayoutContext ()
{
    cairo_surface_t * temp_surface;
    cairo_t * context;

    temp_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 0, 0);
    context = cairo_create (temp_surface);
    cairo_surface_destroy (temp_surface);

    return context;
}

static void getLayoutWidthHeight (PangoLayout * layout, int * width,
                                  int * height)
{
    pango_layout_get_size (layout, width, height);
    *width /= PANGO_SCALE;
    *height /= PANGO_SCALE;
}

static void draw_cb (Fl_Group * cw, cairo_t * cr)
{
    TextView * text = static_cast<TextView *> (cw);
    text->cairoDraw (cr);
}

TextView::TextView (int x, int y, int w, int h, MVGridPresenter * presenter,
                    const char * l)
    : Fl_Group (x, y, w, h, l), presenter (presenter)
{
    PangoLayout * layout;
    cairo_t * ctxLayout;
    int pHeight, pWidth;
    pangoFontDesc = pango_font_description_from_string ("Monospace");

    ctxLayout = createLayoutContext ();

    layout = pango_cairo_create_layout (ctxLayout);
    pango_layout_set_font_description (layout, pangoFontDesc);
    pango_layout_set_text (layout, "@", -1);
    pango_layout_get_size (layout, &pWidth, &pHeight);
    g_object_unref (layout);
    cairo_destroy (ctxLayout);

    charHeight = pHeight / PANGO_SCALE;
    charWidth = pWidth / PANGO_SCALE;
    rowCount = 0;
    columnCount = 0;
    cursorRow = 0;
    cursorColumn = 0;
    midAnimation = false;

    box (FL_FLAT_BOX);
    color (FL_BLACK);
    labeltype (FL_NO_LABEL);
    // set_draw_cb (draw_cb);

    resize (x, y, w, h);
}

void anim_timer (void * data)
{
    TextView * view = static_cast<TextView *> (data);
    view->redraw ();
}

void TextView::cairoDraw (cairo_t * cr)
{
    cairo_move_to (cr, x (), y ());
    cairo_set_source_rgba (cr, 0, 0, 0, 1);
    cairo_rectangle (cr, x (), y (), w (), h ());
    cairo_fill (cr);
    for (int i = 0; i < rowCount; i++)
    {
        cairo_move_to (cr, x (), y () + i * charHeight);
        cairo_set_source_rgba (cr, 1, 1, 1, 1);
        pango_cairo_show_layout (cr, pangoLayouts[i]);
    }

    cairo_set_source_rgba (cr, 1, 1, 1, .5);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    if (midAnimation)
    {
        double dX = cursorColumnOld - cursorColumn,
               dY = cursorRowOld - cursorRow;

        cairo_move_to (cr,
                       x () + colToX (cursorColumnOld) + (charWidth * 0.5),
                       y () + rowToY (cursorRowOld) + (charHeight * 0.5));
        cairo_line_to (cr,
                       x () + colToX (cursorColumn) + (charWidth * 0.5),
                       y () + rowToY (cursorRow) + (charHeight * 0.5));
        cairo_set_line_width (cr, charWidth);
        cairo_stroke (cr);

        cursorColumnOld -= factorX;
        cursorRowOld -= factorY;
        animCount++;
        if (animCount > 12)
            midAnimation = false;
        Fl::add_timeout (0.02, anim_timer, this);
    }

    cairo_rectangle (cr,
                     x () + colToX (cursorColumn),
                     y () + rowToY (cursorRow),
                     (charWidth),
                     charHeight);
    cairo_fill (cr);
}

void TextView::draw ()
{
    // Fl_Group::draw ();
    cairoDraw (Fl::cairo_make_current (window ()));
}

void TextView::resize (int X, int Y, int W, int H)
{
    bool shouldTellPresenter = false;
    int oldRows = rowCount, oldCols = columnCount;

    rowCount = ceil (H / charHeight) - 1;
    columnCount = ceil (W / charWidth) - 1;

    for (auto layout : pangoLayouts)
    {
        g_object_unref (layout);
    }
    for (auto context : layoutContexts)
    {
        cairo_destroy (context);
    }
    rowText.resize (rowCount);
    pangoLayouts.resize (rowCount);
    layoutContexts.resize (rowCount);
    for (int i = 0; i < rowCount; i++)
    {
        PangoLayout * layout;
        cairo_t * ctxLayout;

        ctxLayout = createLayoutContext ();
        layout = pango_cairo_create_layout (ctxLayout);
        pango_layout_set_font_description (layout, pangoFontDesc);
        pango_layout_set_markup (layout, rowText[i].c_str (), -1);

        layoutContexts[i] = ctxLayout;
        pangoLayouts[i] = layout;
    }

    if (oldCols && ((rowCount != oldRows) || (columnCount != oldCols)) &&
        presenter)
    {
        presenter->viewDidResizeToRowsColumns (rowCount, columnCount);
    }

    Fl_Group::resize (X, Y, W, H);
}

void TextView::clearRowText (int row)
{
    rowText[row] = "";
}
void TextView::addRowText (int row, const char * txt, int fgColour,
                           int bgColour)
{
    std::ostringstream fg, bg;
    gchar * text = g_markup_escape_text (txt, -1);
    fg << std::setfill ('0') << std::setw (6) << std::right << std::hex
       << fgColour;
    bg << std::setfill ('0') << std::setw (6) << std::right << std::hex
       << bgColour;
    rowText[row] += "<span foreground=\"#" + fg.str () + "\"";
    rowText[row] += " background=\"#" + bg.str () + "\"";
    rowText[row] += ">";
    rowText[row] += text;
    rowText[row] += "</span>";
    free (text);
}

void TextView::renderRowText (int row)
{
    parent ()->redraw ();
    pango_layout_set_markup (pangoLayouts[row], rowText[row].c_str (), -1);
}

void TextView::redraw ()
{
    parent ()->redraw ();
}

void TextView::moveCursorToRowColumn (int row, int col)
{
    double dX, dY;
    cursorRowOld = cursorRow;
    cursorColumnOld = cursorColumn;
    cursorRow = row;
    cursorColumn = col;

    dX = cursorColumnOld - cursorColumn, dY = cursorRowOld - cursorRow;

    if (abs (dX) > 1 || abs (dY) > 1)
    {
        factorX = dX / 12;
        factorY = dY / 12;
        animCount = 0;
        midAnimation = true;
    }
    else
        midAnimation = false;
}

int TextView::handle (int event)
{
    // printf ("View got event %d\n", event);

    if (event == FL_PUSH)
        Fl::focus (this);
    if (event == FL_KEYBOARD)
    {
        switch (Fl::event_key ())
        {
#define IfThen(FlKey, SendKey)                                                 \
    case FlKey:                                                                \
        presenter->viewDidInputKey (SendKey);                                  \
        break
            IfThen (FL_Escape, "<Esc>");
            IfThen (FL_Up, "<Up>");
            IfThen (FL_Down, "<Down>");
            IfThen (FL_Left, "<Left>");
            IfThen (FL_Right, "<Right>");

        default:
            presenter->viewDidInputKey (Fl::event_text ());
        };
        /*printf ("Event keycode <%d>, text <%d:%s>\n",
                Fl::event_key (),
                Fl::event_length (),
                Fl::event_text ());*/
    }
    // Fl_Group::handle (event);
    return 1;
}