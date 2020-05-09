// Copyright 2016 - 2019 gnuwimp@gmail.com
// Released under the GNU Lesser General Public License (LGPL)

#ifndef FLW_SPLIT_H
#define FLW_SPLIT_H

#include <FL/Fl_Group.H>
#include <stdlib.h>

//|--------------------------- FLW_SPLIT_TYPE
//| FLW_SPLIT_TYPE_HORIZONTAL => Resize widgets in a horizontal row
//| FLW_SPLIT_TYPE_VERTICAL   => Resize widget in a vertical row
//|
typedef enum FLW_SPLIT_TYPE {
    FLW_SPLIT_TYPE_HORIZONTAL,
    FLW_SPLIT_TYPE_VERTICAL,
} FLW_SPLIT_TYPE;

//|--------------------------- Flw_Split
//| A resizable type widget for two child widgets
//| The split layout widget is a container widget for two child widgets that can be resized using a mouse
//| The child widgets will be positioned horizontaly by default but can be changed to vertically
//| Use add1() and add2() to add or remove widgets
//| Use add1() and add2() to add or remove widgets
//|
class Flw_Split : public Fl_Group {
public:
                    Flw_Split(int X = 0, int Y = 0, int W = 0, int H = 0, const char* l = NULL);

    void            add1(Fl_Widget* widget);
    void            add2(Fl_Widget* widget);
    void            clear();
    int             get_border_space() const;
    int             get_split_pos() const;
    FLW_SPLIT_TYPE  get_type() const;
    int             get_widget_space() const;
    Fl_Widget*      get_widget1();
    Fl_Widget*      get_widget2();
    virtual int     handle(int event);
    void            resize();
    virtual void    resize(int X, int Y, int W, int H);
    void            set_border_space(int border_space);
    void            set_type(FLW_SPLIT_TYPE type);
    void            set_split_pos(int split_pos);
    void            set_widget_space(int widget_space);

private:
    int             _border_space;
    bool            _drag;
    int             _split_pos;
    FLW_SPLIT_TYPE  _type;
    int             _widget_space;
    Fl_Widget*      _widgets[2];
    int             _h;
    int             _w;
};

#endif // FLW_SPLIT_H
