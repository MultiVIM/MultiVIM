#include "Fl_Html_Object.H"
#include "Fl_Html_Formatter.H"
#include <math.h>
#include <stdio.h>
#include <string.h>

bool Fl_Html_Object::in_drawing_area (const Fl_Html_Drawing_Device_ * d) const
{
    return x <= d->x () + d->w () && x + w >= d->x () &&
           y <= d->y () + d->h () && y + h >= d->y ();
}

void Fl_Html_Drawing_Device_::draw (Fl_Html_Object * o, int x, int y, float z,
                                    int box_x, int box_y, int box_w, int box_h)
{
    zoom (z);
    origin (x, y);
    init_draw ();
    z = 1.0 / z;
    bounding_box (floor ((box_x - x) * z),
                  floor ((box_y - y) * z),
                  ceil (box_w * z + 1.0001),
                  ceil (box_h * z + 1.001));
    while (o)
    {
        if (o->in_drawing_area (this))
            // if(o->x <= this->x() + this->w() && o->x + o->w >= this->x() &&
            // o->y <= this->y()+ this->h() && o->y + o->h >= this->y())
            o->draw (this);
        o = (Fl_Html_Object *)(o->next ());
    }
}

void html_draw (Fl_Html_Object * o, Fl_Html_Drawing_Device_ * d, int x, int y,
                float zoom, int box_x, int box_y, int box_w, int box_h)
{
    d->draw (o, x, y, zoom, box_x, box_y, box_w, box_h);
}

void Fl_Html_Object::draw_children (
    Fl_Html_Drawing_Device_ * d /*, bool check_selection */)
{
    Fl_Xml_Object * c = children ();
    if (!c)
        return;
    /*
    bool sv = d->selection_valid();
    if(sv && check_selection)
      d->selection_valid(in_selection(d->selection()));
    */
    while (c)
    {
        Fl_Html_Object * o = (Fl_Html_Object *)c;
        //      double t1 = ci::time();
        if (o->in_drawing_area (d))
            // if(o->x <= d->x() + d->w() && o->x + o->w >= d->x() && o->y <=
            // d->y()+ d->h() && o->y + o->h >= d->y())
            o->draw (d);
        /*
        t1 = ci::time() - t1;
        if(t1>0.01){
          if(o->name())
            printf("Draw Time for %s: %g\n", o->name(), t1);
          else
           printf("Draw Time for word: %g\n", t1);

        }
        */
        c = c->next ();
    }
    // d->selection_valid(sv);
}

unsigned Fl_Html_Object::read_with_percents (const char * str, int & value,
                                             bool & percents, int str_len)
{

    long long v;
    percents = false;
    int ret = Fl_Html_Tokenizer::int_value (
        str, v, Fl_Html_Tokenizer::FORMAT_10, str_len);
    if (ret && v > 0)
    {
        value = v;
        if (ret < str_len)
        {
            str += ret;
            str_len -= ret;
            int a = Fl_Html_Tokenizer::spaces (str, str_len);
            ret += a;
            if (str_len > a && str[a] == '%')
            {
                ret++;
                percents = true;
            }
        }
        return ret;
    }
    return 0;
    /*


      int vvv;
      double f_value = 0;
      percents = false;
      int ret = Fl_Html_Tokenizer::float_value(str, f_value, str_len);
      vvv = int(f_value+0.5);
      if(ret && vvv>0) {
        value = vvv;
        if(ret<str_len) {
          str += ret;
          str_len -=ret;
          int a = Fl_Html_Tokenizer::spaces(str, str_len);
          ret +=a;
          if(str_len>a && str[a]=='%') {
            ret++;
            percents = true;
          }
        }
        return ret;
      }
      return 0;
      */
}

void Fl_Html_Object::handle_bullet (Fl_Html_Formatter * s)
{
    if (!s->bullet ())
    {
        s->bullet (this);
        s->bullet_x (0);
    }
}

///////////////////////////////////////////////////////////
///////////////////////  All elements  ////////////////////
///////////////////////////////////////////////////////////

///////////////   Html_Word  ///////////////////////////

class Html_Word : public Fl_Html_Object
{
  protected:
    char * value_;
    int len;
    int space;

  public:
    void export_body (Text_Buffer & text, int type) const
    {
        text.add (value_);
        text.newlines (0);
    }
    int drawable () const
    {
        return 1;
    }
    const char * word () const
    {
        return value_;
    }

    // Implementing virtual functions of Fl_Xml_Object
    const char * name () const
    {
        return 0;
    };
    void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                            const char * value, int value_len){};
    const char * value () const
    {
        return value_;
    };
    Fl_Xml_Object * children () const
    {
        return 0;
    } // word does not have children - aux_ is used for concated word

    /*
      bool point_within(int X, int Y) const {
        if(X<x) return false;
        if(X>x+w + space) return false;
        if(Y<y) return false;
        if(Y>y+h) return false;
        return true;
      }
    */
    // Implementing virtual functions of Fl_Html_Object_
    int type () const
    {
        return 0;
    };
    int closed_by () const
    {
        return 0;
    };

    bool point_in_bounding_box (int X, int Y) const
    {
        if (X < x)
            return false;
        if (X > x + w + space)
            return false;
        if (Y < y)
            return false;
        if (Y > y + h)
            return false;
        return true;
    }

    //  bool closed_by_similar() const {return false;};

    // Implementing virtual functions of Fl_Html_Object_
    void init_format (Fl_Html_Formatter * s)
    {
        len = strlen (value_);
        if (value_[len - 1] == ' ')
        {
            len--;
            space = s->drawing_device ()->space ();
        }
        else
            space = 0;
        s->drawing_device ()->measure (value_, len, w, h);
    }
    void format (Fl_Html_Formatter * s)
    {
        s->check_space ();
        s->find_place (Html_Word::multiple_word_width ());
        s->place_inline (this);

        s->add_space (space);
        // handle_bullet(s);

        s->y_align (Fl_Html_Formatter::BASELINE);
        s->baseline (y + h - s->drawing_device ()->descent ());
    }

    int multiple_word_width () const
    {
        Html_Word * o = (Html_Word *)aux ();
        if (o)
            return w + ((Html_Word *)o)->multiple_word_width ();
        return w;
    }
    void min_width (int & width) const
    {
        int w = multiple_word_width ();
        if (w > width)
            width = w;
    }

    void draw (Fl_Html_Drawing_Device_ * d)
    {
        // if(d->selection_valid() && in_selection(d->selection())){
        if (index () <= d->selection_last () &&
            index () >= d->selection_first ())
        {
            int old_color = d->color ();
            int background, foreground;
            d->contrast_selection (background, foreground);
            d->color (background);
            int sp = space;
            if (!sp)
                sp = d->space ();
            sp /= 2;
            d->rectf (x - sp - 1, y, w + sp + space / 2 + 1, h, true);
            d->color (foreground);
            d->draw (value (), len, x, y + h);
            d->color (old_color);
        }
        else
            d->draw (value (), len, x, y + h);
    }

    Html_Word (Fl_Html_Tokenizer::Special_Character_Table * t, const char * src,
               int src_len)
    {
        t->translate_copy (src, src_len, value_);
    }

    // This constructor copies the string "as-is" without translation (used fo
    // words within <!CDATA[[ ... ]]> )
    Html_Word (const char * src, int src_len)
    {
        value_ = new char[src_len + 2];
        memcpy (value_, src, src_len);
        space = 0;
        value_[src_len] = 0;     // null termination
        value_[src_len + 1] = 0; // null termination in a case a space is added
    }
    ~Html_Word ()
    {
        delete[] value_;
    }
};

// This is for <pre> tag only

class Html_Line_Word : public Html_Word
{
    char * preformated_value_;
    int preformated_len_;

  public:
    void min_width (int & width) const
    {
        if (w > width)
            width = w;
    }
    void format (Fl_Html_Formatter * s)
    {
        s->check_space ();
        s->find_place (w);
        s->place_inline (this);
        s->y_align (Fl_Html_Formatter::BASELINE);
        s->baseline (y + h - s->drawing_device ()->descent ());
        s->finish_line ();
    }
    void init_format (Fl_Html_Formatter * s)
    {
        space = 0;
        s->drawing_device ()->measure (
            preformated_value_, preformated_len_, w, h);
    }
    void draw (Fl_Html_Drawing_Device_ * d)
    {
        // if(d->selection_valid() && in_selection(d->selection())){
        if (index () <= d->selection_last () &&
            index () >= d->selection_first ())
        {
            int old_color = d->color ();
            int background, foreground;
            d->contrast_selection (background, foreground);
            d->color (background);
            int sp = d->space () / 2;
            d->rectf (x - sp - 1, y, w + sp + space / 2 + 1, h, true);
            d->color (foreground);
            d->draw (preformated_value_, preformated_len_, x, y + h);
            d->color (old_color);
        }
        else
            d->draw (preformated_value_, preformated_len_, x, y + h);
    }
    ~Html_Line_Word ()
    {
        delete[] preformated_value_;
    }
    Html_Line_Word (Fl_Html_Tokenizer::Special_Character_Table * t,
                    const char * src, int src_len)
        : Html_Word (t, src, src_len)
    {
        len = strlen (value_);
        int count = 0;
        for (int i = 0; i < len; i++)
            if (value_[i] == '\t')
                count++;
        preformated_value_ = new char[len + 7 * count + 1];
        int where = 0;
        for (int i = 0; i < len; i++)
        {
            char c = value_[i];
            if (c == '\t')
            {
                preformated_value_[where] = ' ';
                where++;
                while (where % 8)
                {
                    preformated_value_[where] = ' ';
                    where++;
                }
            }
            else
            {
                preformated_value_[where] = c;
                where++;
            }
        }
        preformated_len_ = where;
        preformated_value_[where] = 0;
    }
};

////////////// Html_Unknown_Tag ////////////////////////

class Html_Unknown_Tag : public Fl_Html_Object
{
    char * name_;

  public:
    int drawable () const
    {
        return 0;
    }
    /*  bool point_within(int X, int Y) const {
        if(!point_in_bounding_box(X, Y)) return false;
        return point_within_children(X, Y);
      } */
    bool point_in_selection_box (int X, int Y, bool & overlapping) const
    {
        overlapping = true;
        return point_in_bounding_region_ (X, Y);
    }
    void init_format (Fl_Html_Formatter * s)
    {
        w = 0;
        h = 0;
        init_format_children (s);
    }
    void format (Fl_Html_Formatter * s)
    {
        format_children (s);
        // setting placement to bounding box of children
        set_out ();
        s->add_todo_object (this);
    }
    const char * name () const
    {
        return name_;
    }
    int type () const
    {
        return 0;
    };
    int closed_by () const
    {
        return 0;
    };
    //  bool closed_by_similar() const {return false;};
    Html_Unknown_Tag (const char * src, int src_len)
    {
        name_ = new char[src_len + 2];
        memcpy (name_, src, src_len);
        name_[src_len] = 0;     // null termination
        name_[src_len + 1] = 0; // null termination in a case a space is added
    }
    ~Html_Unknown_Tag ()
    {
        delete[] name_;
    }
    void draw (Fl_Html_Drawing_Device_ * d)
    {
        draw_children (d);
    }

    void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                            const char * value, int value_len){};
};

#define TAG_ENTRY(NAME)                                                        \
    {                                                                          \
        _tag_name_##NAME, (const void *)(&Html_Tag_##NAME::create)             \
    }

/////////////// <ul>  ///////////////////

static const char * bullet_disc = "\xE2\x80\xA2 ";
static const char * bullet_square = "\xE2\x96\xAA ";
static const char * bullet_circle = "\xE2\x97\x8B ";

BUILD_TAG (ul, Fl_Html_Object::BLOCK, 0)
int bullet_type;
int bullet_width;
int bullet_height;
int padding;
int bullet_start;

/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
} */
int drawable () const
{
    return 0;
}

void export_start (Text_Buffer & text, int type) const
{

    if (!text.newlines ())
        text.add_newline ();
    text.add_tab ();
    text.add ("\t");
}

void export_end (Text_Buffer & text, int type) const
{
    text.remove_tab ();
    // if(!text.newlines())
    //  text.add_newline();
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}

void process_style (const Fl_Css_Parser::Property & p)
{
    if (is_word (p.property (), "padding-left", p.property_length ()))
    {
        if (!p.no_values ())
            return;
        long long bb = 0;
        Fl_Html_Tokenizer::int_value ((p.values ())[0],
                                      bb,
                                      Fl_Html_Tokenizer::FORMAT_10,
                                      (p.values_lengths ())[0]);
        if (bb >= 0)
            padding = bb;
        return;
    }
}
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (is_word (name, "type", name_len))
    {
        if (Fl_Html_Object_::is_first_word (value, "square", value_len))
            bullet_type = BULLET_SQUARE;
        else if (Fl_Html_Object_::is_first_word (value, "circle", value_len))
            bullet_type = BULLET_CIRCLE;
        else
            bullet_type = BULLET_DISC;
        return;
    }
    if (handle_style (name, name_len, value, value_len))
        return;
}

void format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int p_indent = d->font_size () / 2;
    Fl_Html_Object * old_bullet = s->bullet ();
    s->bullet (0);
    s->finish_last_line (p_indent);
    s->add_left_shift (bullet_width);
    format_children (s);
    s->finish_last_line (p_indent);
    s->add_left_shift (-bullet_width);
    // s->bullet_x(s->bullet_x()- bullet_width);
    set_out ();
    bounding_box_children ();
    if (old_bullet)
        s->bullet (old_bullet);
    else
        s->bullet_x (s->bullet_x () + bullet_width);
}

static void preset_bullet (Fl_Html_Formatter * d, int & old_bullet_width,
                           int & old_bullet_height, int & old_bullet_type,
                           int & old_bullet_index, int & bullet_type,
                           int bullet_index)
{

    if (!bullet_type)
    {
        bullet_type = d->bullet_type ();
        if (bullet_type == BULLET_CIRCLE)
            bullet_type = BULLET_SQUARE;
        else if (bullet_type == BULLET_DISC)
            bullet_type = BULLET_CIRCLE;
        else
            bullet_type = BULLET_DISC;
    }

    old_bullet_width = d->bullet_width ();
    old_bullet_height = d->bullet_height ();
    old_bullet_type = d->bullet_type ();
    old_bullet_index = d->bullet_index ();
    d->bullet_width (0);
    d->bullet_height (0);
    d->bullet_type (bullet_type);
    if (bullet_type == 'a')
        d->bullet_index ('a' + bullet_index - 1);
    else if (bullet_type == 'A')
        d->bullet_index ('A' + bullet_index - 1);
    else
        d->bullet_index (bullet_index);
}

static void postset_bullet (Fl_Html_Formatter * d, int & old_bullet_width,
                            int & old_bullet_height, int & old_bullet_type,
                            int & old_bullet_index)
{
    d->bullet_width (old_bullet_width);
    d->bullet_height (old_bullet_height);
    d->bullet_type (old_bullet_type);
    d->bullet_index (old_bullet_index);
}

void init_format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_bullet_width;
    int old_bullet_height;
    int old_bullet_type;
    int old_bullet_index;

    int bt = bullet_type;

    preset_bullet (s,
                   old_bullet_width,
                   old_bullet_height,
                   old_bullet_type,
                   old_bullet_index,
                   bt,
                   bullet_start);

    if (bt == 0x200)
    {
        d->measure (
            bullet_square, strlen (bullet_square), bullet_width, bullet_height);
        s->bullet_width (bullet_width);
        s->bullet_height (bullet_height);
    }
    else if (bt == 0x100)
    {
        d->measure (
            bullet_disc, strlen (bullet_disc), bullet_width, bullet_height);
        s->bullet_width (bullet_width);
        s->bullet_height (bullet_height);
    }
    else if (bt == 0x300)
    {
        d->measure (
            bullet_circle, strlen (bullet_circle), bullet_width, bullet_height);
        s->bullet_width (bullet_width);
        s->bullet_height (bullet_height);
    }
    else
    {
        bullet_width = 0;
        bullet_height = 0;
    }
    if (padding > bullet_width)
    {
        s->bullet_width (padding);
        bullet_width = padding;
    }
    init_format_children (s);

    if (bt && bt < 0x100)
    {
        bullet_width = s->bullet_width ();
        bullet_height = s->bullet_height ();
    }
    postset_bullet (s,
                    old_bullet_width,
                    old_bullet_height,
                    old_bullet_type,
                    old_bullet_index);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    draw_children (d);
    d->discontinue_underline ();
}

void min_width (int & width)
{
    int w = 0;
    min_width_children (w);
    w += bullet_width;
    if (w > width)
        width = w;
}

Html_Tag_ul () : bullet_type (0), padding (40), bullet_start (1)
{
}
}
;

static const char * const _tag_name_ol = "ol";
class Html_Tag_ol : public Html_Tag_ul
{
  public:
    void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                            const char * value, int value_len)
    {
        if (is_word (name, "type", name_len))
        {
            if (Fl_Html_Object_::is_first_word (value, "a", value_len))
            {
                if (*value == 'A')
                    bullet_type = 'A';
                else
                    bullet_type = 'a';
            }
            else
                bullet_type = BULLET_NUMBER;
            return;
        }
        if (is_word (name, "start", name_len))
        {
            long long bb = HIGH_VAL;
            Fl_Html_Tokenizer::int_value (
                value, bb, Fl_Html_Tokenizer::FORMAT_10, value_len);
            if (bb == HIGH_VAL)
                bullet_start = 1;
            else
                bullet_start = bb;
            return;
        }
        if (handle_style (name, name_len, value, value_len))
            return;
    }

    Html_Tag_ol ()
    {
        bullet_type = BULLET_NUMBER;
    }
    const char * name () const
    {
        return _tag_name_ol;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_ol ();
    }
};

BUILD_TAG (li, Fl_Html_Object::BLOCK | Fl_Html_Object::LI, Fl_Html_Object::LI)

int value;
int bullet_shift;
Fl_Html_Object * bullet;
int bullet_width;
int bullet_height;
int bullet_size;
const char * bullet_string;
char * bullet_copied;
Fl_Html_Object *
    bullet_object; // objec in front oof which the bullet should be drawn
bool value_int;

Html_Tag_li ()
{
    bullet_copied = 0;
    bullet_string = 0;
    value = HIGH_VAL;
    value_int = false;
}
~Html_Tag_li ()
{
    delete[] bullet_copied;
}

int drawable () const
{
    return 2;
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
    text.add (bullet_string);
}

void export_end (Text_Buffer & text, int type) const
{
}

void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * val, int val_len)
{
    if (is_word (name, "value", name_len))
    {
        long long bb = HIGH_VAL;
        Fl_Html_Tokenizer::int_value (
            val, bb, Fl_Html_Tokenizer::FORMAT_10, val_len);
        if (bb == HIGH_VAL)
        {
            value = val[0];
            value_int = false;
        }
        else
        {
            value = bb;
            value_int = true;
        }
        return;
    }
}

void format (Fl_Html_Formatter * s)
{
    s->finish_last_line ();
    // Fl_Html_Drawing_Device_ * d = s->drawing_device();
    Fl_Html_Object * old_bullet = s->bullet ();
    s->bullet (0);
    format_children (s);
    s->finish_last_line ();
    bullet = s->bullet ();
    if (s->bullet ())
    {
        bullet_shift = s->bullet_x ();
    }
    else
        bullet_string = 0;
    set_out ();
    bounding_box_children ();
    if (old_bullet)
        s->bullet (old_bullet);
}

static void sprintf_bullet (char * buffer, Fl_Html_Formatter * s, int index)
{
    if (s->bullet_type () == 1)
        snprintf (buffer, 256, "%i.", index);
    else
        snprintf (buffer, 256, "%c.", index);
    index++;
    if (s->bullet_type () == 'a')
    {
        if (index > 'z')
            index = 'a';
    }
    else if (s->bullet_type () == 'A')
    {
        if (index > 'Z')
            index = 'A';
    }
    s->bullet_index (index);
}

void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
    char buffer[256];
    delete[] bullet_copied;
    bullet_copied = 0;
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    if (s->bullet_type () > 0xFF)
    { // non-counting bullet
        bullet_width = s->bullet_width ();
        bullet_height = s->bullet_height ();
        if (s->bullet_type () == Html_Tag_ul::BULLET_SQUARE)
            bullet_string = bullet_square;
        else if (s->bullet_type () == Html_Tag_ul::BULLET_CIRCLE)
            bullet_string = bullet_circle;
        else
            bullet_string = bullet_disc;
        int blh;
        d->measure (bullet_string, strlen (bullet_string), bullet_size, blh);
    }
    else
    {
        int vv = value;
        if (value_int)
        {
            if (s->bullet_type () == 'a')
            {
                vv = 'a' + value - 1;
                if (vv < 'a')
                    vv = 'a';
                else if (vv > 'z')
                    vv = 'z';
            }
            else if (s->bullet_type () == 'A')
            {
                vv = 'A' + value - 1;
                if (vv < 'A')
                    vv = 'A';
                else if (vv > 'Z')
                    vv = 'Z';
            }
        }
        sprintf_bullet (
            buffer, s, (value == HIGH_VAL) ? s->bullet_index () : vv);
        int len = strlen (buffer) + 1;
        bullet_string = bullet_copied = new char[len];
        memcpy (bullet_copied, buffer, len);
        d->measure (buffer, strlen (buffer), bullet_width, bullet_height);
        bullet_width += d->space ();
        bullet_size = bullet_width;
        if (bullet_width > s->bullet_width ())
            s->bullet_width (bullet_width);
        if (bullet_height > s->bullet_height ())
            s->bullet_height (bullet_height);
    }
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    draw_children (d);
    d->discontinue_underline ();
    if (bullet_string)
    {
        int old_underline = d->underline ();
        d->underline (0);
        d->draw (bullet_string,
                 strlen (bullet_string),
                 bullet->x - bullet_shift - bullet_size,
                 bullet->y + bullet_height);
        d->underline (old_underline);
    }
}
}
;

/////////////// <dl>  ///////////////////
BUILD_TAG (dl, Fl_Html_Object::BLOCK, 0)
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
} */
int drawable () const
{
    return 0;
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}

void format (Fl_Html_Formatter * s)
{
    int p_indent = s->drawing_device ()->font_size () / 2;
    s->finish_last_line (p_indent);
    int old_align = s->x_align ();
    s->x_align (ALIGN_LEFT);
    format_children (s);
    s->finish_last_line (p_indent);
    set_out ();
    bounding_box_children ();
    s->x_align (old_align);
}

void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    draw_children (d);
    d->discontinue_underline ();
}

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void export_end (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}
}
;

/////////////// <dt>  ///////////////////
BUILD_TAG (dt, Fl_Html_Object::BLOCK | Fl_Html_Object::DT,
           Fl_Html_Object::DT | Fl_Html_Object::DD)
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
} */
int drawable () const
{
    return 2;
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}

void format (Fl_Html_Formatter * s)
{
    s->finish_last_line (0);
    format_children (s);
    s->finish_last_line (0);
    set_out ();
    bounding_box_children ();
}

void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    draw_children (d);
    d->discontinue_underline ();
}

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void export_end (Text_Buffer & text, int type) const
{
    // if(!text.newlines())
    //   text.add_newline();
}
}
;

////////////// <dd>  ///////////////////
BUILD_TAG (dd, Fl_Html_Object::BLOCK | Fl_Html_Object::DD,
           Fl_Html_Object::DT | Fl_Html_Object::DD)
int margin_left;
int drawable () const
{
    return 2;
}
Html_Tag_dd () : margin_left (40)
{
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}
void process_style (const Fl_Css_Parser::Property & p)
{
    if (is_word (p.property (), "margin-left", p.property_length ()))
    {
        if (!p.no_values ())
            return;
        long long bb = 0;
        Fl_Html_Tokenizer::int_value ((p.values ())[0],
                                      bb,
                                      Fl_Html_Tokenizer::FORMAT_10,
                                      (p.values_lengths ())[0]);
        if (bb >= 0)
            margin_left = bb;
        return;
    }
}
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (handle_style (name, name_len, value, value_len))
        return;
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
} */

void format (Fl_Html_Formatter * s)
{
    s->finish_last_line (0);
    s->add_left_shift (margin_left);
    format_children (s);
    s->finish_last_line (0);
    s->add_left_shift (-margin_left);
    set_out ();
    bounding_box_children ();
}

void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    draw_children (d);
    d->discontinue_underline ();
}

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
    text.add_tab ();
    text.add ("\t");
}

void export_end (Text_Buffer & text, int type) const
{
    text.remove_tab ();
    // if(!text.newlines())
    //  text.add_newline();
}
}
;

/////////////// <body>  ///////////////////
BUILD_TAG (body, 0, 0)
int bgcolor;
int link;
int text;
int flags;

int drawable () const
{
    return 0;
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return 1;
}
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (is_word (name, "bgcolor", name_len))
    {
        if (read_color (bgcolor, value, value_len))
            flags |= 1;
        return;
    }
    if (is_word (name, "link", name_len))
    {
        if (read_color (link, value, value_len))
            flags |= 2;
        return;
    }
    if (is_word (name, "text", name_len))
    {
        if (read_color (text, value, value_len))
            flags |= 4;
        return;
    }
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
}
*/

void format (Fl_Html_Formatter * s)
{
    format_children (s);
    s->finish_last_line (0);
    set_out ();
    bounding_box_children ();
}

void init_format (Fl_Html_Formatter * s)
{
    if (flags & 1)
        s->background_color (bgcolor);
    init_format_children (s);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    int old_base_font_color = d->base_font_color ();
    int old_color = d->color ();
    if (flags & 4)
    {
        d->base_font_color (text);
        d->color (text);
    }
    int old_link_color = d->link_color ();
    if (flags & 2)
        d->link_color (link);
    draw_children (d);
    if (flags & 4)
    {
        d->base_font_color (old_base_font_color);
        d->color (old_color);
    }
    if (flags & 2)
        d->link_color (old_link_color);
}

Html_Tag_body () : flags (0)
{
}
}
;

/////////////// <title>  ///////////////////
BUILD_TAG (title, 0, 0)
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}
// bool point_within(int X, int Y) const { return false;}
void format (Fl_Html_Formatter * s)
{
    set_out ();
}
void init_format (Fl_Html_Formatter * s)
{
}
void draw (Fl_Html_Drawing_Device_ * d)
{
}
void min_width (int & width) const
{
}
int drawable () const
{
    return 0x1000000;
}
bool point_in_bounding_box (int X, int Y) const
{
    return false;
}
}
;

/////////////// <head> //////////////////////
static const char * _tag_name_head = "head";
class Html_Tag_head : public Html_Tag_title
{
  public:
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_head ();
    }
    const char * name () const
    {
        return _tag_name_head;
    }
};

static const char * _tag_name_meta = "meta";
class Html_Tag_meta : public Html_Tag_title
{
  public:
    bool no_body () const
    {
        return true;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_meta ();
    }
    const char * name () const
    {
        return _tag_name_meta;
    }
};

//////////////// <span> ///////////////////////
BUILD_TAG (span, 0, 0)
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}
int drawable () const
{
    return 0;
}
void format (Fl_Html_Formatter * s)
{
    format_children (s);
    set_out ();
    s->add_todo_object (this);
}
void draw (Fl_Html_Drawing_Device_ * d)
{
    draw_children (d);
}
void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
}
}
;

/////////////// <div>  ///////////////////
BUILD_TAG (div, Fl_Html_Object::BLOCK, 0)
int drawable () const
{
    return 0;
}
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void export_end (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void format (Fl_Html_Formatter * s)
{
    int p_indent = s->drawing_device ()->font_size () / 2;
    s->finish_last_line (p_indent);
    format_children (s);
    s->finish_last_line (p_indent);
    set_out ();
    bounding_box_children ();
}

void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    draw_children (d);
    d->discontinue_underline ();
}
}
;

/////////////// <pre>  ///////////////////
BUILD_TAG (pre, Fl_Html_Object::BLOCK, 0)
int drawable () const
{
    return 0;
}
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void export_end (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}
/*
void format(Fl_Html_Formatter * s) {
  Fl_Html_Drawing_Device_ * d = s->drawing_device();
  int old_font = d->font_face();
  d->font(old_font|mod(), d->font_size());
  format_children(s);
  set_out();
  // note: we cant call bounding_box_children yet because finish_line() might
still
  // reposition the children, adding to "todo" list when line is finished
  s->add_todo_object(this);
  d->font(old_font, d->font_size());
}

void draw(Fl_Html_Drawing_Device_ * d) {
  int old_font = d->font_face();
  d->font(old_font|mod(), d->font_size());
  draw_children(d);
  d->font(old_font, d->font_size());
}


void init_format(Fl_Html_Formatter * s) {
  Fl_Html_Drawing_Device_ * d = s->drawing_device();
  int old_font = d->font_face();
  d->font(old_font|mod(), d->font_size());
  init_format_children(s);
  d->font(old_font, d->font_size());
}


*/

void format (Fl_Html_Formatter * s)
{
    int p_indent = s->drawing_device ()->font_size () / 2;
    s->finish_last_line (p_indent);
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    d->font (Fl_Html_Drawing_Device_::COURIER, d->font_size ());
    format_children (s);
    d->font (old_font, d->font_size ());
    s->finish_last_line (p_indent);
    set_out ();
    bounding_box_children ();
}

void init_format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    d->font (Fl_Html_Drawing_Device_::COURIER, d->font_size ());
    init_format_children (s);
    d->font (old_font, d->font_size ());
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    int old_font = d->font_face ();
    d->font (Fl_Html_Drawing_Device_::COURIER, d->font_size ());
    draw_children (d);
    d->font (old_font, d->font_size ());
}

int process_body () const
{
    return 1;
}
}
;

/////////////// <p>  ///////////////////
BUILD_TAG (p, Fl_Html_Object::BLOCK, Fl_Html_Object::BLOCK)
int align;
int drawable () const
{
    return 0;
}
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (is_word (name, "align", name_len))
    {
        if (Fl_Html_Object_::is_first_word (value, "center", value_len))
            align = Fl_Html_Formatter::CENTER;
        else if (Fl_Html_Object_::is_first_word (value, "right", value_len))
            align = Fl_Html_Formatter::RIGHT;
        else if (Fl_Html_Object_::is_first_word (value, "justify", value_len))
            align = Fl_Html_Formatter::JUSTIFY;
        else if (Fl_Html_Object_::is_first_word (value, "left", value_len))
            align = Fl_Html_Formatter::LEFT;
        return;
    }
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void export_end (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
}
*/

void format (Fl_Html_Formatter * s)
{
    int p_indent = s->drawing_device ()->font_size () / 2;
    s->finish_last_line (p_indent);
    int old_align;
    if (align >= 0)
    {
        old_align = s->x_align ();
        s->x_align (align);
    }
    format_children (s);
    s->finish_last_line (p_indent);
    set_out ();
    bounding_box_children ();
    if (align >= 0)
        s->x_align (old_align);
}

void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    draw_children (d);
    d->discontinue_underline ();
}

Html_Tag_p () : align (-1)
{
}
}
;

static const char * const _tag_name_h6 = "h6";
class Html_Tag_h6 : public Html_Tag_p
{
  public:
    int size;
    int relative;
    Html_Tag_h6 () : size (2), relative (-1)
    {
    }
    const char * name () const
    {
        return _tag_name_h6;
    }
    int type () const
    {
        return Fl_Html_Object::BLOCK;
    }
    int closed_by () const
    {
        return 0;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_h6 ();
    }

    int drawable () const
    {
        return 0;
    }
    /*
      bool point_within(int X, int Y) const {
        if(!point_in_bounding_box(X, Y)) return false;
        return point_within_children(X, Y);
      }
    */
    void init_format (Fl_Html_Formatter * s)
    {
        Fl_Html_Drawing_Device_ * d = s->drawing_device ();
        int old_font = d->font_face ();
        float old_size = d->font_size ();
        d->font (old_font | Fl_Html_Drawing_Device_::BOLD, size, relative);
        Html_Tag_p::init_format (s);
        d->font (old_font, old_size);
    }
    void format (Fl_Html_Formatter * s)
    {
        Fl_Html_Drawing_Device_ * d = s->drawing_device ();
        int old_font = d->font_face ();
        float old_size = d->font_size ();
        d->font (old_font | Fl_Html_Drawing_Device_::BOLD, size, relative);
        Html_Tag_p::format (s);
        d->font (old_font, old_size);
    }
    void draw (Fl_Html_Drawing_Device_ * d)
    {
        int old_font = d->font_face ();
        float old_size = d->font_size ();
        d->font (old_font | Fl_Html_Drawing_Device_::BOLD, size, relative);
        Html_Tag_p::draw (d);
        d->font (old_font, old_size);
    }

    void export_start (Text_Buffer & text, int type) const
    {
        int no = 2 - text.newlines ();
        for (int i = 0; i < no; i++)
            text.add_newline ();
    }

    void export_end (Text_Buffer & text, int type) const
    {
        int no = 2 - text.newlines ();
        for (int i = 0; i < no; i++)
            text.add_newline ();
    }
};
static const char * const _tag_name_h5 = "h5";
class Html_Tag_h5 : public Html_Tag_h6
{
  public:
    Html_Tag_h5 ()
    {
        size = 1;
        relative = -1;
    }
    const char * name () const
    {
        return _tag_name_h5;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_h5 ();
    }
};

static const char * const _tag_name_h4 = "h4";
class Html_Tag_h4 : public Html_Tag_h6
{
  public:
    Html_Tag_h4 ()
    {
        size = 0;
        relative = 1;
    }
    const char * name () const
    {
        return _tag_name_h4;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_h4 ();
    }
};

static const char * const _tag_name_h3 = "h3";
class Html_Tag_h3 : public Html_Tag_h6
{
  public:
    Html_Tag_h3 ()
    {
        size = 1;
        relative = 1;
    }
    const char * name () const
    {
        return _tag_name_h3;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_h3 ();
    }
};

static const char * const _tag_name_h2 = "h2";
class Html_Tag_h2 : public Html_Tag_h6
{
  public:
    Html_Tag_h2 ()
    {
        size = 2;
        relative = 1;
    }
    const char * name () const
    {
        return _tag_name_h2;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_h2 ();
    }
};

static const char * const _tag_name_h1 = "h1";
class Html_Tag_h1 : public Html_Tag_h6
{
  public:
    Html_Tag_h1 ()
    {
        size = 3;
        relative = 1;
    }
    const char * name () const
    {
        return _tag_name_h1;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_h1 ();
    }
};

/////////////// <br>  ///////////////////
BUILD_TAG (br, 0, 0) // Fl_Html_Object::BLOCK, 0)

int flags;

Html_Tag_br () : flags (0)
{
}
// bool point_within(int X, int Y) const { return false;}
int drawable () const
{
    return 0x1000000;
}
int breaks_word () const
{
    return 1;
}

void format (Fl_Html_Formatter * s)
{
    // int old_align = s->x_align();
    // if(old_align==Fl_Html_Formatter::JUSTIFY) // we do not justify ast line
    //  s->x_align(Fl_Html_Formatter::LEFT);
    s->finish_last_line (0);
    if (flags &
        (Fl_Html_Formatter::CLEAR_LEFT | Fl_Html_Formatter::CLEAR_RIGHT))
        s->clear_margin (flags);
    set_out ();
    // s->x_align(old_align);
}

void init_format (Fl_Html_Formatter * s)
{
}
void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
}
bool no_body () const
{
    return true;
}
bool point_in_bounding_box (int X, int Y) const
{
    return false;
}

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void export_end (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void process_style (const Fl_Css_Parser::Property & p)
{
    if (is_word (p.property (), "clear", p.property_length ()))
    {
        if (!p.no_values ())
            return;
        if (Fl_Html_Object_::is_first_word (
                (p.values ())[0], "both", (p.values_lengths ())[0]))
            flags |=
                Fl_Html_Formatter::CLEAR_LEFT | Fl_Html_Formatter::CLEAR_RIGHT;
        else if (Fl_Html_Object_::is_first_word (
                     (p.values ())[0], "left", (p.values_lengths ())[0]))
            flags |= Fl_Html_Formatter::CLEAR_LEFT;
        else if (Fl_Html_Object_::is_first_word (
                     (p.values ())[0], "right", (p.values_lengths ())[0]))
            flags |= Fl_Html_Formatter::CLEAR_RIGHT;
    }
}

void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    handle_style (name, name_len, value, value_len);
}
}
;

/////////////// <hr>  ///////////////////
BUILD_TAG (hr, Fl_Html_Object::BLOCK, 0)
int width;
unsigned int flags;

Html_Tag_hr () : width (100), flags (WIDTH_PERCENT)
{
    h = 3;
}

int drawable () const
{
    return 1;
}
/*
bool point_within(int X, int Y) const {
    return point_in_bounding_box(X, Y);
}
*/
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (read_align (flags, name, name_len, value, value_len))
        return;
    if (is_word (name, "size", name_len))
    {
        long long cs = h;
        if (Fl_Html_Tokenizer::int_value (
                value, cs, Fl_Html_Tokenizer::FORMAT_10, value_len))
        {
            if (cs < 1)
                cs = 1;
            if (cs > 10000)
                cs = 10000;
            h = cs;
        }
        return;
    }
    if (is_word (name, "width", name_len))
    {
        bool percents = 0;
        if (read_with_percents (value, width, percents, value_len))
        {
            if (width < 0)
                width = 0; // make it non-negative
            if (percents)
            {
                flags |= WIDTH_PERCENT;
                if (!width)
                    width = 1; // avoid zero-division later
            }
            else
                flags &= ~WIDTH_PERCENT;
        }
        return;
    }
    if (is_word (name, "noshade", name_len))
    {
        flags |= NOSHADE;
        return;
    }
}

void format (Fl_Html_Formatter * s)
{
    int old_align = s->x_align ();
    int p_indent = s->drawing_device ()->font_size () / 2;
    s->finish_last_line (p_indent);
    if (flags & ALIGN_SET)
        s->x_align (flags & ALIGN_MASK);
    else
        s->x_align (old_align);

    if (flags & WIDTH_PERCENT)
        w = ((s->right_margin () - s->x ()) * width) / 100;
    else
        w = width;
    s->place_inline (this);
    s->finish_line (p_indent);

    s->x_align (old_align);
}

void init_format (Fl_Html_Formatter * s)
{
}
void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    int old_color = d->color ();
    d->color (d->color_gray ());
    if (flags & NOSHADE)
        d->rectf (x, y, w, h);
    else
    {
        // d->line(x, y+h-1, w-1);
        // d->vline(x+w-1, y, h-1);
        d->rectf (x, y + h - 1, w, 1);
        d->rectf (x + w - 1, y, 1, h);
        d->color (0);
        // d->line(x, y, w-1);
        // d->vline(x, y, h-1);
        d->rectf (x, y, w, 1);
        d->rectf (x, y, 1, h);
    }
    d->color (old_color);
}
bool no_body () const
{
    return true;
}

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void export_end (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void export_body (Text_Buffer & text, int type) const
{
    text.add ("-----");
    text.newlines (0);
    text.add_newline ();
}
}
;

//////////////// <u> ///////////////////////
BUILD_TAG (u, 0, 0)

int drawable () const
{
    return 0;
}

void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
}
*/
void format (Fl_Html_Formatter * s)
{
    format_children (s);
    set_out ();
    s->add_todo_object (this);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    int old_underline = d->underline ();
    d->underline (1);
    draw_children (d);
    d->underline (old_underline);
}

void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}
}
;

//////////////// <a> ///////////////////////
BUILD_TAG (a, 0, 0)
bool hasFocus = false;
int flags;
char * href;
char * h_name;
Html_Tag_a ()
{
    flags = 0;
    href = 0;
    h_name = 0;
}
const char * link () const
{
    return href;
}
const char * anchor () const
{
    return h_name;
}

int drawable () const
{
    return 0;
}
~Html_Tag_a ()
{
    delete[] href;
    delete[] h_name;
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
}
*/
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (is_word (name, "href", name_len))
    {
        delete[] href;
        href = 0;
        if (value_len)
        {
            href = new char[value_len + 1];
            memcpy (href, value, value_len);
            href[value_len] = 0; // assure null-termination
        }
        return;
    }
    if (is_word (name, "name", name_len))
    {
        delete[] h_name;
        h_name = 0;
        if (value_len)
        {
            h_name = new char[value_len + 1];
            url_decode (h_name, value, value_len);
        }
        return;
    }
    if (handle_style (name, name_len, value, value_len))
        return;
}
void process_style (const Fl_Css_Parser::Property & p)
{
    if (is_word (p.property (), "text-decoration", p.property_length ()))
    {
        if (!p.no_values ())
            return;
        if (is_word ((p.values ())[0], "none", (p.values_lengths ())[0]))
            flags |= 1;
        return;
    }
}

void format (Fl_Html_Formatter * s)
{
    format_children (s);
    if (children ())
    {
        set_out ();
        // note: we cant call bounding_box_children yet because finish_line()
        // might still reposition the children, adding to "todo" list when line
        // is finished
        s->add_todo_object (this);
    }
    else
    {
        w = 0;
        h = 0;
        s->place_inline (this);
    }
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    int old_underline = d->underline ();
    int old_color = -1;

    if (hasFocus)
    {
        d->draw_box (FL_FRAME_BOX, x, y, w, h);
    }
    // printf ("MY XY: %d@%d WH: %d@%d\n", x, y, w, h);
    if (href)
    {
        old_color = d->color ();
        d->color (d->link_color ());
    }
    if (!href || (flags & 1))
        d->underline (0);
    else
        d->underline (1);
    draw_children (d);
    d->underline (old_underline);
    if (old_color >= 0)
        d->color (old_color);
    d->discontinue_underline ();
}

void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
}

bool takeFocus (bool should) override
{
    hasFocus = should;
    return true;
}
}
;

namespace
{ // anonymous namespace

struct Html_Tag_Finder : public Fl_Html_Object::Finder
{
    const char * val;
    Fl_Html_Object * valid (Fl_Html_Object * o)
    {
        if (o->name ())
        {
            if (Fl_Html_Parser::str_equal (o->name (), val))
                return o;
        }
        else if (!val)
            return o;
        return find ((Fl_Html_Object *)(o->children ()));
    }
    Html_Tag_Finder (const char * v) : val (v)
    {
    }
};

struct Html_Anchor_Finder : public Fl_Html_Object::Finder
{
    const char * val;
    Fl_Html_Object * valid (Fl_Html_Object * o)
    {
        if (o->name () && Fl_Html_Parser::str_equal (o->name (), "a") &&
            ((Html_Tag_a *)o)->h_name && val &&
            Fl_Html_Parser::str_equal (((Html_Tag_a *)o)->h_name, val) &&
            ((Html_Tag_a *)o)->x != HIGH_VAL)
            return o;
        return find ((Fl_Html_Object *)(o->children ()));
    }
    Html_Anchor_Finder (const char * v) : val (v)
    {
    }
};

} // end of anonymous namespace

Fl_Html_Object * Fl_Html_Object::find_anchor (const char * link,
                                              Fl_Html_Object * list)
{
    if (!link)
        return 0;
    int len = strlen (link);
    Fl_Html_Object::Iterator it;
    while (list)
    {
        if (list && list->anchor () &&
            Fl_Html_Object_::is_word (link, list->anchor (), len))
            return list;
        list = it.next (list);
    }
    return 0;
}

unsigned Fl_Html_Object::extract_title (Fl_Html_Object * o, char * title,
                                        unsigned len)
{
    Fl_Html_Object::Iterator it;
    while (o)
    {
        if (o->name () && Fl_Html_Parser::str_equal (o->name (), "title"))
            break;
        o = it.next (o);
    }
    if (len)
        *title = 0;
    if (!o)
        return 0;
    o = (Fl_Html_Object *)(o->children ());
    it.reset ();
    unsigned res = 0;
    while (o)
    {
        if (!o->name ())
        {
            Html_Word * w = (Html_Word *)o;
            const char * val = w->value ();
            int length = strlen (val);
            if (length < int (len))
            {
                strncpy (title, val, len);
                title += length;
                len -= length;
            }
            res += length;
        }
        o = it.next (o);
    }
    if (len)
        *title = 0;
    return res;
}

//////////////// <code> ///////////////////////
BUILD_TAG (code, 0, 0)
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}
int drawable () const
{
    return 0;
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
}
*/

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}
void format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    int mod =
        (Fl_Html_Drawing_Device_::BOLD | Fl_Html_Drawing_Device_::ITALIC) &
        old_font;
    d->font (Fl_Html_Drawing_Device_::COURIER | mod, d->font_size ());
    format_children (s);
    set_out ();
    // note: we cant call bounding_box_children yet because finish_line() might
    // still reposition the children, adding to "todo" list when line is
    // finished
    s->add_todo_object (this);
    d->font (old_font, d->font_size ());
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    int old_font = d->font_face ();
    int mod =
        (Fl_Html_Drawing_Device_::BOLD | Fl_Html_Drawing_Device_::ITALIC) &
        old_font;
    d->font (Fl_Html_Drawing_Device_::COURIER | mod, d->font_size ());
    draw_children (d);
    d->font (old_font, d->font_size ());
}

void init_format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    int mod =
        (Fl_Html_Drawing_Device_::BOLD | Fl_Html_Drawing_Device_::ITALIC) &
        old_font;
    d->font (Fl_Html_Drawing_Device_::COURIER | mod, d->font_size ());
    init_format_children (s);
    d->font (old_font, d->font_size ());
}
}
;

//////////////// <kbd>  ///////////////////////////

static const char * const _tag_name_kbd = "kbd";
class Html_Tag_kbd : public Html_Tag_code
{
  public:
    const char * name () const
    {
        return _tag_name_kbd;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_kbd ();
    }
};

//////////////// <tt>  ///////////////////////////

static const char * const _tag_name_tt = "tt";
class Html_Tag_tt : public Html_Tag_code
{
  public:
    const char * name () const
    {
        return _tag_name_tt;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_tt ();
    }
};

//////////////// <b> ///////////////////////
BUILD_TAG (b, 0, 0)
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}
virtual int mod () const
{
    return Fl_Html_Drawing_Device_::BOLD;
}

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_region_ (X, Y);
}

int drawable () const
{
    return 0;
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
}
*/
void format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    d->font (old_font | mod (), d->font_size ());
    format_children (s);
    set_out ();
    // note: we cant call bounding_box_children yet because finish_line() might
    // still reposition the children, adding to "todo" list when line is
    // finished
    s->add_todo_object (this);
    d->font (old_font, d->font_size ());
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    int old_font = d->font_face ();
    d->font (old_font | mod (), d->font_size ());
    draw_children (d);
    d->font (old_font, d->font_size ());
}

void init_format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    d->font (old_font | mod (), d->font_size ());
    init_format_children (s);
    d->font (old_font, d->font_size ());
}
}
;

static const char * const _tag_name_strong = "strong";
class Html_Tag_strong : public Html_Tag_b
{
  public:
    const char * name () const
    {
        return _tag_name_strong;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_strong ();
    }
};

static const char * const _tag_name_i = "i";
class Html_Tag_i : public Html_Tag_b
{
  public:
    int mod () const
    {
        return Fl_Html_Drawing_Device_::ITALIC;
    }

    const char * name () const
    {
        return _tag_name_i;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_i ();
    }
};

static const char * const _tag_name_var = "var";
class Html_Tag_var : public Html_Tag_i
{
  public:
    const char * name () const
    {
        return _tag_name_var;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_var ();
    }
};

static const char * const _tag_name_em = "em";
class Html_Tag_em : public Html_Tag_i
{
  public:
    const char * name () const
    {
        return _tag_name_em;
    }
    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_em ();
    }
};

//////////////// <font> ///////////////////////

struct _Color_Table
{
    const char * name;
    unsigned value;
};
static _Color_Table color_table[] = {
    "red",       0xFF0000, "white",    0xFFFFFF, "cyan",    0x00FFFF,
    "silver",    0xC0C0C0, "blue",     0x0000FF, "gray",    0x808080,
    "grey",      0x808080, "darkblue", 0x0000A0, "black",   0x000000,
    "lightblue", 0xADD8E6, "orange",   0xFFA500, "purple",  0x800080,
    "brown",     0xA52A2A, "yellow",   0xFFFF00, "maroon",  0x800000,
    "lime",      0x00FF00, "green",    0x008000, "magenta", 0xFF00FF,
    "olive",     0x808000, 0,          0 // must finish with no name
};

struct _Font_Table
{
    const char * name;
    int value;
};

_Font_Table font_table[] = {"helvetica",
                            Fl_Html_Drawing_Device_::HELVETICA,
                            "courier",
                            Fl_Html_Drawing_Device_::COURIER,
                            "times",
                            Fl_Html_Drawing_Device_::TIMES,
                            "symbol",
                            Fl_Html_Drawing_Device_::SYMBOL,
                            "arial",
                            Fl_Html_Drawing_Device_::ARIAL,
                            "sans",
                            Fl_Html_Drawing_Device_::SANS,
                            "serif",
                            Fl_Html_Drawing_Device_::SERIF,
                            0,
                            0};

unsigned Fl_Html_Object::read_color (int & value, const char * str, int str_len)
{
    long long v;
    int ret = Fl_Html_Tokenizer::int_value (
        str, v, Fl_Html_Tokenizer::FORMAT_HASH, str_len);
    if (ret)
    {
        value = v;
        return ret;
    }
    _Color_Table * cn = color_table;
    const char * name = cn->name;
    while (name)
    {
        unsigned len = Fl_Html_Object_::is_first_word (str, name, str_len);
        if (len)
        {
            value = cn->value;
            return len;
        }
        cn++;
        name = cn->name;
    }
    return 0;
}

static unsigned read_font (const char * str, int & value, int str_len)
{
    _Font_Table * cn = font_table;
    const char * name = cn->name;
    while (name)
    {
        unsigned len = Fl_Html_Object_::is_first_word (str, name, str_len);
        if (len)
        {
            value = cn->value;
            return len;
        }
        cn++;
        name = cn->name;
    }
    return 0;
}

///////////////////  <font>   //////////////////////////////////

BUILD_TAG (font, 0, 0)
int color;
int face;
int size;
int relative;

int drawable () const
{
    return 0;
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
}
*/
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (is_word (name, "size", name_len))
    {
        long long val;
        long long sign;
        if (Fl_Html_Tokenizer::signed_int_value (value, val, sign, value_len))
        {
            size = val;
            if (size > 7)
                size = 7;
            relative = sign;
        }
        return;
    }
    if (is_word (name, "face", name_len))
    {
        read_font (value, face, value_len);
        return;
    }
    if (is_word (name, "color", name_len))
    {
        read_color (color, value, value_len);
        return;
    }
}

void format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    float old_size = d->font_size ();
    if (face >= 0)
        d->font (face, size, relative);
    else
        d->font (old_font, size, relative);
    format_children (s);
    set_out ();
    // note: we cant call bounding_box_children yet because finish_line() might
    // still reposition the children, adding to "todo" list when line is
    // finished
    s->add_todo_object (this);
    d->font (old_font, old_size);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    int old_font = d->font_face ();
    float old_size = d->font_size ();
    if (face >= 0)
        d->font (face, size, relative);
    else
        d->font (old_font, size, relative);
    int old_color;
    if (color > 0)
    {
        old_color = d->color ();
        d->color (color);
    }
    draw_children (d);
    d->font (old_font, old_size);
    if (color > 0)
    {
        d->color (old_color);
    }
}

void init_format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    float old_size = d->font_size ();
    if (face >= 0)
        d->font (face, size, relative);
    else
        d->font (old_font, size, relative);
    init_format_children (s);
    d->font (old_font, old_size);
}

Html_Tag_font ()
{
    color = -1; // negative is unchanged because 0 is black
    face = -1;  // negative is unchanged because 0 is helvetica
    size = 0;
    relative = 1;
}
}
;

///////////////////  <sub>   //////////////////////////////////

BUILD_TAG (sub, 0, 0)

int drawable () const
{
    return 0;
}

void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}

void format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    float old_size = d->font_size ();
    int old_baseline = s->baseline ();
    int baseline_shift = old_size / 4;
    s->baseline (old_baseline + baseline_shift);
    d->font (old_font, 1, -1);
    format_children (s);
    set_out ();
    // note: we cant call bounding_box_children yet because finish_line() might
    // still reposition the children, adding to "todo" list when line is
    // finished
    s->add_todo_object (this);
    d->font (old_font, old_size);
    s->baseline (s->baseline () - baseline_shift);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    int old_font = d->font_face ();
    float old_size = d->font_size ();
    d->font (old_font, 1, -1);
    draw_children (d);
    d->font (old_font, old_size);
}

void init_format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    float old_size = d->font_size ();
    d->font (old_font, 1, -1);
    init_format_children (s);
    d->font (old_font, old_size);
}
}
;

///////////////////  <sup>   //////////////////////////////////

BUILD_TAG (sup, 0, 0)

int drawable () const
{
    return 0;
}

void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}

void format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    float old_size = d->font_size ();
    int old_baseline = s->baseline ();
    int baseline_shift = old_size / 3;
    s->baseline (old_baseline - baseline_shift);
    d->font (old_font, 1, -1);
    format_children (s);
    set_out ();
    // note: we cant call bounding_box_children yet because finish_line() might
    // still reposition the children, adding to "todo" list when line is
    // finished
    s->add_todo_object (this);
    d->font (old_font, old_size);
    s->baseline (s->baseline () + baseline_shift);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    int old_font = d->font_face ();
    float old_size = d->font_size ();
    d->font (old_font, 1, -1);
    draw_children (d);
    d->font (old_font, old_size);
}

void init_format (Fl_Html_Formatter * s)
{
    Fl_Html_Drawing_Device_ * d = s->drawing_device ();
    int old_font = d->font_face ();
    float old_size = d->font_size ();
    d->font (old_font, 1, -1);
    init_format_children (s);
    d->font (old_font, old_size);
}
}
;

//////////////// <center> ///////////////////////
BUILD_TAG (center, Fl_Html_Object::BLOCK, 0)
int drawable () const
{
    return 0;
}
void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
}
*/

void export_start (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void export_end (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add_newline ();
}

void format (Fl_Html_Formatter * s)
{
    s->finish_last_line ();
    int old_align = s->x_align ();
    s->x_align (Fl_Html_Formatter::CENTER);
    format_children (s);
    s->finish_last_line ();
    set_out ();
    bounding_box_children (x, y, w, h);
    s->x_align (old_align);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    draw_children (d);
    d->discontinue_underline ();
}

void init_format (Fl_Html_Formatter * s)
{
    init_format_children (s);
}
}
; // end of tag

Fl_Html_Image_ * html_get_image (Fl_Html_Parser * p, const char * val,
                                 int val_len);

//////////////  <img>    /////////////////
BUILD_TAG (img, 0 /*(flags & ALIGN_INLINE)? 0:Fl_Html_Object::BLOCK)*/, 0)
int flags;
char * alt;
Fl_Html_Image_ * img;
Fl_Html_Image_ * img_cache;
int width;
int height;
int hspace;
int vspace;
int border;
int dynamic_width;
int calculated_height;
char word_space;

/*
bool point_within(int X, int Y) const {
    return point_in_bounding_box(X, Y);
}
*/
int drawable () const
{
    return 1;
}

void export_body (Text_Buffer & text, int type) const
{
    if (!text.newlines ())
        text.add (" ");
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();
    if (border)
    {
        d->rectf (x + hspace,
                  y + vspace,
                  width + 2 * border,
                  calculated_height + 2 * border);
        int old_color = d->color ();
        // int dd = d->background_color();
        d->color (d->background_color ());
        d->rectf (
            x + hspace + border, y + vspace + border, width, calculated_height);
        d->color (old_color);
    }
    d->draw_image (img,
                   img_cache,
                   x + border + hspace,
                   y + border + vspace,
                   width,
                   calculated_height);
    if (!img && alt)
    {
        int old_font = d->font_face ();
        int old_size = d->font_size ();
        int old_color = d->color ();
        d->font (d->base_font_face (), d->base_font_size ());
        d->color (d->base_font_color ());
        d->draw (alt, sizeof (alt), x, y + h);
        d->font (old_font, old_size);
        d->color (old_color);
    }
}

Html_Tag_img ()
    : flags (ALIGN_INLINE | VALIGN_BOTTOM), alt (0), img (0), img_cache (0),
      width (0), height (0), hspace (3), vspace (0), border (0),
      dynamic_width (0), word_space (0)
{
}
~Html_Tag_img ()
{
    delete[] alt;
    delete img_cache;
    delete img;
}

void get_static_width ()
{
    if (img)
    {
        int nw = width;
        int nh = height;
        if (!width)
        {
            if (height)
                nw = (height * img->w () + img->h () / 2) / img->h ();
            else
                nw = img->w ();
        }

        if (!height)
        {
            if (width)
                nh = (width * img->h () + img->w () / 2) / img->w ();
            else
                nh = img->h ();
        }
        width = nw;
        calculated_height = nh;
    }
    w = width + 2 * (border + hspace);
    h = calculated_height + 2 * (border + vspace);
}

void get_dynamic_width (Fl_Html_Formatter * s)
{
    int base_w = s->base_width ();
    int nw;
    if (base_w == HIGH_VAL)
    {
        nw = 1;
        if (flags & ALIGN_INLINE)
            s->percentual_width_inline (dynamic_width);
        else
            switch (flags & ALIGN_MASK)
            {
            case ALIGN_RIGHT:
                s->percentual_width_right (dynamic_width);
                break;
            default:
                s->percentual_width_left (dynamic_width);
            }
    }
    else
        nw = (dynamic_width * base_w) / 100;
    if (width != nw)
    {
        width = nw;
        get_static_width ();
    }
}

void init_format (Fl_Html_Formatter * s)
{
    if (!dynamic_width)
        get_static_width ();
}

void format (Fl_Html_Formatter * s)
{
    // Fl_Html_Object * old_bullet = s->bullet();
    /*
    if(!(flags & ALIGN_INLINE))
      s->finish_last_line();
    */
    if (dynamic_width)
        get_dynamic_width (s);
    if (s->no_line_objects () && s->available_width () < w &&
        !(flags & ALIGN_INLINE))
    {
        s->add_todo_format_object (
            this); // schedullingg for foanat after line finish
        return;
    }
    Fl_Html_Object * old_bullet = s->bullet ();
    s->find_place (w, !(flags & ALIGN_INLINE));
    if (flags & ALIGN_INLINE)
    {
        s->y_align (flags & VALIGN_MASK);
        if (!old_bullet)
            s->bullet (0);
        s->place_inline (this);
        if (word_space)
            s->add_space (s->drawing_device ()->space ());
    }
    else
    {
        switch (flags & ALIGN_MASK)
        {
        case ALIGN_RIGHT:
            s->place_right (this, ALIGN_WRAP);
            break;
        default:
            s->place_left (this, ALIGN_WRAP);
            break;
        }
    }
}

void min_width (int & width) const
{
    if (!dynamic_width && width < w)
        width = w;
}
void finish (Fl_Xml_Parser * p)
{
    if (img)
        ((Fl_Html_Parser *)p)->last_word_space (&word_space);
}

void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (is_word (name, "src", name_len))
    {
        delete img;
        // printf("%.*s\n", value_len, value);
        img = html_get_image ((Fl_Html_Parser *)p, value, value_len);
        //((Fl_Html_Parser *)p)->last_word_space(&word_space);
        return;
    }
    if (is_word (name, "align", name_len))
    {
        if (Fl_Html_Object_::is_first_word (value, "right", value_len))
        {
            flags &= ~ALIGN_MASK;
            flags |= ALIGN_RIGHT;
            flags &= ~ALIGN_INLINE;
        }
        else if (Fl_Html_Object_::is_first_word (value, "left", value_len))
        {
            flags &= ~ALIGN_MASK;
            flags |= ALIGN_LEFT;
            flags &= ~ALIGN_INLINE;
        }
        else if (Fl_Html_Object_::is_first_word (value, "top", value_len))
        {
            flags &= ~VALIGN_MASK;
            flags |= VALIGN_TOP;
            flags |= ALIGN_INLINE;
        }
        else if (Fl_Html_Object_::is_first_word (value, "bottom", value_len))
        {
            flags &= ~VALIGN_MASK;
            flags |= VALIGN_BOTTOM;
            flags |= ALIGN_INLINE;
        }
        else if (Fl_Html_Object_::is_first_word (value, "middle", value_len))
        {
            flags &= ~VALIGN_MASK;
            flags |= VALIGN_MIDDLE;
            flags |= ALIGN_INLINE;
        };
        return;
    }
    if (is_word (name, "alt", name_len))
    {
        delete[] alt;
        alt = 0;
        ((Fl_Html_Parser *)p)
            ->special_character_table ()
            ->translate_copy (value, value_len, alt);
        return;
    }
    if (is_word (name, "width", name_len))
    {
        bool percents = 0;
        if (read_with_percents (value, width, percents, value_len))
        {
            if (width < 0)
                width = 0; // make it non-negative
            if (width > 10000)
                width = 10000;
            if (percents)
                dynamic_width = width;
        }
        return;
    }
    /*
    if (is_word(name, "width", name_len)) {
      long long cs = 0;
      if(Fl_Html_Tokenizer::int_value(value, cs, Fl_Html_Tokenizer::FORMAT_10,
    value_len)) { if(cs<0) cs = 0; if(cs>100000) cs = 100000; width = cs;
      }
      return;
    }
    */
    if (is_word (name, "height", name_len))
    {
        long long cs = 0;
        if (Fl_Html_Tokenizer::int_value (
                value, cs, Fl_Html_Tokenizer::FORMAT_10, value_len))
        {
            if (cs < 0)
                cs = 0;
            if (cs > 100000)
                cs = 100000;
            height = cs;
        }
        return;
    }
    if (is_word (name, "hspace", name_len))
    {
        long long cs = 0;
        if (Fl_Html_Tokenizer::int_value (
                value, cs, Fl_Html_Tokenizer::FORMAT_10, value_len))
        {
            if (cs < 0)
                cs = 0;
            if (cs > 100000)
                cs = 100000;
            hspace = cs;
        }
        return;
    }
    if (is_word (name, "vspace", name_len))
    {
        long long cs = 0;
        if (Fl_Html_Tokenizer::int_value (
                value, cs, Fl_Html_Tokenizer::FORMAT_10, value_len))
        {
            if (cs < 0)
                cs = 0;
            if (cs > 100000)
                cs = 100000;
            vspace = cs;
        }
        return;
    }
    if (is_word (name, "border", name_len))
    {
        long long cs = 0;
        if (Fl_Html_Tokenizer::int_value (
                value, cs, Fl_Html_Tokenizer::FORMAT_10, value_len))
        {
            if (cs < 0)
                cs = 0;
            if (cs > 100000)
                cs = 100000;
            border = cs;
        }
        return;
    }
}

bool no_body () const
{
    return true;
}
}
;

#include "Fl_Html_Tag_table.H" // we have <table>, <tr> and <td> in a sepparate file

void Fl_Html_Formatter::remove_formatter ()
{
    delete table_formatter_;
    table_formatter_ = 0;
}

Fl_Html_Table_Formatter * Fl_Html_Formatter::table_formatter ()
{
    if (!table_formatter_)
        table_formatter_ = new Fl_Html_Table_Formatter ();
    return table_formatter_;
}

/*   ------------------- NEVER FORGET TO CHECK AND POSSIBLY REIMPLEMENT
FOLLOWING VIRTUAL FUNCTIONS: --------------------------


void init_format(Fl_Html_Formatter * s){
void format(Fl_Html_Formatter * s){
void min_width (int &width) const {
void draw(Fl_Html_Drawing_Device_ * d) {


// If you do not use macro

BUILD_TAG(NAME, BLOCK_LEVEL, CLOSED_BY_BLOCK_LEVEL, CLOSED_BY_SIMILAR,
BREAKS_WORD)

// but you are just subclassing, implement also virtual functions

const char * name() const  {
int type()() const {
int closed_by() const {


// and static function

static Fl_Xml_Object * create() {

// and possibly override other virtual functions of Fl_Html_Object
-------------------------------------------------------------------------------------------------------------------------------*/

static const int no_tags = 41;
static const int extra_tag_space =
    8; // some space that adding tags to it does not reallocate it immediatelly

/////// Here place new tags...
static Fl_Html_Pair_Table::Entry tag_entry_table[no_tags + extra_tag_space] = {
    TAG_ENTRY (span), TAG_ENTRY (div),    TAG_ENTRY (pre), TAG_ENTRY (meta),
    TAG_ENTRY (head), TAG_ENTRY (title),  TAG_ENTRY (sup), TAG_ENTRY (sub),
    TAG_ENTRY (body), TAG_ENTRY (code),   TAG_ENTRY (tt),  TAG_ENTRY (kbd),
    TAG_ENTRY (var),  TAG_ENTRY (strong), TAG_ENTRY (em),  TAG_ENTRY (dl),
    TAG_ENTRY (dt),   TAG_ENTRY (dd),     TAG_ENTRY (hr),  TAG_ENTRY (h6),
    TAG_ENTRY (h5),   TAG_ENTRY (h4),     TAG_ENTRY (h3),  TAG_ENTRY (h2),
    TAG_ENTRY (h1),   TAG_ENTRY (ul),     TAG_ENTRY (ol),  TAG_ENTRY (li),
    TAG_ENTRY (u),    TAG_ENTRY (img),    TAG_ENTRY (a),   TAG_ENTRY (table),
    TAG_ENTRY (th),   TAG_ENTRY (tr),     TAG_ENTRY (td),  TAG_ENTRY (center),
    TAG_ENTRY (font), TAG_ENTRY (p),      TAG_ENTRY (b),   TAG_ENTRY (i),
    TAG_ENTRY (br)

};

// Object creation table
class Create_Html_Object_Table : public Fl_Html_Object_::Create_Object_Table
{
  public:
    Create_Html_Object_Table (Fl_Html_Pair_Table::Entry * const table,
                              int filled, int table_size)
        : Fl_Html_Object_::Create_Object_Table (table, filled, table_size)
    {
    }
    Create_Html_Object_Table (const Create_Object_Table & table)
        : Fl_Html_Object_::Create_Object_Table (table){}; // copy constructor
    Create_Html_Object_Table (int size = 64)
        : Fl_Html_Object_::Create_Object_Table (size){};

    // Implementing pure virtual functions of base class
    Fl_Html_Object_ *
    create_word (Fl_Html_Tokenizer::Special_Character_Table * t,
                 const char * src, int src_len, int type = 0) const
    {
        if (type == 1)
            return new Html_Line_Word (t, src, src_len);
        return new Html_Word (t, src, src_len);
    };
    Fl_Html_Object_ * create_word (const char * word, unsigned word_len) const
    {
        return new Html_Word (word, word_len);
    };
    Fl_Html_Object_ * create_unknown_tag (const char * name,
                                          unsigned name_len) const
    {
        return new Html_Unknown_Tag (name, name_len);
    }
};

static Create_Html_Object_Table tag_table (tag_entry_table, no_tags,
                                           no_tags + extra_tag_space);

Fl_Html_Object_::Create_Object_Table *
Fl_Html_Parser::default_create_object_table ()
{
    return &tag_table;
}

static inline int ishex (int x)
{
    return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') ||
           (x >= 'A' && x <= 'F');
}

int Fl_Html_Object::url_decode (char * dec, const char * s, int s_len)
{
    char * o;
    if (!s_len)
        s_len = strlen (s);
    const char * end = s + s_len;
    int c;
    for (o = dec; s < end;)
    {
        c = *s++;
        if (c == '+')
            c = ' ';
        else if (c == '%' &&
                 (!ishex (*s++) || !ishex (*s++) || !sscanf (s - 2, "%2x", &c)))
            return -1;
        *o++ = c;
    }
    *o = 0; // assure null-termination
    return o - dec;
}

static int is_external_uri (const char * url, int url_len)
{
    Fl_Html_Object::skip_spaces (url, url_len);
    if (Fl_Html_Tokenizer::case_substring (url, "http:", url_len))
        return 1;
    if (Fl_Html_Tokenizer::case_substring (url, "https:", url_len))
        return 2;
    if (Fl_Html_Tokenizer::case_substring (url, "ftp:", url_len))
        return 3;
    if (Fl_Html_Tokenizer::case_substring (url, "mailto:", url_len))
        return 4;
    if (Fl_Html_Tokenizer::case_substring (url, "news:", url_len))
        return 5;
    if (Fl_Html_Tokenizer::case_substring (url, "ipp:", url_len))
        return 6;
    return 0;
}

static const char * my_strrchr (const char * s, char c, int len)
{
    s += len - 1;
    while (len--)
    {
        if (*s == c)
            return s;
        s--;
    }
    return 0;
}

int Fl_Html_Object::local_filename_from_uri (char * result, char * anchor,
                                             const char * directory,
                                             const char * url, int url_len,
                                             bool percent_encoded)
{
    if (!url)
        return -1;
    if (!url_len)
        url_len = strlen (url);
    if (!url_len)
        return -1;
    *result = 0;
    *anchor = 0;
    Fl_Html_Object_::skip_spaces (url, url_len);
    int r = is_external_uri (url, url_len);
    if (r)
        return r;
    // if(!directory || !*directory) {
    r = Fl_Html_Tokenizer::case_substring (url, "file:///", url_len);
    if (!r)
        r = Fl_Html_Tokenizer::case_substring (url, "file:", url_len);
    //}
    bool is_absolute = false;
    if (!r)
        Fl_Html_Object_::skip_spaces (url, url_len);
    else
        is_absolute = true;
    url += r;
    url_len -= r;

    if (!r && *url != '/' && *url != '#'
#ifdef WIN32
        && *url != '\\' && (url_len < 2 || url[1] != ':')
#endif
    )
    { // it is relative
        int len = 0;
        if ((!is_absolute) && directory && *directory)
        {
            len = strlen (directory);
            memcpy (result, directory, len);
        }
        result += len;
        if (len)
        {
            if (result[-1] != '/'
#ifdef WIN32
                && result[-1] != '\\'
#endif // WIN32
            )
            {
                *result = '/';
                result++;
            }
        }
        *result = 0;
    }
    const char * hash = my_strrchr (url, '#', url_len);
    if (hash)
    {
        if (percent_encoded)
            Fl_Html_Object::url_decode (anchor, hash + 1);
        else
            strcpy (anchor, hash + 1);
        if (hash == url)
            return 0;
        url_len = hash - url;
    }
    else
        *anchor = 0;
    if (percent_encoded)
        Fl_Html_Object::url_decode (result, url, url_len);
    else
    {
        strncpy (result, url, url_len);
        result[url_len] = 0;
    }
    return 0;
}

#define WHITESPACE 64
#define EQUALS 65
#define INVALID 66

// This is simple & fast base64 decoder. Output buffer should be at least of
// size (len * 3 + 2)/4.
static int base64_decode (unsigned char * out, const char * in, size_t len = 0)
{
    static const unsigned char d[] = {
        66, 66, 66, 66, 66, 66, 66, 66, 66, 64, 64, 64, 64, 64, 66, 66, 66, 66,
        66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
        66, 66, 66, 66, 66, 66, 66, 62, 66, 66, 66, 63, 52, 53, 54, 55, 56, 57,
        58, 59, 60, 61, 66, 66, 66, 65, 66, 66, 66, 0,  1,  2,  3,  4,  5,  6,
        7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
        25, 66, 66, 66, 66, 66, 66, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
        37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 66, 66, 66,
        66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
        66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
        66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
        66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
        66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
        66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
        66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
        66, 66, 66, 66};
    unsigned char * st1 = out;
    if (!len)
        len = strlen (in);
    const char * end = in + len;
    size_t buf = 1;
    while (in < end)
    {
        unsigned char c = d[(int)(*in++)];
        switch (c)
        {
        case WHITESPACE:
            continue; /* skip whitespace */
        case INVALID:
            return -1; /* invalid input, return error */
        case EQUALS:   /* pad character, end of data */
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
            if (buf & 0x1000000)
            {
                *out++ = buf >> 16;
                *out++ = buf >> 8;
                *out++ = buf;
                buf = 1;
            }
        }
    }
    if (buf & 0x40000)
    {
        *out++ = buf >> 10;
        *out++ = buf >> 2;
    }
    else if (buf & 0x1000)
        *out++ = buf >> 4;
    return out - st1;
}

Fl_Html_Image_ * html_get_image (Fl_Html_Parser * p, const char * val,
                                 int val_len)
{
    if (!val || !val_len)
        return 0;
    int sub = 0;
    int type = 0;
    char * data = 0;
    if ((sub = Fl_Html_Tokenizer::case_substring (
             val, "data:image/jpeg;base64,", val_len)))
        type = HTML_URI_DATA_JPEG;
    else if ((sub = Fl_Html_Tokenizer::case_substring (
                  val, "data:image/png;base64,", val_len)))
        type = HTML_URI_DATA_PNG;
    else if ((sub = Fl_Html_Tokenizer::case_substring (
                  val, "data:image/gif;base64,", val_len)))
        type = HTML_URI_DATA_GIF;
    if (sub)
    {
        Fl_Html_Parser::Image_Creator cr = p->image_creator ();
        if (cr)
        {
            val += sub;
            val_len -= sub;
            data = new char[(val_len * 3 + 3) / 4 + 1];
            val_len = base64_decode ((unsigned char *)data, val, val_len);
            if (val_len < 0)
            {
                val_len = 0;
                *data = 0;
            }
            val = (const char *)data;
            data[val_len] = 0; // in any case assure null-termination
        }
    }
    else
    {
        int url_len = 3;
        if (p->directory ())
            url_len += strlen (p->directory ());
        url_len += val_len;
        data = new char[url_len];
        char * anchor = new char[url_len];
        if (0 < Fl_Html_Object::local_filename_from_uri (
                    data, anchor, p->directory (), val, val_len, true))
            type = HTML_URI_URL;
        else
        {
            type = HTML_URI_FILE, val = data;
            val_len = strlen (data);
        }
        delete[] anchor;
    }

    printf ("Val: %s\n", val);

    Fl_Html_Image_ * im = (*(p->image_creator ())) (val, val_len, type);
    delete[] data;
    return im;
}

Fl_Html_Object * Fl_Html_Object::first_drawable (Fl_Html_Object::Iterator * it,
                                                 Fl_Html_Object * o, int y,
                                                 const Fl_Html_Object * limit)
{
    Fl_Html_Object * last = o;

    while (o && o != limit)
    {
        /*
         if(o->name())
          printf("Tag %s\n", o->name());
         else
          printf("Word: %s\n", ((Html_Word *)o)->value());
        */

        int drawable = o->drawable ();
        bool over = (!(drawable & 0xFF000000)) && ((o->y + o->h) >= y);
        if (over)
        {
            if (drawable & (4 | 8))
            {
                if (!o->children ())
                    return o;
                int dx;
                int dy;
                o->translate_for_children (dx, dy);
                // int xx = o->y;
                Fl_Html_Object * lim = (Fl_Html_Object *)(o->next ());
                last = o;
                o = it->next (o);
                Fl_Html_Object * fd = first_drawable (it, o, y - dy, lim);
                if (fd)
                    return fd;
                else
                    return last;
            }
            else if (drawable & 1)
                return o;
            else
            {
                last = o;
                o = it->next (o);
            }
        }
        else
        {
            last = o;
            o = it->next (o, false);
        }
    }
    return 0;
}

Fl_Html_Object * Fl_Html_Object::point_within (Fl_Html_Object::Iterator * it,
                                               Fl_Html_Object * o, int x, int y)
{
    while (o)
    {
        if (o->point_in_bounding_box (x, y))
        {
            Fl_Html_Object * children = (Fl_Html_Object *)(o->children ());
            if (!children)
                return o;
            int dx;
            int dy;
            o->translate_for_children (dx, dy);
            it->add_parent (o);
            Fl_Html_Object * pw = point_within (it, children, x - dx, y - dy);
            if (pw)
                return pw;
            it->remove_parent ();
            if (!(o->drawable () & 16))
                return o;
        }
        o = (Fl_Html_Object *)(o->next ());
    }
    return 0;
}

const char * Fl_Html_Object::get_word_value (const Fl_Html_Object * o)
{
    if (!o || o->name ())
        return 0;
    return ((Html_Word *)o)->value ();
}

void Fl_Html_Object::global_coordinates (
    const Fl_Html_Object::Iterator * parents, const Fl_Html_Object * o, int & x,
    int & y)
{
    x = 0;
    y = 0;
    for (int i = 0; i < parents->no_parents (); i++)
    {
        int dx, dy;
        parents->parent (i)->translate_for_children (dx, dy);
        x += dx;
        y += dy;
    }
    x += o->x;
    y += o->y;
}

static void get_selection_limits (Fl_Html_Object::Iterator & it, int & start,
                                  int & stop)
{
    for (int i = it.no_parents () - 1; i >= 0; i--)
    {
        Fl_Html_Object * o = it.parent (i);
        if (o->drawable () & 1)
        {
            start = stop = o->index ();
        }
        else if (o->selection_bounding ())
        {
            int st = o->index ();
            if (st > start)
                start = st;
            o = (Fl_Html_Object *)(o->next ());
            i--;
            while ((!o) && i >= 0)
            {
                o = (Fl_Html_Object *)(it.parent (i)->next ());
                i--;
            }
            if (o)
            {
                st = o->index ();
                if (st < stop)
                    stop = st;
            }
            break;
        }
    }
}

Fl_Html_Object * Fl_Html_Object::get_selection (Fl_Html_Object * list, int x1,
                                                int y1, int x2, int y2,
                                                int & first, int & last)
{
    first = 0;
    last = 0;
    Fl_Html_Object::Iterator it1, it2;

    Fl_Html_Object * o1 = Fl_Html_Object::point_within (&it1, list, x1, y1);
    Fl_Html_Object * o2 = Fl_Html_Object::point_within (&it2, list, x2, y2);

    int no_common_parents = it1.no_parents ();
    if (it2.no_parents () < no_common_parents)
        no_common_parents = it2.no_parents ();

    Fl_Html_Object * common_parent = 0;
    if (o1)
        it1.add_parent (o1);
    if (o2)
        it2.add_parent (o2);

    for (int i = 0; i < no_common_parents; i++)
    {
        if (it1.parent (i) == it2.parent (i))
            common_parent = it1.parent (i);
    }

    int start1 = 0;
    int start2 = 0;
    int stop1 = 0xFFFFFFF;
    int stop2 = 0xFFFFFFF;

    int j = it1.no_parents ();
    int xx1 = x1;
    int yy1 = y1;
    Fl_Html_Object * list1 = list;
    while (j)
    {
        j--;
        if (it1.parent (j)->drawable () & (4 | 8))
        {
            list1 = (Fl_Html_Object *)(it1.parent (j)->children ());
            for (int i = 0; i <= j; i++)
            {
                int dx, dy;
                it1.parent (i)->translate_for_children (dx, dy);
                xx1 -= dx;
                yy1 -= dy;
            }
            break;
        }
    }

    Fl_Html_Object * list2 = list;
    int xx2 = x2;
    int yy2 = y2;
    j = it2.no_parents ();
    while (j)
    {
        j--;
        if (it2.parent (j)->drawable () & (4 | 8))
        {
            list2 = (Fl_Html_Object *)(it2.parent (j)->children ());
            for (int i = 0; i <= j; i++)
            {
                int dx, dy;
                it2.parent (i)->translate_for_children (dx, dy);
                xx2 -= dx;
                yy2 -= dy;
            }
            break;
        }
    }

    if (o1)
        it1.add_parent (o1);
    if (o2)
        it2.add_parent (o2);

    get_selection_limits (it1, start1, stop1);
    get_selection_limits (it2, start2, stop2);

    Fl_Html_Object * o = 0;
    get_selection_last_non_drawable (list1, xx1, yy1, o);
    if (o)
    {
        int index = o->index () + 1;
        if (index > start1)
            start1 = index;
    }

    o = 0;
    get_selection_last_drawable (list1, xx1, yy1, o);

    if (o)
    {
        int index = o->index ();
        if (index < stop1)
            stop1 = index;
    }
    else
        stop1 = start1 - 1;
    o = 0;
    get_selection_last_non_drawable (list2, xx2, yy2, o);
    if (o)
    {
        int index = o->index () + 1;
        if (index > start2)
            start2 = index;
    }
    o = 0;
    get_selection_last_drawable (list2, xx2, yy2, o);
    if (o)
    {
        int index = o->index ();
        if (index < stop2)
            stop2 = index;
    }
    else
        stop2 = start2 - 1;

    if (start1 > start2 || (start1 == start2 && stop1 > stop2))
    {
        first = start2;
        last = stop1;
    }
    else
    {
        first = start1;
        last = stop2;
    }
    return common_parent;
}

void Fl_Html_Object::export_selection (Fl_Html_Object::Text_Buffer & text,
                                       Fl_Html_Object * list, int first,
                                       int last, int type)
{
    Fl_Html_Object * o = list;
    while (o)
    {
        if (o->index () > last)
            break;
        Fl_Html_Object * n = (Fl_Html_Object *)(o->next ());
        if (!n || (n->index () > first))
        {
            o->export_start (text, type);
            if (o->index () >= first)
                o->export_body (text, type);
            export_selection (
                text, (Fl_Html_Object *)(o->children ()), first, last, type);
            o->export_end (text, type);
        }
        o = n;
    }
}

char * Fl_Html_Object::export_selection (Fl_Html_Object * list, int first,
                                         int last, int type)
{
    if (type != CLIPBOARD_TEXT)
        return 0;
    Fl_Html_Object::Text_Buffer text;
    export_selection (text, list, first, last, type);
    char * b = text.buffer ();
    return b;
}
