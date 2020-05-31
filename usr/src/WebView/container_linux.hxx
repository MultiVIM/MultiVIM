/* Portions copyright: */
/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of QLiteHtml.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include <cairo.h>
#include <gtkmm.h>
#include <map>
#include <vector>

#include "litehtml.h"

class Selection
{
  public:
    struct Element
    {
        litehtml::element::ptr element;
        int index = -1;
        int x = -1;
        Element () = default;
        Element (litehtml::element::ptr element, int index, int x)
            : element (element), index (index), x (x)
        {
        }
    };

    enum class Mode
    {
        Free,
        Word
    };

    bool isValid () const;

    void update ();
    litehtml::position boundingRect () const;

    Element startElem;
    Element endElem;
    std::vector<litehtml::position> selection;
    std::string text;

    int selectionStartDocumentPosX, selectionStartDocumentPosY;
    Mode mode = Mode::Free;
    bool isSelecting = false;
};

struct cairo_clip_box
{
    typedef std::vector<cairo_clip_box> vector;
    litehtml::position box;
    litehtml::border_radiuses radius;

    cairo_clip_box (const litehtml::position & vBox,
                    litehtml::border_radiuses vRad)
    {
        box = vBox;
        radius = vRad;
    }

    cairo_clip_box (const cairo_clip_box & val)
    {
        box = val.box;
        radius = val.radius;
    }
    cairo_clip_box & operator= (const cairo_clip_box & val)
    {
        box = val.box;
        radius = val.radius;
        return *this;
    }
};

struct cairo_font
{
    cairo_font_face_t * font;
    int size;
    bool underline;
    bool strikeout;
};

class container_linux : public litehtml::document_container
{
    typedef std::map<litehtml::tstring, Glib::RefPtr<Gdk::Pixbuf>> images_map;

  protected:
    cairo_surface_t * m_temp_surface;
    cairo_t * m_temp_cr;
    images_map m_images;
    cairo_clip_box::vector m_clips;
    std::string m_base_url, m_url;
    litehtml::document::ptr doc;
    double scaleFactor = 1.5;

    Selection selection;
    void drawSelection (cairo_t * cr, litehtml::position border_box);

    void mousePressAt (int x, int y, int client_x, int client_y);
    void mouseDragAt (int x, int y, int client_x, int client_y);
    void mouseReleaseAt (int x, int y, int client_x, int client_y);

  public:
    container_linux (void);
    virtual ~container_linux (void);

    virtual litehtml::uint_ptr
    create_font (const litehtml::tchar_t * faceName, int size, int weight,
                 litehtml::font_style italic, unsigned int decoration,
                 litehtml::font_metrics * fm) override;
    virtual void delete_font (litehtml::uint_ptr hFont) override;
    virtual int text_width (const litehtml::tchar_t * text,
                            litehtml::uint_ptr hFont) override;
    virtual void draw_text (litehtml::uint_ptr hdc,
                            const litehtml::tchar_t * text,
                            litehtml::uint_ptr hFont, litehtml::web_color color,
                            const litehtml::position & pos) override;
    virtual int pt_to_px (int pt) override;
    virtual int get_default_font_size () const override;
    virtual const litehtml::tchar_t * get_default_font_name () const override;
    virtual void load_image (const litehtml::tchar_t * src,
                             const litehtml::tchar_t * baseurl,
                             bool redraw_on_ready) override;
    virtual void get_image_size (const litehtml::tchar_t * src,
                                 const litehtml::tchar_t * baseurl,
                                 litehtml::size & sz) override;
    virtual void
    draw_background (litehtml::uint_ptr hdc,
                     const litehtml::background_paint & bg) override;
    virtual void draw_borders (litehtml::uint_ptr hdc,
                               const litehtml::borders & borders,
                               const litehtml::position & draw_pos,
                               bool root) override;
    virtual void
    draw_list_marker (litehtml::uint_ptr hdc,
                      const litehtml::list_marker & marker) override;
    virtual std::shared_ptr<litehtml::element>
    create_element (const litehtml::tchar_t * tag_name,
                    const litehtml::string_map & attributes,
                    const std::shared_ptr<litehtml::document> & doc) override;
    virtual void
    get_media_features (litehtml::media_features & media) const override;
    virtual void get_language (litehtml::tstring & language,
                               litehtml::tstring & culture) const override;
    virtual void link (const std::shared_ptr<litehtml::document> & ptr,
                       const litehtml::element::ptr & el) override;

    virtual void transform_text (litehtml::tstring & text,
                                 litehtml::text_transform tt) override;
    virtual void set_clip (const litehtml::position & pos,
                           const litehtml::border_radiuses & bdr_radius,
                           bool valid_x, bool valid_y) override;
    virtual void del_clip () override;

    virtual void make_url (const litehtml::tchar_t * url,
                           const litehtml::tchar_t * basepath,
                           litehtml::tstring & out);
    virtual void set_base_url (const litehtml::tchar_t * base_url) override;

    virtual Glib::RefPtr<Gdk::Pixbuf> get_image (const litehtml::tchar_t * url,
                                                 bool redraw_on_ready) = 0;

    void clear_images ();

  protected:
    virtual void draw_ellipse (cairo_t * cr, int x, int y, int width,
                               int height, const litehtml::web_color & color,
                               int line_width);
    virtual void fill_ellipse (cairo_t * cr, int x, int y, int width,
                               int height, const litehtml::web_color & color);
    virtual void rounded_rectangle (cairo_t * cr,
                                    const litehtml::position & pos,
                                    const litehtml::border_radiuses & radius);

  private:
    void apply_clip (cairo_t * cr);
    void add_path_arc (cairo_t * cr, double x, double y, double rx, double ry,
                       double a1, double a2, bool neg);
    void set_color (cairo_t * cr, litehtml::web_color color)
    {
        cairo_set_source_rgba (cr,
                               color.red / 255.0,
                               color.green / 255.0,
                               color.blue / 255.0,
                               color.alpha / 255.0);
    }
    void draw_pixbuf (cairo_t * cr, const Glib::RefPtr<Gdk::Pixbuf> & bmp,
                      int x, int y, int cx, int cy);
    cairo_surface_t *
    surface_from_pixbuf (const Glib::RefPtr<Gdk::Pixbuf> & bmp);

    Selection::Element selectionDetails (const litehtml::element::ptr & element,
                                         const std::string & text,
                                         const int & posX, const int & posY);
    Selection::Element deepest_child_at_point (int x, int y, int client_x,
                                               int client_y,
                                               Selection::Mode mode);
};
