#include <FL/Fl_Group.H>
#include <string>

#include "LGPL/Controls/Fl_Drag_Tabs.H"

class TabContainer : public Fl_Group
{
    const int tabHeight = 24;
    Fl_Drag_Tabs tabs;

  public:
    TabContainer (int x, int y, int w, int h, const char * l = NULL)
        : Fl_Group (x, y, w, h, l), tabs (x, y, w, tabHeight)
    {
        tabs.deletable (true);
        tabs.end ();
        this->add (tabs);
        end ();
        printf ("Created at %d@%d\n", x, y);
    }

    void addTab (Fl_Widget * tab)
    {
        tab->resize (x (), y () + tabHeight, w (), h () - tabHeight);
        this->add (tab);
        tabs.add (tab);
    }

    void resize (int x, int y, int w, int h)
    {
        tabs.resize (x, y, w, tabHeight);
        printf ("RESIZED at %d@%d\n", x, y);

        for (int i = 0; i < tabs.children (); i++)
        {
            Fl_Widget * ch = tabs.child (i);
            ch->resize (x, y + tabHeight, w, h - tabHeight);
        }
        Fl_Widget::resize (x, y, w, h);
    }

    int handle (int event)
    {
        return tabs.handle (event);
    }
};
