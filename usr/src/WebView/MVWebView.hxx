#ifndef NEWWEBVIEW_HXX_
#define NEWWEBVIEW_HXX_

#include "BSD/WebView/http_loader.hxx"
#include "container_linux.hxx"
#include <FL/Fl_Group.H>

class MVNewWebView : public Fl_Group, container_linux
{
    std::string title;
    static litehtml::context htmlContext;
    http_loader httpLoader;

    void toLiteHTMLCoords (int & x, int & y, int & cX, int & cY);

  public:
    MVNewWebView (int x, int y, int w, int h, const char * l = nullptr);

    virtual void get_client_rect (litehtml::position & client) const;
    virtual void on_anchor_click (const litehtml::tchar_t * url,
                                  const litehtml::element::ptr & el);
    virtual void set_cursor (const litehtml::tchar_t * cursor);
    virtual void import_css (litehtml::tstring & text,
                             const litehtml::tstring & url,
                             litehtml::tstring & baseurl);
    virtual void set_caption (const litehtml::tchar_t * caption);
    virtual Glib::RefPtr<Gdk::Pixbuf> get_image (const litehtml::tchar_t * url,
                                                 bool redraw_on_ready);

    void navigateTo (std::string url);

    void cairoDraw (cairo_t * cr);
    void draw ();
    void redraw ();
    int handle (int event);

    void resize (int x, int y, int w, int h);
};

#endif