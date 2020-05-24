
class Html_Tag_td;
class Html_Tag_table;

class Fl_Html_Table_Formatter
{
    int conflicts_size;
    int no_cols;
    int * conflicts;
    int cur_column;

    int no_rows;
    int cur_row;

    int last_colspan;
    int last_rowspan;

    int no_td;
    int td_size;
    Html_Tag_td ** td;

    int first_td_in_row_size;
    int * first_td_in_row;

    void add_element (int col, int row, Html_Tag_td * d)
    {
        if (no_td >= td_size)
        {
            td_size *= 2;
            xrealloc (td, td_size);
        }
        td[no_td] = d;
        no_td++;
    }

    void resize_conflicts (int no)
    {
        if (no <= no_cols)
            return;
        if (no > conflicts_size)
        {
            conflicts_size = 2 * no;
            xrealloc (conflicts, conflicts_size);
        }
        for (int i = no_cols; i < no; i++)
            conflicts[i] = 0;
        no_cols = no;
    }

    int percentages_;

  public:
    void copy (Html_Tag_td **& elements, int *& first_in_row, int & no_elements,
               int & no_columns, int & no_rows_)
    {
        no_elements = no_td;
        no_columns = no_cols;
        no_rows_ = no_rows;
        if (!no_cols)
            return;
        elements = new Html_Tag_td *[no_elements];
        xcopy (elements, td, no_elements);
        first_in_row = new int[no_rows];
        xcopy (first_in_row, first_td_in_row, no_rows);
    }
    int percentages () const
    {
        return percentages_;
    }
    void update_percentages (int v)
    {
        percentages_ |= v;
    }
    int columns () const
    {
        return no_cols;
    }
    int rows () const
    {
        return no_rows;
    }
    int next (int colspan, int rowspan, Html_Tag_td * element = 0)
    {
        cur_column += last_colspan;
        last_colspan = colspan;
        int conf;
        do
        {
            int to_check = colspan;
            if (cur_column + to_check > no_cols)
                to_check = no_cols - cur_column;
            conf = 0;
            for (int i = 0; i < to_check; i++)
                if (conflicts[cur_column + i] > 0)
                    conf = i + 1;
            cur_column += conf;
        } while (conf);
        resize_conflicts (cur_column + colspan);
        int nr = cur_row + rowspan;
        if (nr > no_rows)
            no_rows = nr;
        for (int i = 0; i < colspan; i++)
            conflicts[cur_column + i] = rowspan;
        last_rowspan = rowspan;
        add_element (cur_column, cur_row, element);
        return cur_column;
        // printf("cell at: %i  %i   colspan = %i rowspan = %i\n\n", cur_column,
        // cur_row, last_colspan, last_rowspan);
    }

    void print_elements () const;

    void reset ()
    {
        no_cols = 0;
        cur_column = 0;
        last_colspan = 0;
        last_rowspan = 0;
        no_rows = 0;
        cur_row = -1;
        no_td = 0;
        percentages_ = 0;
    }

    // this should be called before each line
    void next_line ()
    {
        cur_column = 0;
        cur_row++;
        if (no_rows > first_td_in_row_size)
        {
            first_td_in_row_size *= 2;
            xrealloc (first_td_in_row, first_td_in_row_size);
        }
        first_td_in_row[cur_row] = no_td;
        last_colspan = 0;
        last_rowspan = 0;
        for (int i = 0; i < no_cols; i++)
            --(conflicts[i]);
    }

    Fl_Html_Table_Formatter (int size = 64) : conflicts_size (size)
    {
        xalloc (conflicts, size);
        first_td_in_row_size = size;
        xalloc (first_td_in_row, first_td_in_row_size);
        td_size = size * 1;
        xalloc (td, td_size);
        reset ();
    }
    ~Fl_Html_Table_Formatter ()
    {
        xfree (conflicts);
        xfree (first_td_in_row);
        xfree (td);
        xfree (td);
    }

    typedef int (Html_Tag_td::*Get_Position) () const;

    // void calculate_position_left(int * positions, Get_Position fn, int
    // csllspacing)const;
};

/////////////////// <tr> /////////////////////////////

BUILD_TAG (tr, Fl_Html_Object::BLOCK | Fl_Html_Object::TR, Fl_Html_Object::TR)

int drawable () const
{
    return 16;
}
int breaks_word () const
{
    return 2;
}

void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len){};

bool point_in_selection_box (int X, int Y, bool & overlapping) const
{
    overlapping = true;
    return point_in_bounding_box_ (X, Y);
}
/*
bool point_within(int X, int Y) const {
    if(!point_in_bounding_box(X, Y)) return false;
    return point_within_children(X, Y);
}
*/
void init_format (Fl_Html_Formatter * s)
{

    s->table_formatter ()->next_line ();
    init_format_children (s);
}

void format (Fl_Html_Formatter * s)
{
    void * back = s->aux_object (Fl_Html_Formatter::ROW);
    s->aux_object (Fl_Html_Formatter::ROW, this);
    h = 0;
    format_children (s);
    set_out ();
    bounding_box_children ();
    s->aux_object (Fl_Html_Formatter::ROW, back);
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    draw_children (d /*, false */);
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

static int cellpadding (const Html_Tag_table * t);

///////////////////  <td> ///////////////////////////
BUILD_TAG (td, Fl_Html_Object::BLOCK | Fl_Html_Object::TD,
           Fl_Html_Object::TD | Fl_Html_Object::TR)
int minimum_w;
int maximum_w;
int minimum_h;
int width; // width as read by attrubute
int height;
unsigned flags;
int colspan;
int rowspan;
int column;
int cell_h;
int padding;
int bgcolor;
const char * link_;

/*
bool point_within(int X, int Y) const {
    return point_in_bounding_box(X, Y);
} */
int drawable () const
{
    return 4;
}
const char * link () const
{
    return link_;
}
int breaks_word () const
{
    return 2;
}

bool selection_bounding () const
{
    return true;
}

Html_Tag_td ()
{
    width = 0;
    height = 0;
    flags = VALIGN_MIDDLE;
    colspan = 1;
    rowspan = 1;
    link_ = 0;
}

void export_end (Text_Buffer & text, int type) const
{
    text.add ("\t");
    text.newlines (0);
}

bool is_static_width (bool percent = 0)
{
    if (percent)
    {
        if (width && (flags & WIDTH_PERCENT))
            return true;
        return false;
    }
    if (width && !(flags & WIDTH_PERCENT))
        return true;
    return false;
}

bool is_static_height (bool percent = 0)
{
    if (percent)
    {
        if (height && (flags & HEIGHT_PERCENT))
            return true;
        return false;
    }
    if (height && !(flags & HEIGHT_PERCENT))
        return true;
    return false;
}

void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (is_word (name, "bgcolor", name_len))
    {
        if (read_color (bgcolor, value, value_len))
            flags |= BGCOLOR;
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
    if (is_word (name, "height", name_len))
    {
        bool percents = 0;
        if (read_with_percents (value, height, percents, value_len))
        {
            if (percents)
                flags |= HEIGHT_PERCENT;
            else
                flags &= ~HEIGHT_PERCENT;
        }
        return;
    }
    if (read_align (flags, name, name_len, value, value_len))
        return;

    if (is_word (name, "valign", name_len))
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
        }
        return;
    }
    if (is_word (name, "colspan", name_len))
    {
        long long cs = 1;
        if (Fl_Html_Tokenizer::int_value (
                value, cs, Fl_Html_Tokenizer::FORMAT_10, value_len))
        {
            if (cs < 1)
                cs = 1;
            if (cs > 100000)
                cs = 100000;
            colspan = cs;
        }
        return;
    }
    if (is_word (name, "rowspan", name_len))
    {
        long long cs = 1;
        if (Fl_Html_Tokenizer::int_value (
                value, cs, Fl_Html_Tokenizer::FORMAT_10, value_len))
        {
            if (cs < 1)
                cs = 1;
            if (cs > 100000)
                cs = 100000;
            rowspan = cs;
        }
        return;
    }
    if (is_word (name, "nowrap", name_len))
    {
        flags |= NOWRAP;
        return;
    }
}

// \a s must be the aux state with \a  width set to HIGH_VAL
void reset_for_format (Fl_Html_Formatter * state, int width,
                       int align = ALIGN_LEFT)
{
    state->reset_for_format (width, 0, align);
}

virtual void before_format (Fl_Html_Formatter * s)
{
}
void init_format (Fl_Html_Formatter * s)
{

    Fl_Html_Formatter * state = s->aux_formatter ();
    reset_for_format (state, 0);
    before_format (state);
    init_format_children (state);

    reset_for_format (state, HIGH_VAL);
    before_format (state);
    state->format ((Fl_Html_Object *)(children ()));

    Fl_Html_Table_Formatter * table_formatter =
        (Fl_Html_Table_Formatter *)(s->aux_object (
            Fl_Html_Formatter::TABLE_FORMATTER));
    if (table_formatter)
    {
        column = table_formatter->next (colspan, rowspan, this);
        table_formatter->update_percentages (flags &
                                             (WIDTH_PERCENT | HEIGHT_PERCENT));
    }

    Html_Tag_table * table =
        (Html_Tag_table *)(s->aux_object (Fl_Html_Formatter::TABLE));
    padding = cellpadding (table);
    int ex = 2 * padding;

    int ww = HIGH_VAL - state->extra_space ();
    int perc_add = state->percentual_width ();
    if (perc_add)
    {
        perc_add = 100 - perc_add;
        if (perc_add < 5)
            perc_add = 5;
        if (perc_add > 100)
            perc_add = 100;
        ww = (ww * 100) / perc_add;
    }

    maximum_w = ww + ex;
    if (flags & NOWRAP)
        minimum_w = maximum_w;
    else
    {
        minimum_w = 0;
        min_width_children (minimum_w);
        minimum_w += ex;
        if (width)
        {
            if (!(flags & WIDTH_PERCENT))
            {
                if (width > minimum_w)
                    minimum_w = width;
                maximum_w = width;
            }
        }
    }
    if (maximum_w < minimum_w)
        maximum_w = minimum_w;
    minimum_h = state->bottom () + ex;
}

int get_minimum_w () const
{
    return minimum_w;
}
int get_maximum_w () const
{
    return maximum_w;
}

void min_width (int & w) const
{
    if (minimum_w > w)
        w = minimum_w;
}

void format (Fl_Html_Formatter * s)
{
}

// \a s must be the aux state with properly set width.
// Note that the parent table moves later all the elements.
void format_ (Fl_Html_Formatter * s)
{
    s = s->aux_formatter ();
    reset_for_format (s, w + 1 - 2 * padding, ALIGN_MASK & flags);
    before_format (s);
    s->format ((Fl_Html_Object *)(children ()));
    cell_h = h = s->bottom () + 2 * padding;
}

int min_height () const
{
    if (height && (!(flags & HEIGHT_PERCENT)) && (height > cell_h))
        return height;
    return cell_h;
}

void translate_for_children (int & dx, int & dy)
{
    dx = x + padding;
    dy = y + padding;
    switch (flags & VALIGN_MASK)
    {
    case VALIGN_MIDDLE:
        dy += (h - cell_h) / 2;
        break;
    case VALIGN_BOTTOM:
        dy += h - cell_h;
        break;
    }
}

void in_selection_children (int * selection, int & first, int & last)
{
    int dx, dy;
    Html_Tag_td::translate_for_children (dx, dy);
    selection[0] -= dx;
    selection[2] -= dx;
    selection[1] -= dy;
    selection[3] -= dy;
    Fl_Html_Object::in_selection_children (selection, first, last);
    selection[0] += dx;
    selection[2] += dx;
    selection[1] += dy;
    selection[3] += dy;
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    d->discontinue_underline ();

    int old_background = 0xFFFFFF;
    if (flags & BGCOLOR)
    {
        // int old = d->color();
        old_background = d->background_color ();
        // d->color(bgcolor);
        d->background_color (bgcolor);
        // d->rectf(x, y, w, h);
        // d->color(old);
    }

    int dx, dy;
    Html_Tag_td::translate_for_children (dx, dy);

    /*
    int dx_s, dy_s;
    bool selection_translated = false;
    if(d->selection_valid()){
      //if(in_selection(d->selection())){
         selection_translated = true;
         dx_s = dx;
         dy_s = dy;
         d->translate_selection(dx_s, dy_s);
      //}
    }
    */

    dx = int (dx * d->zoom () + 0.5);
    dy = int (dy * d->zoom () + 0.5);
    d->translate (dx, dy);
    draw_children (d /*, false */);
    /*
    if(selection_translated)
       d->translate_selection(-dx_s, -dy_s);
    */

    if (flags & BGCOLOR)
        d->background_color (old_background);

    d->translate (-dx, -dy);
    d->discontinue_underline ();
}
}
; // end tag

////////////////////// <th> ////////////////////////////////
static const char * const _tag_name_th = "th";
class Html_Tag_th : public Html_Tag_td
{

    const char * name () const
    {
        return _tag_name_th;
    }

  public:
    void before_format (Fl_Html_Formatter * state)
    {
        state->x_align (ALIGN_CENTER);
        Fl_Html_Drawing_Device_ * d = state->drawing_device ();
        d->font (d->font_face () | Fl_Html_Drawing_Device_::BOLD,
                 d->font_size ());
    }

    void draw (Fl_Html_Drawing_Device_ * d)
    {
        int old_font = d->font_face ();
        int old_size = d->font_size ();
        d->font (old_font | Fl_Html_Drawing_Device_::BOLD, old_size);
        Html_Tag_td::draw (d);
        d->font (old_font, old_size);
    }

    static Fl_Xml_Object * create ()
    {
        return new Html_Tag_th ();
    }
};

////////////////////////////    <table>  ///////////////////////////////////

BUILD_TAG (
    table,
    0 /*((flags & (ALIGN_WRAP | ALIGN_INLINE))? 0:Fl_Html_Object::BLOCK)*/, 0)
int static_width;
int static_height;
int cellspacing;
int border;
int bordercolor;
int * col_positions_min;
int * col_positions_max;

int no_columns;
int no_rows;
int cellpadding;

int max_total_table_width;

// bool initialized;
int margin;
int bgcolor;
unsigned flags;
int height_percentages;
int rules;
char word_space;
enum
{
    FRAME_LEFT = 1,
    FRAME_RIGHT = 2,
    FRAME_TOP = 4,
    FRAME_BOTTOM = 8,
    FRAME_ALL = 0xF,
    RULES_ROWS = 16,
    RULES_COLUMNS = 32
};

Html_Tag_td ** tags;
int no_tags;
int * first_tag_in_row;
char * resizable_column;
/*
bool point_within(int X, int Y) const {
    return point_in_bounding_box(X, Y);
}
*/
int drawable () const
{
    return 8;
}

void export_start (Text_Buffer & text, int type) const
{
    int n = 2 - text.newlines ();
    for (int i = 0; i < n; i++)
        text.add_newline ();
}

void export_end (Text_Buffer & text, int type) const
{
    int n = 2 - text.newlines ();
    for (int i = 0; i < n; i++)
        text.add_newline ();
}

bool selection_bounding () const
{
    return true;
}

int minimum_width () const
{
    int clsp = (rules & 0xF0) ? 1 : cellspacing;
    return col_positions_min[no_columns - 1] + clsp + 2 * (margin + border);
}
int maximum_width () const
{
    ;
    int clsp = (rules & 0xF0) ? 1 : cellspacing;
    return col_positions_max[no_columns - 1] + clsp + 2 * (margin + border);
}

void min_width (int & width) const
{
    int w = minimum_width ();
    if (flags & WIDTH_PERCENT)
    {
        if (static_width > 0 && static_width <= 100)
            w = (w * 100 + static_width - 1) / static_width;
    }
    else if (static_width > w)
        w = static_width;
    if (w > width)
        width = w;
}

typedef int (Html_Tag_td::*Get_Position) () const;

void calculate_positions_top (int * positions) const
{
    int min_pos = 0;
    int clsp = (rules & 0xF0) ? 1 : cellspacing; // + 2 * border;
    for (int which = 0; which < no_rows; which++)
    {
        for (int j = 0; j <= which; j++)
        {
            Html_Tag_td ** a = tags + first_tag_in_row[j];
            Html_Tag_td ** limit;
            if ((j + 1) < no_rows)
                limit = tags + first_tag_in_row[j + 1];
            else
                limit = tags + no_tags;
            while (a != limit)
            {
                Html_Tag_td * t = *a;
                int tc = j + t->rowspan - 1;
                if (tc == which)
                {
                    int pos = t->min_height () + clsp;
                    if (j)
                        pos += positions[j - 1];
                    if (pos > min_pos)
                        min_pos = pos;
                }
                a++;
            }
        }
        positions[which] = min_pos;
    }
}

int calculate_positions_right (int * static_from_columns,
                               int * static_widths) const
{
    int min_pos1 = 0;
    int min_pos2 = 0;
    col_positions_min[no_columns - 1] = 0;
    col_positions_max[no_columns - 1] = 0;
    int clsp = (rules & 0xF0) ? 1 : cellspacing; // + 2 * border;
    for (int column = no_columns - 1; column >= 0; column--)
    {
        int from_col = column - 1;
        int min_pos3 = 0;
        int static_to =
            get_static_to (from_col, static_from_columns, no_columns);
        for (int j = 0; j < no_rows; j++)
        {
            Html_Tag_td ** a = tags + first_tag_in_row[j];
            Html_Tag_td ** limit;
            if ((j + 1) < no_rows)
                limit = tags + first_tag_in_row[j + 1];
            else
                limit = tags + no_tags;
            while (a != limit)
            {
                Html_Tag_td * t = *a;
                int col = t->column;
                if (col == column)
                {
                    int tc = col + t->colspan - 1;
                    int pos1 = -(t->get_minimum_w () + clsp);
                    int pos2 = -(t->get_maximum_w () + clsp);
                    if (tc < no_columns)
                    {
                        pos1 += col_positions_min[tc];
                        pos2 += col_positions_max[tc];
                    }
                    if (pos1 < min_pos1)
                        min_pos1 = pos1;
                    if (static_to != from_col && tc > static_to &&
                        pos2 < min_pos3)
                        min_pos3 = pos2;
                    if (t->is_static_width ())
                        break;
                    if (pos2 < min_pos2)
                        min_pos2 = pos2;
                    break;
                }
                a++;
            }
        }

        if (static_to != from_col)
        {
            get_static_end (from_col, static_from_columns, no_columns);
            int ss = col_positions_max[static_to];
            int w = static_widths[static_to];
            int xx = col_positions_min[static_to];
            int new_min = xx - w;
            if (new_min < min_pos1)
                min_pos1 = new_min;
            if (xx - min_pos1 > w)
                w = xx - min_pos1;
            int limit = ss - (xx - min_pos1);
            int diff = min_pos2;
            min_pos2 = ss - w;
            if (limit < min_pos2)
                min_pos2 = limit;
            if (min_pos3 < min_pos2)
                min_pos2 = min_pos3;
            diff -= min_pos2;
            if (static_to < no_columns - 1)
                col_positions_max[static_to] = min_pos2 + w;

            for (int i = static_to - 1; i > from_col; i--)
            {
                int start1 = col_positions_min[i + 1];
                int start2 = col_positions_max[i + 1];
                int limit = col_positions_min[i] - start1;
                int new_width = col_positions_max[i] - start2 - diff;
                if (new_width > limit)
                    new_width = limit;
                col_positions_max[i] = start2 + new_width;
            }
        }

        if (column)
        {
            col_positions_min[column - 1] = min_pos1;
            col_positions_max[column - 1] = min_pos2;
        }
    }
    for (int i = 0; i < no_columns; i++)
        col_positions_min[i] -= min_pos1;

    for (int i = 0; i < no_columns; i++)
        col_positions_max[i] -= min_pos2;
    return col_positions_max[no_columns - 1];
}

void get_static_widths (int * from_column, int * width, bool percent = false,
                        bool is_rows = false) const
{
    int clsp = (rules & 0xF0) ? 1 : cellspacing; // + 2 * border;
    int no_sets = (is_rows) ? no_rows : no_columns;
    for (int i = 0; i < no_sets; i++)
        from_column[i] = -2;

    for (int j = 0; j < no_rows; j++)
    {
        Html_Tag_td ** a = tags + first_tag_in_row[j];
        Html_Tag_td ** limit;
        if ((j + 1) < no_rows)
            limit = tags + first_tag_in_row[j + 1];
        else
            limit = tags + no_tags;
        while (a != limit)
        {
            Html_Tag_td * t = *a;
            // for(int i = 0; i<no_tags; i++) {
            //  Html_Tag_td * t = tags[i];
            if (is_rows)
            {
                if (!t->is_static_height (percent))
                {
                    a++;
                    continue;
                }
            }
            else if (!t->is_static_width (percent))
            {
                a++;
                continue;
            }
            int colspan = (is_rows) ? t->rowspan : t->colspan;
            int from = (is_rows) ? j - 1 : t->column - 1;
            int to = from + colspan;

            int w = 0;
            if (is_rows)
                w = (percent) ? t->height : t->h + clsp;
            else
                w = (percent) ? t->width : t->get_maximum_w () + clsp;
            for (int i = from + 1; i < to; i++)
            {
                if (from_column[i] == from)
                {
                    from = i;
                    w -= width[i];
                }
            }
            if (w <= 0)
                w = 0; // security for badly written html
            if (from_column[to] > -1)
            {
                if (from_column[to] == from)
                { // possibly replacing old width
                    if (width[to] < w)
                        width[to] = w;
                    return;
                }
                if (from_column[to] < from)
                {
                    int x = width[to] - w;
                    if (x < 0)
                        x = 0;
                    width[from] = x;
                    from_column[from] = from_column[to];
                    from_column[to] = from;
                }
            }
            for (int i = to + 1; i < no_sets; i++)
            {
                if (from_column[i] == from)
                {
                    int x = width[i] - w;
                    if (x < 0)
                        x = 0;
                    width[i] = x;
                    from_column[i] = to;
                    break;
                }
            }
            from_column[to] = from;
            width[to] = w;

            a++;
        }
    }
}

static int width_from_percentages (int no_columns,
                                   int * percentages_from_columns,
                                   int * percentages_widths, int * positions,
                                   int & width, int & nonpercentage_width,
                                   int & new_nonpercentages,
                                   int & no_nonpercentages)
{
    int ww = 0;
    int tot_percentages = 0;
    int tot_nonpercentages = 0;
    no_nonpercentages = 0;
    for (int i = no_columns - 1; i >= 0; i--)
    {
        int from = percentages_from_columns[i];
        int start = 0;
        if (from > -2)
        {
            if (from >= 0)
                start = positions[from];
            int pp = percentages_widths[i];
            tot_percentages += pp;
            int x = (positions[i] - start);
            if (pp > 0)
                x = (x * 100 + pp - 1) / pp;
            if (ww < x)
                ww = x;
            i = from + 1;
        }
        else
        { // nonpercentages
            no_nonpercentages++;
            if (i > 0)
                start = positions[i - 1];
            tot_nonpercentages += positions[i] - start;
        }
    }
    new_nonpercentages = nonpercentage_width = tot_nonpercentages;

    if (tot_nonpercentages > 0)
    {
        int ntp = 100 - tot_percentages;
        if (ntp < 5)
            ntp = 5; // we give for non-percentage columns at least 5 %
        int x = (tot_nonpercentages * 100 + ntp - 1) / ntp;
        new_nonpercentages = x;
        if (ww < x)
            ww = x;
        tot_percentages += ntp; // it can be over 100 % if html is badly written
        if (tot_percentages > 0)
        {
            int nwnp = (ww * ntp + tot_percentages - 1) / tot_percentages;
            new_nonpercentages = nwnp;
        }
    }
    width = ww;
    return tot_percentages;
}

static void calculate_widths_percentages (int no_columns,
                                          int * percentages_from_columns,
                                          int * percentages_widths,
                                          int * min_positions,
                                          int * max_positions)
{
    int width1 = 0;
    int nonpercentage_width1 = 0;
    int new_nonpercentages1 = 0;
    int no_nonpercentages = 0;
    width_from_percentages (no_columns,
                            percentages_from_columns,
                            percentages_widths,
                            min_positions,
                            width1,
                            nonpercentage_width1,
                            new_nonpercentages1,
                            no_nonpercentages);

    int width2 = 0;
    int nonpercentage_width2 = 0;
    int new_nonpercentages2 = 0;
    if (max_positions)
        width_from_percentages (no_columns,
                                percentages_from_columns,
                                percentages_widths,
                                max_positions,
                                width2,
                                nonpercentage_width2,
                                new_nonpercentages2,
                                no_nonpercentages);

    transform_positions_to_sizes (min_positions, no_columns);
    if (max_positions)
        transform_positions_to_sizes (max_positions, no_columns);

    int static_norm = 0;

    for (int i = no_columns - 1; i >= 0; i--)
    {
        int from = percentages_from_columns[i];
        if (from > -2)
        {
            int norm = 0;
            int w1 = 0;
            int w2 = 0;
            for (int j = from + 1; j <= i; j++)
            {
                w1 += min_positions[j];
                if (max_positions)
                {
                    w2 += max_positions[j];
                    norm += max_positions[j] - min_positions[j];
                }
            }
            int new_w1 = w1;
            int new_w2 = w2;
            int pp = percentages_widths[i];
            if (pp > 0)
            {
                new_w1 = (width1 * pp + 99) / 100;
                new_w2 = (width2 * pp + 99) / 100;
                int diff1 = new_w1 - w1;
                if (diff1 < 0)
                    diff1 = 0;
                int diff2 = new_w2 - w2;
                if (diff2 < 0)
                    diff2 = 0;
                if (norm > 0)
                {
                    for (int j = from + 1; j <= i; j++)
                    {
                        min_positions[j] +=
                            (diff1 * (max_positions[j] - min_positions[j]) +
                             norm - 1) /
                            norm;
                        max_positions[j] +=
                            (diff2 * (max_positions[j] - min_positions[j]) +
                             norm - 1) /
                            norm;
                    }
                }
                else
                {

                    if (w2)
                    {
                        for (int j = from + 1; j <= i; j++)
                        {
                            min_positions[j] +=
                                (diff1 * max_positions[j] + w2 - 1) / w2;
                            max_positions[j] +=
                                (diff2 * max_positions[j] + w2 - 1) / w2;
                        }
                    }
                    else if (w1)
                    {
                        for (int j = from + 1; j <= i; j++)
                            min_positions[j] +=
                                (diff1 * min_positions[j] + w1 - 1) / w1;
                    }
                }
            }
            i = from + 1;
        }
        else if (max_positions)
        {
            int dd = max_positions[i] - min_positions[i];
            if (dd > 0)
                static_norm += dd;
        }
    }
    int static_diff = new_nonpercentages1 - nonpercentage_width1;
    if (static_diff > 0)
    {
        if (static_norm > 0)
        {
            for (int i = no_columns - 1; i >= 0; i--)
            {
                int from = percentages_from_columns[i];
                if (from > -2)
                    i = from + 1;
                else
                    min_positions[i] +=
                        (static_diff * (max_positions[i] - min_positions[i]) +
                         static_norm - 1) /
                        static_norm;
            }
        }
        else
        {
            for (int i = no_columns - 1; i >= 0; i--)
            {
                int from = percentages_from_columns[i];
                if (from > -2)
                    i = from + 1;
                else if (max_positions)
                    min_positions[i] += (static_diff * max_positions[i] +
                                         nonpercentage_width2 - 1) /
                                        nonpercentage_width2;
                else
                    min_positions[i] += (static_diff * min_positions[i] +
                                         nonpercentage_width1 - 1) /
                                        nonpercentage_width1;
            }
        }
        if (max_positions && no_nonpercentages)
        {
            static_diff = new_nonpercentages2 - nonpercentage_width2;
            for (int i = no_columns - 1; i >= 0; i--)
            {
                int from = percentages_from_columns[i];
                if (from > -2)
                    i = from + 1;
                else
                    max_positions[i] += (static_diff + no_nonpercentages - 1) /
                                        no_nonpercentages;
            }
        }
    }

    transform_sizes_to_positions (min_positions, no_columns);
    if (max_positions)
        transform_sizes_to_positions (max_positions, no_columns);
}

static int get_static_start (int f, const int * from_columns)
{
    if (f < 0)
        return f;
    while (1)
    {
        int f2 = from_columns[f];
        if (f2 < -1)
            return f;
        f = f2;
        if (f < 0)
            return f;
    }
    return f;
}

static int get_static_end (int col, const int * from_columns, int no_columns)
{
    for (int i = col + 1; i < no_columns; i++)
        if (from_columns[i] == col)
            col = i;
    return col;
}

static int get_static_to (int f, const int * from_columns, int no_columns)
{
    for (int i = f + 1; i < no_columns; i++)
    {
        if (from_columns[i] == f)
            return i;
    }
    return f;
}

int calculate_positions_left (int * static_from_columns,
                              int * static_widths) const
{
    int min_pos1 = 0;
    int min_pos2 = 0;
    int clsp = (rules & 0xF0) ? 1 : cellspacing; // + 2 * border;
    for (int which = 0; which < no_columns; which++)
    {
        int static_from = static_from_columns[which];
        int min_pos3 = 0;
        for (int j = 0; j < no_rows; j++)
        {
            Html_Tag_td ** a = tags + first_tag_in_row[j];
            Html_Tag_td ** limit;
            if ((j + 1) < no_rows)
                limit = tags + first_tag_in_row[j + 1];
            else
                limit = tags + no_tags;
            while (a != limit)
            {
                Html_Tag_td * t = *a;
                int col = t->column;
                int tc = col + t->colspan - 1;
                if (tc == which)
                {
                    int pos1 = t->get_minimum_w () + clsp;
                    int pos2 = t->get_maximum_w () + clsp;
                    if (col)
                    {
                        pos1 += col_positions_min[col - 1];
                        pos2 += col_positions_max[col - 1];
                    }
                    if (pos1 > min_pos1)
                        min_pos1 = pos1;
                    if (col - 1 < static_from && pos2 > min_pos3)
                        min_pos3 = pos2;
                    if (t->is_static_width ())
                        break;
                    if (pos2 > min_pos2)
                        min_pos2 = pos2;
                    break;
                }
                else if (tc > which)
                    break;
                a++;
            }
        }
        if (static_from > -2)
        {
            get_static_start (which, static_from_columns);
            int ss = 0;
            if (static_from >= 0)
                ss = col_positions_max[static_from];
            int w = static_widths[which];
            int xx = 0;
            if (static_from >= 0)
                xx = col_positions_min[static_from];
            int new_min = xx + w;
            if (new_min > min_pos1)
                min_pos1 = new_min;
            if (min_pos1 - xx > w)
                w = min_pos1 - xx;
            int limit = ss + (min_pos1 - xx);
            int diff = 0;
            diff = min_pos2;
            min_pos2 = ss + w;
            if (limit > min_pos2)
                min_pos2 = limit;
            if (min_pos3 > min_pos2)
                min_pos2 = min_pos3;
            diff -= min_pos2;
            if (static_from >= 0)
                col_positions_max[static_from] = min_pos2 - w;
            for (int i = static_from + 1; i < which; i++)
            {
                int start1 = 0;
                int start2 = 0;
                if (i > 0)
                {
                    start1 = col_positions_min[i - 1];
                    start2 = col_positions_max[i - 1];
                }
                int limit = col_positions_min[i] - start1;
                int new_width = col_positions_max[i] - start2 - diff;
                if (new_width < limit)
                    new_width = limit;
                col_positions_max[i] = start2 + new_width;
            }
        }
        col_positions_min[which] = min_pos1;
        col_positions_max[which] = min_pos2;
    }
    return col_positions_max[no_columns - 1];
}

void init_format (Fl_Html_Formatter * s)
{
    // if(!initialized) {
    //    initialized = true;
    Fl_Html_Table_Formatter * t_formatter = s->table_formatter ();
    RESET_HTML_DRAWING (s->drawing_device ());
    t_formatter->reset ();
    // void * old_formatter = s->aux_object(Fl_Html_Formatter::TABLE_FORMATTER);
    s->aux_object (Fl_Html_Formatter::TABLE_FORMATTER, t_formatter);
    void * old_table = s->aux_object (Fl_Html_Formatter::TABLE);
    s->aux_object (Fl_Html_Formatter::TABLE, this);
    init_format_children (s);
    s->aux_object (Fl_Html_Formatter::TABLE, old_table);
    s->aux_object (Fl_Html_Formatter::TABLE_FORMATTER, old_table);
    t_formatter->copy (tags, first_tag_in_row, no_tags, no_columns, no_rows);
    col_positions_min = new int[no_columns];
    col_positions_max = new int[no_columns];
    //    col_measurement_positions =  new int[no_columns];
    resizable_column = new char[no_columns];
    for (int i = 0; i < no_columns; i++)
        resizable_column[i] = 1;

    int * static_from_columns = new int[no_columns];
    int * static_widths = new int[no_columns];
    get_static_widths (static_from_columns, static_widths);
    int clsp = (rules & 0xF0) ? 1 : cellspacing;
    max_total_table_width =
        calculate_positions_left (static_from_columns, static_widths) +
        2 * (border + margin) + clsp;
    // max_total_table_width = calculate_positions_right(static_from_columns,
    // static_widths) + 2 * border + cellspacing;
    if (static_width)
    {
        max_total_table_width = minimum_width ();
        if (static_width > max_total_table_width)
            max_total_table_width = static_width;
    }
    int * c1 = col_positions_min;
    int * c2 = col_positions_max;
    col_positions_min = new int[no_columns];
    col_positions_max = new int[no_columns];
    calculate_positions_right (static_from_columns, static_widths);
    for (int i = 0; i < no_columns; i++)
    {
        col_positions_min[i] = (col_positions_min[i] + c1[i] + 1) / 2;
        col_positions_max[i] = (col_positions_max[i] + c2[i] + 1) / 2;
    }
    height_percentages = t_formatter->percentages () & HEIGHT_PERCENT;
    if (t_formatter->percentages () & WIDTH_PERCENT)
    {
        int * percentages_from_columns = new int[no_columns];
        int * percentages_widths = new int[no_columns];
        get_static_widths (percentages_from_columns, percentages_widths, true);
        calculate_widths_percentages (no_columns,
                                      percentages_from_columns,
                                      percentages_widths,
                                      col_positions_min,
                                      col_positions_max);
        if (!static_width)
            max_total_table_width = col_positions_max[no_columns - 1] +
                                    2 * (border + margin) + clsp;
        else
        {
            max_total_table_width = minimum_width ();
            if (static_width > max_total_table_width)
                max_total_table_width = static_width;
        }
        delete[] percentages_from_columns;
        delete[] percentages_widths;
    }
    delete[] c1;
    delete[] c2;
    delete[] static_from_columns;
    delete[] static_widths;
    // }
}

static void transform_positions_to_sizes (int * pos, int no)
{
    for (int i = no - 1; i; i--)
        pos[i] -= pos[i - 1];
}

void transform_column_positions_to_sizes () const
{
    transform_positions_to_sizes (col_positions_max, no_columns);
    transform_positions_to_sizes (col_positions_min, no_columns);
}

static void transform_sizes_to_positions (int * pos, int no)
{
    for (int i = 1; i < no; i++)
        pos[i] += pos[i - 1];
}

void transform_column_sizes_to_positions () const
{
    transform_sizes_to_positions (col_positions_max, no_columns);
    transform_sizes_to_positions (col_positions_min, no_columns);
}

static int set_static_to (int * static_columns, int which, int colspan)
{
    int i2 = static_columns[which];
    int i1 = which - colspan;
    if (i1 >= 0)
        i1 = static_columns[i1];
    if (i2 < i1)
    {
        int i = i2;
        i2 = i1;
        i1 = i;
    }
    else
        static_columns[which] = i1;
    for (int i = 0; i < which; i++)
        if (static_columns[i] == i2)
            static_columns[i] = i1;
    return i1;
}

static int set_static_to_right (int * static_columns, int no_columns, int which,
                                int colspan)
{
    int i1 = static_columns[which];
    int i2 = which + colspan;
    if (i2 < no_columns)
        i2 = static_columns[i2];
    if (i2 > i1)
    {
        static_columns[which] = i2;
    }
    else
    {
        int i = i2;
        i2 = i1;
        i1 = i;
    }
    for (int i = which + 1; i < no_columns - 1; i++)
        if (static_columns[i] == i1)
            static_columns[i] = i2;
    return i2;
}

void format (Fl_Html_Formatter * s)
{
    if (!no_columns)
        return;
    if (s->no_line_objects () &&
        s->available_width () < max_total_table_width &&
        !(flags & ALIGN_INLINE) && (flags & ALIGN_WRAP))
    {
        s->add_todo_format_object (
            this); // schedullingg for fornat after line finish
        return;
    }

    Fl_Html_Object * old_bullet = s->bullet ();

    if (/*(!(flags & (ALIGN_WRAP|ALIGN_INLINE))) || */ s->available_width () <
        max_total_table_width)
        s->finish_last_line ();

    RESET_HTML_DRAWING (s->drawing_device ());
    int mw = minimum_width ();
    int base = s->base_width ();
    if (WIDTH_PERCENT & flags)
    {

        int nc = 0;
        if (base < (HIGH_VAL / 2))
            nc = (s->base_width () * static_width) / 100;
        else
            nc = maximum_width ();
        if (nc > mw)
            mw = nc;
    }
    else if (static_width > mw)
        mw = static_width;

    int available_width =
        s->find_place (mw, (!(flags & ALIGN_INLINE)) && !(flags & ALIGN_WRAP));
    int new_width = mw;
    if (!static_width)
    {
        if (available_width >= max_total_table_width)
        {
            new_width = max_total_table_width;
        }
        else if (available_width > mw) // calculating new widths
            new_width = available_width;
    }
    if ((WIDTH_PERCENT & flags) && (base > HIGH_VAL / 2))
    {
    }

    //  if(new_width!= w) { // new width, must do formatting
    int clsp = (rules & 0xF0) ? 1 : cellspacing; // + 2 * border;
    w = new_width;

    // calculating new widths

    float * cur_col_positions = new float[no_columns];
    float coef = float (w - minimum_width ());
    int denom =
        col_positions_max[no_columns - 1] - col_positions_min[no_columns - 1];

    if (denom <= 0)
    {
        denom = col_positions_max[no_columns - 1];
        coef /= denom;
        for (int i = 0; i < no_columns; i++)
            cur_col_positions[i] =
                col_positions_min[i] + (coef * col_positions_max[i]);
    }
    else
    {
        coef /= denom;
        for (int i = 0; i < no_columns; i++)
            cur_col_positions[i] =
                col_positions_min[i] +
                (col_positions_max[i] - col_positions_min[i]) * coef;
    }
    void * old_table = s->aux_object (Fl_Html_Formatter::TABLE);
    s->aux_object (Fl_Html_Formatter::TABLE, this);
    for (int i = 0; i < no_rows; i++)
    {
        Html_Tag_td ** a = tags + first_tag_in_row[i];
        Html_Tag_td ** limit;
        if ((i + 1) < no_rows)
            limit = tags + first_tag_in_row[i + 1];
        else
            limit = tags + no_tags;
        while (a != limit)
        {
            Html_Tag_td * t = *a;
            int column = t->column;
            int x = 0;
            float fx = 0;
            if (column)
            {
                fx = cur_col_positions[column - 1];
                x = (int)fx;
            }
            t->w = cur_col_positions[column + t->colspan - 1] - fx - clsp;
            t->format_ (s);
            t->w = cur_col_positions[column + t->colspan - 1] - x - clsp;
            a++;
        }
    }

    // for(int i = 0; i<no_tags; i++)
    // tags[i]->format_(s);
    // format_children(s);
    s->aux_object (Fl_Html_Formatter::TABLE, old_table);
    int * cur_row_positions = new int[no_rows];
    calculate_positions_top (cur_row_positions);

    if (height_percentages)
    {

        int * percentages_from_rows = new int[no_rows];
        int * percentages_rows = new int[no_rows];
        get_static_widths (percentages_from_rows, percentages_rows, true, true);
        calculate_widths_percentages (no_rows,
                                      percentages_from_rows,
                                      percentages_rows,
                                      cur_row_positions,
                                      0);
        delete[] percentages_from_rows;
        delete[] percentages_rows;
    }

    if (static_height && static_height > cur_row_positions[no_rows - 1] + clsp)
    {
        int norm = cur_row_positions[no_rows - 1];
        if (norm > 0)
        {
            int diff = static_height - clsp;
            for (int i = 0; i < no_rows; i++)
                cur_row_positions[i] = (cur_row_positions[i] * diff) / norm;
        }
    }

    h = cur_row_positions[no_rows - 1] + clsp + 2 * (margin + border);
    // now we know iverall size, we need to place cells relative to this table
    // and adjust the height of the cells
    for (int i = 0; i < no_rows; i++)
    {
        Html_Tag_td ** a = tags + first_tag_in_row[i];
        Html_Tag_td ** limit;
        if ((i + 1) < no_rows)
            limit = tags + first_tag_in_row[i + 1];
        else
            limit = tags + no_tags;
        int y = clsp + margin + border;
        int h = -clsp; // +cur_row_positions[i] ;
        if (i)
        {
            y += cur_row_positions[i - 1];
            h -= cur_row_positions[i - 1];
        }
        while (a != limit)
        {
            Html_Tag_td * t = *a;
            float x = clsp + margin + border;
            int column = t->column;
            if (column)
                x += cur_col_positions[column - 1];
            t->x = x;
            t->y = y;
            int height_index = i + t->rowspan - 1;
            if (height_index >= no_rows)
                height_index = no_rows - 1;
            t->h = h + cur_row_positions[height_index];
            a++;
        }
    }
    delete[] cur_row_positions;
    delete[] cur_col_positions;
    //  }
    if ((WIDTH_PERCENT & flags) && (base > HIGH_VAL / 2))
        s->minimum_width ((w * 100 + static_width - 1) / static_width);
    if ((HEIGHT_PERCENT & flags))
        s->minimum_bottom ((h * 100 + static_height - 1) / static_height);

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
            s->place_right (this, flags & ALIGN_WRAP);
            break;
        case ALIGN_CENTER:
            s->place_center (this);
            break;
        default:
            if (!(flags & ALIGN_WRAP) &&
                (s->x_align () == Fl_Html_Formatter::CENTER))
                s->place_center (this);
            else
                s->place_left (this, flags & ALIGN_WRAP);
            break;
        }
    }
    format_children (s);
}

void translate_for_children (int & dx, int & dy)
{
    dx = x;
    dy = y;
}

void in_selection_children (int * selection, int & first, int & last)
{
    int dx, dy;
    Html_Tag_table::translate_for_children (dx, dy);
    selection[0] -= dx;
    selection[2] -= dx;
    selection[1] -= dy;
    selection[3] -= dy;
    Fl_Html_Object::in_selection_children (selection, first, last);
    selection[0] += dx;
    selection[2] += dx;
    selection[1] += dy;
    selection[3] += dy;
}

void draw (Fl_Html_Drawing_Device_ * d)
{
    RESET_HTML_DRAWING (d);
    if (flags & BGCOLOR)
    {
        int old = d->color ();
        d->color (bgcolor);
        d->rectf (x + margin, y + margin, w - 2 * margin, h - 2 * margin);
        d->color (old);
    }

    for (int i = 0; i < no_tags; i++)
    {
        int old_color = d->color ();
        if (tags[i]->flags & BGCOLOR)
        {
            d->color (tags[i]->bgcolor);
            d->rectf (
                x + tags[i]->x, y + tags[i]->y, tags[i]->w, tags[i]->h, true);
        }
        d->color (old_color);
    }
    float z = d->zoom ();
    int dx = int (x * z + 0.5);
    int dy = int (y * z + 0.5);
    /*
    bool sw = d->selection_valid();
    if(sw)
      d->translate_selection(x, y);
    */
    d->translate (dx, dy);
    draw_children (d /*, false */);

    d->translate (-dx, -dy);
    /*
    if(sw)
      d->translate_selection(-x, -y);
    */
    // if(rules)
    if (border || rules)
    {
        int old = d->color ();
        d->color (bordercolor);
        /*
           if((rules & FRAME_ALL) == FRAME_ALL || border)
             d->rect(x+margin, y + margin, w - 2*margin, h - 2*margin);
           else
       */

        if (rules & FRAME_ALL)
        {
            int clsp = (rules & 0xF0) ? 1 : 0;
            clsp += margin;

            if (rules & FRAME_TOP)
                d->rectf (x + clsp, y + clsp, w - 2 * clsp, border);
            if (rules & FRAME_BOTTOM)
                d->rectf (
                    x + clsp, y + h - clsp - border, w - 2 * clsp, border);
            if (rules & FRAME_LEFT)
                d->rectf (x + clsp, y + clsp, border, h - 2 * clsp);
            if (rules & FRAME_RIGHT)
                d->rectf (
                    x + w - clsp - border, y + clsp, border, h - 2 * clsp);
        }
        if (border && !(rules & 0xF0))
        {
            for (int i = 0; i < no_tags; i++)
                d->rect (x + tags[i]->x - 1,
                         y + tags[i]->y - 1,
                         tags[i]->w + 2,
                         tags[i]->h + 2);
        }
        else if (rules & 0xF0)
        {
            if (rules & RULES_ROWS)
            {
                for (int j = 0; j < no_rows; j++)
                {
                    int last_in_row =
                        (j + 1 < no_rows) ? first_tag_in_row[j + 1] : no_tags;
                    for (int i = first_tag_in_row[j]; i < last_in_row; i++)
                    {
                        Html_Tag_td * t = tags[i];
                        if (j + t->rowspan < no_rows)
                            // d->line(x + t->x - 1, y + t->y  + t->h, t->w +1);
                            d->rectf (
                                x + t->x - 1, y + t->y + t->h, t->w + 2, 1);
                    }
                }
            }
            if (rules & RULES_COLUMNS)
            {
                for (int i = 0; i < no_tags; i++)
                {
                    Html_Tag_td * t = tags[i];
                    if (t->column + t->colspan < no_columns)
                        // d->vline(x + t->x + t->w, y + t->y - 1, t->h +1);
                        d->rectf (x + t->x + t->w, y + t->y - 1, 1, t->h + 2);
                }
            }
        }
        d->color (old);
    }
}

void finish (Fl_Xml_Parser * p)
{
    ((Fl_Html_Parser *)p)->last_word_space (&word_space);
}

Html_Tag_table ()
{
    cellspacing = 3;
    border = 0;
    bordercolor = 0;
    col_positions_min = 0;
    col_positions_max = 0;
    w = 0;
    //  initialized = false;
    no_tags = 0;
    tags = 0;
    first_tag_in_row = 0;
    static_width = 0;
    static_height = 0;
    resizable_column = 0;
    cellpadding = 1; // this seems to be default for most browsers
    margin = 0;
    flags = 0;
    height_percentages = 0;
    rules = 0;
    word_space = 0;
}
~Html_Tag_table ()
{
    delete[] col_positions_min;
    delete[] col_positions_max;
    delete[] tags;
    delete[] first_tag_in_row;
    delete[] resizable_column;
}

void process_style (const Fl_Css_Parser::Property & p)
{
    if (is_word (p.property (), "margin", p.property_length ()))
    {
        if (!p.no_values ())
            return;
        long long bb = 0;
        Fl_Html_Tokenizer::int_value ((p.values ())[0],
                                      bb,
                                      Fl_Html_Tokenizer::FORMAT_10,
                                      (p.values_lengths ())[0]);
        if (bb >= 0)
            margin = bb;
        return;
    }
    if (is_word (p.property (), "display", p.property_length ()))
    {
        if (!p.no_values ())
            return;
        if (Fl_Html_Object_::is_first_word (
                (p.values ())[0], "inline-table", (p.values_lengths ())[0]))
            flags |= ALIGN_INLINE;
        else if (Fl_Html_Object_::is_first_word (
                     (p.values ())[0], "inline", (p.values_lengths ())[0]))
            flags |= ALIGN_INLINE;
        return;
    }
    if (is_word (p.property (), "vertical-align", p.property_length ()))
    {
        if (!p.no_values ())
            return;
        read_valign (flags, (p.values ())[0], (p.values_lengths ())[0]);
        if ((flags & VALIGN_MASK) == VALIGN_BASELINE)
            flags &= ~VALIGN_MASK; // baseline not yet supported
        return;
    }
}

void process_attribute (Fl_Xml_Parser * p, const char * name, int name_len,
                        const char * value, int value_len)
{
    if (read_align (flags, name, name_len, value, value_len))
    {
        unsigned a = flags & ALIGN_MASK;
        if (a == ALIGN_CENTER || a == ALIGN_JUSTIFY)
            flags &= ~ALIGN_WRAP;
        else
            flags |= ALIGN_WRAP;
        return;
    }
    if (is_word (name, "width", name_len))
    {
        bool percents = 0;
        if (read_with_percents (value, static_width, percents, value_len))
        {
            if (static_width < 0)
                static_width = 0; // make it non-negative
            if (percents)
            {
                flags |= WIDTH_PERCENT;
                if (!static_width)
                    static_width = 1; // avoid zero-division later
            }
            else
                flags &= ~WIDTH_PERCENT;
        }
        return;
    }
    if (is_word (name, "height", name_len))
    {
        bool percents = 0;
        if (read_with_percents (value, static_height, percents, value_len))
        {
            if (static_height < 0)
                static_height = 0; // make it non-negative
            if (percents)
            {
                flags |= HEIGHT_PERCENT;
                if (!static_height)
                    static_height = 1; // avoid zero-division later
            }
            else
                flags &= ~HEIGHT_PERCENT;
        }
        return;
    }
    if (is_word (name, "border", name_len))
    {
        long long bb = 1;
        Fl_Html_Tokenizer::int_value (
            value, bb, Fl_Html_Tokenizer::FORMAT_10, value_len);
        border = bb;
        if (border)
            rules |= FRAME_ALL;
        else
            rules &= ~FRAME_ALL;
        // printf("border: %i\n", border);
        return;
    }
    if (is_word (name, "cellspacing", name_len))
    {
        int old = cellspacing;
        long long new_val = old;
        if (Fl_Html_Tokenizer::int_value (
                value, new_val, Fl_Html_Tokenizer::FORMAT_10, value_len))
        {
            if (new_val < -1)
                cellspacing = old;
            else
                cellspacing = new_val;
        }
        return;
    }

    if (is_word (name, "cellpadding", name_len))
    {
        int old = cellpadding;
        long long new_val = old;
        if (Fl_Html_Tokenizer::int_value (
                value, new_val, Fl_Html_Tokenizer::FORMAT_10, value_len))
        {
            if (new_val < -1)
                cellpadding = old;
            else
                cellpadding = new_val;
        }
        return;
    }
    if (is_word (name, "bgcolor", name_len))
    {
        if (read_color (bgcolor, value, value_len))
            flags |= BGCOLOR;
        return;
    }
    if (is_word (name, "bordercolor", name_len))
    {
        if (read_color (bordercolor, value, value_len))
            return;
    }
    if (is_word (name, "rules", name_len))
    {
        if (Fl_Html_Object_::is_first_word (value, "all", value_len))
            rules |= RULES_COLUMNS | RULES_ROWS;
        else if (Fl_Html_Object_::is_first_word (value, "columns", value_len))
            rules |= RULES_COLUMNS;
        else if (Fl_Html_Object_::is_first_word (value, "rows", value_len))
            rules |= RULES_ROWS;
        return;
    }

    if (handle_style (name, name_len, value, value_len))
        return;
}
}
;

static int cellpadding (const Html_Tag_table * t)
{
    return t->cellpadding;
}
