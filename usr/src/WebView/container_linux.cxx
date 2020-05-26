#include "container_linux.hxx"
#include <Poco/URI.h>
#include <cairo.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static std::string urljoin (const std::string & base,
                            const std::string & relative)
{
    try
    {
        Poco::URI uri_base (base);
        Poco::URI uri_res (uri_base, relative);
        return uri_res.toString ();
    }
    catch (...)
    {
        return relative;
    }
}

static litehtml::elements_vector path (const litehtml::element::ptr & element)
{
    litehtml::elements_vector result;
    litehtml::element::ptr current = element;
    while (current)
    {
        result.push_back (current);
        current = current->parent ();
    }
    std::reverse (std::begin (result), std::end (result));
    return result;
}

static std::pair<litehtml::element::ptr, size_t>
getCommonParent (const litehtml::elements_vector & a,
                 const litehtml::elements_vector & b)
{
    litehtml::element::ptr parent;
    const size_t minSize = std::min (a.size (), b.size ());
    for (size_t i = 0; i < minSize; ++i)
    {
        if (a.at (i) != b.at (i))
            return {parent, i};
        parent = a.at (i);
    }
    return {parent, minSize};
}

static std::pair<Selection::Element, Selection::Element>
getStartAndEnd (const Selection::Element & a, const Selection::Element & b)
{
    if (a.element == b.element)
    {
        if (a.index <= b.index)
            return {a, b};
        return {b, a};
    }
    const litehtml::elements_vector aPath = path (a.element);
    const litehtml::elements_vector bPath = path (b.element);
    litehtml::element::ptr commonParent;
    size_t firstDifferentIndex;
    std::tie (commonParent, firstDifferentIndex) =
        getCommonParent (aPath, bPath);
    if (!commonParent)
    {
        std::cerr
            << "internal error: litehtml elements do not have common parent";
        return {a, b};
    }
    if (commonParent == a.element)
        return {a, a}; // 'a' already contains 'b'
    if (commonParent == b.element)
        return {b, b};
    // find out if a or b is first in the child sub-trees of commonParent
    const litehtml::element::ptr aBranch = aPath.at (firstDifferentIndex);
    const litehtml::element::ptr bBranch = bPath.at (firstDifferentIndex);
    for (int i = 0; i < int (commonParent->get_children_count ()); ++i)
    {
        const litehtml::element::ptr child = commonParent->get_child (i);
        if (child == aBranch)
            return {a, b};
        if (child == bBranch)
            return {b, a};
    }
    std::cerr
        << "internal error: failed to find out order of litehtml elements";
    return {a, b};
}

static int findChild (const litehtml::element::ptr & child,
                      const litehtml::element::ptr & parent)
{
    for (int i = 0; i < int (parent->get_children_count ()); ++i)
        if (parent->get_child (i) == child)
            return i;
    return -1;
}

// 1) stops right away if element == stop, otherwise stops whenever stop element
// is encountered 2) moves down the first children from element until there is
// none anymore
static litehtml::element::ptr firstLeaf (const litehtml::element::ptr & element,
                                         const litehtml::element::ptr & stop)
{
    if (element == stop)
        return element;
    litehtml::element::ptr current = element;
    while (current != stop && current->get_children_count () > 0)
        current = current->get_child (0);
    return current;
}

// 1) stops right away if element == stop, otherwise stops whenever stop element
// is encountered 2) starts at next sibling (up the hierarchy chain) if
// possible, otherwise root 3) returns first leaf of the element found in 2
static litehtml::element::ptr nextLeaf (const litehtml::element::ptr & element,
                                        const litehtml::element::ptr & stop)
{
    if (element == stop)
        return element;
    litehtml::element::ptr current = element;
    if (current->have_parent ())
    {
        // find next sibling
        const litehtml::element::ptr parent = current->parent ();
        const int childIndex = findChild (current, parent);
        if (childIndex < 0)
        {
            std::cerr << "internal error: filed to find litehtml child "
                         "element in parent";
            return stop;
        }
        if (childIndex + 1 >=
            int (parent->get_children_count ())) // no sibling, move up
            return nextLeaf (parent, stop);
        current = parent->get_child (childIndex + 1);
    }
    return firstLeaf (current, stop);
}

Selection::Element
container_linux::selectionDetails (const litehtml::element::ptr & element,
                                   const std::string & text, const int & posX,
                                   const int & posY)
{
    int previous = 0;

    // shortcut, which _might_ not really be correct
    if (element->get_children_count () > 0)
    {
        return {element, -1, -1}; // everything selected
    }

    for (int i = 0; i < text.size (); ++i)
    {
        std::string sub = text.substr (0, i + 1);
        int width = text_width (sub.c_str (), element->get_font ());
        // printf ("width: %d\n", width);
        if ((width + previous) / 2 >= posX)
        {
            return Selection::Element (element, i, previous);
        }
        previous = width;
    }
    return {element, (int)text.size (), previous};
}

Selection::Element
container_linux::deepest_child_at_point (int x, int y, int client_x,
                                         int client_y, Selection::Mode mode)
{
    if (!doc)
        return {};

    // the following does not find the "smallest" element, it often consists of
    // children with individual words as text...
    const litehtml::element::ptr element =
        doc->root ()->get_element_by_point (x, y, client_x, client_y);
    // ...so try to find a better match
    const std::function<Selection::Element (litehtml::element::ptr,
                                            litehtml::position)>
        recursion =
            [this, &recursion, x, y, mode] (
                const litehtml::element::ptr & element,
                const litehtml::position & placement) -> Selection::Element {
        if (!element)
            return {};
        Selection::Element result;
        for (int i = 0; i < int (element->get_children_count ()); ++i)
        {
            const litehtml::element::ptr child = element->get_child (i);
            result = recursion (
                child,
                child->get_position ().translatedBy (placement.x, placement.y));
            if (result.element)
                return result;
        }
        if (placement.is_point_inside (x, y))
        {
            litehtml::tstring text;
            element->get_text (text);
            if (!text.empty ())
            {
                return mode == Selection::Mode::Free
                           ? selectionDetails (element,
                                               text,
                                               x - placement.x,
                                               y - placement.y)
                           : Selection::Element ({element, -1, -1});
            }
        }
        return {};
    };
    return recursion (
        element, element ? element->get_placement () : litehtml::position ());
}

bool Selection::isValid () const
{
    return !selection.empty ();
}

void Selection::update ()
{
    const auto addElement = [this] (const Selection::Element & element,
                                    const Selection::Element & end = {}) {
        litehtml::tstring elemText;
        element.element->get_text (elemText);
        const std::string textStr (elemText);
        if (!textStr.empty ())
        {
            litehtml::position rect =
                element.element->get_placement ().adjusted (-1, -1, 1, 1);
            if (element.index < 0)
            { // fully selected
                text += textStr;
            }
            else if (end.element)
            { // select from element "to end"
                if (element.element == end.element)
                {
                    // end.index is guaranteed to be >= element.index by caller,
                    // same for x
                    text += textStr.substr (element.index,
                                            end.index - element.index);
                    const int left = rect.left ();
                    rect.setLeft (left + element.x);
                    // rect.width = 0; // element.x;
                    rect.setRight (left + end.x);
                }
                else
                {
                    text += textStr.substr (element.index);
                    rect.setLeft (rect.left () + element.x);
                }
            }
            else
            { // select from start of element
                text += textStr.substr (0, element.index);
                rect.setRight (rect.left () + element.x);
            }
            selection.push_back (rect);
        }
    };

    if (startElem.element && endElem.element)
    {
        // Edge cases:
        // start and end elements could be reversed or children of each other
        Selection::Element start;
        Selection::Element end;
        std::tie (start, end) = getStartAndEnd (startElem, endElem);

        selection.clear ();
        text.clear ();

        // Treats start element as a leaf even if it isn't, because it already
        // contains all its children
        addElement (start, end);
        if (start.element != end.element)
        {
            litehtml::element::ptr current = start.element;
            do
            {
                current = nextLeaf (current, end.element);
                if (current == end.element)
                    addElement (end);
                else
                    addElement ({current, -1, -1});
            } while (current != end.element);
        }
    }
    else
    {
        selection = {};
        text.clear ();
    }
    /*QClipboard * cb = QGuiApplication::clipboard ();
    if (cb->supportsSelection ())
        cb->setText (text, QClipboard::Selection);*/
}

litehtml::position Selection::boundingRect () const
{
    litehtml::position rect;
    for (const auto & r : selection)
        rect = rect.united (r);
    return rect;
}

void container_linux::drawSelection (cairo_t * cr, litehtml::position clip)
{
    cairo_save (cr);
    for (const auto r : selection.selection)
    {
        litehtml::position pos = r; //.translatedBy
        cairo_rectangle (cr, pos.x, pos.y, pos.width, pos.height);
        cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
        cairo_set_operator (cr, CAIRO_OPERATOR_DIFFERENCE);
        cairo_fill (cr);
    }
    cairo_restore (cr);
}

void container_linux::mousePressAt (int x, int y, int client_x, int client_y)
{
    selection = {};
    selection.selectionStartDocumentPosX = client_x;
    selection.selectionStartDocumentPosY = client_y;
    selection.startElem =
        deepest_child_at_point (x, y, client_x, client_y, selection.mode);
    selection.endElem = {};
}
void container_linux::mouseDragAt (int x, int y, int client_x, int client_y)
{
    if (1) // m_selection.startElem.element))
    {
        const Selection::Element element =
            deepest_child_at_point (x, y, client_x, client_y, selection.mode);
        if (element.element)
        {
            selection.endElem = element;
            selection.update ();
        }
        selection.isSelecting = true;
    }
}
void container_linux::mouseReleaseAt (int x, int y, int client_x, int client_y)
{
    selection.isSelecting = false;
    selection.selectionStartDocumentPosX = -1;
    selection.selectionStartDocumentPosY = -1;

    if (selection.isValid ())
    {
        printf ("Valid selection: %s\n", selection.text.c_str ());
        //  m_blockLinks = true;
    }
    else
        selection = {};
}

container_linux::container_linux (void)
{
    m_temp_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 2, 2);
    m_temp_cr = cairo_create (m_temp_surface);
}

container_linux::~container_linux (void)
{
    clear_images ();
    cairo_surface_destroy (m_temp_surface);
    cairo_destroy (m_temp_cr);
}

litehtml::uint_ptr
container_linux::create_font (const litehtml::tchar_t * faceName, int size,
                              int weight, litehtml::font_style italic,
                              unsigned int decoration,
                              litehtml::font_metrics * fm)
{
    litehtml::string_vector fonts;
    litehtml::split_string (faceName, fonts, ",");
    litehtml::trim (fonts[0]);

    cairo_font_face_t * fnt = 0;

    FcPattern * pattern = FcPatternCreate ();
    bool found = false;
    for (litehtml::string_vector::iterator i = fonts.begin ();
         i != fonts.end ();
         i++)
    {
        if (FcPatternAddString (
                pattern, FC_FAMILY, (unsigned char *)i->c_str ()))
        {
            found = true;
            break;
        }
    }
    if (found)
    {
        if (italic == litehtml::fontStyleItalic)
        {
            FcPatternAddInteger (pattern, FC_SLANT, FC_SLANT_ITALIC);
        }
        else
        {
            FcPatternAddInteger (pattern, FC_SLANT, FC_SLANT_ROMAN);
        }

        int fc_weight = FC_WEIGHT_NORMAL;
        if (weight >= 0 && weight < 150)
            fc_weight = FC_WEIGHT_THIN;
        else if (weight >= 150 && weight < 250)
            fc_weight = FC_WEIGHT_EXTRALIGHT;
        else if (weight >= 250 && weight < 350)
            fc_weight = FC_WEIGHT_LIGHT;
        else if (weight >= 350 && weight < 450)
            fc_weight = FC_WEIGHT_NORMAL;
        else if (weight >= 450 && weight < 550)
            fc_weight = FC_WEIGHT_MEDIUM;
        else if (weight >= 550 && weight < 650)
            fc_weight = FC_WEIGHT_SEMIBOLD;
        else if (weight >= 650 && weight < 750)
            fc_weight = FC_WEIGHT_BOLD;
        else if (weight >= 750 && weight < 850)
            fc_weight = FC_WEIGHT_EXTRABOLD;
        else if (weight >= 950)
            fc_weight = FC_WEIGHT_BLACK;

        FcPatternAddInteger (pattern, FC_WEIGHT, fc_weight);

        fnt = cairo_ft_font_face_create_for_pattern (pattern);
    }

    FcPatternDestroy (pattern);

    cairo_font * ret = 0;

    if (fm && fnt)
    {
        cairo_save (m_temp_cr);

        cairo_set_font_face (m_temp_cr, fnt);
        cairo_set_font_size (m_temp_cr, size);
        cairo_font_extents_t ext;
        cairo_font_extents (m_temp_cr, &ext);

        cairo_text_extents_t tex;
        cairo_text_extents (m_temp_cr, "x", &tex);

        fm->ascent = (int)ext.ascent;
        fm->descent = (int)ext.descent;
        fm->height = (int)(ext.ascent + ext.descent);
        fm->x_height = (int)tex.height;

        cairo_restore (m_temp_cr);

        ret = new cairo_font;
        ret->font = fnt;
        ret->size = size;
        ret->strikeout =
            (decoration & litehtml::font_decoration_linethrough) ? true : false;
        ret->underline =
            (decoration & litehtml::font_decoration_underline) ? true : false;
    }

    return (litehtml::uint_ptr)ret;
}

void container_linux::delete_font (litehtml::uint_ptr hFont)
{
    cairo_font * fnt = (cairo_font *)hFont;
    if (fnt)
    {
        cairo_font_face_destroy (fnt->font);
        delete fnt;
    }
}

int container_linux::text_width (const litehtml::tchar_t * text,
                                 litehtml::uint_ptr hFont)
{
    cairo_font * fnt = (cairo_font *)hFont;

    cairo_save (m_temp_cr);

    cairo_set_font_size (m_temp_cr, fnt->size);
    cairo_set_font_face (m_temp_cr, fnt->font);
    cairo_text_extents_t ext;
    cairo_text_extents (m_temp_cr, text, &ext);

    cairo_restore (m_temp_cr);

    return (int)ext.x_advance;
}

void container_linux::draw_text (litehtml::uint_ptr hdc,
                                 const litehtml::tchar_t * text,
                                 litehtml::uint_ptr hFont,
                                 litehtml::web_color color,
                                 const litehtml::position & pos)
{
    cairo_font * fnt = (cairo_font *)hFont;
    cairo_t * cr = (cairo_t *)hdc;
    cairo_save (cr);

    apply_clip (cr);

    cairo_set_font_face (cr, fnt->font);
    cairo_set_font_size (cr, fnt->size);
    cairo_font_extents_t ext;
    cairo_font_extents (cr, &ext);

    int x = pos.left ();
    int y = pos.bottom () - ext.descent;
    // printf ("Asked to draw text at %d@%d\n", x, y);

    set_color (cr, color);

    cairo_move_to (cr, x, y);
    cairo_show_text (cr, text);

    int tw = 0;

    if (fnt->underline || fnt->strikeout)
    {
        tw = text_width (text, hFont);
    }

    if (fnt->underline)
    {
        cairo_set_line_width (cr, 1);
        cairo_move_to (cr, x, y + 1.5);
        cairo_line_to (cr, x + tw, y + 1.5);
        cairo_stroke (cr);
    }
    if (fnt->strikeout)
    {
        cairo_text_extents_t tex;
        cairo_text_extents (cr, "x", &tex);

        int ln_y = y - tex.height / 2.0;

        cairo_set_line_width (cr, 1);
        cairo_move_to (cr, x, (double)ln_y - 0.5);
        cairo_line_to (cr, x + tw, (double)ln_y - 0.5);
        cairo_stroke (cr);
    }

    cairo_restore (cr);
}

int container_linux::pt_to_px (int pt)
{
    GdkScreen * screen = gdk_screen_get_default ();
    double dpi = gdk_screen_get_resolution (screen);

    return (int)((double)pt * dpi / 72.0);
}

int container_linux::get_default_font_size () const
{
    return 16;
}

void container_linux::draw_list_marker (litehtml::uint_ptr hdc,
                                        const litehtml::list_marker & marker)
{
    if (!marker.image.empty ())
    {
        /*litehtml::tstring url;
        make_url(marker.image.c_str(), marker.baseurl, url);

        lock_images_cache();
        images_map::iterator img_i = m_images.find(url.c_str());
        if(img_i != m_images.end())
        {
            if(img_i->second)
            {
                draw_txdib((cairo_t*) hdc, img_i->second, marker.pos.x,
        marker.pos.y, marker.pos.width, marker.pos.height);
            }
        }
        unlock_images_cache();*/
    }
    else
    {
        switch (marker.marker_type)
        {
        case litehtml::list_style_type_circle:
        {
            draw_ellipse ((cairo_t *)hdc,
                          marker.pos.x,
                          marker.pos.y,
                          marker.pos.width,
                          marker.pos.height,
                          marker.color,
                          0.5);
        }
        break;
        case litehtml::list_style_type_disc:
        {
            fill_ellipse ((cairo_t *)hdc,
                          marker.pos.x,
                          marker.pos.y,
                          marker.pos.width,
                          marker.pos.height,
                          marker.color);
        }
        break;
        case litehtml::list_style_type_square:
            if (hdc)
            {
                cairo_t * cr = (cairo_t *)hdc;
                cairo_save (cr);

                cairo_new_path (cr);
                cairo_rectangle (cr,
                                 marker.pos.x,
                                 marker.pos.y,
                                 marker.pos.width,
                                 marker.pos.height);

                set_color (cr, marker.color);
                cairo_fill (cr);
                cairo_restore (cr);
            }
            break;
        default:
            /*do nothing*/
            break;
        }
    }
}

void container_linux::load_image (const litehtml::tchar_t * src,
                                  const litehtml::tchar_t * baseurl,
                                  bool redraw_on_ready)
{
    litehtml::tstring url;
    make_url (src, baseurl, url);
    printf ("Looking for image %s\n", url.c_str ());
    if (m_images.find (url.c_str ()) == m_images.end ())
    {
        try
        {
            Glib::RefPtr<Gdk::Pixbuf> img = get_image (url.c_str (), true);
            if (img)
            {
                m_images[url.c_str ()] = img;
            }
        }
        catch (...)
        {
            int iii = 0;
            iii++;
        }
    }
}

void container_linux::get_image_size (const litehtml::tchar_t * src,
                                      const litehtml::tchar_t * baseurl,
                                      litehtml::size & sz)
{
    litehtml::tstring url;
    make_url (src, baseurl, url);

    images_map::iterator img = m_images.find (url.c_str ());
    if (img != m_images.end ())
    {
        sz.width = img->second->get_width ();
        sz.height = img->second->get_height ();
    }
    else
    {
        sz.width = 0;
        sz.height = 0;
    }
}

void container_linux::draw_background (litehtml::uint_ptr hdc,
                                       const litehtml::background_paint & bg)
{
    cairo_t * cr = (cairo_t *)hdc;
    cairo_save (cr);
    apply_clip (cr);

    rounded_rectangle (cr, bg.border_box, bg.border_radius);
    cairo_clip (cr);

    cairo_rectangle (cr,
                     bg.clip_box.x,
                     bg.clip_box.y,
                     bg.clip_box.width,
                     bg.clip_box.height);
    cairo_clip (cr);

    if (bg.color.alpha)
    {
        set_color (cr, bg.color);
        cairo_paint (cr);
    }

    litehtml::tstring url;
    make_url (bg.image.c_str (), bg.baseurl.c_str (), url);

    // lock_images_cache();
    images_map::iterator img_i = m_images.find (url.c_str ());
    if (img_i != m_images.end () && img_i->second)
    {
        Glib::RefPtr<Gdk::Pixbuf> bgbmp = img_i->second;

        Glib::RefPtr<Gdk::Pixbuf> new_img;
        if (bg.image_size.width != bgbmp->get_width () ||
            bg.image_size.height != bgbmp->get_height ())
        {
            new_img = bgbmp->scale_simple (bg.image_size.width,
                                           bg.image_size.height,
                                           Gdk::INTERP_BILINEAR);
            bgbmp = new_img;
        }

        cairo_surface_t * img = surface_from_pixbuf (bgbmp);
        cairo_pattern_t * pattern = cairo_pattern_create_for_surface (img);
        cairo_matrix_t flib_m;
        cairo_matrix_init_identity (&flib_m);
        cairo_matrix_translate (&flib_m, -bg.position_x, -bg.position_y);
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
        cairo_pattern_set_matrix (pattern, &flib_m);

        switch (bg.repeat)
        {
        case litehtml::background_repeat_no_repeat:
            draw_pixbuf (cr,
                         bgbmp,
                         bg.position_x,
                         bg.position_y,
                         bgbmp->get_width (),
                         bgbmp->get_height ());
            break;

        case litehtml::background_repeat_repeat_x:
            cairo_set_source (cr, pattern);
            cairo_rectangle (cr,
                             bg.clip_box.left (),
                             bg.position_y,
                             bg.clip_box.width,
                             bgbmp->get_height ());
            cairo_fill (cr);
            break;

        case litehtml::background_repeat_repeat_y:
            cairo_set_source (cr, pattern);
            cairo_rectangle (cr,
                             bg.position_x,
                             bg.clip_box.top (),
                             bgbmp->get_width (),
                             bg.clip_box.height);
            cairo_fill (cr);
            break;

        case litehtml::background_repeat_repeat:
            cairo_set_source (cr, pattern);
            cairo_rectangle (cr,
                             bg.clip_box.left (),
                             bg.clip_box.top (),
                             bg.clip_box.width,
                             bg.clip_box.height);
            cairo_fill (cr);
            break;
        }

        cairo_pattern_destroy (pattern);
        cairo_surface_destroy (img);
    }

    if (bg.m_gradient.m_is)
    {
        cairo_pattern_t * pat =
            cairo_pattern_create_linear (bg.clip_box.x,
                                         bg.clip_box.y,
                                         bg.clip_box.x,
                                         bg.clip_box.y + bg.clip_box.height);
        cairo_pattern_add_color_stop_rgba (pat,
                                           0.0,
                                           bg.m_gradient.m_start.red / 255.0,
                                           bg.m_gradient.m_start.green / 255.0,
                                           bg.m_gradient.m_start.blue / 255.0,
                                           bg.m_gradient.m_start.alpha / 255.0);
        cairo_pattern_add_color_stop_rgba (pat,
                                           1.0,
                                           bg.m_gradient.m_end.red / 255.0,
                                           bg.m_gradient.m_end.green / 255.0,
                                           bg.m_gradient.m_end.blue / 255.0,
                                           bg.m_gradient.m_end.alpha / 255.0);
        cairo_set_source (cr, pat);

        cairo_rectangle (cr,
                         bg.clip_box.left (),
                         bg.clip_box.top (),
                         bg.clip_box.width,
                         bg.clip_box.height);
        cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
        cairo_fill (cr);
        cairo_pattern_destroy (pat);
    }

    drawSelection (cr, bg.border_box);

    //	unlock_images_cache();
    cairo_restore (cr);
}

void container_linux::make_url (const litehtml::tchar_t * url,
                                const litehtml::tchar_t * basepath,
                                litehtml::tstring & out)
{
    if (!basepath || (basepath && !basepath[0]))
    {
        if (!m_base_url.empty ())
        {
            out = urljoin (m_base_url, std::string (url));
        }
        else
        {
            out = url;
        }
    }
    else
    {
        out = urljoin (std::string (basepath), std::string (url));
    }
}

void container_linux::set_base_url (const litehtml::tchar_t * base_url)
{
    if (base_url)
    {
        m_base_url = urljoin (m_url, std::string (base_url));
    }
    else
    {
        m_base_url = m_url;
    }
}

void container_linux::add_path_arc (cairo_t * cr, double x, double y, double rx,
                                    double ry, double a1, double a2, bool neg)
{
    if (rx > 0 && ry > 0)
    {

        cairo_save (cr);

        cairo_translate (cr, x, y);
        cairo_scale (cr, 1, ry / rx);
        cairo_translate (cr, -x, -y);

        if (neg)
        {
            cairo_arc_negative (cr, x, y, rx, a1, a2);
        }
        else
        {
            cairo_arc (cr, x, y, rx, a1, a2);
        }

        cairo_restore (cr);
    }
    else
    {
        cairo_move_to (cr, x, y);
    }
}

void container_linux::draw_borders (litehtml::uint_ptr hdc,
                                    const litehtml::borders & borders,
                                    const litehtml::position & draw_pos,
                                    bool root)
{
    cairo_t * cr = (cairo_t *)hdc;
    cairo_save (cr);
    apply_clip (cr);

    cairo_new_path (cr);

    int bdr_top = 0;
    int bdr_bottom = 0;
    int bdr_left = 0;
    int bdr_right = 0;

    if (borders.top.width != 0 &&
        borders.top.style > litehtml::border_style_hidden)
    {
        bdr_top = (int)borders.top.width;
    }
    if (borders.bottom.width != 0 &&
        borders.bottom.style > litehtml::border_style_hidden)
    {
        bdr_bottom = (int)borders.bottom.width;
    }
    if (borders.left.width != 0 &&
        borders.left.style > litehtml::border_style_hidden)
    {
        bdr_left = (int)borders.left.width;
    }
    if (borders.right.width != 0 &&
        borders.right.style > litehtml::border_style_hidden)
    {
        bdr_right = (int)borders.right.width;
    }

    // draw right border
    if (bdr_right)
    {
        set_color (cr, borders.right.color);

        double r_top = borders.radius.top_right_x;
        double r_bottom = borders.radius.bottom_right_x;

        if (r_top)
        {
            double end_angle = 2 * M_PI;
            double start_angle =
                end_angle -
                M_PI / 2.0 / ((double)bdr_top / (double)bdr_right + 1);

            add_path_arc (cr,
                          draw_pos.right () - r_top,
                          draw_pos.top () + r_top,
                          r_top - bdr_right,
                          r_top - bdr_right + (bdr_right - bdr_top),
                          end_angle,
                          start_angle,
                          true);

            add_path_arc (cr,
                          draw_pos.right () - r_top,
                          draw_pos.top () + r_top,
                          r_top,
                          r_top,
                          start_angle,
                          end_angle,
                          false);
        }
        else
        {
            cairo_move_to (
                cr, draw_pos.right () - bdr_right, draw_pos.top () + bdr_top);
            cairo_line_to (cr, draw_pos.right (), draw_pos.top ());
        }

        if (r_bottom)
        {
            cairo_line_to (
                cr, draw_pos.right (), draw_pos.bottom () - r_bottom);

            double start_angle = 0;
            double end_angle =
                start_angle +
                M_PI / 2.0 / ((double)bdr_bottom / (double)bdr_right + 1);

            add_path_arc (cr,
                          draw_pos.right () - r_bottom,
                          draw_pos.bottom () - r_bottom,
                          r_bottom,
                          r_bottom,
                          start_angle,
                          end_angle,
                          false);

            add_path_arc (cr,
                          draw_pos.right () - r_bottom,
                          draw_pos.bottom () - r_bottom,
                          r_bottom - bdr_right,
                          r_bottom - bdr_right + (bdr_right - bdr_bottom),
                          end_angle,
                          start_angle,
                          true);
        }
        else
        {
            cairo_line_to (cr, draw_pos.right (), draw_pos.bottom ());
            cairo_line_to (cr,
                           draw_pos.right () - bdr_right,
                           draw_pos.bottom () - bdr_bottom);
        }

        cairo_fill (cr);
    }

    // draw bottom border
    if (bdr_bottom)
    {
        set_color (cr, borders.bottom.color);

        double r_left = borders.radius.bottom_left_x;
        double r_right = borders.radius.bottom_right_x;

        if (r_left)
        {
            double start_angle = M_PI / 2.0;
            double end_angle =
                start_angle +
                M_PI / 2.0 / ((double)bdr_left / (double)bdr_bottom + 1);

            add_path_arc (cr,
                          draw_pos.left () + r_left,
                          draw_pos.bottom () - r_left,
                          r_left - bdr_bottom + (bdr_bottom - bdr_left),
                          r_left - bdr_bottom,
                          start_angle,
                          end_angle,
                          false);

            add_path_arc (cr,
                          draw_pos.left () + r_left,
                          draw_pos.bottom () - r_left,
                          r_left,
                          r_left,
                          end_angle,
                          start_angle,
                          true);
        }
        else
        {
            cairo_move_to (cr, draw_pos.left (), draw_pos.bottom ());
            cairo_line_to (cr,
                           draw_pos.left () + bdr_left,
                           draw_pos.bottom () - bdr_bottom);
        }

        if (r_right)
        {
            cairo_line_to (cr, draw_pos.right () - r_right, draw_pos.bottom ());

            double end_angle = M_PI / 2.0;
            double start_angle =
                end_angle -
                M_PI / 2.0 / ((double)bdr_right / (double)bdr_bottom + 1);

            add_path_arc (cr,
                          draw_pos.right () - r_right,
                          draw_pos.bottom () - r_right,
                          r_right,
                          r_right,
                          end_angle,
                          start_angle,
                          true);

            add_path_arc (cr,
                          draw_pos.right () - r_right,
                          draw_pos.bottom () - r_right,
                          r_right - bdr_bottom + (bdr_bottom - bdr_right),
                          r_right - bdr_bottom,
                          start_angle,
                          end_angle,
                          false);
        }
        else
        {
            cairo_line_to (cr,
                           draw_pos.right () - bdr_right,
                           draw_pos.bottom () - bdr_bottom);
            cairo_line_to (cr, draw_pos.right (), draw_pos.bottom ());
        }

        cairo_fill (cr);
    }

    // draw top border
    if (bdr_top)
    {
        set_color (cr, borders.top.color);

        double r_left = borders.radius.top_left_x;
        double r_right = borders.radius.top_right_x;

        if (r_left)
        {
            double end_angle = M_PI * 3.0 / 2.0;
            double start_angle =
                end_angle -
                M_PI / 2.0 / ((double)bdr_left / (double)bdr_top + 1);

            add_path_arc (cr,
                          draw_pos.left () + r_left,
                          draw_pos.top () + r_left,
                          r_left,
                          r_left,
                          end_angle,
                          start_angle,
                          true);

            add_path_arc (cr,
                          draw_pos.left () + r_left,
                          draw_pos.top () + r_left,
                          r_left - bdr_top + (bdr_top - bdr_left),
                          r_left - bdr_top,
                          start_angle,
                          end_angle,
                          false);
        }
        else
        {
            cairo_move_to (cr, draw_pos.left (), draw_pos.top ());
            cairo_line_to (
                cr, draw_pos.left () + bdr_left, draw_pos.top () + bdr_top);
        }

        if (r_right)
        {
            cairo_line_to (
                cr, draw_pos.right () - r_right, draw_pos.top () + bdr_top);

            double start_angle = M_PI * 3.0 / 2.0;
            double end_angle =
                start_angle +
                M_PI / 2.0 / ((double)bdr_right / (double)bdr_top + 1);

            add_path_arc (cr,
                          draw_pos.right () - r_right,
                          draw_pos.top () + r_right,
                          r_right - bdr_top + (bdr_top - bdr_right),
                          r_right - bdr_top,
                          start_angle,
                          end_angle,
                          false);

            add_path_arc (cr,
                          draw_pos.right () - r_right,
                          draw_pos.top () + r_right,
                          r_right,
                          r_right,
                          end_angle,
                          start_angle,
                          true);
        }
        else
        {
            cairo_line_to (
                cr, draw_pos.right () - bdr_right, draw_pos.top () + bdr_top);
            cairo_line_to (cr, draw_pos.right (), draw_pos.top ());
        }

        cairo_fill (cr);
    }

    // draw left border
    if (bdr_left)
    {
        set_color (cr, borders.left.color);

        double r_top = borders.radius.top_left_x;
        double r_bottom = borders.radius.bottom_left_x;

        if (r_top)
        {
            double start_angle = M_PI;
            double end_angle =
                start_angle +
                M_PI / 2.0 / ((double)bdr_top / (double)bdr_left + 1);

            add_path_arc (cr,
                          draw_pos.left () + r_top,
                          draw_pos.top () + r_top,
                          r_top - bdr_left,
                          r_top - bdr_left + (bdr_left - bdr_top),
                          start_angle,
                          end_angle,
                          false);

            add_path_arc (cr,
                          draw_pos.left () + r_top,
                          draw_pos.top () + r_top,
                          r_top,
                          r_top,
                          end_angle,
                          start_angle,
                          true);
        }
        else
        {
            cairo_move_to (
                cr, draw_pos.left () + bdr_left, draw_pos.top () + bdr_top);
            cairo_line_to (cr, draw_pos.left (), draw_pos.top ());
        }

        if (r_bottom)
        {
            cairo_line_to (cr, draw_pos.left (), draw_pos.bottom () - r_bottom);

            double end_angle = M_PI;
            double start_angle =
                end_angle -
                M_PI / 2.0 / ((double)bdr_bottom / (double)bdr_left + 1);

            add_path_arc (cr,
                          draw_pos.left () + r_bottom,
                          draw_pos.bottom () - r_bottom,
                          r_bottom,
                          r_bottom,
                          end_angle,
                          start_angle,
                          true);

            add_path_arc (cr,
                          draw_pos.left () + r_bottom,
                          draw_pos.bottom () - r_bottom,
                          r_bottom - bdr_left,
                          r_bottom - bdr_left + (bdr_left - bdr_bottom),
                          start_angle,
                          end_angle,
                          false);
        }
        else
        {
            cairo_line_to (cr, draw_pos.left (), draw_pos.bottom ());
            cairo_line_to (cr,
                           draw_pos.left () + bdr_left,
                           draw_pos.bottom () - bdr_bottom);
        }

        cairo_fill (cr);
    }
    cairo_restore (cr);
}

void container_linux::transform_text (litehtml::tstring & text,
                                      litehtml::text_transform tt)
{
}

void container_linux::set_clip (const litehtml::position & pos,
                                const litehtml::border_radiuses & bdr_radius,
                                bool valid_x, bool valid_y)
{
    litehtml::position clip_pos = pos;
    litehtml::position client_pos;
    get_client_rect (client_pos);
    if (!valid_x)
    {
        clip_pos.x = client_pos.x;
        clip_pos.width = client_pos.width;
    }
    if (!valid_y)
    {
        clip_pos.y = client_pos.y;
        clip_pos.height = client_pos.height;
    }
    m_clips.emplace_back (clip_pos, bdr_radius);
}

void container_linux::del_clip ()
{
    if (!m_clips.empty ())
    {
        m_clips.pop_back ();
    }
}

void container_linux::apply_clip (cairo_t * cr)
{
    for (const auto & clip_box : m_clips)
    {
        rounded_rectangle (cr, clip_box.box, clip_box.radius);
        cairo_clip (cr);
    }
}

void container_linux::draw_ellipse (cairo_t * cr, int x, int y, int width,
                                    int height,
                                    const litehtml::web_color & color,
                                    int line_width)
{
    if (!cr)
        return;
    cairo_save (cr);

    apply_clip (cr);

    cairo_new_path (cr);

    cairo_translate (cr, x + width / 2.0, y + height / 2.0);
    cairo_scale (cr, width / 2.0, height / 2.0);
    cairo_arc (cr, 0, 0, 1, 0, 2 * M_PI);

    set_color (cr, color);
    cairo_set_line_width (cr, line_width);
    cairo_stroke (cr);

    cairo_restore (cr);
}

void container_linux::fill_ellipse (cairo_t * cr, int x, int y, int width,
                                    int height,
                                    const litehtml::web_color & color)
{
    if (!cr)
        return;
    cairo_save (cr);

    apply_clip (cr);

    cairo_new_path (cr);

    cairo_translate (cr, x + width / 2.0, y + height / 2.0);
    cairo_scale (cr, width / 2.0, height / 2.0);
    cairo_arc (cr, 0, 0, 1, 0, 2 * M_PI);

    set_color (cr, color);
    cairo_fill (cr);

    cairo_restore (cr);
}

void container_linux::clear_images ()
{
    /*	for(images_map::iterator i = m_images.begin(); i != m_images.end(); i++)
        {
            if(i->second)
            {
                delete i->second;
            }
        }
        m_images.clear();
    */
}

const litehtml::tchar_t * container_linux::get_default_font_name () const
{
    return "Times New Roman";
}

std::shared_ptr<litehtml::element> container_linux::create_element (
    const litehtml::tchar_t * tag_name, const litehtml::string_map & attributes,
    const std::shared_ptr<litehtml::document> & doc)
{
    return 0;
}

void container_linux::rounded_rectangle (
    cairo_t * cr, const litehtml::position & pos,
    const litehtml::border_radiuses & radius)
{
    cairo_new_path (cr);
    if (radius.top_left_x)
    {
        cairo_arc (cr,
                   pos.left () + radius.top_left_x,
                   pos.top () + radius.top_left_x,
                   radius.top_left_x,
                   M_PI,
                   M_PI * 3.0 / 2.0);
    }
    else
    {
        cairo_move_to (cr, pos.left (), pos.top ());
    }

    cairo_line_to (cr, pos.right () - radius.top_right_x, pos.top ());

    if (radius.top_right_x)
    {
        cairo_arc (cr,
                   pos.right () - radius.top_right_x,
                   pos.top () + radius.top_right_x,
                   radius.top_right_x,
                   M_PI * 3.0 / 2.0,
                   2.0 * M_PI);
    }

    cairo_line_to (cr, pos.right (), pos.bottom () - radius.bottom_right_x);

    if (radius.bottom_right_x)
    {
        cairo_arc (cr,
                   pos.right () - radius.bottom_right_x,
                   pos.bottom () - radius.bottom_right_x,
                   radius.bottom_right_x,
                   0,
                   M_PI / 2.0);
    }

    cairo_line_to (cr, pos.left () - radius.bottom_left_x, pos.bottom ());

    if (radius.bottom_left_x)
    {
        cairo_arc (cr,
                   pos.left () + radius.bottom_left_x,
                   pos.bottom () - radius.bottom_left_x,
                   radius.bottom_left_x,
                   M_PI / 2.0,
                   M_PI);
    }
}

void container_linux::draw_pixbuf (cairo_t * cr,
                                   const Glib::RefPtr<Gdk::Pixbuf> & bmp, int x,
                                   int y, int cx, int cy)
{
    cairo_save (cr);

    {
        Cairo::RefPtr<Cairo::Context> crobj (new Cairo::Context (cr, false));

        cairo_matrix_t flib_m;
        cairo_matrix_init (&flib_m, 1, 0, 0, -1, 0, 0);

        if (cx != bmp->get_width () || cy != bmp->get_height ())
        {
            Glib::RefPtr<Gdk::Pixbuf> new_img =
                bmp->scale_simple (cx, cy, Gdk::INTERP_BILINEAR);
            ;
            Gdk::Cairo::set_source_pixbuf (crobj, new_img, x, y);
            crobj->paint ();
        }
        else
        {
            Gdk::Cairo::set_source_pixbuf (crobj, bmp, x, y);
            crobj->paint ();
        }
    }

    cairo_restore (cr);
}

cairo_surface_t *
container_linux::surface_from_pixbuf (const Glib::RefPtr<Gdk::Pixbuf> & bmp)
{
    cairo_surface_t * ret = NULL;

    if (bmp->get_has_alpha ())
    {
        ret = cairo_image_surface_create (
            CAIRO_FORMAT_ARGB32, bmp->get_width (), bmp->get_height ());
    }
    else
    {
        ret = cairo_image_surface_create (
            CAIRO_FORMAT_RGB24, bmp->get_width (), bmp->get_height ());
    }

    Cairo::RefPtr<Cairo::Surface> surface (new Cairo::Surface (ret, false));
    Cairo::RefPtr<Cairo::Context> ctx = Cairo::Context::create (surface);
    Gdk::Cairo::set_source_pixbuf (ctx, bmp, 0.0, 0.0);
    ctx->paint ();

    return ret;
}

void container_linux::get_media_features (
    litehtml::media_features & media) const
{
    litehtml::position client;
    get_client_rect (client);
    media.type = litehtml::media_type_screen;
    media.width = client.width;
    media.height = client.height;
    media.device_width = client.width;
    media.device_height = client.height;
    media.color = 8;
    media.monochrome = 0;
    media.color_index = 256;
    media.resolution = 96;
}

void container_linux::get_language (litehtml::tstring & language,
                                    litehtml::tstring & culture) const
{
    language = _t ("en");
    culture = _t ("");
}

void container_linux::link (const std::shared_ptr<litehtml::document> & ptr,
                            const litehtml::element::ptr & el)
{
    printf ("Link\n");
}
