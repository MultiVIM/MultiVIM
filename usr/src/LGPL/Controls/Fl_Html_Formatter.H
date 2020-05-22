#ifndef _Html_Formatter_H_
#define _Html_Formatter_H_
#include <FL/Fl_Box.H>

//#include "Fl_Html_Parser.H"

class Fl_Html_Object;
class Fl_Html_Object_;

enum Fl_Html_Image_Uri_Type
{
    HTML_URI_FILE = 1,      // ordinary file
    HTML_URI_DATA_JPEG = 2, // url is jpeg memory data already base64 decoded
    HTML_URI_DATA_PNG = 3,
    HTML_URI_DATA_GIF = 4,
    HTML_URI_URL = 0x10 // external url, at the moment not supported for images
};

// assure virtual destructor
class Fl_Html_Image_
{
  public:
    virtual int w () const = 0;
    virtual int h () const = 0;
    virtual ~Fl_Html_Image_ (){};
};

class Fl_Html_Drawing_Device_
{
  public:
    struct Selection
    {
        int x1, y1, x2, y2;
        Selection (int X1, int Y1, int X2, int Y2)
            : x1 (X1), y1 (Y1), x2 (X2), y2 (Y2)
        {
        }
    };

  protected:
    int font_face_;
    float font_size_;
    int base_font_face_;
    unsigned base_font_color_;
    int base_font_size_;
    int color_;
    int link_color_;
    int underline_;
    int x_, y_, w_, h_;

    int underline_x_;
    int underline_y_;
    int background_color_;
    int selection_first_;
    int selection_last_;
    int selection_first_start_;
    int selection_last_end_;
    int selection_color_;
    int selection_color2_;
    int foreground_selection_;
    int foreground_selection2_;
    bool selection_color_calculated_;
    int calc_selection_color_;
    int calc_selection_foreground_;
    void calculate_contrast_selection ()
    {
        selection_color_calculated_ = true;
        int a1 = (selection_color_ & 0xF) - (background_color_ & 0xF);
        a1 *= a1;
        int a = (((selection_color_ >> 8) & 0xF) -
                 ((background_color_ >> 8) & 0xF));
        a1 += a * a;
        a = ((selection_color_ >> 16) & 0xF) - ((background_color_ >> 8) & 0xF);
        a1 += a * a;
        int a2 = (selection_color2_ & 0xF) - (background_color_ & 0xF);
        a2 *= a2;
        a = (((selection_color2_ >> 8) & 0xF) -
             ((background_color_ >> 8) & 0xF));
        a2 += a * a;
        a = ((selection_color2_ >> 16) & 0xF) -
            ((background_color_ >> 8) & 0xF);
        a2 += a * a;
        if (a2 > a1)
        {
            calc_selection_color_ = selection_color2_;
            calc_selection_foreground_ = foreground_selection2_;
        }
        else
        {
            calc_selection_color_ = selection_color_;
            calc_selection_foreground_ = foreground_selection_;
        }
    }

  public:
    void set_selection (int first, int last)
    {
        selection_first_ = first;
        selection_last_ = last;
    }

    int selection_color () const
    {
        return selection_color_;
    }
    void selection_color (int c)
    {
        selection_color_ = c;
    }
    int selection_color2 () const
    {
        return selection_color2_;
    }
    void selection_color2 (int c)
    {
        selection_color2_ = c;
    }
    int selection_first () const
    {
        return selection_first_;
    }
    int selection_last () const
    {
        return selection_last_;
    }
    int foreground_selection () const
    {
        return foreground_selection_;
    }
    void foreground_selection (int c)
    {
        foreground_selection_ = c;
    }
    int foreground_selection2 () const
    {
        return foreground_selection2_;
    }
    void foreground_selection2 (int c)
    {
        foreground_selection2_ = c;
    }
    void contrast_selection (int & background, int & foreground)
    {
        if (!selection_color_calculated_)
            calculate_contrast_selection ();
        background = calc_selection_color_;
        foreground = calc_selection_foreground_;
    }

    void reset_selection ()
    {
        selection_last_ = -1;
    }
    int background_color () const
    {
        return background_color_;
    }
    void background_color (int c)
    {
        background_color_ = c;
        selection_color_calculated_ = false;
    }
    virtual void draw_box (Fl_Boxtype type, int x, int y, int w, int h) = 0;
    virtual void draw_image (Fl_Html_Image_ * image,
                             Fl_Html_Image_ *& image_cache, int x, int y, int w,
                             int h) = 0;
    void discontinue_underline ()
    {
        underline_y_ = 0;
    }

    int x () const
    {
        return x_;
    }
    int y () const
    {
        return y_;
    }
    int w () const
    {
        return w_;
    }
    int h () const
    {
        return h_;
    }
    void bounding_box (int x, int y, int w, int h)
    {
        x_ = x;
        y_ = y;
        w_ = w;
        h_ = h;
    }
    void reset_for_format ()
    {
        init_draw ();
    }
    virtual void font (int face, float size) = 0;
    virtual void font (int face, int html_size, int relative) = 0;
    virtual int space () = 0;
    int base_font_face () const
    {
        return base_font_face_;
    }
    int base_font_size () const
    {
        return base_font_size_;
    }
    int base_font_color () const
    {
        return base_font_color_;
    }
    virtual void base_font (int face, int size) = 0;

    void base_font_color (int color)
    {
        base_font_color_ = color;
    }
    virtual int descent () = 0;
    enum Font
    {
        // Those directly match fltk fonts
        BOLD = 1,
        ITALIC = 2,

        HELVETICA = 0,
        COURIER = 4,
        TIMES = 8,
        SYMBOL = 12,
        // Following are not in fltk and are replaced by similar
        ARIAL = 0x100 | HELVETICA,
        SANS = 0x200 | HELVETICA,
        SERIF = 0x300 | TIMES
    };
    int font_face () const
    {
        return font_face_;
    }
    float font_size () const
    {
        return font_size_;
    }

    virtual void measure (const char * word, int n, int & w, int & h) = 0;
    virtual void draw (const char * word, int n, int x, int y) = 0;
    virtual void rectf (int x, int y, int w, int h, bool extend = false) = 0;
    virtual void color (int c) = 0;
    //  virtual void line(int x, int y, int w, int width = 1) = 0;
    //  virtual void vline(int x, int y, int h, int width = 1) = 0;
    int color () const
    {
        return color_;
    }
    void link_color (int c)
    {
        link_color_ = c;
    }
    int link_color () const
    {
        return link_color_;
    }
    virtual int color_gray () const = 0;
    void underline (int c)
    {
        underline_ = c;
        if (!c)
            discontinue_underline ();
    }
    int underline () const
    {
        return underline_;
    }
    virtual void init_draw (int base_font_face, int base_font_size,
                            int base_font_color) = 0;
    virtual void init_draw () = 0;

    virtual float zoom () const = 0;
    virtual void zoom (float z) = 0;

    virtual void origin (int x, int y) = 0;

    // This translates origin by dx, dy in the "drawable" (non-zoomed) space.
    // Note that the caller should remember dx and dy and call this once again
    // with negative values to translate back, no transformation stack is kept
    // around.
    virtual void translate (int dx, int dy) = 0;
    virtual void rect (int x, int y, int w, int h) = 0;

    void draw (Fl_Html_Object * o, int x, int y, float zoom, int box_x,
               int box_y, int box_w, int box_h);

    virtual ~Fl_Html_Drawing_Device_ ()
    {
    }
    Fl_Html_Drawing_Device_ ()
        : font_face_ (0), font_size_ (0), base_font_face_ (8),
          base_font_color_ (0), base_font_size_ (14), color_ (0),
          link_color_ (0x0000FF), underline_ (0), background_color_ (0xFFFFFF),
          selection_first_ (0), selection_last_ (-1),
          selection_color_ (0x6060FF), selection_color2_ (0xFFFF00),
          foreground_selection_ (0xFFFFFF), foreground_selection2_ (0),
          selection_color_calculated_ (false) /* , selection_valid_(false)*/
    {
    }
};

// This is default object-creation table. To cteate the object structure tree
// the user should do something like:
//   Fl_Html_Parser parser(default_html_create_object_table());
//   Fl_Html_Object * result;
//   if(!parser.parse(my_html_string, (Fl_Xml_Object * *)(&result))){
//      ... // do something with result like format(),  draw() ...
//   }
//   Fl_Xml_Object * destroy_object_list(result) // destroys object
//   tree-structure when not needed any more

class Fl_Html_Table_Formatter;

class Fl_Html_Formatter
{
    int * left_margin_table_;
    int * right_margin_table_;

    int no_left_margins_;
    int no_right_margins_;

    int * cur_left_margin_;
    int * cur_right_margin_;

    // int no_fonts_;
    // int * cur_font_;
    // int * font_table_;

    Fl_Html_Object ** line_objects_;
    Fl_Html_Object ** todo_objects_;
    Fl_Html_Object ** todo_format_objects_;
    int line_objects_size_;
    int no_line_objects_;
    int todo_objects_size_;
    int no_todo_objects_;
    int todo_format_objects_size_;
    int no_todo_format_objects_;
    Fl_Html_Formatter * aux_state_;
    int paragraph_dy_;

    Fl_Html_Drawing_Device_ * drawing_device_;

    static void clear_margin_ (const int * margin_table, int *& cur_margin,
                               int & position)
    {
        while (cur_margin != margin_table)
        {
            if (cur_margin[1] > position)
                position = cur_margin[1];
            cur_margin -= 2;
        }
    }
    static void update_margin_ (int y_position, const int * margin_table,
                                int *& cur_margin)
    {
        while (cur_margin != margin_table)
        {
            if (y_position < cur_margin[1])
                break;
            cur_margin -= 2;
        }
    }
    static void push_margin (int position, int limit, int *& table,
                             int *& current, int & no_margins);
    // These are variables and functions which might be used in
    // Fl_Html_Object::format() function
    int x_;       // position of last created object on the line
    int y1_, y2_; // top and bottom  of the last created object
    int baseline_;

    // Current align.
    // This aligh defines vertical positions between neighbour objects and with
    // each new placement (o->format()), it can be either left unchanged or
    // modified - eg "word" obeys current align with previous object but sets it
    // to bottom after.
    int y_align_;

    int start_y_; // starting position of current line

    // Extends of current "unfinished" line
    int min_y_;
    int max_y_;

    // The align of rows/inline elements.
    // Finish_line
    // If ON, the line is centered during line_finish().
    int x_align_;

    int extra_space_;
    // Fl_Css_Parser css_parser_;
    // Fl_Css_Parser::Property css_property_;

    int bottom_;
    int left_shift_;
    void remove_formatter ();
    int max_width_;

    int percentual_width_left_;
    int percentual_width_right_;
    int percentual_width_inline_;

  protected:
    // Sets the position to \a y and corresponding left margin.
    // Note that y can be only incremented during formatting.
    void start_line (int y)
    {
        baseline_ = y1_ = y2_ = start_y_ = min_y_ = max_y_ = y;
        update_margins (y);
        x_ = left_margin () + left_shift_;

        // below actually does not matter as there is no previous object on the
        // line but it might avoid line colision and repositionning on
        // finish_line() if objects are of the same height (or smaller than the
        // first object on the line) or they are TOP-aligned too
        y_align_ = TOP;
    }
    void add_line_object (Fl_Html_Object * o);

    void push_left_margin (int position, int limit)
    {
        push_margin (position,
                     limit,
                     left_margin_table_,
                     cur_left_margin_,
                     no_left_margins_);
    }

    void push_right_margin (int position, int limit)
    {
        push_margin (position,
                     limit,
                     right_margin_table_,
                     cur_right_margin_,
                     no_right_margins_);
    }

    void update_margins (int y_position)
    {
        update_margin_ (y_position, left_margin_table_, cur_left_margin_);
        update_margin_ (y_position, right_margin_table_, cur_right_margin_);
    }

    Fl_Html_Table_Formatter * table_formatter_;

    // This are general pointers which can be used ser by parent and ised by
    // children.
    //
    void * aux_object_[4];

  private:
    int bullet_type_;
    int bullet_width_;
    int bullet_height_;
    Fl_Html_Object * bullet_;
    int bullet_index_;
    int bullet_x_;
    int background_color_;
    bool background_color_set_;
    int space_added_;

  public:
    bool background_color_set () const
    {
        return background_color_set_;
    }
    int background_color () const
    {
        return background_color_;
    }
    void background_color (int c)
    {
        background_color_ = c;
        background_color_set_ = true;
    }
    int bullet_x () const
    {
        return bullet_x_;
    }
    void bullet_x (int x)
    {
        bullet_x_ = x;
    }

    Fl_Html_Object * bullet () const
    {
        return bullet_;
    }
    void bullet (Fl_Html_Object * o)
    {
        bullet_ = o;
    }
    int bullet_type () const
    {
        return bullet_type_;
    }
    void bullet_type (int t)
    {
        bullet_type_ = t;
    }
    int bullet_index () const
    {
        return bullet_index_;
    }
    void bullet_index (int i)
    {
        bullet_index_ = i;
    }
    int bullet_width () const
    {
        return bullet_width_;
    }
    void bullet_width (int w)
    {
        bullet_width_ = w;
    }
    int bullet_height () const
    {
        return bullet_height_;
    }
    void bullet_height (int w)
    {
        bullet_height_ = w;
    }

    void percentual_width_left (int a)
    {
        if (percentual_width_left_ < a)
            a = percentual_width_left_;
    }
    void percentual_width_right (int a)
    {
        if (percentual_width_right_ < a)
            a = percentual_width_right_;
    }
    void percentual_width_inline (int a)
    {
        if (percentual_width_inline_ < a)
            a = percentual_width_inline_;
    }
    int percentual_width () const
    {
        return percentual_width_left_ + percentual_width_right_ +
               percentual_width_inline_;
    }

    // Those might be used for by tags to temporarily store some info
    enum Aux_Object
    {
        TABLE = 0,
        ROW = 1,
        TABLE_FORMATTER = 2,
        FREE_AUX_OBJECT
    };

    enum Clear
    {
        CLEAR_LEFT = 0x2000,
        CLEAR_RIGHT = 0x4000
    };

    void clear_margin (int which)
    {
        int position = start_y_;
        if (which & CLEAR_LEFT)
            clear_margin_ (left_margin_table_, cur_left_margin_, position);
        if (which & CLEAR_RIGHT)
            clear_margin_ (right_margin_table_, cur_right_margin_, position);
        start_line (position);
    }

    void * aux_object (Aux_Object i) const
    {
        return aux_object_[i];
    }
    void * aux_object (Aux_Object i, void * o)
    {
        void * old = aux_object_[i];
        aux_object_[i] = o;
        return old;
    }
    // returns current carriage position
    int x () const
    {
        return x_;
    }
    int left_shift () const
    {
        return left_shift_;
    }

    // both setting must be performed only if cassiage x_ is at the beginning of
    // the line
    void add_left_shift (int s)
    {
        left_shift_ += s;
        x_ += s;
    }

    void add_space (int s)
    {
        x_ += s;
        space_added_ = s;
    };
    void check_space ()
    {
        if (space_added_)
        {
            int s = drawing_device ()->space ();
            int diff = s - space_added_;
            if (diff > 0)
            {
                x_ += diff;
                space_added_ = s;
            }
        }
    }

    // This is only used for init_format()
    // Table formatter can be re-used as long as it is reset()
    Fl_Html_Table_Formatter * table_formatter ();

    // Calculates minimum width of page so that all objects from the list/tree
    // \a o fit in there
    static int min_page_width (const Fl_Html_Object * o);

    // Bouding box of a tag can be usualy determined only after all children are
    // placed. This function adds object \a o to list which bounding boxes will
    // be recalculated in reverse order later.
    void add_todo_object (Fl_Html_Object * o);

    void add_todo_format_object (Fl_Html_Object * o);

    // Draving device must be set before any formatting or drawing operations
    // are performed. Note that the device can be set different for formatting
    // that for drawing. For instance printing can be performed in these steps:
    // 1) - setting drawing device to "Printer"
    // 2) - peforming formatting
    // 3) - setting the drawing device to "Screen"
    // 4) - perfoming "Preview Drawing": apart from some small differences
    //      (such as small differences in font shapes) the preview will perform
    //      correct rendering to the user as the elements will be placed in the
    //      same positions and the format will be the same as the final print
    // 5) - setting drawing device to "Printer"
    // 6) - performing print drawing

    void drawing_device (Fl_Html_Drawing_Device_ * d)
    {
        drawing_device_ = d;
    }
    Fl_Html_Drawing_Device_ * drawing_device () const
    {
        return drawing_device_;
    }

    enum Y_Align
    {
        TOP = 0,
        MIDDLE = 4,
        BOTTOM = 8,
        BASELINE = 4 | 8
    };
    enum X_Align
    {
        LEFT = 0,
        CENTER = 1,
        RIGHT = 2,
        JUSTIFY = 3
    };

    // Some child elements (like <td>) might reqiure their own independent
    // formatting state. This is available to them to avoid unnecessary state
    // creation and destruction. Note that you can pool it from the child again
    // so that nested tables can be formatted without a conflict. The pooling
    // also assures that the drawing device is the same as that of the parent.
    Fl_Html_Formatter * aux_formatter ()
    {
        if (!aux_state_)
            aux_state_ =
                new Fl_Html_Formatter (drawing_device (), *right_margin_table_);
        aux_state_->drawing_device (
            drawing_device_); // assure it has the same drawing device
        return aux_state_;
    }

    int left_margin () const
    {
        return *cur_left_margin_;
    }
    int right_margin () const
    {
        return *cur_right_margin_;
    }

    int left_margin_limit () const
    {
        return cur_left_margin_[1];
    }
    int right_margin_limit () const
    {
        return cur_right_margin_[1];
    }

    int base_width () const
    {
        return *right_margin_table_;
    }

    int max_width () const
    {
        return max_width_;
    }

    void reset (int page_width, int start_y = 0, int start_align = LEFT);

    X_Align x_align () const
    {
        return (X_Align)x_align_;
    }
    void x_align (int a)
    {
        x_align_ = a;
    }

    Y_Align y_align () const
    {
        return (Y_Align)y_align_;
    }
    void y_align (int a)
    {
        y_align_ = a;
    }

    // This finishest line and places the cursor to left margin.
    // The function might push the the objects of last line down to avoid
    // collision with last previous line. It can also push these objects rigt if
    // the current x_align() is CENTER, RIGHT or JUSTIFY Parameter \a
    // paragraph_dy  tells to add extra space after line to separate paragraphs.
    // Note that several consecutive calls do not add additional seppartion as
    // long as it is as smaller than any previous, if it is bigger it adds only
    // the difference from the previous (it is the same effect as a single call
    // with biggest paragraph_dy)
    void finish_line (int paragraph_dy = 0);

    // This version is used for last line in paragreph: it never justifies last
    // line
    void finish_last_line (int paragraph_dy = 0)
    {
        int old_align = x_align ();
        if (old_align ==
            Fl_Html_Formatter::JUSTIFY) // we do not justify last line
            x_align (Fl_Html_Formatter::LEFT);
        finish_line (paragraph_dy);
        x_align (old_align);
    }

    // This finds the first possible place to fit an object of minimum width
    // min_w. To do that it finishes the line if either the object does not have
    // enough space to fit or it is a block-level element. After that it might
    // move the position further down if the width still does not fit within the
    // margins. When it finds the position with enought space it places current
    // position on the left margin. The function returns available width for the
    // object or -1 in the case min_w can not fit within global page margins
    // (shis would mean a serious error that the page width was incorrectly
    // calculated and set within the reset() call). If y_position is NULL starts
    // line at this new place, otherwise it fills that positionwith found y
    // coordivate.
    int find_place (int min_w, int finis_previous_as_last = 0);

    // Returns space left for unfinished line
    int available_width () const
    {
        return right_margin () - x_;
    }
    int no_line_objects () const
    {
        return no_line_objects_;
    }

    // sets baseline to \a y
    void baseline (int y)
    {
        baseline_ = y;
    }
    int baseline () const
    {
        return baseline_;
    }

    // Placement functions
    void place_inline (Fl_Html_Object * o);
    void place_left (Fl_Html_Object * o, int wrap);
    void place_right (Fl_Html_Object * o, int wrap);
    void place_center (Fl_Html_Object * o);

    // After formatting this returns "extra" available (nonused) width space
    int extra_space () const
    {
        return extra_space_;
    }

    // Returns extend (page height) after formetting
    int bottom () const
    {
        return bottom_;
    }

    // setting extends
    void minimum_width (int w)
    {
        int es = base_width () - w;
        if (es < extra_space_)
            extra_space_ = es;
    }
    void minimum_bottom (int h)
    {
        if (h > bottom_)
            bottom_ = h;
    }

    // Initializes object list
    void init_format (Fl_Html_Object * list);

    // this formats object list. If drawing device is not null it changes this
    // device. Note that the state old drawing decice is reset to (0,0) origin
    // and zoom 1.
    void format (Fl_Html_Object * object_list, int width,
                 Fl_Html_Drawing_Device_ * drawing_device = 0, int start_y = 0,
                 int start_align = LEFT);
    void format (Fl_Html_Object * object_list);

    // Resets formatter for start.
    void reset_for_format (int width, int start_y = 0, int start_align = LEFT);

    Fl_Html_Formatter (Fl_Html_Drawing_Device_ * d, int width,
                       int table_size = 16, int line_objects_size = 256)
        : no_left_margins_ (table_size), no_right_margins_ (table_size),
          line_objects_size_ (line_objects_size),
          todo_objects_size_ (line_objects_size),
          todo_format_objects_size_ (line_objects_size)
    {
        left_margin_table_ = new int[table_size];
        right_margin_table_ = new int[table_size];
        line_objects_ = new Fl_Html_Object *[line_objects_size];
        todo_objects_ = new Fl_Html_Object *[line_objects_size];
        todo_format_objects_ = new Fl_Html_Object *[line_objects_size];
        aux_state_ = 0;
        drawing_device_ = d;
        table_formatter_ = 0;
        left_shift_ = 0;
        space_added_ = 0;
        for (int i = 0; i < 4; i++)
        {
            aux_object_[i] = 0;
        }
        reset (width);
    }

    virtual ~Fl_Html_Formatter ()
    {
        delete[] left_margin_table_;
        delete[] right_margin_table_;
        delete[] line_objects_;
        delete[] todo_objects_;
        delete[] todo_format_objects_;
        delete aux_state_;
        remove_formatter ();
    }
};

class _Fl_Html_Reset_Font
{
    int face;
    int size;
    int color;
    Fl_Html_Drawing_Device_ * dev;

  public:
    _Fl_Html_Reset_Font (Fl_Html_Drawing_Device_ * d)
    {
        dev = d;
        face = d->font_face ();
        size = d->font_size ();
        color = d->color ();
        d->font (d->base_font_face (), d->base_font_size ());
        d->color (d->base_font_color ());
    }
    ~_Fl_Html_Reset_Font ()
    {
        dev->font (face, size);
        dev->color (color);
    }
};

#define RESET_HTML_DRAWING(d) _Fl_Html_Reset_Font _reset_drawing_cache (d)

#endif // _Fl_Html_Parser_H_
