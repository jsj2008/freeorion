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

#include <GG/Button.h>

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/WndEditor.h>
#include <GG/WndEvent.h>

#include <boost/lexical_cast.hpp>


using namespace GG;

namespace {
    void ClickedEcho()
    { std::cerr << "GG SIGNAL : Button::ClickedSignal()\n"; }

    void CheckedEcho(bool checked)
    { std::cerr << "GG SIGNAL : StateButton::CheckedSignal(checked=" << checked << ")\n"; }

    void ButtonChangedEcho(std::size_t index)
    {
        std::cerr << "GG SIGNAL : RadioButtonGroup::ButtonChangedSignal(index="
                  << index << ")\n";
    }

    struct SetCheckedButtonAction : AttributeChangedAction<std::size_t>
    {
        SetCheckedButtonAction(RadioButtonGroup* radio_button_group) :
            m_radio_button_group(radio_button_group) {}
        void operator()(const std::size_t& button)
        {
            m_radio_button_group->SetCheck(RadioButtonGroup::NO_BUTTON);
            m_radio_button_group->SetCheck(button);
        }
        RadioButtonGroup* const m_radio_button_group;
    };
}

////////////////////////////////////////////////
// GG::Button
////////////////////////////////////////////////
Button::Button() :
    TextControl(),
    m_state(BN_UNPRESSED)
{}

Button::Button(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font, Clr color, 
               Clr text_color/* = CLR_BLACK*/, Flags<WndFlag> flags/* = INTERACTIVE*/) :
    TextControl(x, y, w, h, str, font, text_color, FORMAT_NONE, flags),
    m_state(BN_UNPRESSED)
{
    m_color = color;

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(ClickedSignal, &ClickedEcho);
}

Button::ButtonState Button::State() const
{ return m_state; }

const SubTexture& Button::UnpressedGraphic() const
{ return m_unpressed_graphic; }

const SubTexture& Button::PressedGraphic() const
{ return m_pressed_graphic; }

const SubTexture& Button::RolloverGraphic() const
{ return m_rollover_graphic; }

void Button::Render()
{
    switch (m_state) {
    case BN_PRESSED:   RenderPressed(); break;
    case BN_UNPRESSED: RenderUnpressed(); break;
    case BN_ROLLOVER:  RenderRollover(); break;
    }
}

void Button::SetColor(Clr c)
{ Control::SetColor(c); }

void Button::SetState(ButtonState state)
{ m_state = state; }

void Button::SetUnpressedGraphic(const SubTexture& st)
{ m_unpressed_graphic = st; }

void Button::SetPressedGraphic(const SubTexture& st)
{ m_pressed_graphic = st; }

void Button::SetRolloverGraphic(const SubTexture& st)
{ m_rollover_graphic = st; }

void Button::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    TextControl::DefineAttributes(editor);
    // TODO: handle setting graphics
}

void Button::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        ButtonState prev_state = m_state;
        m_state = BN_PRESSED;
        if (prev_state == BN_PRESSED && RepeatButtonDown())
            ClickedSignal();
    }
}

void Button::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = BN_PRESSED;
    Wnd::LDrag(pt, move, mod_keys);
}

void Button::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = BN_UNPRESSED;
}

void Button::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        m_state = BN_ROLLOVER;
        ClickedSignal();
    }
}

void Button::MouseHere(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = BN_ROLLOVER;
}

void Button::MouseLeave()
{
    if (!Disabled())
        m_state = BN_UNPRESSED;
}

void Button::RenderUnpressed()
{
    if (!m_unpressed_graphic.Empty()) {
        glColor(Disabled() ? DisabledColor(m_color) : m_color);
        m_unpressed_graphic.OrthoBlit(UpperLeft(), LowerRight());
    } else {
        RenderDefault();
    }
    // draw text shadow
    Clr temp = TextColor();  // save original color
    SetTextColor(CLR_SHADOW); // shadow color
    OffsetMove(Pt(X(2), Y(2)));
    TextControl::Render();
    OffsetMove(Pt(X(-2), Y(-2)));
    SetTextColor(temp);    // restore original color
    // draw text
    TextControl::Render();
}

void Button::RenderPressed()
{
    if (!m_pressed_graphic.Empty()) {
        glColor(Disabled() ? DisabledColor(m_color) : m_color);
        m_pressed_graphic.OrthoBlit(UpperLeft(), LowerRight());
    } else {
        RenderDefault();
    }
    OffsetMove(Pt(X1, Y1));
    TextControl::Render();
    OffsetMove(Pt(-X1, -Y1));
}

void Button::RenderRollover()
{
    if (!m_rollover_graphic.Empty()) {
        glColor(Disabled() ? DisabledColor(m_color) : m_color);
        m_rollover_graphic.OrthoBlit(UpperLeft(), LowerRight());
    } else {
        RenderDefault();
    }
    // draw text shadow
    Clr temp = TextColor();  // save original color
    SetTextColor(CLR_SHADOW); // shadow color
    OffsetMove(Pt(X(2), Y(2)));
    TextControl::Render();
    OffsetMove(Pt(X(-2), Y(-2)));
    SetTextColor(temp);    // restore original color
    // draw text
    TextControl::Render();
}

void Button::RenderDefault()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    BeveledRectangle(ul, lr,
                     Disabled() ? DisabledColor(m_color) : m_color,
                     Disabled() ? DisabledColor(m_color) : m_color,
                     (m_state != BN_PRESSED), 1);
}


////////////////////////////////////////////////
// GG::StateButton
////////////////////////////////////////////////
StateButton::StateButton() :
    TextControl(),
    m_checked(false),
    m_style(SBSTYLE_3D_XBOX)
{}

StateButton::StateButton(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font, Flags<TextFormat> format, 
                         Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, StateButtonStyle style/* = SBSTYLE_3D_XBOX*/,
                         Flags<WndFlag> flags/* = INTERACTIVE*/) :
    TextControl(x, y, w, h, str, font, text_color, format, flags),
    m_checked(false),
    m_int_color(interior),
    m_style(style)
{
    m_color = color;
    SetDefaultButtonPosition();

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(CheckedSignal, &CheckedEcho);
}

Pt StateButton::MinUsableSize() const
{
    Pt text_lr = m_text_ul + TextControl::MinUsableSize();
    return Pt(std::max(m_button_lr.x, text_lr.x) - std::min(m_button_ul.x, m_text_ul.x),
              std::max(m_button_lr.y, text_lr.y) - std::min(m_button_ul.y, m_text_ul.y));
}

bool StateButton::Checked() const
{ return m_checked; }

Clr StateButton::InteriorColor() const
{ return m_int_color; }

StateButtonStyle StateButton::Style() const
{ return m_style; }

void StateButton::Render()
{
    const int BEVEL = 2;

    // draw button
    Pt cl_ul = ClientUpperLeft();
    Pt cl_lr = ClientLowerRight();
    Pt bn_ul = cl_ul + m_button_ul;
    Pt bn_lr = cl_ul + m_button_lr;

    Pt additional_text_offset;

    const Pt DOUBLE_BEVEL(X(2 * BEVEL), Y(2 * BEVEL));

    switch (m_style) {
    case SBSTYLE_3D_XBOX:
        BeveledRectangle(bn_ul, bn_lr,
                         Disabled() ? DisabledColor(m_int_color) : m_int_color,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         false, BEVEL);
        if (m_checked)
            BeveledX(bn_ul + DOUBLE_BEVEL, bn_lr - DOUBLE_BEVEL,
                     m_disabled ? DisabledColor(m_color) : m_color);
        break;
    case SBSTYLE_3D_CHECKBOX:
        BeveledRectangle(bn_ul, bn_lr,
                         Disabled() ? DisabledColor(m_int_color) : m_int_color,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         false, BEVEL);
        if (m_checked)
            BeveledCheck(bn_ul + DOUBLE_BEVEL, bn_lr - DOUBLE_BEVEL,
                         Disabled() ? DisabledColor(m_color) : m_color);
        break;
    case SBSTYLE_3D_RADIO:
        BeveledCircle(bn_ul, bn_lr,
                      Disabled() ? DisabledColor(m_int_color) : m_int_color,
                      Disabled() ? DisabledColor(m_color) : m_color,
                      false, BEVEL);
        if (m_checked)
            Bubble(bn_ul + DOUBLE_BEVEL, bn_lr - DOUBLE_BEVEL,
                   Disabled() ? DisabledColor(m_color) : m_color);
        break;
    case SBSTYLE_3D_BUTTON:
        BeveledRectangle(bn_ul, bn_lr,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         !m_checked, BEVEL);
        break;
    case SBSTYLE_3D_ROUND_BUTTON:
        BeveledCircle(bn_ul, bn_lr,
                      Disabled() ? DisabledColor(m_color) : m_color,
                      Disabled() ? DisabledColor(m_color) : Color(),
                      !m_checked, BEVEL);
        break;
    case SBSTYLE_3D_TOP_ATTACHED_TAB: {
        Clr color_to_use = m_checked ? m_color : DarkColor(m_color);
        color_to_use = Disabled() ? DisabledColor(color_to_use) : color_to_use;
        if (!m_checked) {
            cl_ul.y += BEVEL;
            additional_text_offset.y = Y(BEVEL / 2);
        }
        BeveledRectangle(cl_ul, cl_lr,
                         color_to_use, color_to_use,
                         true, BEVEL,
                         true, true, true, false);
        break;
    }
    case SBSTYLE_3D_TOP_DETACHED_TAB: {
        Clr color_to_use = m_checked ? m_color : DarkColor(m_color);
        color_to_use = Disabled() ? DisabledColor(color_to_use) : color_to_use;
        if (!m_checked) {
            cl_ul.y += BEVEL;
            additional_text_offset.y = Y(BEVEL / 2);
        }
        BeveledRectangle(cl_ul, cl_lr,
                         color_to_use, color_to_use,
                         true, BEVEL);
        break;
    }
    }

    OffsetMove(m_text_ul + additional_text_offset);
    TextControl::Render();
    OffsetMove(-(m_text_ul + additional_text_offset));
}

void StateButton::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        SetCheck(!m_checked);
        CheckedSignal(m_checked);
    }
}

void StateButton::SizeMove(const Pt& ul, const Pt& lr)
{
    RepositionButton();
    TextControl::SizeMove(ul, lr);
}

void StateButton::Reset()
{ SetCheck(false); }

void StateButton::SetCheck(bool b/* = true*/)
{ m_checked = b; }

void StateButton::RepositionButton()
{
    if (m_style == SBSTYLE_3D_TOP_ATTACHED_TAB ||
        m_style == SBSTYLE_3D_TOP_DETACHED_TAB) {
        m_button_ul = Pt();
        m_button_lr = Pt();
        m_text_ul = Pt();
    } else {
        X w = Width();
        Y h = Height();
        const X BN_W = m_button_lr.x - m_button_ul.x;
        const Y BN_H = m_button_lr.y - m_button_ul.y;
        X bn_x = m_button_ul.x;
        Y bn_y = m_button_ul.y;
        Flags<TextFormat> format = GetTextFormat();
        Flags<TextFormat> original_format = format;
        const double SPACING = 0.5; // the space to leave between the button and text, as a factor of the button's size (width or height)
        if (format & FORMAT_VCENTER)       // center button vertically
            bn_y = (h - BN_H) / 2.0 + 0.5;
        if (format & FORMAT_TOP) {         // put button at top, text just below
            bn_y = Y0;
            m_text_ul.y = BN_H;
        }
        if (format & FORMAT_BOTTOM) {      // put button at bottom, text just above
            bn_y = (h - BN_H);
            m_text_ul.y = h - (BN_H * (1 + SPACING)) - (static_cast<int>(GetLineData().size() - 1) * GetFont()->Lineskip() + GetFont()->Height()) + 0.5;
        }

        if (format & FORMAT_CENTER) {      // center button horizontally
            if (format & FORMAT_VCENTER) { // if both the button and the text are to be centered, bad things happen
                format |= FORMAT_LEFT;     // so go to the default (FORMAT_CENTER|FORMAT_LEFT)
                format &= ~FORMAT_CENTER;
            } else {
                bn_x = (w - bn_x) / 2.0 - BN_W / 2.0 + 0.5;
            }
        }
        if (format & FORMAT_LEFT) {        // put button at left, text just to the right
            bn_x = X0;
            if (format & FORMAT_VCENTER)
                m_text_ul.x = BN_W * (1 + SPACING) + 0.5;
        }
        if (format & FORMAT_RIGHT) {       // put button at right, text just to the left
            bn_x = (w - BN_W);
            if (format & FORMAT_VCENTER)
                m_text_ul.x = -BN_W * (1 + SPACING) + 0.5;
        }
        if (format != original_format)
            SetTextFormat(format);
        m_button_ul = Pt(bn_x, bn_y);
        m_button_lr = m_button_ul + Pt(BN_W, BN_H);
    }
}

void StateButton::SetButtonPosition(const Pt& ul, const Pt& lr)
{
    X bn_x = ul.x;
    Y bn_y = ul.y;
    X bn_w = lr.x - ul.x;
    Y bn_h = lr.y - ul.y;

    if (bn_w <= 0 || bn_h <= 0) {              // if one of these is invalid,
        bn_w = X(GetFont()->PointSize()); // set button width and height to text height
        bn_h = Y(GetFont()->PointSize());
    }

    if (bn_x == -1 || bn_y == -1) {
        m_button_ul = Pt(X0, Y0);
        m_button_lr = Pt(bn_w, bn_h);
        RepositionButton();
    } else {
        m_button_ul = Pt(bn_x, bn_y);
        m_button_lr = m_button_ul + Pt(bn_w, bn_h);
    }
}

void StateButton::SetDefaultButtonPosition()
{ SetButtonPosition(Pt(-X1, -Y1), Pt(-X1, -Y1)); }

void StateButton::SetColor(Clr c)
{ Control::SetColor(c); }

void StateButton::SetInteriorColor(Clr c)
{ m_int_color = c; }

void StateButton::SetStyle(StateButtonStyle bs)
{ m_style = bs; }

void StateButton::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    TextControl::DefineAttributes(editor);
    editor->Label("StateButton");
    editor->Attribute("Checked", m_checked);
    editor->Attribute("Interior Color", m_int_color);
    editor->Attribute("Button Style", m_style,
                      SBSTYLE_3D_XBOX, SBSTYLE_3D_ROUND_BUTTON);
    editor->Attribute("Button Upper Left", m_button_ul);
    editor->Attribute("Button Lower Right", m_button_lr);
    editor->Attribute("Text Upper Left", m_text_ul);
}

Pt StateButton::ButtonUpperLeft() const
{ return m_button_ul; }

Pt StateButton::ButtonLowerRight() const
{ return m_button_lr; }

Pt StateButton::TextUpperLeft() const
{ return m_text_ul; }


////////////////////////////////////////////////
// GG::RadioButtonGroup
////////////////////////////////////////////////
// ButtonClickedFunctor
RadioButtonGroup::ButtonClickedFunctor::ButtonClickedFunctor(RadioButtonGroup* group, StateButton* button, std::size_t index) :
    m_group(group),
    m_button(button),
    m_index(index)
{}

void RadioButtonGroup::ButtonClickedFunctor::operator()(bool checked)
{
    if (checked)
        m_group->SetCheckImpl(m_index, true);
    else
        m_button->SetCheck(true);
}

// ButtonSlot
RadioButtonGroup::ButtonSlot::ButtonSlot() :
    button(0)
{}

RadioButtonGroup::ButtonSlot::ButtonSlot(StateButton* button_) :
    button(button_)
{}

// RadioButtonGroup
// static(s)
const std::size_t RadioButtonGroup::NO_BUTTON = std::numeric_limits<std::size_t>::max();

RadioButtonGroup::RadioButtonGroup() :
    Control(),
    m_orientation(VERTICAL),
    m_checked_button(NO_BUTTON),
    m_expand_buttons(false),
    m_expand_buttons_proportionally(false),
    m_render_outline(false)
{ SetColor(CLR_YELLOW); }

RadioButtonGroup::RadioButtonGroup(X x, Y y, X w, Y h, Orientation orientation) :
    Control(x, y, w, h),
    m_orientation(orientation),
    m_checked_button(NO_BUTTON),
    m_expand_buttons(false),
    m_expand_buttons_proportionally(false),
    m_render_outline(false)
{
    SetColor(CLR_YELLOW);

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(ButtonChangedSignal, &ButtonChangedEcho);
}

Pt RadioButtonGroup::MinUsableSize() const
{
    Pt retval;
    for (std::size_t i = 0; i < m_button_slots.size(); ++i) {
        Pt min_usable_size = m_button_slots[i].button->MinUsableSize();
        if (m_orientation == VERTICAL) {
            retval.x = std::max(retval.x, min_usable_size.x);
            retval.y += min_usable_size.y;
        } else {
            retval.x += min_usable_size.x;
            retval.y = std::max(retval.y, min_usable_size.y);
        }
    }
    return retval;
}

Orientation RadioButtonGroup::GetOrientation() const
{ return m_orientation; }

bool RadioButtonGroup::Empty() const
{ return m_button_slots.empty(); }

std::size_t RadioButtonGroup::NumButtons() const
{ return m_button_slots.size(); }

std::size_t RadioButtonGroup::CheckedButton() const
{ return m_checked_button; }

bool RadioButtonGroup::ExpandButtons() const
{ return m_expand_buttons; }

bool RadioButtonGroup::ExpandButtonsProportionally() const
{ return m_expand_buttons_proportionally; }

bool RadioButtonGroup::RenderOutline() const
{ return m_render_outline; }

void RadioButtonGroup::RaiseCheckedButton()
{
    if (m_checked_button != NO_BUTTON)
        MoveChildUp(m_button_slots[m_checked_button].button);
}

void RadioButtonGroup::Render()
{
    if (m_render_outline) {
        Pt ul = UpperLeft(), lr = LowerRight();
        Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        FlatRectangle(ul, lr, CLR_ZERO, color_to_use, 1);
    }
}

void RadioButtonGroup::SetCheck(std::size_t index)
{
    if (m_button_slots.size() <= index)
        index = NO_BUTTON;
    SetCheckImpl(index, false);
}

void RadioButtonGroup::DisableButton(std::size_t index, bool b/* = true*/)
{
    if (index < m_button_slots.size()) {
        bool was_disabled = m_button_slots[index].button->Disabled();
        m_button_slots[index].button->Disable(b);
        if (b && !was_disabled && index == m_checked_button)
            SetCheck(NO_BUTTON);
    }
}

void RadioButtonGroup::AddButton(StateButton* bn)
{ InsertButton(m_button_slots.size(), bn); }

void RadioButtonGroup::AddButton(const std::string& text, const boost::shared_ptr<Font>& font, Flags<TextFormat> format,
                                 Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                 StateButtonStyle style/* = SBSTYLE_3D_RADIO*/)
{ InsertButton(m_button_slots.size(), text, font, format, color, text_color, interior, style); }

void RadioButtonGroup::InsertButton(std::size_t index, StateButton* bn)
{
    assert(index <= m_button_slots.size());
    if (!m_expand_buttons) {
        Pt min_usable_size = bn->MinUsableSize();
        bn->Resize(Pt(std::max(bn->Width(), min_usable_size.x), std::max(bn->Height(), min_usable_size.y)));
    }
    Pt bn_sz = bn->Size();
    Layout* layout = GetLayout();
    if (!layout) {
        layout = new Layout(X0, Y0, ClientWidth(), ClientHeight(), 1, 1);
        SetLayout(layout);
    }
    const int CELLS_PER_BUTTON = m_expand_buttons ? 1 : 2;
    const int X_STRETCH = (m_expand_buttons && m_expand_buttons_proportionally) ? Value(bn_sz.x) : 1;
    const int Y_STRETCH = (m_expand_buttons && m_expand_buttons_proportionally) ? Value(bn_sz.y) : 1;
    if (m_button_slots.empty()) {
        layout->Add(bn, 0, 0);
        if (m_expand_buttons) {
            if (m_orientation == VERTICAL)
                layout->SetRowStretch(0, Y_STRETCH);
            else
                layout->SetColumnStretch(0, X_STRETCH);
        }
    } else {
        if (m_orientation == VERTICAL) {
            layout->ResizeLayout(layout->Rows() + CELLS_PER_BUTTON, 1);
            layout->SetRowStretch(layout->Rows() - CELLS_PER_BUTTON, Y_STRETCH);
        } else {
            layout->ResizeLayout(1, layout->Columns() + CELLS_PER_BUTTON);
            layout->SetColumnStretch(layout->Columns() - CELLS_PER_BUTTON, X_STRETCH);
        }
        for (std::size_t i = m_button_slots.size() - 1; index <= i; --i) {
            layout->Remove(m_button_slots[i].button);
            layout->Add(m_button_slots[i].button,
                        m_orientation == VERTICAL ? i * CELLS_PER_BUTTON + CELLS_PER_BUTTON : 0,
                        m_orientation == VERTICAL ? 0 : i * CELLS_PER_BUTTON + CELLS_PER_BUTTON);
            if (m_orientation == VERTICAL)
                layout->SetMinimumRowHeight(i * CELLS_PER_BUTTON + CELLS_PER_BUTTON, layout->MinimumRowHeight(i * CELLS_PER_BUTTON));
            else
                layout->SetMinimumColumnWidth(i * CELLS_PER_BUTTON + CELLS_PER_BUTTON, layout->MinimumColumnWidth(i * CELLS_PER_BUTTON));
        }
        layout->Add(bn, m_orientation == VERTICAL ? index * CELLS_PER_BUTTON : 0, m_orientation == VERTICAL ? 0 : index * CELLS_PER_BUTTON);
    }
    if (m_orientation == VERTICAL)
        layout->SetMinimumRowHeight(index * CELLS_PER_BUTTON, bn_sz.y);
    else
        layout->SetMinimumColumnWidth(index * CELLS_PER_BUTTON, bn_sz.x);
    m_button_slots.insert(m_button_slots.begin() + index, ButtonSlot(bn));

    if (m_checked_button != NO_BUTTON && index <= m_checked_button)
        ++m_checked_button;
    Reconnect();
}

void RadioButtonGroup::InsertButton(std::size_t index, const std::string& text, const boost::shared_ptr<Font>& font, Flags<TextFormat> format,
                                    Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                    StateButtonStyle style/* = SBSTYLE_3D_RADIO*/)
{
    assert(index <= m_button_slots.size());
    StateButton* button = GetStyleFactory()->NewStateButton(X0, Y0, X1, Y1, text, font, format, color, text_color, interior, style);
    button->Resize(button->MinUsableSize());
    InsertButton(index, button);
}

void RadioButtonGroup::RemoveButton(StateButton* button)
{
    std::size_t index = NO_BUTTON;
    for (std::size_t i = 0; i < m_button_slots.size(); ++i) {
        if (m_button_slots[i].button == button) {
            index = i;
            break;
        }
    }
    assert(index < m_button_slots.size());

    const int CELLS_PER_BUTTON = m_expand_buttons ? 1 : 2;
    Layout* layout = GetLayout();
    layout->Remove(m_button_slots[index].button);
    for (std::size_t i = index + 1; i < m_button_slots.size(); ++i) {
        layout->Remove(m_button_slots[i].button);
        if (m_orientation == VERTICAL) {
            layout->Add(m_button_slots[i].button, i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, 0);
            layout->SetRowStretch(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->RowStretch(i * CELLS_PER_BUTTON));
            layout->SetMinimumRowHeight(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->MinimumRowHeight(i * CELLS_PER_BUTTON));
        } else {
            layout->Add(m_button_slots[i].button, 0, i * CELLS_PER_BUTTON - CELLS_PER_BUTTON);
            layout->SetColumnStretch(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->ColumnStretch(i * CELLS_PER_BUTTON));
            layout->SetMinimumColumnWidth(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->MinimumColumnWidth(i * CELLS_PER_BUTTON));
        }
    }
    m_button_slots[index].connection.disconnect();
    m_button_slots.erase(m_button_slots.begin() + index);
    if (m_button_slots.empty()) {
        layout->ResizeLayout(1, 1);
    } else {
        if (m_orientation == VERTICAL)
            layout->ResizeLayout(layout->Rows() - CELLS_PER_BUTTON, 1);
        else
            layout->ResizeLayout(1, layout->Columns() - CELLS_PER_BUTTON);
    }

    if (index == m_checked_button)
        m_checked_button = NO_BUTTON;
    else if (index <= m_checked_button)
        --m_checked_button;
    Reconnect();
}

void RadioButtonGroup::ExpandButtons(bool expand)
{
    if (expand != m_expand_buttons) {
        std::size_t old_checked_button = m_checked_button;
        std::vector<StateButton*> buttons(m_button_slots.size());
        while (!m_button_slots.empty()) {
            StateButton* button = m_button_slots.back().button;
            buttons[m_button_slots.size() - 1] = button;
            RemoveButton(button);
        }
        m_expand_buttons = expand;
        for (std::size_t i = 0; i < buttons.size(); ++i) {
            AddButton(buttons[i]);
        }
        SetCheck(old_checked_button);
    }
}

void RadioButtonGroup::ExpandButtonsProportionally(bool proportional)
{
    if (proportional != m_expand_buttons_proportionally) {
        std::size_t old_checked_button = m_checked_button;
        std::vector<StateButton*> buttons(m_button_slots.size());
        while (!m_button_slots.empty()) {
            StateButton* button = m_button_slots.back().button;
            buttons[m_button_slots.size() - 1] = button;
            RemoveButton(button);
        }
        m_expand_buttons_proportionally = proportional;
        for (std::size_t i = 0; i < buttons.size(); ++i) {
            AddButton(buttons[i]);
        }
        SetCheck(old_checked_button);
    }
}

void RadioButtonGroup::RenderOutline(bool render_outline)
{ m_render_outline = render_outline; }

void RadioButtonGroup::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    editor->Label("RadioButtonGroup");
    boost::shared_ptr<SetCheckedButtonAction> set_checked_button_action(new SetCheckedButtonAction(this));
    editor->Attribute<std::size_t>("Checked Button", m_checked_button, set_checked_button_action);
}

const std::vector<RadioButtonGroup::ButtonSlot>& RadioButtonGroup::ButtonSlots() const
{ return m_button_slots; }

void RadioButtonGroup::ConnectSignals()
{
    for (std::size_t i = 0; i < m_button_slots.size(); ++i) {
        m_button_slots[i].connection =
            Connect(m_button_slots[i].button->CheckedSignal,
                    ButtonClickedFunctor(this, m_button_slots[i].button, i));
    }
    SetCheck(m_checked_button);
}

void RadioButtonGroup::SetCheckImpl(std::size_t index, bool signal)
{
    assert(m_checked_button == NO_BUTTON || m_checked_button < m_button_slots.size());
    if (m_checked_button != NO_BUTTON)
        m_button_slots[m_checked_button].button->SetCheck(false);
    if (index != NO_BUTTON)
        m_button_slots[index].button->SetCheck(true);
    m_checked_button = index;
    if (signal)
        ButtonChangedSignal(m_checked_button);
}

void RadioButtonGroup::Reconnect()
{
    for (std::size_t i = 0; i < m_button_slots.size(); ++i) {
        m_button_slots[i].connection.disconnect();
    }
    ConnectSignals();
}
