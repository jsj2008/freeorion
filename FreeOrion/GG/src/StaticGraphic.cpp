/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

#include <GG/StaticGraphic.h>

#include <GG/DrawUtil.h>
#include <GG/WndEditor.h>

#include <boost/assign/list_of.hpp>


using namespace GG;

namespace {
    struct SetStyleAction : AttributeChangedAction<Flags<GraphicStyle> >
    {
        SetStyleAction(StaticGraphic* static_graphic) : m_static_graphic(static_graphic) {}
        void operator()(const Flags<GraphicStyle>& style) {m_static_graphic->SetStyle(style);}
    private:
        StaticGraphic* m_static_graphic;
    };
}

///////////////////////////////////////
// GraphicStyle
///////////////////////////////////////
const GraphicStyle GG::GRAPHIC_NONE          (0);
const GraphicStyle GG::GRAPHIC_VCENTER       (1 << 0);
const GraphicStyle GG::GRAPHIC_TOP           (1 << 1);
const GraphicStyle GG::GRAPHIC_BOTTOM        (1 << 2);
const GraphicStyle GG::GRAPHIC_CENTER        (1 << 3);
const GraphicStyle GG::GRAPHIC_LEFT          (1 << 4);
const GraphicStyle GG::GRAPHIC_RIGHT         (1 << 5);
const GraphicStyle GG::GRAPHIC_FITGRAPHIC    (1 << 6);
const GraphicStyle GG::GRAPHIC_SHRINKFIT     (1 << 7);
const GraphicStyle GG::GRAPHIC_PROPSCALE     (1 << 8);

GG_FLAGSPEC_IMPL(GraphicStyle);

namespace {
    bool RegisterGraphicStyles()
    {
        FlagSpec<GraphicStyle>& spec = FlagSpec<GraphicStyle>::instance();
        spec.insert(GRAPHIC_NONE, "GRAPHIC_NONE", true);
        spec.insert(GRAPHIC_VCENTER, "GRAPHIC_VCENTER", true);
        spec.insert(GRAPHIC_TOP, "GRAPHIC_TOP", true);
        spec.insert(GRAPHIC_BOTTOM, "GRAPHIC_BOTTOM", true);
        spec.insert(GRAPHIC_CENTER, "GRAPHIC_CENTER", true);
        spec.insert(GRAPHIC_LEFT, "GRAPHIC_LEFT", true);
        spec.insert(GRAPHIC_RIGHT, "GRAPHIC_RIGHT", true);
        spec.insert(GRAPHIC_FITGRAPHIC, "GRAPHIC_FITGRAPHIC", true);
        spec.insert(GRAPHIC_SHRINKFIT, "GRAPHIC_SHRINKFIT", true);
        spec.insert(GRAPHIC_PROPSCALE, "GRAPHIC_PROPSCALE", true);
        return true;
    }
    bool dummy = RegisterGraphicStyles();
}


////////////////////////////////////////////////
// GG::StaticGraphic
////////////////////////////////////////////////
StaticGraphic::StaticGraphic() :
    Control(),
    m_style(GRAPHIC_NONE)
{}

StaticGraphic::StaticGraphic(X x, Y y, X w, Y h, const boost::shared_ptr<Texture>& texture, Flags<GraphicStyle> style/* = GRAPHIC_NONE*/,
                             Flags<WndFlag> flags/* = 0*/) :
    Control(x, y, w, h, flags),
    m_style(style)
{ Init(SubTexture(texture, X0, Y0, texture->DefaultWidth(), texture->DefaultHeight())); }

StaticGraphic::StaticGraphic(X x, Y y, X w, Y h, const SubTexture& subtexture, Flags<GraphicStyle> style/* = GRAPHIC_NONE*/,
                             Flags<WndFlag> flags/* = 0*/) :
    Control(x, y, w, h, flags),
    m_style(style)
{ Init(subtexture); }

Flags<GraphicStyle> StaticGraphic::Style() const
{ return m_style; }

Rect StaticGraphic::RenderedArea() const
{
    Pt ul = UpperLeft(), lr = LowerRight();
    Pt window_sz(lr - ul);
    Pt graphic_sz(m_graphic.Width(), m_graphic.Height());
    Pt pt1, pt2(graphic_sz); // (unscaled) default graphic size
    if (m_style & GRAPHIC_FITGRAPHIC) {
        if (m_style & GRAPHIC_PROPSCALE) {
            double scale_x = Value(window_sz.x) / static_cast<double>(Value(graphic_sz.x));
            double scale_y = Value(window_sz.y) / static_cast<double>(Value(graphic_sz.y));
            double scale = std::min(scale_x, scale_y);
            pt2.x = graphic_sz.x * scale;
            pt2.y = graphic_sz.y * scale;
        } else {
            pt2 = window_sz;
        }
    } else if (m_style & GRAPHIC_SHRINKFIT) {
        if (m_style & GRAPHIC_PROPSCALE) {
            double scale_x = (graphic_sz.x > window_sz.x) ? Value(window_sz.x) / static_cast<double>(Value(graphic_sz.x)) : 1.0;
            double scale_y = (graphic_sz.y > window_sz.y) ? Value(window_sz.y) / static_cast<double>(Value(graphic_sz.y)) : 1.0;
            double scale = std::min(scale_x, scale_y);
            pt2.x = graphic_sz.x * scale;
            pt2.y = graphic_sz.y * scale;
        } else {
            pt2 = window_sz;
        }
    }

    X x_shift(0);
    if (m_style & GRAPHIC_LEFT) {
        x_shift = ul.x;
    } else if (m_style & GRAPHIC_CENTER) {
        x_shift = ul.x + (window_sz.x - (pt2.x - pt1.x)) / 2;
    } else { // m_style & GRAPHIC_RIGHT
        x_shift = lr.x - (pt2.x - pt1.x);
    }
    pt1.x += x_shift;
    pt2.x += x_shift;

    Y y_shift(0);
    if (m_style & GRAPHIC_TOP) {
        y_shift = ul.y;
    } else if (m_style & GRAPHIC_VCENTER) {
        y_shift = ul.y + (window_sz.y - (pt2.y - pt1.y)) / 2;
    } else { // m_style & GRAPHIC_BOTTOM
        y_shift = lr.y - (pt2.y - pt1.y);
    }
    pt1.y += y_shift;
    pt2.y += y_shift;

    return Rect(pt1, pt2);
}

void StaticGraphic::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glColor(color_to_use);
    Rect rendered_area = RenderedArea();
    m_graphic.OrthoBlit(rendered_area.ul, rendered_area.lr);
}

void StaticGraphic::SetStyle(Flags<GraphicStyle> style)
{
    m_style = style;
    ValidateStyle();
}

void StaticGraphic::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    editor->Label("StaticGraphic");
    // TODO: handle setting image
    boost::shared_ptr<SetStyleAction> set_style_action(new SetStyleAction(this));
    editor->BeginFlags<GraphicStyle>(m_style, set_style_action);
    typedef std::vector<GraphicStyle> FlagVec;
    using boost::assign::list_of;
    editor->FlagGroup("V. Alignment", FlagVec() = list_of(GRAPHIC_TOP)(GRAPHIC_VCENTER)(GRAPHIC_BOTTOM));
    editor->FlagGroup("H. Alignment", FlagVec() = list_of(GRAPHIC_LEFT)(GRAPHIC_CENTER)(GRAPHIC_RIGHT));
    editor->Flag("Fit Graphic to Size", GRAPHIC_FITGRAPHIC);
    editor->Flag("Shrink-to-Fit", GRAPHIC_SHRINKFIT);
    editor->Flag("Proportional Scaling", GRAPHIC_PROPSCALE);
    editor->EndFlags();
}

void StaticGraphic::Init(const SubTexture& subtexture)
{
    ValidateStyle();  // correct any disagreements in the style flags
    SetColor(CLR_WHITE);
    m_graphic = subtexture;
}

void StaticGraphic::ValidateStyle()
{
    int dup_ct = 0;   // duplication count
    if (m_style & GRAPHIC_LEFT) ++dup_ct;
    if (m_style & GRAPHIC_RIGHT) ++dup_ct;
    if (m_style & GRAPHIC_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GRAPHIC_CENTER by default
        m_style &= ~(GRAPHIC_RIGHT | GRAPHIC_LEFT);
        m_style |= GRAPHIC_CENTER;
    }
    dup_ct = 0;
    if (m_style & GRAPHIC_TOP) ++dup_ct;
    if (m_style & GRAPHIC_BOTTOM) ++dup_ct;
    if (m_style & GRAPHIC_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GRAPHIC_VCENTER by default
        m_style &= ~(GRAPHIC_TOP | GRAPHIC_BOTTOM);
        m_style |= GRAPHIC_VCENTER;
    }
    dup_ct = 0;
    if (m_style & GRAPHIC_FITGRAPHIC) ++dup_ct;
    if (m_style & GRAPHIC_SHRINKFIT) ++dup_ct;
    if (dup_ct > 1) {   // mo more than one may be picked; when both are picked, use GRAPHIC_SHRINKFIT by default
        m_style &= ~GRAPHIC_FITGRAPHIC;
        m_style |= GRAPHIC_SHRINKFIT;
    }
}
