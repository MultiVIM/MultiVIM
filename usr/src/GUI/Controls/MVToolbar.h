#include <string>

#include <FL/Enumerations.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>

class MVToolbarSpacer : public Fl_Box
{
  public:
    MVToolbarSpacer (int x, int y, int w, int h, const char * l = NULL)
        : Fl_Box (x, y, w, h, l)
    {
        labeltype (FL_NO_LABEL);
        box (FL_THIN_DOWN_FRAME);
    }
};

class MVToolbar : public Fl_Group
{
    // std::vector<Fl_Widget *> childs;

    int curPos, spacing, spacerMargin;

    int nextXPos ()
    {
        int old = curPos;
        curPos += h () - 4 + spacing;
        return old;
    }
    int nextXPosForSpacer ()
    {
        int old = curPos;
        curPos += 2 + spacerMargin * 2;
        return old + spacerMargin;
    }

  public:
    MVToolbar (int x, int y, int w, int h) : Fl_Group (x, y, w, h)
    {
        type (Fl_Pack::HORIZONTAL);
        box (FL_EMBOSSED_BOX);
        spacing = 0;
        spacerMargin = spacing + 4;
        curPos = 3;
        end ();
    }

    void addButton (std::string name, std::string tooltip,
                    Fl_Callback * cb = nullptr, void * userData = nullptr)
    {
        addButton (
            tooltip.c_str (),
            new Fl_BMP_Image (("/ws/MultiVIM/usr/src/Resources/Icons/Toolbar/" +
                               name + ".bmp")
                                  .c_str ()),
            cb,
            userData);
    }

    void addButton (const char * name, Fl_Image * image = nullptr,
                    Fl_Callback * callBack = nullptr, void * userData = nullptr)
    {
        Fl_Button * bt;

        bt = new Fl_Button (nextXPos (), y () + 2, h () - 4, h () - 4);
        bt->tooltip (name);
        if (image)
            bt->image (image);
        if (callBack != nullptr)
            bt->callback (callBack, userData);
        bt->clear_visible_focus ();
        // bt->redraw ();
        bt->box (FL_UP_BOX);
        add (bt);
        resize (x (), y (), w (), h ());
    }

    void resize (int x, int y, int w, int h)
    {
        Fl_Widget::resize (x, y, w, h);
    }

    void addSpacer ()
    {
        add (new MVToolbarSpacer (nextXPosForSpacer (), y () + 4, 2, h () - 8));
    }
};