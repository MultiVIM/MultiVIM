#include "Fl_Html_Formatter.H"
#include "Fl_Html_Object.H"
#include <stdio.h>
#include <string.h>

////////////////////////////////////   Implementation of Fl_Html_Formatter
/// functions /////////////////////////////////////

void Fl_Html_Formatter::push_margin (int position, int limit, int *& table,
                                     int *& current, int & no_margins)
{
    int sz = (current - table) + 2;
    if (sz + 2 >= no_margins)
    { // need bigger table
        no_margins *= 2;
        int * nt = new int[no_margins];
        memcpy (nt, table, sz * sizeof (int));
        delete[] table;
        table = nt;
    }
    current = table + sz;
    *current = position;
    current[1] = limit;
}

static void add_object (Fl_Html_Object * o, int & no_objects,
                        Fl_Html_Object **& where, int & where_size)
{
    if (no_objects >= (where_size - 1))
    {
        where_size *= 2;
        Fl_Html_Object ** nt = new Fl_Html_Object *[where_size];
        Fl_Html_Object ** x = where;
        memcpy (nt, x, (no_objects) * sizeof (Fl_Html_Object *));
        delete[] x;
        where = nt;
    }
    where[no_objects] = o;
    no_objects++;
}

void Fl_Html_Formatter::add_line_object (Fl_Html_Object * o)
{
    add_object (o, no_line_objects_, line_objects_, line_objects_size_);
}

void Fl_Html_Formatter::add_todo_object (Fl_Html_Object * o)
{
    add_object (o, no_todo_objects_, todo_objects_, todo_objects_size_);
}
void Fl_Html_Formatter::add_todo_format_object (Fl_Html_Object * o)
{
    add_object (o,
                no_todo_format_objects_,
                todo_format_objects_,
                todo_format_objects_size_);
}

void Fl_Html_Formatter::finish_line (int paragraph_dy)
{
    x_ -= space_added_;
    space_added_ = 0;
    int collision = 0;
    int x_shift = 0;
    float shift_add = 0;
    int xx = x_;
    float tot_sum_spaces = 0;
    if (x_align_ != LEFT)
    {
        if (no_line_objects_)
            xx = line_objects_[no_line_objects_ - 1]->x +
                 line_objects_[no_line_objects_ - 1]->w;
        x_shift = right_margin () - xx;
        if (x_shift < 0)
            x_shift = 0;
        if (x_align_ == CENTER)
            x_shift /= 2;
        else if (x_align_ == JUSTIFY)
        {
            if (no_line_objects_ > 1)
            {
                int sum_spaces = 0;
                for (int i = 1; i < no_line_objects_; i++)
                {
                    sum_spaces += line_objects_[i]->x -
                                  line_objects_[i - 1]->x -
                                  line_objects_[i - 1]->w;
                }
                tot_sum_spaces = sum_spaces;
                if (tot_sum_spaces)
                    shift_add = float (x_shift) / tot_sum_spaces;
            }
            x_shift = 0;
        }
    }

    if (no_line_objects_)
    {
        collision = start_y_ - min_y_; // line collision extend
        if (collision > 0 || x_shift || shift_add)
        { // adjusting position of the objects
            if (shift_add)
            {
                float cx = 0;
                float line_objects_p_end = line_objects_[0]->x;
                for (int i = 0; i < no_line_objects_; i++)
                {
                    float shift =
                        (line_objects_[i]->x - line_objects_p_end) * shift_add;
                    cx += shift;
                    line_objects_p_end =
                        line_objects_[i]->x + line_objects_[i]->w;
                    line_objects_[i]->y += collision;
                    line_objects_[i]->x += int (x_shift + cx + 0.5);
                }
            }
            else
                for (int i = 0; i < no_line_objects_; i++)
                {
                    line_objects_[i]->y += collision;
                    line_objects_[i]->x += int (x_shift + 0.5);
                }
            start_y_ += collision;
        }
        int mw = line_objects_[no_line_objects_ - 1]->x +
                 line_objects_[no_line_objects_ - 1]->w;
        if (mw > max_width_)
            max_width_ = mw;
        no_line_objects_ = 0;
        paragraph_dy_ = 0;
        int end = start_y_ - collision + max_y_ - min_y_;
        if (end > bottom_)
            bottom_ = end;
        start_line (end + paragraph_dy);
    }
    else if (paragraph_dy_ < paragraph_dy)
        start_line (start_y_ + paragraph_dy - paragraph_dy_);
    if (no_todo_format_objects_)
    {
        // we have to do more tricky iteration due to recursion
        int lim = no_todo_format_objects_;
        Fl_Html_Object ** old = todo_format_objects_;
        todo_format_objects_ = new Fl_Html_Object *[todo_format_objects_size_];
        no_todo_format_objects_ = 0; // we have to
        for (int i = 0; i < lim; i++)
        {
            old[i]->format (this);
        }
        delete[] old;
    }
    paragraph_dy_ = paragraph_dy;

    /*

    while(no_todo_objects_) { //
      no_todo_objects_--;
      todo_objects_[no_todo_objects_]->bounding_box_children();
    }
    */

    for (int i = 0; i < no_todo_objects_; i++)
        todo_objects_[i]->bounding_box_children ();
    no_todo_objects_ = 0;
}

int Fl_Html_Formatter::find_place (int min_width, int finish_previous_as_last)
{
    if ((right_margin () - x_ < min_width))
    {
        if (finish_previous_as_last)
            finish_last_line ();
        else
            finish_line ();
    }
    while (right_margin () - x_ < min_width)
    {
        int lim;
        if (cur_left_margin_ != left_margin_table_)
        {
            if (cur_right_margin_ == right_margin_table_)
            {
                lim = left_margin_limit ();
            }
            else if (left_margin_limit () <= right_margin_limit ())
                lim = left_margin_limit ();
            else
                lim = right_margin_limit ();
        }
        else
        {
            if (cur_right_margin_ == right_margin_table_)
                return -1;
            else
                lim = right_margin_limit ();
        }
        // if(lim == INT_MAX) return 1; // infinity, cant update...
        start_line (lim);
    }
    return right_margin () - x_;
}

void Fl_Html_Formatter::place_inline (Fl_Html_Object * o)
{

    switch (y_align_)
    {
    case Fl_Html_Formatter::TOP:
        o->y = y1_;
        y2_ = y1_ + o->h;
        break;
    case Fl_Html_Formatter::BOTTOM:
        y1_ = y2_ - o->h;
        o->y = y1_;
        break;
    case Fl_Html_Formatter::MIDDLE:
        y1_ += (y2_ - y1_) / 2 - o->h / 2;
        o->y = y1_;
        y2_ = y1_ + o->h;
        break;
    case Fl_Html_Formatter::BASELINE:
    {
        y2_ = baseline_ + drawing_device ()->descent ();
        y1_ = y2_ - o->h;
        o->y = y1_;
    }
    break;
    }
    if (y1_ < min_y_)
        min_y_ = y1_;
    if (y2_ > max_y_)
        max_y_ = y2_;
    o->x = x_;
    x_ += o->w;
    int es = right_margin () - x_;
    if (es < extra_space_)
        extra_space_ = es;
    add_line_object (o);
    space_added_ = 0;
    o->handle_bullet (this);
}

void Fl_Html_Formatter::place_left (Fl_Html_Object * o, int wrap)
{
    int w = o->w;
    // int yy = start_y_;
    if (!wrap || (available_width () < w))
        finish_last_line ();
    else
        for (int i = 0; i < no_line_objects_; i++)
            line_objects_[i]->x += w;
    x_ += w;
    int es = right_margin () - x_;
    if (es < extra_space_)
        extra_space_ = es;
    int lm = left_margin ();
    o->x = lm;
    o->y = start_y_;
    int bt = start_y_ + o->h;
    if (bt > bottom_)
        bottom_ = bt;
    if (wrap)
        push_left_margin (lm + w, start_y_ + o->h);
    else
        start_line (start_y_ + o->h);
    lm += o->w;
    if (lm > max_width_)
        max_width_ = lm;
}

void Fl_Html_Formatter::place_right (Fl_Html_Object * o, int wrap)
{
    if (!wrap)
        finish_last_line ();
    int w = o->w;
    int rm = right_margin ();
    int ac = rm - w;
    if (ac < left_margin () + left_shift_)
        ac = left_margin () + left_shift_;
    o->x = ac;
    o->y = start_y_;
    if (max_width_ < rm)
        max_width_ = rm;

    int es = rm - x_ - w;
    if (es < extra_space_)
        extra_space_ = es;
    int bt = start_y_ + o->h;
    if (bt > bottom_)
        bottom_ = bt;
    if (wrap)
        push_right_margin (rm - w, start_y_ + o->h);
    else
        start_line (start_y_ + o->h);
}

void Fl_Html_Formatter::place_center (Fl_Html_Object * o)
{
    finish_last_line ();
    int es = right_margin () - (left_margin () + left_shift_) - o->w;
    if (es < extra_space_)
        extra_space_ = es;
    if (es < 0)
        es = 0;

    int xx = left_margin () + left_shift_ + es / 2;
    o->x = xx;
    xx += o->w;
    if (xx > max_width_)
        max_width_ = xx;
    o->y = start_y_;
    int bt = start_y_ + o->h;
    if (bt > bottom_)
        bottom_ = bt;
    start_line (start_y_ + o->h);
}

void Fl_Html_Formatter::init_format (Fl_Html_Object * o)
{
    // float z = drawing_device()->zoom();
    // drawing_device()->zoom(1);
    drawing_device ()->origin (0, 0);
    drawing_device ()->init_draw ();
    background_color_set_ = false;
    while (o)
    {
        o->init_format (this);
        o = (Fl_Html_Object *)o->next ();
    }
}

void Fl_Html_Formatter::format (Fl_Html_Object * o, int width,
                                Fl_Html_Drawing_Device_ * device, int start_y,
                                int start_align)
{
    if (device)
        drawing_device (device);
    drawing_device ()->origin (0, 0);
    reset_for_format (width, start_y, start_align);
    while (o)
    {
        o->format (this);
        o = (Fl_Html_Object *)(o->next ());
    }
    if (x_align () == JUSTIFY)
        x_align (LEFT); // we do not justigy last line
    finish_line ();     // we need to finis last line
}

void Fl_Html_Formatter::format (Fl_Html_Object * o)
{

    while (o)
    {
        o->format (this);
        o = (Fl_Html_Object *)(o->next ());
    }
    if (x_align () == JUSTIFY)
        x_align (LEFT); // we do not justigy last line
    finish_line ();     // we need to finis last line
}

void Fl_Html_Formatter::reset_for_format (int width, int start_y,
                                          int start_align)
{
    reset (width, start_y, start_align);
    drawing_device ()->reset_for_format ();
}

void Fl_Html_Formatter::reset (int page_width, int start_y, int start_align)
{

    cur_left_margin_ = left_margin_table_;
    cur_right_margin_ = right_margin_table_;
    *left_margin_table_ = 0;
    *right_margin_table_ = page_width;
    percentual_width_left_ = 0;
    percentual_width_right_ = 0;
    percentual_width_inline_ = 0;
    no_line_objects_ = 0;
    no_todo_objects_ = 0;
    no_todo_format_objects_ = 0;
    bullet_type_ = Fl_Html_Object::BULLET_SQUARE;
    x_ = 0;
    max_width_ = 0;

    x_align_ = start_align;
    y_align_ = TOP;

    max_y_ = min_y_ = bottom_ = baseline_ = start_y_ = y1_ = y2_ = start_y;
    left_shift_ = 0;
    space_added_ = 0;

    paragraph_dy_ = 0;
    extra_space_ = INT_MAX >> 3;
}

int Fl_Html_Object::min_page_width () const
{
    int w = 0;
    min_width (w);
    Fl_Html_Object * o = (Fl_Html_Object *)next ();
    while (o)
    {
        ((Fl_Html_Object *)o)->min_width (w);
        o = (Fl_Html_Object *)(o->next ());
    }
    return w;
}

int Fl_Html_Formatter::min_page_width (const Fl_Html_Object * o)
{
    return o->min_page_width ();
}
