#include <string>

#include <FL/Enumerations.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>

class StatusbarSpacer : public Fl_Box
{
  public:
    StatusbarSpacer (int x, int y, int w, int h, const char * l = NULL)
        : Fl_Box (x, y, w, h, l)
    {
        labeltype (FL_NO_LABEL);
        box (FL_THIN_DOWN_FRAME);
    }
};

class StatusbarEntry : public Fl_Box
{
    const char * _default_label;

  public:
    StatusbarEntry (int x, int y, int w, int h, const char * l)
        : Fl_Box (x, y, w, h, l)
    {
        box (FL_THIN_DOWN_BOX);
        align (FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM_LEFT);
    }
};

class Statusbar : public Fl_Group
{
    int curPos, spacing, spacerMargin;

    int nextXPos (int entryWidth)
    {
        int old = curPos;
        curPos += entryWidth + spacing;
        return old;
    }

  public:
    Statusbar (int x, int y, int w, int h) : Fl_Group (x, y, w, h)
    {
        type (Fl_Pack::HORIZONTAL);
        box (FL_DOWN_BOX);
        labeltype (FL_NO_LABEL);
        spacing = 4;
        curPos = 3;
        end ();
    }

    void addEntry (StatusbarEntry * entry)
    {
        int eW = 0, eH = 0;
        entry->measure_label (eW, eH);
        eW += 16;
        entry->resize (nextXPos (eW), y () + 3, eW, h () - 5);
        add (entry);
    }

    void resize (int x, int y, int w, int h)
    {
        Fl_Widget::resize (x, y, w, h);
        for (int i = 0; i < children (); i++)
        {
            Fl_Widget * ch = child (i);
            ch->resize (ch->x (), y + 3, ch->w (), ch->h ());
        }
    }
};