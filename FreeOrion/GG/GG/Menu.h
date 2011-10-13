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

/** \file Menu.h \brief Contains the MenuItem class, which represents menu
    data; the MenuBar control class; and the PopupMenu class, which is used to
    provide immediate context menus. */

#ifndef _GG_Menu_h_
#define _GG_Menu_h_

#include <GG/ClrConstants.h>
#include <GG/Control.h>


namespace GG {

class Font;
class TextControl;

/** \brief Serves as a single menu entry in a GG::MenuBar or GG::PopupMenu.

    May include a submenu.  All legal item_IDs are positive (and so non-zero);
    any item_ID <= 0 is considered invalid.  Each MenuItem has a signal that
    is emmitted with its menu_ID member whenever it is selected. Such signals
    may be emitted even when the menu_ID is 0.  These signals allow each
    MenuItem to be attached directly to code that should be executed when that
    item is selected.  Note that the signal is not serialized.  The user must
    restore it after the MenuItem is reloaded. */
struct GG_API MenuItem
{
    /** \name Signal Types */ ///@{
    typedef boost::signal<void (int)> SelectedIDSignalType; ///< invokes the appropriate functor to handle the menu selection, and passes the ID assigned to the item
    typedef boost::signal<void ()>    SelectedSignalType;   ///< invokes the appropriate functor to handle the menu selection
    //@}

    /** \name Slot Types */ ///@{
    typedef SelectedIDSignalType::slot_type SelectedIDSlotType; ///< type of functor(s) invoked on a SelectedSignalType
    typedef SelectedSignalType::slot_type   SelectedSlotType;   ///< type of functor(s) invoked on a SelectedSignalType
    //@}

    /** \name Structors */ ///@{
    MenuItem(); ///< default ctor
    MenuItem(const std::string& str, int id, bool disable, bool check); ///< ctor

    /** Ctor that allows direct attachment of this item's signal to a "slot"
        function or functor */
    MenuItem(const std::string& str, int id, bool disable, bool check, const SelectedIDSlotType& slot);

    /** Ctor that allows direct attachment of this item's signal to a "slot"
        function or functor */
    MenuItem(const std::string& str, int id, bool disable, bool check, const SelectedSlotType& slot);

    /** Ctor that allows direct attachment of this item's signal to a "slot"
        member function of a specific object */
    template <class T1, class T2>
    MenuItem(const std::string& str, int id, bool disable, bool check, void (T1::* slot)(int), T2* obj);

    /** Ctor that allows direct attachment of this item's signal to a "slot"
        member function of a specific object */
    template <class T1, class T2>
    MenuItem(const std::string& str, int id, bool disable, bool check, void (T1::* slot)(), T2* obj);

    virtual ~MenuItem(); ///< virtual dtor
    //@}

    /** \name Accessors */ ///@{
    mutable boost::shared_ptr<SelectedIDSignalType> SelectedIDSignal; ///< the selected signal object for this MenuItem that conveys the selected menu item ID
    mutable boost::shared_ptr<SelectedSignalType>   SelectedSignal;   ///< the selected signal object for this MenuItem
    //@}

    std::string           label;      ///< text shown for this menu item
    int                   item_ID;    ///< ID number associated with this menu item
    bool                  disabled;   ///< set to true when this menu item is disabled
    bool                  checked;    ///< set to true when this menu item can be toggled, and is currently on
    std::vector<MenuItem> next_level; ///< submenu off of this menu item; may be emtpy

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


struct SetFontAction;
struct SetTextColorAction;

/** \brief A menu bar control providing "browse" updates to user navigation of
    the menu.

    Whenever a menu item is selected, a signal is emitted which includes the
    ID of the selected item.  It is recommended that the user attach each menu
    item to an appropriate function the will execute the actions associated
    with the menu item, rather than attaching all the items to a single slot
    which uses the int ID parameter to deduce the appropriate action.  The int
    ID parameter is best used when there are several menu items that should
    execute the same code with different parameters.  For instance, if a
    submenu contains a list of recently used files, each item that contains a
    filename might be attached to a Reopen(int) function, and the int can be
    used to determine which file from the list should be opened. If some
    action is to be taken as the user browses the menu items, such as
    displaying some visual cue to indicate the result of chosing a particular
    menu entry, you can attach a slot function to the BrowsedSignalType object
    returned by BrowsedSignal.  Whenever the mouse moves to a new menu item,
    this signal is emitted with the ID number of the item under the
    cursor.  */
class GG_API MenuBar : public Control
{
public:
    /** \name Signal Types */ ///@{
    typedef boost::signal<void (int)> BrowsedSignalType; ///< emits the ID of an item in the menu when the cursor moves over it
    //@}

    /** \name Structors */ ///@{
    /** Ctor.  Parameter \a m should contain the desired menu in its
        next_level member. */
    MenuBar(X x, Y y, X w, const boost::shared_ptr<Font>& font, Clr text_color = CLR_WHITE, Clr color = CLR_BLACK, Clr interior = CLR_SHADOW); ///< ctor
    MenuBar(X x, Y y, X w, const boost::shared_ptr<Font>& font, const MenuItem& m, Clr text_color = CLR_WHITE, Clr color = CLR_BLACK, Clr interior = CLR_SHADOW); ///< ctor that takes a MenuItem containing menus with which to populate the MenuBar
    //@}

    /** \name Accessors */ ///@{
    virtual Pt        MinUsableSize() const;

    const MenuItem&   AllMenus() const;                           ///< returns a const reference to the MenuItem that contains all the menus and their contents
    bool              ContainsMenu(const std::string& str) const; ///< returns true if there is a top-level menu in the MenuBar whose label is \a str
    std::size_t       NumMenus() const;                           ///< returns the number of top-level menus in the MenuBar

    /** returns a const reference to the top-level menu in the MenuBar whose label is \a str.  \note No check is made to ensure such a menu exists. */
    const MenuItem&   GetMenu(const std::string& str) const;

    const MenuItem&   GetMenu(std::size_t n) const;///< returns a const reference to the \a nth menu in the MenuBar; not range-checked

    Clr               BorderColor() const;       ///< returns the color used to render the border of the control
    Clr               InteriorColor() const;     ///< returns the color used to render the interior of the control
    Clr               TextColor() const;         ///< returns the color used to render menu item text
    Clr               HiliteColor() const;       ///< returns the color used to indicate a hilited menu item
    Clr               SelectedTextColor() const; ///< returns the color used to render a hilited menu item's text

    mutable BrowsedSignalType BrowsedSignal; ///< the browsed signal object for this PopupMenu
    //@}

    /** \name Mutators */ ///@{
    virtual void   Render();
    virtual void   LButtonDown(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void   MouseHere(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void   MouseLeave();

    virtual void   SizeMove(const Pt& ul, const Pt& lr);

    MenuItem&      AllMenus();                    ///< returns a reference to the MenuItem that contains all the menus and their contents

    /** Returns a reference to the top-level menu in the MenuBar whose label
        is \a str.  \note No check is made to ensure such a menu exists. */
    MenuItem&      GetMenu(const std::string& str);

    MenuItem&      GetMenu(int n);                ///< returns a reference to the \a nth menu in the MenuBar; not range-checked
    void           AddMenu(const MenuItem& menu); ///< adds \a menu to the end of the top level of menus

    void           SetBorderColor(Clr clr);       ///< sets the color used to render the border of the control
    void           SetInteriorColor(Clr clr);     ///< sets the color used to render the interior of the control
    void           SetTextColor(Clr clr);         ///< sets the color used to render menu item text
    void           SetHiliteColor(Clr clr);       ///< sets the color used to indicate a hilited menu item
    void           SetSelectedTextColor(Clr clr); ///< sets the color used to render a hilited menu item's text

    virtual void   DefineAttributes(WndEditor* editor);
    //@}

    static const std::size_t INVALID_CARET;

protected:
    /** \name Structors */ ///@{
    MenuBar(); ///< default ctor
    //@}

    /** \name Accessors */ ///@{
    const boost::shared_ptr<Font>&   GetFont() const;    ///< returns the font used to render text in the control
    const std::vector<TextControl*>& MenuLabels() const; ///< returns the text for each top-level menu item
    std::size_t                      Caret() const;      ///< returns the current position of the caret
    //@}

private:
    /** Determines the rects in m_menu_labels, and puts the menus in multiple
        rows if they will not fit in one */
    void AdjustLayout(bool reset = false);

    boost::shared_ptr<Font>   m_font;           ///< the font used to render the text in the control
    Clr                       m_border_color;   ///< the color of the menu's border
    Clr                       m_int_color;      ///< color painted into the client area of the control
    Clr                       m_text_color;     ///< color used to paint text in control
    Clr                       m_hilite_color;   ///< color behind selected items
    Clr                       m_sel_text_color; ///< color of selected text

    MenuItem                  m_menu_data;      ///< this is not just a single menu item; the next_level element represents the entire menu
    std::vector<TextControl*> m_menu_labels;    ///< the text for each top-level menu item
    std::size_t               m_caret;          ///< the currently indicated top-level menu (open or under the cursor)

    friend struct SetFontAction;
    friend struct SetTextColorAction;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** \brief A modal pop-up menu.

    PopupMenu gives calling code the abiltiy to create a pop-up menu (usually
    in response to a right mouse click), allow the pop-up to execute, and then
    obtain an integer ID representing the selected menu item, by calling
    MenuID().  If no menu item has been selected, MenuID() returns 0.  Though
    every MenuItem in a PopupMenu may be attached to a slot directly, it is
    not recommended.  The intent of this class is to act as a tool to get
    immediate input from the user, inline.  However, attaching MenuItem
    signals directly to slots will work, and it will certainly be useful in
    some cases to do this.  Note also that there is no way to serialize a
    PopupMenu.  This is also because of the intent to use PopupMenus in an
    immediate, short-lived manner.  If you wish to save an often-used popup
    menu, simply create the MenuItem that the popup is based on, and save and
    load that.  Also, if some action is to be taken as the user browses the
    menu items, such as displaying some visual cue to indicate the result of
    chosing a particular menu entry, you can attach a slot function to the
    BrowsedSignalType object returned by BrowsedSignal.  Whenever the mouse
    moves to a new menu item, this signal is emitted with the ID number of the
    item under the cursor. */
class GG_API PopupMenu : public Wnd
{
public:
    /** \name Signal Types */ ///@{
    typedef boost::signal<void (int)> BrowsedSignalType; ///< emits the ID of an item in the menu when the cursor moves over it
    //@}

    /** \name Structors */ ///@{
    /** Ctor.  Parameter \a m should contain the desired menu in its
        next_level member. */
    PopupMenu(X x, Y y, const boost::shared_ptr<Font>& font, const MenuItem& m, Clr text_color = CLR_WHITE, Clr color = CLR_BLACK, Clr interior = CLR_SHADOW);
    //@}

    /** \name Accessors */ ///@{
    virtual Pt  ClientUpperLeft() const;

    int         MenuID() const;            ///< returns the integer ID of the menu item selected by the user, or 0 if none was selected
    Clr         BorderColor() const;       ///< returns the color used to render the border of the control
    Clr         InteriorColor() const;     ///< returns the color used to render the interior of the control
    Clr         TextColor() const;         ///< returns the color used to render menu item text
    Clr         HiliteColor() const;       ///< returns the color used to indicate a hilited menu item
    Clr         SelectedTextColor() const; ///< returns the color used to render a hilited menu item's text

    mutable BrowsedSignalType BrowsedSignal; ///< the browsed signal object for this PopupMenu
    //@}

    /** \name Mutators */ ///@{
    virtual void   Render();
    virtual void   LButtonUp(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void   LClick(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void   LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys);
    virtual void   RButtonUp(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void   RClick(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void   MouseHere(const Pt& pt, Flags<ModKey> mod_keys);

    virtual bool   Run();

    void           SetBorderColor(Clr clr);       ///< sets the color used to render the border of the control
    void           SetInteriorColor(Clr clr);     ///< sets the color used to render the interior of the control
    void           SetTextColor(Clr clr);         ///< sets the color used to render menu item text
    void           SetHiliteColor(Clr clr);       ///< sets the color used to indicate a hilited menu item
    void           SetSelectedTextColor(Clr clr); ///< sets the color used to render a hilited menu item's text
    //@}

    static const std::size_t INVALID_CARET;

protected:
    /** \name Accessors */ ///@{
    const boost::shared_ptr<Font>&  GetFont() const;      ///< returns the font used to render text in the control
    const MenuItem&                 MenuData() const;     ///< returns a const reference to the MenuItem that contains all the menu contents
    const std::vector<Rect>&        OpenLevels() const;   ///< returns the bounding rectangles for each open submenu, used to detect clicks in them
    const std::vector<std::size_t>& Caret() const;        ///< returns the stack representing the caret's location's path (eg 0th subitem of 1st subitem of item 3) back() is the most recent push
    const MenuItem*                 ItemSelected() const; ///< returns the menu item selected (0 if none)
    //@}

private:
    boost::shared_ptr<Font>
                      m_font;           ///< the font used to render the text in the control
    Clr               m_border_color;   ///< the color of the menu's border
    Clr               m_int_color;      ///< color painted into the client area of the control
    Clr               m_text_color;     ///< color used to paint text in control
    Clr               m_hilite_color;   ///< color behind selected items
    Clr               m_sel_text_color; ///< color of selected text

    MenuItem          m_menu_data;   ///< this is not just a single menu item; the next_level element represents the entire menu

    std::vector<Rect> m_open_levels; ///< bounding rectangles for each open submenu, used to detect clicks in them
    std::vector<std::size_t>
                      m_caret;       ///< stack representing the caret's location's path (eg 0th subitem of 1st subitem of item 3) back() is the most recent push

    const Pt          m_origin;         ///< the upper left hand corner of the control's visible area
    MenuItem*         m_item_selected;  ///< the menu item selected (0 if none)
};

} // namespace GG

// template implemetations
template <class T1, class T2>
GG::MenuItem::MenuItem(const std::string& str, int id, bool disable, bool check, void (T1::* slot)(int), T2* obj) :
    SelectedIDSignal(new SelectedIDSignalType()),
    SelectedSignal(new SelectedSignalType()),
    label(str), 
    item_ID(id), 
    disabled(disable), 
    checked(check)
{ SelectedIDSignal->connect(boost::bind(slot, obj, _1)); }

template <class T1, class T2>
GG::MenuItem::MenuItem(const std::string& str, int id, bool disable, bool check, void (T1::* slot)(), T2* obj) :
    SelectedIDSignal(new SelectedIDSignalType()),
    SelectedSignal(new SelectedSignalType()),
    label(str), 
    item_ID(id), 
    disabled(disable), 
    checked(check)
{ SelectedSignal->connect(boost::bind(slot, obj)); }

template <class Archive>
void GG::MenuItem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(label)
        & BOOST_SERIALIZATION_NVP(item_ID)
        & BOOST_SERIALIZATION_NVP(disabled)
        & BOOST_SERIALIZATION_NVP(checked)
        & BOOST_SERIALIZATION_NVP(next_level);
}

template <class Archive>
void GG::MenuBar::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_font)
        & BOOST_SERIALIZATION_NVP(m_border_color)
        & BOOST_SERIALIZATION_NVP(m_int_color)
        & BOOST_SERIALIZATION_NVP(m_text_color)
        & BOOST_SERIALIZATION_NVP(m_hilite_color)
        & BOOST_SERIALIZATION_NVP(m_sel_text_color)
        & BOOST_SERIALIZATION_NVP(m_menu_data)
        & BOOST_SERIALIZATION_NVP(m_menu_labels)
        & BOOST_SERIALIZATION_NVP(m_caret);
}

#endif // _GG_Menu_h_
