#include <FL/Enumerations.H>
#include <FL/Fl_Group.H>
#include <cairo.h>
#include <fstream>
#include <sstream>
#include <string>

#include "MVWebView.hxx"

litehtml::context MVNewWebView::htmlContext;

static cairo_t * createOffscreen (int w, int h)
{
    cairo_surface_t * temp_surface =
        cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
    return cairo_create (temp_surface);
}

static std::string loadTextFile (http_loader & httpLoader, std::string url)
{
    Glib::RefPtr<Gio::InputStream> stream = httpLoader.load_file (url);
    std::string out;
    gssize sz;
    char buff[1025];
    while ((sz = stream->read (buff, 1024)) > 0)
    {
        buff[sz] = 0;
        out += buff;
    }
    return out;
}

MVNewWebView::MVNewWebView (int x, int y, int w, int h, const char * l)
    : Fl_Group (x, y, w, h, l)
{
    std::ifstream in ("/ws/MultiVIM/usr/closed/litehtml/include/master.css");
    std::ostringstream contents;
    contents << in.rdbuf ();
    in.close ();
    new Gtk::Main;

    htmlContext.load_master_stylesheet (contents.str ().c_str ());

    box (FL_FLAT_BOX);
    color (FL_BLACK);
    labeltype (FL_NO_LABEL);
    doc = litehtml::document::createFromString (
        "<H1>Loading...</H1>", this, &htmlContext);

    resize (x, y, w, h);
    printf ("W: %d\n", w);
    navigateTo ("file:///ws/MultiVIM/usr/src/Resources/HTML/Welcome.html");
}

void MVNewWebView::get_client_rect (litehtml::position & client) const
{
    client.width = w ();
    client.height = h ();
    client.x = 0;
    client.y = 0;
}

void MVNewWebView::on_anchor_click (const litehtml::tchar_t * url,
                                    const litehtml::element::ptr & el)
{
    printf ("Anchor clicked %s\n", url);
}

void MVNewWebView::set_cursor (const litehtml::tchar_t * cursor)
{
}

void MVNewWebView::import_css (litehtml::tstring & text,
                               const litehtml::tstring & url,
                               litehtml::tstring & baseurl)
{
    std::string css_url;
    make_url (url.c_str (), baseurl.c_str (), css_url);
    text = loadTextFile (httpLoader, css_url);
    if (!text.empty ())
    {
        baseurl = css_url;
    }
}

void MVNewWebView::set_caption (const litehtml::tchar_t * caption)
{
    title = caption;
    label (title.c_str ());
}

Glib::RefPtr<Gdk::Pixbuf>
MVNewWebView::get_image (const litehtml::tchar_t * url, bool redraw_on_ready)
{
    Glib::RefPtr<Gio::InputStream> stream = httpLoader.load_file (url);
    Glib::RefPtr<Gdk::Pixbuf> ptr = Gdk::Pixbuf::create_from_stream (stream);
    printf ("trying to load image. Filename: %s\n", url);
    return ptr;
}

void MVNewWebView::navigateTo (std::string url)
{
    std::string text = loadTextFile (httpLoader, url);
    selection = {};
    m_url = httpLoader.get_url ();
    m_base_url = httpLoader.get_url ();
    doc = litehtml::document::createFromString (
        text.c_str (), this, &htmlContext);
    doc->render (w ());
}

void MVNewWebView::cairoDraw (cairo_t * cr)
{
    litehtml::position pos;
    double xStart, yStart, xEnd, yEnd;
    cairo_t * temp = createOffscreen (w (), h ());
    // printf ("draw\n");

    cairo_move_to (cr, x (), y ());
    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    cairo_rectangle (cr, x (), y (), w (), h ());
    cairo_fill (cr);
    cairo_clip_extents (cr, &xStart, &yStart, &xEnd, &yEnd);

    pos.width = xEnd - xStart;
    pos.height = yEnd - yStart;
    pos.x = 0; // x (); // xStart;
    pos.y = 0; // y (); // yStart;
    doc->draw ((litehtml::uint_ptr)temp, 0, 0, &pos);
    cairo_set_source_surface (cr, cairo_get_target (temp), x (), y ());
    // cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_surface_destroy (cairo_get_target (temp));
    cairo_destroy (temp);
    // printf ("DRawing %d@%d %d@%d\n", pos.x, pos.y, pos.width, pos.height);
}

void MVNewWebView::draw ()
{
    // Fl_Group::draw ();
    cairoDraw (Fl::cairo_make_current (window ()));
}

void MVNewWebView::resize (int X, int Y, int W, int H)
{
    bool shouldTellPresenter = false;
    doc->media_changed ();
    doc->render (W);
    Fl_Group::resize (X, Y, W, H);
}

void MVNewWebView::redraw ()
{
    parent ()->redraw ();
}

void MVNewWebView::toLiteHTMLCoords (int & nx, int & ny, int & cX, int & cY)
{
    nx = Fl::event_x () - x ();
    ny = Fl::event_y () - y ();
    cX = nx;
    cY = ny;
}

int MVNewWebView::handle (int event)
{
    // printf ("View got event %d\n", event);

    if (event == FL_DRAG)
    {
        int x, y, cX, cY;
        toLiteHTMLCoords (x, y, cX, cY);
        mouseDragAt (x, y, cX, cY);
    }
    if (event == FL_MOVE || event == FL_DRAG)
    {
        int x, y, cX, cY;
        litehtml::position::vector redraw_boxes;

        toLiteHTMLCoords (x, y, cX, cY);
        doc->on_mouse_over (x, y, cX, cY, redraw_boxes);
        redraw ();
        return 1;
    }
    if (event == FL_PUSH)
    {
        int x, y, cX, cY;
        litehtml::position::vector redraw_boxes;

        toLiteHTMLCoords (x, y, cX, cY);
        Fl::focus (this);
        mousePressAt (x, y, cX, cY);
        doc->on_lbutton_down (x, y, cX, cY, redraw_boxes);
        redraw ();
        return 1;
    }
    else if (event == FL_RELEASE)
    {
        int x, y, cX, cY;
        litehtml::position::vector redraw_boxes;

        toLiteHTMLCoords (x, y, cX, cY);
        Fl::focus (this);
        mouseReleaseAt (x, y, cX, cY);

        doc->on_lbutton_up (x, y, cX, cY, redraw_boxes);
        redraw ();
        return 1;
    }
    else if (event == FL_FOCUS)
    {
        Fl::focus (this);
        return 1;
    }

    return Fl_Group::handle (event);
    //   return 1;
}