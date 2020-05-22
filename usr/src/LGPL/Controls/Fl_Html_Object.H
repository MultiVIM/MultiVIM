#ifndef _Html_Object_H_
#define _Html_Object_H_
#include <stdio.h>
#include <typeinfo>

// Do not include this file unless you want to
// create new tags and subclas Fl_Html_Object.

#include "Fl_Html_Parser.H"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// Some usefull macros

#define XALLOC(type, size) ((type *)malloc (size * sizeof (type)))
#define XREALLOC(where, type, size)                                            \
    ((type *)realloc (where, size * sizeof (type)))
#define XFREE(where)                                                           \
    {                                                                          \
        if (where)                                                             \
        {                                                                      \
            free ((void *)where);                                              \
        }                                                                      \
    }

// #include <time/time.h>
// #include <stdio.h>

template <typename T> void xalloc (T *& where, int size)
{
    where = XALLOC (T, size);
}

template <typename T> void xrealloc (T *& where, int size)
{
    if (where)
        where = XREALLOC (where, T, size);
    else
        where = XALLOC (T, size);
}

template <typename T> void xfree (T *& where)
{
    XFREE (where);
    where = 0;
}

template <typename T> void xcopy (T * destination, const T * source, int no)
{
    memcpy (destination, source, no * sizeof (T));
}

#define _xQ(x) #x
#define _xQUOTE(x) _xQ (x)

#define BUILD_TAG(NAME, TYPE, CLOSED_BY)                                       \
    static const char * const _tag_name_##NAME = _xQUOTE (NAME);               \
    class Html_Tag_##NAME : public Fl_Html_Object                              \
    {                                                                          \
      public:                                                                  \
        const char * name () const                                             \
        {                                                                      \
            return _tag_name_##NAME;                                           \
        }                                                                      \
        int type () const                                                      \
        {                                                                      \
            return TYPE;                                                       \
        }                                                                      \
        int closed_by () const                                                 \
        {                                                                      \
            return CLOSED_BY;                                                  \
        }                                                                      \
        static Fl_Xml_Object * create ()                                       \
        {                                                                      \
            return new Html_Tag_##NAME ();                                     \
        }

const int HIGH_VAL = INT_MAX >> 3;

class Fl_Html_Formatter;

// class Fl_Html_Image;
class Fl_Html_Drawing_Device_;

//////////////////////// Tthis is the base definition for  oaal our objects
/////////////////////////////////

class Fl_Html_Object : public Fl_Html_Object_
{

  protected:
    // current position, width and height - updated during format()

    static void css_calback (Fl_Css_Parser::Property & p, void * data)
    {
        ((Fl_Html_Object *)(data))->process_style (p);
    }

    // This must be called for y2 >= y2, that is selection[1]>selection[3]
    bool in_selection (const int * selection)
    {
        if (y > selection[3])
            return false;
        if (y + h < selection[1])
            return false;
        if (y + h > selection[3] && x >= selection[2] &&
            (x >= selection[0] || y > selection[1]))
            return false;
        if (y < selection[1] && x + w <= selection[0] &&
            (x + w <= selection[2] || y + h < selection[3]))
            return false;
        return true;
    }

    virtual void in_selection_children (int * selection, int & first,
                                        int & last)
    {
        Fl_Html_Object * o = (Fl_Html_Object *)children ();
        if (!o)
            return;
        while (o)
        {
            o->in_selection (selection, first, last);
            o = (Fl_Html_Object *)(o->next ());
        }
    }

    void in_selection (int * selection, int & first, int & last)
    {
        if (y > selection[3])
            return;
        if (y + h < selection[1])
            return;
        int dr = drawable ();
        if (dr & 1)
        {
            if (y + h > selection[3] && x >= selection[2] &&
                (x >= selection[0] || y > selection[1]))
                return;
            if (y < selection[1] && x + w <= selection[0] &&
                (x + w <= selection[2] || y + h < selection[3]))
                return;
            if (!first)
                first = index ();
            last = index ();
        }
        else if (dr == 0x1000000)
            return;
        return in_selection_children (selection, first, last);
    }

    static Fl_Html_Object * get_selection_first_drawable (Fl_Html_Object * list,
                                                          int x, int y)
    {
        Fl_Html_Object * result = 0;
        while (list)
        {
            int dr = list->drawable ();
            if (dr == 0x1000000)
            {
                list = (Fl_Html_Object *)(list->next ());
                continue;
            }
            if ((list->y > y) ||
                (list->y + list->h > y && list->x + list->w > x))
            {
                if (dr & 1)
                {
                    if (!result)
                        result = list;
                }
                else
                {
                    int dx, dy;
                    list->translate_for_children (dx, dy);
                    Fl_Html_Object * ret = get_selection_first_drawable (
                        (Fl_Html_Object *)(list->children ()), x - dx, y - dy);
                    if (ret)
                        return ret;
                }
            }
            list = (Fl_Html_Object *)(list->next ());
        }
        return 0;
    }

    static void get_selection_last_drawable (Fl_Html_Object * list, int x,
                                             int y, Fl_Html_Object *& drawable)
    {
        while (list)
        {
            int dr = list->drawable ();
            if (dr == 0x1000000)
            {
                list = (Fl_Html_Object *)(list->next ());
                continue;
            }
            if (list->y > y)
                return;
            if (list->y + list->h < y)
                drawable = list;
            else if (dr & 1 && list->y < y && list->x < x)
                drawable = list;
            Fl_Html_Object * children = (Fl_Html_Object *)(list->children ());
            if (children)
            {
                int dx, dy;
                list->translate_for_children (dx, dy);
                get_selection_last_drawable (
                    children, x - dx, y - dy, drawable);
            }
            list = (Fl_Html_Object *)(list->next ());
        }
    }

    static void get_selection_last_non_drawable (Fl_Html_Object * list, int x,
                                                 int y,
                                                 Fl_Html_Object *& drawable)
    {
        while (list)
        {
            int dr = list->drawable ();
            if (dr == 0x1000000)
            {
                list = (Fl_Html_Object *)(list->next ());
                continue;
            }
            if (list->y > y)
                return;
            if ((dr & 1) && ((list->y + list->h < y) ||
                             (list->y < y && list->x + list->w < x)))
                drawable = list;
            Fl_Html_Object * children = (Fl_Html_Object *)(list->children ());
            if (children)
            {
                int dx, dy;
                list->translate_for_children (dx, dy);
                get_selection_last_non_drawable (
                    children, x - dx, y - dy, drawable);
            }
            list = (Fl_Html_Object *)(list->next ());
        }
    }

    static void get_selection_first_last (Fl_Html_Object * list, int x, int y,
                                          int & first, int & last)
    {
        while (list)
        {
            int dr = list->drawable ();
            if (dr == 0x1000000)
            {
                list = (Fl_Html_Object *)(list->next ());
                continue;
            }

            Fl_Html_Object * n = (Fl_Html_Object *)(list->next ());
            bool inside = 0;
            bool overlapping;
            if (list->point_in_selection_box (x, y, overlapping))
            {
                inside = 1;
                if (dr & 1)
                {
                    first = list->index () - 1;
                    last = first + 2;
                    return;
                }
                int dx, dy;
                list->translate_for_children (dx, dy);
                x -= dx;
                y -= dy;

                get_selection_first_last (
                    ((Fl_Html_Object *)(list->children ())), x, y, first, last);
                x += dx;
                y += dy;
                if (!overlapping)
                { // non-overlapping
                    if (list->index () - 1 > first)
                        first = list->index () - 1;

                    else if (n)
                    {
                        if (n->index () < last)
                        {
                            last = n->index ();
                            // printf("ll: %i\n", last);
                        }
                    }
                }
            }
            if (!inside)
            {
                if ((list->y + list->h < y) ||
                    (list->y < y && x > list->x + list->w))
                {
                    // if(n){
                    if (list->index () > first)
                        first = list->index ();
                    //}
                }
                if ((list->y > y) || (list->y + list->h > y && list->x > x))
                {
                    if (n)
                    {
                        if (n->index () < last)
                            last = n->index ();
                    }
                    else if (!list->children ())
                    {
                        if (list->index () < last)
                            last = list->index ();
                    }
                    // if(!overlapping && (list->index() <last))
                    //   last = list->index();
                }
            }
            list = n;
        }
    }

  protected:
    class Text_Buffer
    {
        int size_;
        const char * newline_;
        int position_;
        char * buffer_;
        int newlines_;
        int tabs_;

      public:
        void add_tab ()
        {
            tabs_++;
        }
        void remove_tab ()
        {
            tabs_--;
        }
        int newlines () const
        {
            return newlines_;
        }
        void newlines (int n)
        {
            newlines_ = n;
        }
        void add (const char * str, int len = 0)
        {
            if (!str)
                return;
            if (!len)
                len = strlen (str);
            int pl = position_ + len + 1;
            if (pl > size_)
            {
                pl *= 2;
                char * nb = new char[pl];
                memcpy (nb, buffer_, position_ + 1);
                delete[] buffer_;
                buffer_ = nb;
            }
            memcpy (buffer_ + position_, str, len);
            position_ += len;
            buffer_[position_] = 0;
        }
        void add_newline ()
        {
            add (newline_);
            newlines_++;
            for (int i = 0; i < tabs_; i++)
                add ("\t");
        }
        int length () const
        {
            return position_;
        }
        char * buffer ()
        {
            char * b = buffer_;
            buffer_ = 0;
            size_ = 0;
            return b;
        }
        Text_Buffer (int n = 1024, const char * newline = "\n")
            : size_ (n), newline_ (newline), position_ (0),
              newlines_ (0xFFFFFF), tabs_ (0)
        {
            buffer_ = new char[n];
            *buffer_ = 0;
        }
        ~Text_Buffer ()
        {
            delete[] buffer_;
        }
    };
    static void export_selection (Fl_Html_Object::Text_Buffer & text,
                                  Fl_Html_Object * list, int first, int last,
                                  int type);

    virtual void export_start (Text_Buffer & text, int type) const
    {
    }
    virtual void export_end (Text_Buffer & text, int type) const
    {
    }
    virtual void export_body (Text_Buffer & text, int type) const
    {
    }

  protected:
    bool point_in_bounding_box_ (int X, int Y) const
    {
        if (Y < y)
            return false;
        if (Y > y + h)
            return false;
        if (X < x)
            return false;
        if (X > x + w)
            return false;
        return true;
    }

    bool point_in_bounding_region_ (int X, int Y) const
    {
        if (Y < y)
            return false;
        // if(Y>y+h) return false;
        return true;
    }

  public:
    enum Flags
    {
        ALIGN_MASK = 3,
        ALIGN_LEFT = 0,
        ALIGN_CENTER = 1,
        ALIGN_RIGHT = 2,
        ALIGN_JUSTIFY = 3,

        VALIGN_MASK = 4 | 8,
        VALIGN_TOP = 0,
        VALIGN_MIDDLE = 4,
        VALIGN_BOTTOM = 8,
        VALIGN_BASELINE = 4 | 8,

        WIDTH_PERCENT = 0x10,
        HEIGHT_PERCENT = 0x20,

        NOWRAP = 0x40,
        BGCOLOR = 0x80,

        ALIGN_WRAP = 0x100,
        ALIGN_INLINE = 0x200,

        ALIGN_SET = 0x400,
        VALIGN_SET = 0x800,
        NOSHADE = 0x1000

    };

    enum Bullet_Type
    {
        BULLET_DISC = 0x100,
        BULLET_SQUARE = 0x200,
        BULLET_CIRCLE = 0x300,
        BULLET_NUMBER = 1,
        BULLET_a = 'a',
        BULLET_A = 'A'
    };

    // Following are available after format() - before they are just rubbisd.
    int x;
    int y;
    int w;
    int h;

    // This is ececured only once right after parsing
    // The purpose is to calculate minimum width of the objects.

    // After init_format at least min_w() should be known.
    int min_page_width () const;
    virtual const char * word () const
    {
        return 0;
    }
    virtual bool selection_bounding () const
    {
        return false;
    }

    virtual void process_style (const Fl_Css_Parser::Property & p)
    {
    }
    bool handle_style (const char * name, int name_len, const char * value,
                       int value_len)
    {
        if (!is_word (name, "style", name_len))
            return false;
        Fl_Css_Parser parser;
        Fl_Css_Parser::Property p;
        parser.parse_inline (value, value_len, &css_calback, p, this);
        return true;
    }

    virtual bool point_in_bounding_box (int X, int Y) const
    {
        return point_in_bounding_box_ (X, Y);
    }
    virtual bool point_in_selection_box (int X, int Y, bool & overlapping) const
    {
        overlapping = false;
        return point_in_bounding_box_ (X, Y);
    }

    //  virtual bool point_within(int X, int Y) const = 0;
    virtual int drawable () const = 0;

    virtual void init_format (Fl_Html_Formatter * s) = 0;
    void init_format_children (Fl_Html_Formatter * s)
    {
        Fl_Html_Object * o = (Fl_Html_Object *)aux ();
        while (o)
        {
            o->init_format (s);
            o = (Fl_Html_Object *)o->next ();
        }
    };

    void set_out ()
    {
        x = HIGH_VAL;
        y = HIGH_VAL;
        w = -2 * HIGH_VAL;
        h = -2 * HIGH_VAL;
    }
    void handle_bullet (Fl_Html_Formatter * s);

    // must be called AFTER all children are formatted
    void bounding_box_children (int & x, int & y, int & w, int & h)
    {
        int x1 = x;
        int y1 = y;
        int x2 = x + w;
        int y2 = y + h;
        Fl_Html_Object * o = (Fl_Html_Object *)aux ();
        while (o)
        {
            int a = o->x;
            if (a < x1)
                x1 = a;
            a += o->w;
            if (a > x2)
                x2 = a;
            a = o->y;
            if (a < y1)
                y1 = a;
            a += o->h;
            if (a > y2)
                y2 = a;
            o = (Fl_Html_Object *)o->next ();
        }
        x = x1;
        y = y1;
        w = x2 - x1;
        h = y2 - y1;
    };

    void bounding_box_children ()
    {
        bounding_box_children (x, y, w, h);
    }

    virtual void format (Fl_Html_Formatter * s) = 0;

    static void format_list (Fl_Html_Object * o, Fl_Html_Formatter * s)
    {
        while (o)
        {
            o->format (s);
            o = (Fl_Html_Object *)o->next ();
        }
    }
    void format_children (Fl_Html_Formatter * s)
    {
        format_list ((Fl_Html_Object *)children (), s);
    }

    /*
    void move_children(int dx, int dy) {
      Fl_Html_Object * o = (Fl_Html_Object *)(children());
      while(o) {
        o->x += dx;
        o->y += dx;
        o->move_children(dx, dy);
        o = (Fl_Html_Object *) o->next();
      }
    }
    */

    // This gives the minimum possible width of the page
    // to fit the object and all subobjects on the page.
    // It should be availabke after init_format().

    virtual void min_width (int & width) const
    {
        min_width_children (width);
    };
    void min_width_children (int & width) const
    {
        Fl_Xml_Object * o = children ();
        while (o)
        {
            ((Fl_Html_Object *)o)->min_width (width);
            o = o->next ();
        }
    }

    virtual void translate_for_children (int & dx, int & dy)
    {
        dx = 0;
        dy = 0;
    }

    virtual bool no_body () const
    {
        return false;
    }
    virtual const char * link () const
    {
        return 0;
    }
    virtual const char * anchor () const
    {
        return 0;
    }

    virtual void draw (Fl_Html_Drawing_Device_ * d) = 0;
    void draw_children (
        Fl_Html_Drawing_Device_ * d /*, bool check_selection = true */);
    bool in_drawing_area (const Fl_Html_Drawing_Device_ * d) const;

    static bool read_align (unsigned & flags, const char * name, int name_len,
                            const char * value, int value_len)
    {
        if (is_word (name, "align", name_len))
        {
            if (Fl_Html_Object_::is_first_word (value, "center", value_len))
            {
                flags &= ~ALIGN_MASK;
                flags |= ALIGN_CENTER;
                flags |= ALIGN_SET;
            }
            else if (Fl_Html_Object_::is_first_word (value, "right", value_len))
            {
                flags &= ~ALIGN_MASK;
                flags |= ALIGN_RIGHT;
                flags |= ALIGN_SET;
            }
            else if (Fl_Html_Object_::is_first_word (
                         value, "justify", value_len))
            {
                flags &= ~ALIGN_MASK;
                flags |= ALIGN_JUSTIFY;
                flags |= ALIGN_SET;
            }
            else if (Fl_Html_Object_::is_first_word (value, "left", value_len))
            {
                flags &= ~ALIGN_MASK;
                flags |= ALIGN_LEFT;
                flags |= ALIGN_SET;
            };
            return true;
        }
        return false;
    }

    static void read_valign (unsigned & flags, const char * value,
                             int value_len)
    {
        if (Fl_Html_Object_::is_first_word (value, "top", value_len))
        {
            flags &= ~VALIGN_MASK;
            flags |= VALIGN_TOP;
        }
        else if (Fl_Html_Object_::is_first_word (value, "bottom", value_len))
        {
            flags &= ~VALIGN_MASK;
            flags |= VALIGN_BOTTOM;
        }
        else if (Fl_Html_Object_::is_first_word (value, "middle", value_len))
        {
            flags &= ~VALIGN_MASK;
            flags |= VALIGN_MIDDLE;
        }
        else if (Fl_Html_Object_::is_first_word (value, "baseline", value_len))
        {
            flags &= ~VALIGN_MASK;
            flags |= VALIGN_BASELINE;
        };
    }

    static unsigned read_with_percents (const char * str, int & value,
                                        bool & percents, int str_len);
    static unsigned read_color (int & value, const char * str, int str_len);
    static int url_decode (char * dec, const char * s, int s_len = 0);
    // static int extract_title(const Fl_Html_Object * list, char * buffer, int
    // buf_len);

    struct Finder
    {
        virtual Fl_Html_Object * valid (Fl_Html_Object * o) = 0;
        Fl_Html_Object * find (Fl_Html_Object * o)
        {
            while (o)
            {
                Fl_Html_Object * nc = valid (o);
                if (nc)
                    return nc;
                o = (Fl_Html_Object *)(o->next ());
            }
            return 0;
        }
    };

    class Iterator
    {
        Fl_Html_Object * spt[16];
        Fl_Html_Object ** pt;
        Fl_Html_Object ** dpt;
        int pt_size;
        int np;

      public:
        void reset ()
        {
            np = 0;
        }
        int no_parents () const
        {
            return np;
        }
        Fl_Html_Object * parent (int i) const
        {
            return pt[i];
        }
        void add_parent (Fl_Html_Object * o)
        {
            if (!o)
                return;
            int np_1 = np;
            np++;
            if (np > pt_size)
            {
                pt_size = 2 * np;
                Fl_Html_Object ** n_dpt = new Fl_Html_Object *[pt_size];
                for (int i = 0; i < np_1; i++)
                    n_dpt[i] = pt[i];
                delete[] dpt;
                dpt = n_dpt;
                pt = n_dpt;
            }
            pt[np_1] = o;
        }
        void remove_parent ()
        {
            if (np)
                np--;
        }
        Fl_Html_Object * next (Fl_Html_Object * o, bool descend = true)
        {
            if (!o)
                return 0;
            Fl_Html_Object * c = descend ? (Fl_Html_Object *)o->children () : 0;
            if (c)
            {
                add_parent (o);
                return c;
            }
            c = (Fl_Html_Object *)o->next ();
            while (!c)
            {
                if (!np)
                    return 0;
                np--;
                c = (Fl_Html_Object *)(pt[np]->next ());
                pt[np] = c;
            }
            return c;
        }
        Iterator () : pt (spt), dpt (0), pt_size (16), np (0)
        {
        }
        virtual ~Iterator ()
        {
            delete[] dpt;
        }
    };

    /**
     * Try to take the keyboard focus. Return true if taken. Otherwise return
     * false.
     */
    virtual bool takeFocus (bool should)
    {
        return false;
    }

    // Some useful functions.
    // We keep these in Fl_Html_Object not to polute global namespace.

    static unsigned extract_title (Fl_Html_Object * o, char * title,
                                   unsigned len);
    static Fl_Html_Object * first_drawable (Fl_Html_Object::Iterator * it,
                                            Fl_Html_Object * o, int y,
                                            const Fl_Html_Object * limit = 0);
    static void global_coordinates (const Fl_Html_Object::Iterator * parents,
                                    const Fl_Html_Object * o, int & x, int & y);

    static Fl_Html_Object * point_within (Fl_Html_Object::Iterator * it,
                                          Fl_Html_Object * o, int x, int y);
    static const char * get_word_value (const Fl_Html_Object * o);
    static Fl_Html_Object * find_anchor (const char * link,
                                         Fl_Html_Object * list);
    // static Fl_Html_Object * firs_drawable_from(int x, int y, Fl_Html_Object *
    // list, int error)

    // This function translates URI to local filename which can then be open by
    // fl_fopen(). If directory is not null, the url is relative and \a
    // directory is prepended to the filename. The filename is stripped from
    // anchor (if any), this anchor is copied to array \a anchor. To avoid
    // buffer overflow please use \a filename and \a anchor of size at least \a
    // url_len + strlen(directory) + 1.
    static int local_filename_from_uri (char * filename, char * anchor,
                                        const char * directory,
                                        const char * url, int url_len,
                                        bool percent_encoded = 0);

    // Returns common parent
    static Fl_Html_Object * get_selection (Fl_Html_Object * list, int x1,
                                           int y1, int x2, int y2, int & first,
                                           int & last);

    enum Clipboard_Format
    {
        CLIPBOARD_TEXT = 0
    };
    static char * export_selection (Fl_Html_Object * list, int first, int last,
                                    int type = CLIPBOARD_TEXT);
};

class NewIter
{
  public:
    Fl_Html_Object * curObj;

    NewIter (Fl_Html_Object * obj = NULL) : curObj (obj)
    {
    }

    Fl_Html_Object * next ()
    {
        Fl_Html_Object * candidate;
        if ((candidate = (Fl_Html_Object *)curObj->next ()))
        {
            curObj = candidate;
            printf ("Next: %s\n", typeid (*curObj).name ());
            return curObj;
        }
        else
        {
            printf ("%s must descend into children\n",
                    typeid (*curObj).name ());
            return curObj;
            /* code */
        }
    }

    Fl_Html_Object * prev ()
    {
        Fl_Html_Object * candidate;
        if ((candidate = (Fl_Html_Object *)curObj->prev ()))
        {
            curObj = candidate;
            printf ("Prev: %s\n", typeid (*curObj).name ());
            return curObj;
        }
        else
        {
            printf ("%s must ascend into parent\n", typeid (*curObj).name ());
            return curObj;
            /* code */
        }
    }
};

#endif // _Html_Object_H_
