#ifndef _Fl_Html_View_H_
#define _Fl_Html_View_H_

#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <string.h>

#include "Fl_Html_Object.H"

class Fl_Html_Object;
// class Html_Object_Iterator;
class Fl_Html_Drawing_Device;

class FL_EXPORT Fl_Html_View : public Fl_Group
{
    char * value_;
    char * filename_;
    char * directory_;
    float zoom_;
    bool background_set;
    int background;
    int scrollbar_size_;
    Fl_Scrollbar scrollbar_, hscrollbar_;
    int min_width;
    int width_;
    int margin_;
    int box_w;
    int box_h;
    int box_x;
    int box_y;
    Fl_Html_Object * html_;
    bool take_control_;
    int html_height_;
    int html_width_;
    int initialized_;
    int formatted_;
    int topline_;
    int leftline_;
    int original_leftline_;
    float original_leftline_zoom_;
    Fl_Html_Object * first_drawable_;
    int first_drawable_offset_;
    static void scrollbar_callback (Fl_Widget * s, void *);
    static void hscrollbar_callback (Fl_Widget * s, void *);
    static void scrollbar_timeout (void *);
    static void hscrollbar_timeout (void *);
    void * it;
    void * within_iterator;
    Fl_Html_Object * within_object;
    int drawn_x;
    int drawn_y;
    bool inside;
    bool recalc_point_within;
    Fl_Html_Object * link_;
    char * anchor_;
    const char * (*link_function_) (Fl_Widget *, const char *);
    bool dragged_;
    bool pushed_;
    int selection_start_x_;
    int selection_start_y_;
    int selection_first_;
    int selection_last_;
    int scrollbar_timeout_;
    int hscrollbar_timeout_;
    Fl_Html_Object * selection_parent_;
    Fl_Html_Drawing_Device * drawing_device_;
    char title_[1024];
    bool zoomed_;
    float min_zoom_;
    float max_zoom_;
    float zoom_factor_;

    Fl_Html_Object * selected;
    // NewIter selectionIter;

    void first_drawable ();
    void drawable_topline ();
    Fl_Html_Object * get_link (void * within_iteration);
    void set_cursor ();
    void transform_to_document (int & xx, int & yy);
    bool point_inside_drawable (int xx, int yy)
    {
        return (xx >= box_x && yy >= box_y && xx <= box_x + box_w &&
                yy <= box_y + box_h);
    }
    bool point_within ();
    void init_format ();
    void update_selection ();
    void remove_scroll_timeouts ();

  public:
    void copy_selection (int clipboard = 1);

  protected:
    int handle (int event);

    void draw ();
    void follow_link (Fl_Html_Object * o);

  public:
    Fl_Html_View (int x, int y, int w, int h, const char * label = 0);
    ~Fl_Html_View ();

    /* Keyboard navigation */

    /* Focuses the next link, making sure that the window has scrolled to let it
     * be visible. Returns 0 if no links remain.*/
    int focusNextLink ();
    /* Converts a y-coordinate to a line number. */
    int yToLine (int yPos);
    /* Converts an x-coordinate to a column number. */
    int xToCol (int xPos);

    // Following functions are direct equivalents to functions in Fl_Help_View
    void resize (int xx, int yy, int ww, int hh);
    void link (const char * (*link_function) (Fl_Widget *, const char *))
    {
        link_function_ = link_function;
    }
    const char * directory () const
    {
        return directory_;
    }
    const char * filename () const
    {
        return filename_;
    }
    void select_all ();
    void clear_selection ();
    int load (const char * uri)
    {
        return load (uri, true);
    }
    int scrollbar_size () const
    {
        return scrollbar_size_;
    }
    void scrollbar_size (int s)
    {
        scrollbar_size_ = s;
    }
    int size () const
    {
        return html_width_;
    }
    void textcolor (Fl_Color c);
    Fl_Color textcolor () const;
    void textfont (Fl_Font f);
    Fl_Font textfont () const;
    void textsize (int size);
    int textsize () const;
    const char * title () const
    {
        if (*title_)
            return title_;
        return 0;
    }

    int hScrollbarSize ();
    int numLinesTotal ();
    int numLinesVisible ();

    int wScrollbarSize ();
    int numColsTotal ();
    int numColsVisible ();

    void topline (int top);
    void topline (const char * anchor);
    void leftline (int left);
    int leftline () const
    {
        return leftline_;
    }

    // These are new functions for Fl_Html_View

    // Sets the magnification factor.
    // Note that zooming does not need to do reformat the content if
    // format_width==0 or it is static (bigger than 0)" Zooming is also
    // performed using FL_COMMAND (CTRL) + SCROLLWHEEL when mouse pointer is
    // above the widget. Zoom is also called

    void zoom (float z);
    float zoom () const
    {
        return zoom_;
    }

    // This parameters is used to define if precise measurement of zoomed fonts
    // should be performed each time the zoom changes. FLTK does not have
    // precise zoom scaling so font scaling is emulated by changing font size
    // with respect to the zooming factor. However the measurements do not scale
    // well: For instance if we increase font size twice, the width of the same
    // word might not be exactly twice than for "unzoomed" font. Although there
    // is a work-around to adjust the font size to fit the zoom factor more
    // precisely, there still might be some mismatch (wchih might result with
    // slightly bigger or smaller space gaps between long words). To avoid that,
    // set zoomed_font_measurement() to true: in such a case the words are
    // re-measured each time the zooming changes. Note that this requires call
    // to init_format() so it is slower than pure reformat() and there migh be
    // certain (but usually very small, non-noticeable) lag with each call to
    // zoom().The zoomed_font_measurement() is true by default.
    void zoomed_font_measurement (bool z);
    bool zoomed_font_measurement () const;

    // Modifies zoom by multiplying (x>0) or dividing (x<0) zooming factor x
    // times.
    void modify_zoom (int x);

    // Sets zooming limits.
    // It applies to modify_zoom but not to zoom() Default range is 0.3 - 10.
    void min_zoom (float z)
    {
        min_zoom_ = z;
    }
    float min_zoom () const
    {
        return min_zoom_;
    }
    void max_zoom (float z)
    {
        max_zoom_ = z;
    }
    float max_zoom () const
    {
        return max_zoom_;
    }

    // This function sets the formatting mode:
    // w > 0  is a static mode, formats to particular width \a w. Note that
    // certain parts of the document
    //        can still exceed this width if particular elements (eg table,
    //        image or very_loooooooooooooooong_word exceeds thar width: in this
    //        case  horisontal scrollbar is shown with respect  to either this
    //        size or minimum width of the document  (whichever is bigger).
    // w = 0  Formats the whole document to either the widget size or the
    // minimum width (the widest element -eg table,
    //        image or very long word, whichever is bigger) for all parts of the
    //        document. This is the default mode.
    // w < 0  Formats the whole document to the widget size. If certain parts
    // exceed this size (that is
    //        the the document minimum width is bigger than the widget size),
    //        the horisontal scrollbar is shown  to allow rendering of those
    //        parts of the document too but the rest of the document is more
    //        narrow - formatted just to the widget width. This is mode how
    //        browsers usualy do formatting but it is not default here because
    //        it seems to me that mode w==0 is usually what user would want.
    void format_width (int w)
    {
        if (w == width_)
            return;
        width_ = w;
        formatted_ = 0;
    }
    int format_width () const
    {
        return width_;
    }

    // Forces reformat  (for tweaking purposes only)
    // Nomally user does not need to call this explicitly as it is called every
    // time user resizes the window.
    void reformat ();

    // Forces init_format() with the next redraw() (for tweaking purposes only).
    // Normally this does not need to be called explicitly because the function
    // is called each time a new document is loaded or the base font
    // (textfont(), textsize()) changes.
    void reset ();

    // Sets directly html object tree (for tweaking putposes only)
    // If the object-tree \a list was created by some external parser, the
    // result can be directly assigned to the widget. If \a take_control is true
    // this tree will be destroyed if the widget is destroyed or a new document
    // is loaded.
    void html (Fl_Html_Object * list, bool take_control);

    // This function loads html from \a html_string.
    // If \a copy is true then it makes an internal copy of the string,
    // otherwise it just parses the string to Fl_Html_Object tree structure.
    // If filename is non-zero then the string is assumed to be placed at
    // location \a filename so the relative links and images can be resolved.
    int value (const char * html_string, bool copy = false,
               const char * filename = 0);
    const char * value () const
    {
        return value_;
    }

    // This function loads file from local filename or "URI" scheme,
    // including anchor (#) appendix. If the uri can not be loaded
    // (eg it is an external file) and \a open_external is true
    // it calls fl_open_uri() upon given string.
    // If \a percent_encoded is true, it first percent-decodes uri to utf-8
    // encoded url and then loads the file. \see also backward-compatible
    // load(const char * uri)
    int load (const char * uri, bool open_external,
              bool percent_encoded = false);
};

// FL_HTML_LABEL
// Following are temporary implementations, they should be replaced by single
// FL_HTML_LABEL when Fl_Widget is made aware of Fl_Label_Cache class...

#define FL_HTML_LABEL_SRC_ 14
#define FL_HTML_LABEL_CACHE_ 15

FL_EXPORT Fl_Labeltype fl_define_html_label_src ();
FL_EXPORT Fl_Labeltype fl_define_html_label_cache ();

#define FL_HTML_LABEL_SRC fl_define_html_label_src ()
#define FL_HTML_LABEL_CACHE fl_define_html_label_cache ()

// Those are HTML drawing and measurement dunctions. You dont need to use them
// if you use above macros or fl_html_label(Fl_Widget * , Fl_Html_Label_Cache *)
// function (see below)
void fl_draw_html_src (const Fl_Label * label, int x, int y, int w, int h,
                       Fl_Align align);
void fl_measure_html_src (const Fl_Label * label, int & w, int & h);

// Those are functions which operate on "binary" pre-parsed label (and
// preformatted after sequential draw if parameters like fent, size, labeltype,
// the string itself or formatting width dont change) instead of (const char *)
// strings (see below)

void fl_draw_html_cache (const Fl_Label * label, int x, int y, int w, int h,
                         Fl_Align align);
void fl_measure_html_cache (const Fl_Label * label, int & w, int & h);

// Following should go later to the header where Fl_Label is defined...
// The purpose is to unify various label caches even without a need need of
// extra space for the cache pointer if it would be an union of recent plain
// string and  pointer to label cache. The  COPIED_LABEL flag would then
// indicate how this pointer should be interpreted: whenever COPIED_LABEL is
// set, it means that the pointer to label string should be reinterpreted as
// label cache. The pointer to string is kept internally within the cache and
// can be accessed ny Fl_Label_Cache::label() method. Following is possible
// modification of Fl_Widget::label() API: class Fl_Widget {
//   ...
//   enum {CACHED_LABEL = COPIED_LABEL };
//   const char * label() const {
//      if(flags() & COPIED_LABEL)
//         return reinterpret_cast<Fl_Label_Cache *>(label_.value)->label();
//      else
//         return label_.value;
//   }
//   void copy_label(const char * label){
//       if(flags() & COPIED_LABEL)  delete reinterpret_cast<Fl_Label_Cache
//       *>(label_.value); label_.value = reinterpret_chast<char *>(new
//       Fl_Label_Cache(label, true)); flags_ |= COPIED_LABEL;
//   }
//   ~Fl_Widget(){
//       ...
//       if(flags() & COPIED_LABEL)  delete reinterpret_cast<Fl_Label_Cache
//       *>(label_.value);
//    }

class FL_EXPORT Fl_Label_Cache
{
    const char * l_;
    bool copied_;

  public:
    virtual ~Fl_Label_Cache ()
    {
        if (copied_)
            delete (char *)l_;
    }
    const char * label () const
    {
        return l_;
    };
    virtual void draw (const Fl_Label * label, int x, int y, int w, int h,
                       Fl_Align align)
    {
        label->draw (x, y, w, h, align);
    };
    Fl_Label_Cache (const char * label, bool copy = false) : copied_ (copy)
    {
        if (copy && label)
        {
            int len = strlen (label) + 1;
            l_ = new char[len];
            memcpy ((void *)l_, label, len);
        }
        else
            l_ = label;
    }
    // This version steals label from existing cache and destroys that cache
    Fl_Label_Cache (Fl_Label_Cache * from)
        : l_ (from->l_), copied_ (from->copied_)
    {
        from->copied_ = false;
        delete from;
    }
};

class FL_EXPORT Fl_Html_Label_Cache : public Fl_Label_Cache
{
    int min_w;
    int max_w;
    int formatted;
    int max_formatted;
    int width;
    int height;
    Fl_Html_Object * list;
    Fl_Font font;
    int font_size;
    Fl_Align align;
    int max_width;
    void init_format (const Fl_Label * label);
    void format (int w);

  public:
    void draw (const Fl_Label * label, int x, int y, int w, int h,
               Fl_Align align);
    void measure (const Fl_Label * label, int & w, int & h);
    Fl_Html_Label_Cache (const char * label, bool copy = false)
        : Fl_Label_Cache (label, copy), min_w (0), max_w (0), formatted (0),
          max_formatted (0), list (0), font (0), font_size (0), align (0),
          max_width (0)
    {
    }
    ~Fl_Html_Label_Cache ();
};

// Temporary convenient way to set cached html label.
// Note that Fl_Widget::label() will return pointer to a binary rubbish until
// label caches are properly implemented within the Fl_Widget and/or
// Fl_Labeltype class.
inline void fl_html_label (Fl_Widget * w,
                           Fl_Html_Label_Cache * html_label_cache)
{
    w->label (reinterpret_cast<const char *> (html_label_cache));
    w->labeltype (FL_HTML_LABEL_CACHE);
}

#endif // _Fl_Html_View_H_
