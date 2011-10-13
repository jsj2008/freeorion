// -*- C++ -*-
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

/** \file ThreeButtonDlg.h \brief Contains the standard modal
    user-input/-notification dialog. */

#ifndef _GG_ThreeButtonDlg_h_
#define _GG_ThreeButtonDlg_h_

#include <GG/Wnd.h>


namespace GG {

class Button;
class Font;

/** \brief A general pop-up message or user input box with one, two, or three
    buttons.

    This is designed to be used as a generic message window, with just an "ok"
    button, or for any input consisting of only two or three choices, such as
    "yes" and "no", "abort", "retry", and "fail", etc.  The enter key can be
    pressed to select the default button; the first button is always the
    default, unless the user sets a different one via SetDefaultButton().
    Similarly, the escape key can be pressed to select the button that will
    get the user out of the dialog without taking any action, if one exists;
    the last button is always the escape button, unless a different one is set
    via SetEscapeButton().  Note that this means that in a one-button dialog
    both enter and escape do the same thing.  The default labels for the
    buttons depends on the number of buttons.  For a one-button dialog, the
    default label is "ok"; for a two-button dialog, the default labels are
    "ok" and "cancel"; and for a three-button dialog, the default labels are
    "yes", "no", and "cancel".*/
class GG_API ThreeButtonDlg : public Wnd
{
public:
    /** \name Structors */ ///@{
    /** Basic ctor*/
    ThreeButtonDlg(X x, Y y, X w, Y h, const std::string& msg, const boost::shared_ptr<Font>& font, Clr color, 
                   Clr border_color, Clr button_color, Clr text_color, std::size_t buttons, const std::string& zero = "", 
                   const std::string& one = "", const std::string& two = "");

    /** Ctor that automatically centers the dialog in the app's area*/
    ThreeButtonDlg(X w, Y h, const std::string& msg, const boost::shared_ptr<Font>& font, Clr color, 
                   Clr border_color, Clr button_color, Clr text_color, std::size_t buttons, const std::string& zero = "", 
                   const std::string& one = "", const std::string& two = "");
    //@}

    /** \name Accessors */ ///@{
    Clr         ButtonColor() const;   ///< returns the color of the buttons in the dialog
    std::size_t Result() const;        ///< returns 0, 1, or 2, depending on which buttoon was clicked
    std::size_t DefaultButton() const; ///< returns the number of the button that will be chosen by default if the user hits enter (NO_BUTTON if none)
    std::size_t EscapeButton() const;  ///< returns the number of the button that will be chosen by default if the user hits ESC (NO_BUTTON if none)
    //@}

    /** \name Mutators */ ///@{
    virtual void Render();
    virtual void KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys);

    void SetButtonColor(Clr color);       ///< sets the color used to render the dialog's buttons
    void SetDefaultButton(std::size_t i); ///< sets the number of the button that will be chosen by default if the user hits enter (NO_BUTTON to disable)
    void SetEscapeButton(std::size_t i);  ///< sets the number of the button that will be chosen by default if the user hits ESC (NO_BUTTON to disable)
    //@}

    static const std::size_t NO_BUTTON;

protected:
    /** \name Structors */ ///@{
    ThreeButtonDlg(); ///< default ctor
    //@}

private:
    std::size_t NumButtons() const;
    void Init(const std::string& msg, const boost::shared_ptr<Font>& font, std::size_t buttons,
              const std::string& zero = "", const std::string& one = "", const std::string& two = "");
    void ConnectSignals();
    void Button0Clicked();
    void Button1Clicked();
    void Button2Clicked();

    Clr         m_color;
    Clr         m_border_color;
    Clr         m_text_color;
    Clr         m_button_color;
    std::size_t m_default;
    std::size_t m_escape;
    std::size_t m_result;
    Button*     m_button_0;
    Button*     m_button_1;
    Button*     m_button_2;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::ThreeButtonDlg::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Wnd)
        & BOOST_SERIALIZATION_NVP(m_color)
        & BOOST_SERIALIZATION_NVP(m_border_color)
        & BOOST_SERIALIZATION_NVP(m_text_color)
        & BOOST_SERIALIZATION_NVP(m_button_color)
        & BOOST_SERIALIZATION_NVP(m_default)
        & BOOST_SERIALIZATION_NVP(m_escape)
        & BOOST_SERIALIZATION_NVP(m_result)
        & BOOST_SERIALIZATION_NVP(m_button_0)
        & BOOST_SERIALIZATION_NVP(m_button_1)
        & BOOST_SERIALIZATION_NVP(m_button_2);

    if (Archive::is_loading::value)
        ConnectSignals();
}

#endif // _GG_ThreeButtonDlg_h_
