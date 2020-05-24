#include "Fl_Html_View.H"
#include "Fl_Html_Formatter.H"
#include "Fl_Html_Object.H"

#include "Fl_Html_Parser.H"
#include <FL/Fl.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/filename.H>
#include <FL/fl_draw.H>
#include <math.h>
#include <string.h>

//#include <time/time.h>

#define F 1.2
static const float font_sizes[] = {1,
                                   F,
                                   F * F,
                                   F * F * F,
                                   F * F * F * F,
                                   F * F * F * F * F,
                                   F * F * F * F * F * F,
                                   F * F * F * F * F * F};
#undef F

static const char * measure_string =
    "abcdefghijklhhhhhhhhhhhhhhhhmmmmnoprstuvwxyz";

class Fl_Html_Drawing_Device : public Fl_Html_Drawing_Device_
{

    int height_;
    int origin_x_;
    int origin_y_;
    float zoom_;
    int fltk_font_;
    int int_font_size_;
    int descent_;
    int space_;
    int measure_set;
    float font_zoom_coef_;
    bool zoomed_font_measurement_;

  public:
    void zoomed_font_measurement (bool z)
    {
        zoomed_font_measurement_ = z;
        font_zoom_coef_ = 0;
        measure_set = 0;
    }
    bool zoomed_font_measurement () const
    {
        return zoomed_font_measurement_;
    }
    int descent ()
    {
        if (descent_ < 0)
        {
            bool do_draw_descent =
                true; //(measure_set==2); // this is work-around of some bug -
                      // at least on windows...
            if (zoomed_font_measurement_)
            {
                // do_draw_descent = false;
                if (measure_set != 2)
                {
                    measure_set = 2;
                    fl_font ((Fl_Font)fltk_font_, int_font_size_);
                }
                descent_ = ceil (float (fl_descent ()) / zoom_);
            }
            else
            {
                if (measure_set != 1)
                {
                    measure_set = 1;
                    fl_font ((Fl_Font)fltk_font_, font_size_);
                }
                descent_ = ceil (fl_descent ());
            }
            // descent_ = fl_descent();
            if (do_draw_descent)
            {
                fl_font ((Fl_Font)fltk_font_, int_font_size_);
                // measure_set = 2;
            }
        }
        return descent_;
    }
    int space ()
    {

        if (space_ < 0)
        {
            if (zoomed_font_measurement_)
            {
                if (measure_set != 2)
                {
                    measure_set = 2;
                    fl_font ((Fl_Font)fltk_font_, int_font_size_);
                }
                space_ = ceil (float (fl_width (" ")) / zoom_);
            }
            else
            {
                if (measure_set != 1)
                {
                    measure_set = 1;
                    fl_font ((Fl_Font)fltk_font_, int_font_size_);
                }
                // float z = 1.0/zoom_;
                space_ = ceil ((fl_width (" ") / zoom_));
                // space_ = int(wc + 0.5);
            }
        }
        return space_;
    }
    void draw_box (Fl_Boxtype type, int x, int y, int w, int h);
    void draw_image (Fl_Html_Image_ * image, Fl_Html_Image_ *& image_cache,
                     int x, int y, int w, int h);
    void font (int face, float size)
    {
        int new_font = face & 0xFF;
        int new_size;
        if (zoomed_font_measurement_)
        {
            new_size = int (size * zoom_);
        }
        else
        {
            if (font_size_ != size || font_face_ != face)
            {
                if (zoom_ == 1.0)
                    font_zoom_coef_ = 1;
                else
                    font_zoom_coef_ = 0;
            }

            if (!font_zoom_coef_)
            {
                fl_font (face, size);
                float w1 = fl_width (measure_string);
                int iz =
                    /*(zoom_<1)?*/ ceil (size * zoom_); //:ceil(size*zoom_);
                // iz &= ~1;

                // if(iz==size){
                if (zoom_ < 1)
                    iz -= 3;
                else
                    iz += 3;
                //}

                iz &= ~1;
                if (iz < 1)
                    iz = 1;
                float zz = float (iz) / float (size);
                fl_font (face, iz);
                float w2 = fl_width (measure_string);
                font_zoom_coef_ = (float (w1 * (zz - 1))) / (float (w2 - w1));
                measure_set = 0;
                height_ = -1; // int(fl_height()*z + 0.5);
                descent_ = -1;
                space_ = -1;
            }
            // float coef = (zoom_>(1.0/0.88))? 0.88 : 1; // adjusting for
            // zoomed drawing

            // int new_size = (int)(size * zoom_ * coef + 0.0001);
            new_size =
                (int)(size + size * (zoom_ - 1) * font_zoom_coef_ + 0.00001);
            // if(new_size>12)
            //  new_size &= 0xFFFFFFFC; //floor(size * zoom_ * coef - 2);
        }

        if (new_font != fltk_font_ || new_size != int_font_size_)
        {
            fltk_font_ = new_font;
            measure_set = 0;

            // fl_font((Fl_Font)fltk_font_, new_size);
            // float z = 1.0/zoom_;
            height_ = -1; // int(fl_height()*z + 0.5);
            descent_ = -1;
            space_ = -1;
        }
        int_font_size_ = new_size;
        font_face_ = face;
        font_size_ = size;
    }
    void font (int face, int html_size, int relative)
    {
        if (relative > 0)
            font (face, font_size_ * font_sizes[html_size]);
        else if (relative < 0)
            font (face, font_size_ / font_sizes[html_size]);
        else
        {
            html_size -= 3;
            if (html_size > 0)
                font (face, base_font_size_ * font_sizes[html_size]);
            else if (html_size < 0)
                font (face, base_font_size_ / font_sizes[-html_size]);
            else
                font (face, base_font_size_);
        }
    }

    void base_font (int face, int size)
    {
        base_font_face_ = face;
        base_font_size_ = size;
        descent_ = -1;
        if (zoom_ != 1)
            font_zoom_coef_ = 0;
    }

    void origin (int x, int y)
    {
        origin_x_ = x;
        origin_y_ = y;
    }
    void zoom (float z)
    {
        zoom_ = z;
        if (zoom_ != 1)
            font_zoom_coef_ = 0;
        else
            font_zoom_coef_ = 1;
    }

    float zoom () const
    {
        return zoom_;
    }
    void translate (int dx, int dy)
    {
        origin_x_ += dx;
        origin_y_ += dy;
        float z = 1.0 / zoom ();
        x_ -= dx * z;
        y_ -= dy * z;
    }
    /*
      void _transform(int &x, int &y, int &w, int &h) {
        float W = w * zoom_;
        if(W < 0.2) return;
        float H = h * zoom_;
        if(H<0.2) return;
        w = int(W + 0.5);
        h = int(H + 0.5);
        if(!w) w = 1;
        if(!h) h = 1;
        _transform(x, y);
      }
    */
    void _transform (int & x, int & y)
    {
        x = floor (x * zoom_ + 0.5);
        y = floor (y * zoom_ + 0.5);
    }
    void rectf_ (int x, int y, int w, int h)
    {
        if (w < 0)
        {
            x += w;
            w = -w;
        }
        if (h < 0)
        {
            y += h;
            h = -h;
        }

        if (!(w | h))
            return;
        if (w == 1)
        {
            fl_yxline (x + origin_x_, y + origin_y_, origin_y_ + y + h - 1);
            return;
        }
        if (h == 1)
        {
            fl_xyline (origin_x_ + x, origin_y_ + y, origin_x_ + x + w - 1);
            return;
        }

        fl_rectf (origin_x_ + x, origin_y_ + y, w, h);
    }
    void rectf (int x, int y, int w, int h, bool extend = false)
    {
        if (!w)
            return;
        if (!h)
            return;
        /*
            float W = w * zoom_;
            float H = h * zoom_;

            w = int(W + 0.5);
            h = int(H + 0.5);
            //if(!w) w = 1;
            //if(!h) h = 1;
            _transform(x, y);
            rectf_(x, y, w, h);
        */

        int x1 = x + w;
        int y1 = y + h;
        if (extend)
        {
            x--;
            y--;
            _transform (x, y);
            x = floor (x + zoom_ + 0.5);
            y = floor (y + zoom_ + 0.5);
            _transform (x1, y1);
        }
        else
        {
            _transform (x, y);
            x1--;
            y1--;
            _transform (x1, y1);
            x1 = floor (x1 + zoom_ + 0.5);
            y1 = floor (y1 + zoom_ + 0.5);
        }
        x1 -= x;
        y1 -= y;
        if (!x1)
            x1 = 1;
        if (!y1)
            y1 = 1;
        rectf_ (x, y, x1, y1);
    }

    void rect (int x, int y, int w, int h)
    {
        rectf (x, y, w, 1);
        rectf (x, y + h - 1, w, 1);
        rectf (x, y, 1, h);
        rectf (x + w - 1, y, 1, h);
    }
    /*
      void line(int x, int y, int w, int width = 1) {
        rectf(x, y, w + 1, 1);
      }
      */
    /*
      void vline(int x, int y, int h, int width = 1) {
        rectf(x, y, 1, h);
      }

    */

    void measure (const char * word, int n, int & w, int & h)
    {
        if (zoomed_font_measurement_)
        {
            if (measure_set != 2)
            {
                measure_set = 2;
                fl_font ((Fl_Font)fltk_font_, int_font_size_);
            }
            w = ceil (fl_width (word, n) / zoom_);
            h = ceil (fl_height () / zoom_);
        }
        else
        {
            if (measure_set != 1)
            {
                measure_set = 1;
                fl_font ((Fl_Font)fltk_font_, int_font_size_);
            }
            w = ceil (fl_width (word, n) / zoom_);
            h = ceil (fl_height () / zoom_);
        }
    }
    void draw (const char * word, int n, int x, int y)
    {
        if (!measure_set)
        {
            fl_font ((Fl_Font)fltk_font_, int_font_size_);
        }
        fl_draw (word,
                 n,
                 origin_x_ + (int)(x * zoom_ + 0.5),
                 origin_y_ + (int)(zoom_ * (y - descent ()) + 0.5));
        if (underline ())
        {
            int ww = ceil (fl_width (word, n) / zoom ());
            // measure(word, n, ww, hh);
            if (underline_y_ == y)
            {
                int dx = x - underline_x_;
                x = underline_x_; // join underline to previous word
                ww += dx;
            }
            // line(x, y - 1, ww);
            int width = (font_size_ + 7) / 14;
            if (!width)
                width = 1;
            rectf (x, y - (3 * width) / 2, ww + 1, width);
            underline_x_ = x + ww;
            underline_y_ = y;
        }
    }
    void color (int c)
    {
        if (c != color_)
        {
            color_ = c;
            fl_color (c << 8);
        }
    }

    int color_gray () const
    {
        unsigned char r, g, b;
        Fl::get_color (FL_DARK3, r, g, b);
        int rr = r;
        int gg = g;
        int bb = b;
        return ((rr << 16) | (gg << 8) | bb);
    }

    void init_draw (int base_font_face, int base_font_size, int base_font_color)
    {
        // fl_line_style(0);
        base_font_face_ = base_font_face;
        base_font_size_ = base_font_size;
        base_font_color_ = base_font_color;

        init_draw ();
        // color(0);
    }
    void init_draw ()
    {
        descent_ = -1;
        space_ = -1;
        measure_set = 0;
        selection_color_calculated_ = false;
        font_zoom_coef_ = 0;
        fltk_font_ = (~base_font_face_) &
                     0xFF; // assure that it is different and its not empty call
        font (base_font_face_, base_font_size_);
        color_ = (~base_font_color_) & 0xFF; // assure it is different
        color (base_font_color_);
        underline_x_ = 0;

        // color(0);
    }
    Fl_Html_Drawing_Device ()
        : height_ (0), origin_x_ (0), origin_y_ (0), zoom_ (1), fltk_font_ (0),
          int_font_size_ (0), descent_ (-1), space_ (-1), font_zoom_coef_ (1.0),
          zoomed_font_measurement_ (true)
    {
    }
};

Fl_Html_Drawing_Device_ * fl_html_drawing_device ()
{
    static Fl_Html_Drawing_Device * d = 0;
    if (!d)
        d = new Fl_Html_Drawing_Device ();
    d->init_draw ();
    return d;
}

Fl_Html_Formatter * fl_html_state ()
{
    static Fl_Html_Formatter * s = 0;
    if (!s)
        s = new Fl_Html_Formatter (fl_html_drawing_device (), 0);
    return s;
}

///// Some useful functions////////

class Fl_Html_Image : public Fl_Html_Image_
{
  public:
    Fl_Shared_Image * im;
    void draw (int x, int y)
    {
        if (im)
            im->draw (x, y);
    }
    int w () const
    {
        return im ? im->w () : 48;
    }
    int h () const
    {
        return im ? im->h () : 48;
    }
    Fl_Html_Image (Fl_Shared_Image * i = 0) : im (i)
    {
    }
    ~Fl_Html_Image ()
    {
        if (im)
            im->release ();
    }
};

void Fl_Html_Drawing_Device::draw_box (Fl_Boxtype type, int x, int y, int w,
                                       int h)
{
    _transform (x, y);
    _transform (w, h);
    Fl_Color color = fl_color ();
    fl_draw_box (type, x + origin_x_, y + origin_y_, w, h, FL_SELECTION_COLOR);
    fl_color (color);
}

void Fl_Html_Drawing_Device::draw_image (Fl_Html_Image_ * image,
                                         Fl_Html_Image_ *& image_cache, int x,
                                         int y, int w, int h)
{
    if (!image)
        return;
    _transform (x, y);
    _transform (w, h);
    Fl_Html_Image * im = (Fl_Html_Image *)image;
    Fl_Html_Image * im_cache = (Fl_Html_Image *)image_cache;
    if (im_cache)
    {
        if (im_cache->w () == w && im_cache->h () == h)
        {
            im_cache->draw (x + origin_x_, y + origin_y_);
            return;
        }
        delete im_cache;
        image_cache = 0;
    }
    if (im->w () == w && im->h () == h)
    {
        im->draw (x + origin_x_, y + origin_y_);
        return;
    }
    if (im->im)
    {
        image_cache = im_cache =
            new Fl_Html_Image (Fl_Shared_Image::get (im->im->name (), w, h));
        im_cache->draw (x + origin_x_, y + origin_y_);
    }
}

static int inline_image_count = 1;

Fl_Html_Image_ * fl_create_html_image (const char * url, int url_len, int type)
{
    if (!type)
        return 0;
    char buffer[64];
    const char * b = buffer;
    switch (type)
    {
    case HTML_URI_DATA_JPEG:
        snprintf (
            buffer, sizeof (buffer), "/html_data_image_%i", inline_image_count);
        inline_image_count++;
        new Fl_JPEG_Image (buffer, (const unsigned char *)url);
        break;
    case HTML_URI_DATA_PNG:
        snprintf (
            buffer, sizeof (buffer), "/html_data_image_%i", inline_image_count);
        inline_image_count++;
        new Fl_PNG_Image (buffer, (const unsigned char *)url, url_len);
        break;
    case HTML_URI_FILE:
        Fl_Shared_Image::get (url);
        b = url;
        break;
    default:
        return 0; // dont know what to do for now
    }
    Fl_Shared_Image * im = Fl_Shared_Image::find (b);
    if (im)
    {
        im->release (); // one instance is enought;
        return new Fl_Html_Image (im);
    }
    return 0;
}

Fl_Html_Parser * fl_html_parser ()
{
    static Fl_Html_Parser * p = 0;
    if (!p)
    {
        p = new Fl_Html_Parser (Fl_Html_Parser::default_create_object_table ());
        p->image_creator (&fl_create_html_image);
    }
    return p;
}

// ----------------------------------------------------------------

typedef Fl_Html_Object::Iterator Html_Object_Iterator;

void Fl_Html_View::first_drawable ()
{

    if (!first_drawable_)
    {
        ((Fl_Html_Object::Iterator *)it)->reset ();
        int y = floor ((float (topline_ - margin_) / zoom_) + 0.5);
        first_drawable_ = Fl_Html_Object::first_drawable (
            (Fl_Html_Object::Iterator *)it, html_, y);

        if (first_drawable_)
        {
            int xx, yy;
            Fl_Html_Object::global_coordinates (
                (Fl_Html_Object::Iterator *)it, first_drawable_, xx, yy);
            first_drawable_offset_ = y - yy;
        }
    }
}

void Fl_Html_View::zoom (float z)
{
    if (fabs (z - zoom_) < 0.0001)
        return;
    bool do_init_format = drawing_device_->zoomed_font_measurement ();
    bool do_reformat = do_init_format;
    if (format_width () <= 0)
        do_reformat = true;
    if (do_reformat && original_leftline_ < 0)
    {
        original_leftline_ = leftline ();
        original_leftline_zoom_ = zoom_;
    }
    zoom_ = z;
    if (do_init_format)
        initialized_ = 0;
    if (do_reformat)
    {
        formatted_ = 0;
        drawing_device_->zoom (z);
        reformat ();
        int il = (int)(original_leftline_ * (zoom_ / original_leftline_zoom_));
        if (il < 0)
            il = 0;
        leftline (il);
    }
}

void Fl_Html_View::zoomed_font_measurement (bool z)
{
    drawing_device_->zoomed_font_measurement (z);
}
bool Fl_Html_View::zoomed_font_measurement () const
{
    return drawing_device_->zoomed_font_measurement ();
}

void Fl_Html_View::drawable_topline ()
{
    if (!first_drawable_)
        return;
    int xx, yy;
    Fl_Html_Object::global_coordinates (
        (Fl_Html_Object::Iterator *)it, first_drawable_, xx, yy);

    int ca = floor ((yy + first_drawable_offset_) * zoom_ + 0.5) + margin_;
    int lim = floor (html_height_ * zoom_ + 0.5) + 2 * margin_;
    // if(!hscrollbar_.visible())
    Fl_Boxtype b = box () ? box () : FL_DOWN_BOX;
    int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size ();
    int box_h = h () - Fl::box_dh (b) - scrollsize;
    if (lim - ca < box_h)
        ca = lim - box_h;

    topline (ca);
}

Fl_Html_Object * Fl_Html_View::get_link (void * within_iteration)
{
    int n = ((Fl_Html_Object::Iterator *)within_iteration)->no_parents ();
    if (!n)
        return 0;
    Fl_Html_Object * o = 0;
    while (n)
    {
        n--;
        o = ((Fl_Html_Object::Iterator *)within_iteration)->parent (n);
        if (o->drawable () & (1 | 4 | 8))
            break; // it is within word, image, table or table cell
    }
    if (!o)
        return 0; //
    // looking if any parent is a link
    while (n)
    {
        n--;
        o = ((Fl_Html_Object::Iterator *)within_iteration)->parent (n);
        if (o->link ())
            return o;
    }
    return 0;
}

void Fl_Html_View::set_cursor ()
{
    if (dragged_)
    {
        fl_cursor (FL_CURSOR_INSERT);
    }
    else if (link_ && !pushed_)
        fl_cursor (FL_CURSOR_HAND);
    else
        fl_cursor (FL_CURSOR_DEFAULT);
}

void Fl_Html_View::transform_to_document (int & xx, int & yy)
{
    xx -= drawn_x;
    yy -= drawn_y;
    xx = floor (float (xx) / zoom_ + 0.5);
    yy = floor (float (yy) / zoom_ + 0.5);
}

bool Fl_Html_View::point_within ()
{
    int xx = Fl::event_x ();
    int yy = Fl::event_y ();
    if (inside && point_inside_drawable (xx, yy))
    {
        transform_to_document (xx, yy);

        ((Fl_Html_Object::Iterator *)within_iterator)->reset ();
        within_object = Fl_Html_Object::point_within (
            (Fl_Html_Object::Iterator *)within_iterator, html_, xx, yy);
        ((Fl_Html_Object::Iterator *)within_iterator)
            ->add_parent (within_object);

        link_ = get_link (within_iterator);
        set_cursor ();
        return true;
    }
    link_ = 0;
    set_cursor ();
    within_object = 0;
    return false;
}

void Fl_Html_View::init_format ()
{
    if (!html_)
        return;
    if (initialized_)
        return;
    initialized_ = 1;
    Fl_Html_Formatter * s = fl_html_state ();
    s->drawing_device ((Fl_Html_Drawing_Device_ *)drawing_device_);
    s->reset (5);
    s->init_format (html_);
    if (s->background_color_set ())
    {
        background_set = true;
        background = s->background_color ();
    }
    else
        background_set = false;
    min_width = Fl_Html_Formatter::min_page_width (html_);
}

const double SCROLL_TIMEOUT = 0.1;

void Fl_Html_View::update_selection ()
{
    if (!pushed_)
        return;
    dragged_ = true;
    int xx = Fl::event_x ();
    int yy = Fl::event_y ();
    int XX = xx;
    int YY = yy;
    transform_to_document (XX, YY);
    selection_parent_ = Fl_Html_Object::get_selection (html_,
                                                       selection_start_x_,
                                                       selection_start_y_,
                                                       XX,
                                                       YY,
                                                       selection_first_,
                                                       selection_last_);
    drawing_device_->set_selection (selection_first_, selection_last_);
    fl_cursor (FL_CURSOR_INSERT);
    if (hscrollbar_.visible ())
    {
        if (xx < box_x)
        {
            if (!hscrollbar_timeout_)
                Fl::add_timeout (SCROLL_TIMEOUT, hscrollbar_timeout, this);
            hscrollbar_timeout_ = -1;
        }
        else if (xx > box_x + box_w)
        {
            if (!hscrollbar_timeout_)
                Fl::add_timeout (SCROLL_TIMEOUT, hscrollbar_timeout, this);
            hscrollbar_timeout_ = 1;
        }
        else if (hscrollbar_timeout_)
        {
            Fl::remove_timeout (hscrollbar_timeout, this);
            hscrollbar_timeout_ = 0;
        }
    }
    else if (hscrollbar_timeout_)
    {
        Fl::remove_timeout (hscrollbar_timeout, this);
        hscrollbar_timeout_ = 0;
    }
    if (scrollbar_.visible ())
    {
        if (yy < box_y)
        {
            if (!scrollbar_timeout_)
            {
                Fl::add_timeout (SCROLL_TIMEOUT, scrollbar_timeout, this);
                // printf("adding\n");
            }
            scrollbar_timeout_ = -1;
        }
        else if (yy > box_y + box_h)
        {
            if (!scrollbar_timeout_)
            {
                Fl::add_timeout (SCROLL_TIMEOUT, scrollbar_timeout, this);
                // printf("adding\n");
            }
            scrollbar_timeout_ = 1;
        }
        else if (scrollbar_timeout_)
        {
            Fl::remove_timeout (scrollbar_timeout, this);
            scrollbar_timeout_ = 0;
            // printf("removing\n");
        }
    }
    else if (scrollbar_timeout_)
    {
        Fl::remove_timeout (scrollbar_timeout, this);
        scrollbar_timeout_ = 0;
        // printf("removing\n");
    }
    redraw ();
}

void Fl_Html_View::remove_scroll_timeouts ()
{
    if (hscrollbar_timeout_)
    {
        Fl::remove_timeout (hscrollbar_timeout, this);
        hscrollbar_timeout_ = 0;
    }
    if (scrollbar_timeout_)
    {
        Fl::remove_timeout (scrollbar_timeout, this);
        scrollbar_timeout_ = 0;
    }
}

void Fl_Html_View::select_all ()
{
    selection_first_ = 1;
    selection_last_ = 0xFFFFFFF;
    selection_parent_ = 0;
    drawing_device_->set_selection (selection_first_, selection_last_);
    redraw ();
}

void Fl_Html_View::clear_selection ()
{
    if (selection_last_)
    {
        selection_last_ = 0;
        selection_parent_ = 0;
        drawing_device_->set_selection (0, 0);
        redraw ();
    }
}

void Fl_Html_View::copy_selection (int clipboard)
{
    if (html_ && selection_last_)
    {
        Fl_Html_Object * o = html_;
        if (selection_parent_)
            o = (Fl_Html_Object *)(selection_parent_->children ());
        char * txt =
            Fl_Html_Object::export_selection (o,
                                              selection_first_,
                                              selection_last_,
                                              Fl_Html_Object::CLIPBOARD_TEXT);
        if (txt && *txt)
            Fl::copy (txt, (int)strlen (txt), clipboard);
        delete[] txt;
    }
}

void Fl_Html_View::modify_zoom (int dy)
{
    float z = zoom ();
    if (dy >= 0)
    {
        while (dy)
        {
            if (z > max_zoom_)
                break;
            z *= zoom_factor_;
            dy--;
        }
    }
    else
    {
        dy = -dy;
        const float coef = 1.0 / zoom_factor_;
        while (dy)
        {
            if (z < 0.3)
                break;
            z *= coef;
            dy--;
        }
    }
    if (fabs (z - 1) < 0.01)
        z = 1;
    if (zoom () != z)
    {
        first_drawable ();
        zoom (z);
        redraw ();
        drawable_topline ();
    }
    zoomed_ = true;
}

int Fl_Html_View::handle (int event)
{
    // printf ("Handle %d!\n", event);
    switch (event)
    {
    case FL_FOCUS:
        printf ("Acquire focus\n");
        return 1;

    case FL_UNFOCUS:
        printf ("Lose focus\n");
        return 1;

    case FL_KEYBOARD:
        if (Fl::event_key () == FL_Tab)
        {
            focusNextLink ();
            return 1;
        }
        else
            return 1;

    case FL_ENTER:
        inside = 1;
        Fl_Group::handle (event);
        return 1;
    case FL_LEAVE:
        fl_cursor (FL_CURSOR_DEFAULT);
        inside = 0;
        link_ = 0;
        break;
    case FL_MOVE:
        if (point_within ())
            return 1;
        else
            return 0;
    case FL_RELEASE:
        pushed_ = false;
        remove_scroll_timeouts ();
        if (!dragged_)
            follow_link (link_);
        else
            copy_selection (0);
        dragged_ = false;
        point_within ();
        return 1;
    case FL_PUSH:
    {
        int xx = Fl::event_x ();
        int yy = Fl::event_y ();
        if (point_inside_drawable (xx, yy))
        {
            pushed_ = true;
            dragged_ = false;
            clear_selection ();
            if (!link_)
                fl_cursor (FL_CURSOR_INSERT);
            transform_to_document (xx, yy);
            selection_start_x_ = xx;
            selection_start_y_ = yy;

            return 1;
        }
    }
    break;
    case FL_KEYUP:

        printf ("Key pressed\n");

        if (zoomed_)
        {
            original_leftline_ = -1;
            zoomed_ = false;
        }

        if (Fl::event_key () == FL_Tab)
            return 0;
        else
            return 1;

        break;
    case FL_MOUSEWHEEL:
        if (Fl::event_state (FL_COMMAND))
        {
            modify_zoom (-Fl::event_dy ());
            return 1;
        }
        break;
    case FL_DRAG:
        if (pushed_)
        {
            if (link_)
            {
                link_ = 0;
            }
            update_selection ();
            // printf("dragging\n");
            return 1;
        }
        break;
    case FL_SHORTCUT:
    {
        if (Fl::event_state (FL_COMMAND))
        {
            switch (Fl::event_key ())
            {
            case 'a':
                select_all ();
                redraw ();
                return 1;
            case 'c':
            case 'x':
                copy_selection (1);
                return 1;
            }
        }
        break;
    }
    }
    return Fl_Group::handle (event);
}

void Fl_Html_View::follow_link (Fl_Html_Object * o)
{
    if (!o || !o->link ())
        return;
    const char * link = o->link ();
    if (link_function_)
        link = (*link_function_) (this, link);
    if (link)
        load (link, true, true);
}

int Fl_Html_View::yToLine (int yPos)
{
    return int (ceil (yPos * zoom_)) + 2 * margin_;
}

int Fl_Html_View::xToCol (int xPos)
{
    return int (ceil (xPos * zoom_)) + 2 * margin_;
}

extern "C" int usleep (int);

int Fl_Html_View::focusNextLink ()
{
    if (!selected)
    {
        selected = html_;
    }
    else
    {
        selected->takeFocus (false);
        selected = (Fl_Html_Object *)selected->nextObject ();
    }

    while (selected)
    {
        if (selected->takeFocus (true))
            break;
        selected = (Fl_Html_Object *)selected->nextObject ();
    }

    if (selected)
    {
        int line = yToLine (selected->y);
        int xCol = xToCol (selected->x);
        int xPlusWCol = xToCol (selected->x + selected->w);

        if (line > topline_ + numLinesVisible () - 1)
            topline (line - scrollbar_.linesize ());
        if (line < topline_)
            topline (line - scrollbar_.linesize ());
        //      if (xCol > leftline_ + numColsVisible () - 1)
        //        leftline ()

        printf ("Linesize: %d\n", scrollbar_.linesize ());
    }

    redraw ();
    return selected != NULL;
}

void Fl_Html_View::resize (int xx, int yy, int ww, int hh)
{
    Fl_Boxtype b = box () ? box () : FL_DOWN_BOX;
    Fl_Widget::resize (xx, yy, ww, hh);
    int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size ();
    scrollbar_.resize (x () + w () - scrollsize - Fl::box_dw (b) +
                           Fl::box_dx (b),
                       y () + Fl::box_dy (b),
                       scrollsize,
                       h () - scrollsize - Fl::box_dh (b));
    hscrollbar_.resize (x () + Fl::box_dx (b),
                        y () + h () - scrollsize - Fl::box_dh (b) +
                            Fl::box_dy (b),
                        w () - scrollsize - Fl::box_dw (b),
                        scrollsize);
    first_drawable ();
    if (original_leftline_ < 0)
    {
        original_leftline_ = leftline_;
        original_leftline_zoom_ = zoom_;
    }
    reformat ();
    drawable_topline ();
    int ol = original_leftline_;
    leftline (ol);
    within_object = 0;
    redraw ();
}

void Fl_Html_View::reformat ()
{
    if (!html_)
        return;
    init_format ();
    int ww = width_;
    Fl_Boxtype b = box () ? box () : FL_DOWN_BOX;
    if (ww <= 0)
    {
        ww = w () - 2 * margin_ - Fl::box_dw (b);
        // if(scrollbar_.visible())
        ww -= scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size ();
        ww = int (float (ww) / zoom_);
        if (ww < 0)
            ww = 0;
        if (!width_ && ww < min_width)
            ww = min_width;
    }
    if (formatted_ != ww)
    {
        formatted_ = ww;
        Fl_Html_Formatter * s = fl_html_state ();
        s->drawing_device (drawing_device_);
        // double t1 = ci::time();
        s->format (html_, ww);
        within_object = 0;
        html_height_ = s->bottom ();
        html_width_ = s->max_width ();
    }

    int size_ = ceil (html_height_ * zoom_) + 2 * margin_;
    int hsize_ = ceil (html_width_ * zoom_) + 2 * margin_;

    // printf("height: %i\n", html_height_);

    int ss = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size ();
    int dw = Fl::box_dw (b) + ss;
    int dh = Fl::box_dh (b);
    int dx = Fl::box_dw (b) - Fl::box_dx (b);
    int dy = Fl::box_dh (b) - Fl::box_dy (b);

    if (hsize_ > (w () - dw))
    {
        hscrollbar_.show ();
        dh += ss;
        if (size_ < (h () - dh))
        {
            scrollbar_.hide ();
            hscrollbar_.resize (x () + Fl::box_dx (b),
                                y () + h () - ss - dy,
                                w () - Fl::box_dw (b),
                                ss);
        }
        else
        {
            scrollbar_.show ();
            scrollbar_.resize (x () + w () - ss - dx,
                               y () + Fl::box_dy (b),
                               ss,
                               h () - ss - Fl::box_dh (b));
            hscrollbar_.resize (x () + Fl::box_dx (b),
                                y () + h () - ss - dy,
                                w () - ss - Fl::box_dw (b),
                                ss);
        }
    }
    else
    {
        hscrollbar_.hide ();

        if (size_ < (h () - dh))
            scrollbar_.hide ();
        else
        {
            scrollbar_.resize (x () + w () - ss - dx,
                               y () + Fl::box_dy (b),
                               ss,
                               h () - Fl::box_dh (b));
            scrollbar_.show ();
        }
    }
    // printf("Time: %g\n", ci::time() - t1);
}

void Fl_Html_View::draw ()
{

    Fl_Boxtype b = box () ? box () : FL_DOWN_BOX;
    int ww = w ();
    int hh = h ();
    if (!formatted_)
    {
        reformat ();
        topline (0);
        leftline (0);
    }
    if (anchor_)
    {
        topline (anchor_);
        free (anchor_);
        anchor_ = 0;
    }

    int ver_vis = scrollbar_.visible ();
    int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size ();
    int hor_vis = hscrollbar_.visible ();

    if (hscrollbar_.visible () || scrollbar_.visible ())
    {
        int scorn_x = x () + ww - (ver_vis ? scrollsize : 0) - Fl::box_dw (b) +
                      Fl::box_dx (b);
        int scorn_y = y () + hh - (hor_vis ? scrollsize : 0) - Fl::box_dh (b) +
                      Fl::box_dy (b);
        if (hor_vis)
        {
            if (hscrollbar_.h () != scrollsize)
            { // scrollsize changed?
                hscrollbar_.resize (x (), scorn_y, scorn_x - x (), scrollsize);
                init_sizes ();
            }
            draw_child (hscrollbar_);
            hh -= scrollsize;
        }
        if (ver_vis)
        {
            if (scrollbar_.w () != scrollsize)
            { // scrollsize changed?
                scrollbar_.resize (scorn_x, y (), scrollsize, scorn_y - y ());
                init_sizes ();
            }
            draw_child (scrollbar_);
            ww -= scrollsize;
        }
    }
    draw_box (
        b, x (), y (), ww, hh, background_set ? background << 8 : color ());
    if (!html_)
        return;

    box_x = x () + Fl::box_dx (b);
    box_y = y () + Fl::box_dy (b);
    box_w = ww - Fl::box_dw (b);
    box_h = hh - Fl::box_dh (b);

    int left_add = ver_vis ? 0 : scrollsize / 2;
    if (left_add)
    {

        if (margin_ - leftline_ < 0)
        {
            int ac = leftline_ - margin_ + left_add;
            if (scrollsize / 2 < ac)
                ac = scrollsize / 2;
            left_add -= ac;
        }
    }
    drawn_x = box_x + margin_ - leftline_ + left_add;
    drawn_y = box_y + margin_ - topline_;
    fl_push_clip (box_x, box_y, box_w, box_h);
    ((Fl_Html_Drawing_Device_ *)drawing_device_)
        ->draw (html_, drawn_x, drawn_y, zoom_, box_x, box_y, box_w, box_h);
    fl_pop_clip ();
    if (recalc_point_within)
        point_within ();
}

int Fl_Html_View::hScrollbarSize ()
{
    int scr = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size ();
    return hscrollbar_.visible () ? scr : 0;
}

int Fl_Html_View::numLinesTotal ()
{
    int scr = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size ();
    return int (ceil (html_height_ * zoom_)) + 2 * margin_ + scr -
           hScrollbarSize ();
}

int Fl_Html_View::numLinesVisible ()
{
    Fl_Boxtype b = box () ? box () : FL_DOWN_BOX;
    return h () - Fl::box_dh (b) - hScrollbarSize ();
}

void Fl_Html_View::topline (int top)
{
    if (!html_)
        return;
    int scrollsize = hScrollbarSize ();
    // int left_add = scrollsize? 0 : scr/2;
    /* number of lines total */

    int wh = numLinesVisible ();
    if (numLinesTotal () < wh || top < 0)
        top = 0;
    else if (top > numLinesTotal ())
        top = numLinesTotal ();
    topline_ = top;
    scrollbar_.value (topline_, wh, 0, numLinesTotal ());
    within_object = 0;
    do_callback ();
    redraw ();
    if (inside)
        recalc_point_within = true;
}

int Fl_Html_View::wScrollbarSize ()
{
    int scr = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size ();
    return hscrollbar_.visible () ? scr : 0;
}

int Fl_Html_View::numColsTotal ()
{
    return int (ceil (html_width_ * zoom_)) + 2 * margin_;
}

int Fl_Html_View::numColsVisible ()
{
    Fl_Boxtype b = box () ? box () : FL_DOWN_BOX;
    int scrollsize = scrollbar_size_ ? scrollbar_size_ : Fl::scrollbar_size ();
    return w () - scrollsize - Fl::box_dw (b);
}

void Fl_Html_View::leftline (int left)
{
    if (!html_)
        return;
    int size_ = numColsTotal ();
    int wh = numColsVisible ();
    if (/*width_>=0 && */ (size_ <= wh || left < 0))
        left = 0;
    else if (size_ - left <= wh)
    {
        left = size_ - wh;
        if (left < 0)
            left = 0;
    }
    else if (left > size_)
        left = size_;
    leftline_ = left;
    hscrollbar_.value (leftline_, wh, 0, size_);
    redraw ();
    if (inside)
        recalc_point_within = true;
}

extern "C" int sleep (int);

void Fl_Html_View::html (Fl_Html_Object * list, bool take_control)
{
    if (take_control_ && html_ != list)
        Fl_Xml_Object::destroy_object_list ((Fl_Xml_Object *)html_);
    html_ = list;
    if (html_)
        Fl_Html_Object::extract_title (html_, title_, sizeof (title_));
    else
        *title_ = 0;
    take_control_ = take_control;
    reset ();

    /*    selectionIter = NewIter (html_);
        for (int i = 0; i < 4; i++)
        {
            selectionIter.next ();
        }
        for (int i = 0; i < 4; i++)
        {
            selectionIter.prev ();
        }*/
}

static void get_error_position (const char * source, int position, int & line,
                                int & column)
{
    if (!source)
    {
        line = 0;
        column = 0;
        return;
    }
    int lin = 1;
    int col = 1;
    char c = *source++;
    while (position && c)
    {
        col++;
        if (c == '\n')
        {
            lin++;
            col = 1;
        }
        c = *source++;
        position--;
    }
    line = lin;
    column = col;
}

int Fl_Html_View::value (const char * html_string, bool copy,
                         const char * filename)
{
    clear_selection ();
    if (value_)
        free (value_);
    if (filename_ && (filename_ != filename))
        free (filename_);
    if (directory_)
        free (directory_);
    directory_ = 0;
    value_ = 0;
    filename_ = 0;
    char error_string[256];
    if (filename)
    {
        filename_ = strdup (filename);
        const char * slash = strrchr (filename, '/');
#ifdef WIN32
        const char * slash2 = strrchr (filename, '\\');
        if (slash2 > slash)
            slash = slash2;
#endif // WIN32
        if (slash && slash > filename + 1)
        {
            int len = slash - filename;
            directory_ = (char *)malloc (sizeof (char) * (len + 1));
            memcpy (directory_, filename, len);
            directory_[len] = 0;
        }
    }
    printf ("Directory: %s\n", directory_);
    if (html_string)
    {
        if (copy)
        {
            value_ = strdup (html_string);
            html_string = value_;
        }
        Fl_Xml_Object * result = 0;
        Fl_Html_Parser * p = fl_html_parser ();
        p->filename (filename_);
        int error = p->parse (html_string, &result);
        if (error)
        {
            Fl_Xml_Object::destroy_object_list ((Fl_Xml_Object *)result);
            int epos, line, column;
            epos = (p->value () - html_string);
            get_error_position (html_string, epos, line, column);

            sprintf (error_string,
                     "<head><title>Error</title></head> <h1>Parse error "
                     "%i</h1><i>%s </i> occured nearby position %i (line %i, "
                     "column %i) ",
                     error,
                     Fl_Html_Parser::friendly_error (error),
                     epos,
                     line,
                     column);
            p->parse (error_string, &result);
        }
        html ((Fl_Html_Object *)result, true);
        redraw ();
        return error;
    }
    return 0;
}

static int read_file (FILE * file, char *& data_return, int append_zeros)
{
    struct stat file_stats;
    if (-1 == fstat (fileno (file), &file_stats))
        return -1;
    int length = file_stats.st_size;
    char * data = new char[length + append_zeros];
    fseek (file, 0, SEEK_SET);
    if (fread ((void *)data, length, 1, file) < 1)
    {
        delete[] data;
        data_return = 0;
        return -1;
    }
    char * d = data + length;
    for (int i = 0; i < append_zeros; i++)
        *d++ = 0;
    data_return = data;
    return length;
}

int Fl_Html_View::load (const char * uri, bool open_external,
                        bool percent_encoded)
{
    if (!uri)
        return 0;
    int len = strlen (uri);
    if (directory_)
        len += strlen (directory_);
    char filename[2048];
    *filename = 0;
    char anchor[2048];
    int error = Fl_Html_Object::local_filename_from_uri (
        filename, anchor, directory_, uri, strlen (uri), percent_encoded);
    if (error > 0)
    { // it is external
        if (open_external)
            return fl_open_uri (uri);
        return 1;
    }
    if (error < 0)
        return error;
    if (*filename)
    {

        FILE * f = fl_fopen (filename, "rb");
        if (f)
        {
            char * data = 0;
            if (read_file (f, data, 1) >= 0)
            {
                error = value (data, 0, filename);
                topline (anchor);
            }
            else
                error = -2;
            fclose (f);
        }
        else
        {
            error = -1;
            value ("<head><title>Error</title></head><h1>Unable to follow "
                   "link</h1>",
                   0,
                   0);
            redraw ();
        }
    } // it is just an anchor
    if (error)
        background_set = 0;
    if (anchor_)
        free (anchor_);
    anchor_ = 0;
    if (*anchor)
    {
        anchor_ = strdup (anchor);
        redraw ();
    }
    return error;
}

void Fl_Html_View::topline (const char * anchor)
{
    if (!anchor || !(*anchor))
        return;
    Fl_Html_Object * o = Fl_Html_Object::find_anchor (anchor, html_);
    if (o)
    {
        topline (int (zoom_ * o->y));
        int lz = leftline_;
        int oz = int (o->x * zoom_);
        int over = oz - leftline_ - box_w;
        if (over > 0)
            lz += over;
        over = oz - lz;
        if (over < 0)
            lz += over;
        if (lz != leftline_)
            leftline (lz);
    }
    first_drawable_ = 0;
}

static const char * fl_html_link_function (Fl_Widget *, const char * link)
{
    return link;
}

void Fl_Html_View::reset ()
{
    initialized_ = 0;
    formatted_ = 0;
    html_height_ = 0;
    topline_ = 0;
    leftline_ = 0;
    original_leftline_ = -1;
    first_drawable_ = 0;
    link_ = 0;
    within_object = 0;
    selection_last_ = 0;
    clear_selection ();
}

Fl_Html_View::Fl_Html_View (int x, int y, int w, int h, const char * label)
    : Fl_Group (x, y, w, h, label),
      scrollbar_ (x + w - Fl::scrollbar_size (), y, 0,
                  h - Fl::scrollbar_size ()),
      hscrollbar_ (x, y + h - Fl::scrollbar_size (), w - Fl::scrollbar_size (),
                   0)
{
    drawing_device_ = new Fl_Html_Drawing_Device ();
    box (FL_DOWN_BOX);
    it = new Fl_Html_Object::Iterator ();
    within_iterator = new Fl_Html_Object::Iterator ();
    // selectionIter = new Fl_Html_Object::Iterator ();
    *title_ = 0;
    first_drawable_ = 0;
    box_x = 0;
    box_y = 0;
    box_w = 0;
    box_h = 0;
    width_ = 0;
    margin_ = 5;
    scrollbar_.hide ();
    hscrollbar_.hide ();
    initialized_ = 0;
    formatted_ = 0;
    background_set = 0;
    zoom_ = 1;
    scrollbar_size_ = 0;
    hscrollbar_.type (FL_HORIZONTAL);
    end ();
    init_sizes ();
    html_ = 0;
    take_control_ = 0;
    topline_ = 0;
    leftline_ = 0;
    original_leftline_ = -1;
    html_width_ = 0;
    scrollbar_.show ();
    scrollbar_.callback (scrollbar_callback);
    hscrollbar_.callback (hscrollbar_callback);
    drawn_x = 0;
    drawn_y = 0;
    within_object = 0;
    recalc_point_within = false;
    inside = false;
    value_ = 0;
    filename_ = 0;
    directory_ = 0;
    link_ = 0;
    anchor_ = 0;
    dragged_ = false;
    pushed_ = false;
    selection_first_ = 0;
    selection_last_ = 0;
    scrollbar_timeout_ = 0;
    hscrollbar_timeout_ = 0;
    zoomed_ = false;
    min_zoom_ = 0.3;
    max_zoom_ = 10;
    zoom_factor_ = 1.0905;
    link_function_ = &fl_html_link_function;
}
Fl_Html_View::~Fl_Html_View ()
{
    remove_scroll_timeouts ();
    if (take_control_)
        Fl_Xml_Object::destroy_object_list ((Fl_Xml_Object *)html_);
    delete ((Fl_Html_Object::Iterator *)it);
    delete ((Fl_Html_Object::Iterator *)within_iterator);
    if (value_)
        free (value_);
    if (filename_)
        free (filename_);
    if (directory_)
        free (directory_);
    if (anchor_)
        free (anchor_);
    delete drawing_device_;
    // delete iterator_;
}

void Fl_Html_View::textcolor (Fl_Color c)
{
    drawing_device_->base_font_color ((Fl::get_color (c)) >> 8);
}

Fl_Color Fl_Html_View::textcolor () const
{
    return Fl_Color ((drawing_device_->base_font_color ()) << 8);
}

void Fl_Html_View::textfont (Fl_Font f)
{
    drawing_device_->base_font (f, drawing_device_->base_font_size ());
}

Fl_Font Fl_Html_View::textfont () const
{
    return (Fl_Font) (drawing_device_->base_font_face ());
}

void Fl_Html_View::textsize (int size)
{
    drawing_device_->base_font (drawing_device_->base_font_face (), size);
}

int Fl_Html_View::textsize () const
{
    return drawing_device_->base_font_size ();
}

void Fl_Html_View::scrollbar_timeout (void * d)
{
    Fl_Html_View * w = (Fl_Html_View *)d;
    w->scrollbar_.value (w->scrollbar_.clamp (w->scrollbar_.value () +
                                              w->scrollbar_.linesize () *
                                                  w->scrollbar_timeout_));
    w->scrollbar_.redraw ();
    w->scrollbar_.do_callback ();
    Fl::add_timeout (SCROLL_TIMEOUT, scrollbar_timeout, d);
}

void Fl_Html_View::hscrollbar_timeout (void * d)
{
    Fl_Html_View * w = (Fl_Html_View *)d;
    w->hscrollbar_.value (w->hscrollbar_.clamp (w->hscrollbar_.value () +
                                                w->hscrollbar_.linesize () *
                                                    w->hscrollbar_timeout_));
    w->hscrollbar_.redraw ();
    w->hscrollbar_.do_callback ();
    Fl::add_timeout (SCROLL_TIMEOUT, hscrollbar_timeout, d);
}

void Fl_Html_View::scrollbar_callback (Fl_Widget * s, void *)
{
    ((Fl_Html_View *)(s->parent ()))
        ->topline (int (((Fl_Scrollbar *)s)->value ()));
    ((Fl_Html_View *)(s->parent ()))->first_drawable_ = 0;
    ((Fl_Html_View *)(s->parent ()))->original_leftline_ = -1;
    ((Fl_Html_View *)(s->parent ()))->update_selection ();
}

void Fl_Html_View::hscrollbar_callback (Fl_Widget * s, void *)
{
    ((Fl_Html_View *)(s->parent ()))
        ->leftline (int (((Fl_Scrollbar *)s)->value ()));
    ((Fl_Html_View *)(s->parent ()))->original_leftline_ = -1;
    ((Fl_Html_View *)(s->parent ()))->first_drawable_ = 0;
    ((Fl_Html_View *)(s->parent ()))->update_selection ();
}

static Fl_Html_Drawing_Device label_drawing_device;

Fl_Html_Label_Cache::~Fl_Html_Label_Cache ()
{
    Fl_Xml_Object::destroy_object_list (list);
}
void Fl_Html_Label_Cache::init_format (const Fl_Label * lab)
{
    if (!label () || min_w < 0)
        return;
    if (min_w && lab->font == font && lab->size == font_size &&
        (lab->align_ & FL_ALIGN_WRAP) == align)
        return;
    max_w = 0;
    formatted = 0;
    font = lab->font;
    font_size = lab->size;
    align = (lab->align_) & FL_ALIGN_WRAP;
    label_drawing_device.base_font (font, font_size);
    Fl_Html_Formatter * s = fl_html_state ();
    s->drawing_device ((Fl_Html_Drawing_Device_ *)(&label_drawing_device));
    s->reset (0);
    Fl_Xml_Object * result = 0;
    if (fl_html_parser ()->parse (label (), &result))
    {
        min_w = -1;
        Fl_Xml_Object::destroy_object_list (result);
        return;
    }
    list = (Fl_Html_Object *)result;
    s->init_format (list);
    min_w = Fl_Html_Formatter::min_page_width (list);
    s->format (list, HIGH_VAL);
    max_width = HIGH_VAL - s->extra_space ();
}

void Fl_Html_Label_Cache::format (int w)
{
    if (w <= min_w)
        w = min_w;
    if (w > max_width || !align)
        w = max_width;
    if (formatted && (width >= w && max_formatted <= w))
    {
        formatted = w;
        return;
    }
    formatted = w;
    Fl_Html_Formatter * s = fl_html_state ();
    s->drawing_device ((Fl_Html_Drawing_Device_ *)(&label_drawing_device));
    // double t1 = ci::time();
    s->reset (0);
    s->format (list, w);

    max_formatted = w;
    width = w;
    int xx = s->extra_space ();
    if (xx > 0 && align)
    {
        width -= xx;
        s->reset (0);
        s->format (list, width);
    }
    height = s->bottom ();
}

void Fl_Html_Label_Cache::measure (const Fl_Label * label, int & w, int & h)
{
    init_format (label);
    format (w);
    w = width;
    h = height;
    if (label->image)
    {
        if (label->image->w () > w)
            w = label->image->w ();
        h += label->image->h ();
    }
}

void Fl_Html_Label_Cache::draw (const Fl_Label * l, int x, int y, int w, int h,
                                Fl_Align align)
{
    Fl_Align old_align = l->align_;
    Fl_Label * lab = const_cast<Fl_Label *> (l);
    lab->align_ = (align & FL_ALIGN_WRAP);
    init_format (lab);
    format (w);
    lab->align_ = old_align;
    int y_image = y;
    int y_text = y;
    int x_image = x;
    int x_text = x;

    int hh = height;

    if (l->image)
    {
        hh += l->image->h ();
        if (align & FL_ALIGN_TEXT_OVER_IMAGE)
            y_image += l->image->h ();
        else
            y_text += l->image->h ();
    }
    if (align & FL_ALIGN_BOTTOM)
    {
        int dy = h - hh;
        y_text += dy;
        y_image += dy;
    }
    else if (!(align & FL_ALIGN_TOP))
    {
        int dy = (h - hh) / 2;
        y_text += dy;
        y_image += dy;
    }
    if (align & FL_ALIGN_RIGHT)
    {
        if (l->image)
            x_image += w - l->image->w ();
        x_text += w - width;
    }
    else if (!(align & FL_ALIGN_LEFT))
    {
        if (l->image)
            x_image += (w - l->image->w ()) / 2;
        x_text += (w - width) / 2;
    }
    int box_x, box_y, box_w, box_h;
    if (align & FL_ALIGN_CLIP)
    {
        fl_push_clip (x, y, w, h);
        box_x = x;
        box_y = y;
        box_w = w;
        box_h = h;
    }
    else
    {
        box_x = x_text;
        box_y = y_text;
        box_w = width;
        box_h = height;
    }

    ((Fl_Html_Drawing_Device_ *)(&label_drawing_device))
        ->draw (list, x_text, y_text, 1, box_x, box_y, box_w, box_h);
    if (align & FL_ALIGN_CLIP)
        fl_pop_clip ();
}

void fl_draw_html_src (const Fl_Label * label, int x, int y, int w, int h,
                       Fl_Align align)
{
    if (!label->value)
        return;
    Fl_Html_Label_Cache cache (label->value);
    cache.draw (label, x, y, w, h, align);
}

void fl_draw_html_cache (const Fl_Label * label, int x, int y, int w, int h,
                         Fl_Align align)
{
    if (!label->value)
        return;
    reinterpret_cast<Fl_Html_Label_Cache *> (const_cast<char *> (label->value))
        ->draw (label, x, y, w, h, align);
}

void fl_measure_html_src (const Fl_Label * label, int & w, int & h)
{
    if (!label->value)
        return;
    Fl_Html_Label_Cache cache (label->value);
    cache.measure (label, w, h);
}

void fl_measure_html_cache (const Fl_Label * label, int & w, int & h)
{
    if (!label->value)
        return;
    reinterpret_cast<Fl_Html_Label_Cache *> (const_cast<char *> (label->value))
        ->measure (label, w, h);
}

Fl_Labeltype fl_define_html_label_src ()
{
    static bool being_here = false;
    if (!being_here)
    {
        being_here = true;
        Fl::set_labeltype (Fl_Labeltype (FL_HTML_LABEL_SRC_),
                           &fl_draw_html_src,
                           &fl_measure_html_src);
    }
    return Fl_Labeltype (FL_HTML_LABEL_SRC_);
}

Fl_Labeltype fl_define_html_label_cache ()
{
    static bool being_here = false;
    if (!being_here)
    {
        being_here = true;
        Fl::set_labeltype (Fl_Labeltype (FL_HTML_LABEL_CACHE_),
                           &fl_draw_html_cache,
                           &fl_measure_html_cache);
    }
    return Fl_Labeltype (FL_HTML_LABEL_CACHE_);
}

/*
fl_html_measure(const Fl_Label* o, int& W, int& H) {
  fl_font(o->font, o->size);
  fl_measure(o->value, W, H);
  if (o->image) {
    if (o->image->w() > W) W = o->image->w();
    H += o->image->h();
  }
}
*/

// static inline_image_count;
// class Fl_Html_Image_Image: public Fl_Shared_Image{
