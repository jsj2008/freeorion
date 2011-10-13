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

/** \file ListBox.h \brief Contains the ListBox class, a control that contains
    rows of other controls, commonly TextControls. */

#ifndef _GG_ListBox_h_
#define _GG_ListBox_h_

#include <GG/AlignmentFlags.h>
#include <GG/ClrConstants.h>
#include <GG/Control.h>
#include <GG/Timer.h>

#include <set>

#include <boost/serialization/version.hpp>


namespace GG {

class Font;
class Scroll;
class SubTexture;
class WndEvent;

/** Styles for ListBox controls. */
GG_FLAG_TYPE(ListBoxStyle);
extern GG_API const ListBoxStyle LIST_NONE;           ///< Default style selected.
extern GG_API const ListBoxStyle LIST_VCENTER;        ///< Cells are aligned with the top of the list box control.
extern GG_API const ListBoxStyle LIST_TOP;            ///< Cells are aligned with the top of the list box control. This is the default.
extern GG_API const ListBoxStyle LIST_BOTTOM;         ///< Cells are aligned with the bottom of the list box control.
extern GG_API const ListBoxStyle LIST_CENTER;         ///< Cells are center-aligned.
extern GG_API const ListBoxStyle LIST_LEFT;           ///< Cells are left-aligned. This is the default.
extern GG_API const ListBoxStyle LIST_RIGHT;          ///< Cells are right-aligned.
extern GG_API const ListBoxStyle LIST_NOSORT;         ///< List items are not sorted. Items are sorted by default.  When used with drag-and-drop, this style allows arbitrary rearrangement of list elements by dragging.
extern GG_API const ListBoxStyle LIST_SORTDESCENDING; ///< Items are sorted based on item text in descending order. Ascending order is the default.
extern GG_API const ListBoxStyle LIST_NOSEL;          ///< No selection, dragging, or dropping allowed.  This makes the list box effectively read-only.
extern GG_API const ListBoxStyle LIST_SINGLESEL;      ///< Only one item at a time can be selected. By default, multiple items may be selected.
extern GG_API const ListBoxStyle LIST_QUICKSEL;       ///< Each click toggles an item without affecting any others; ignored when used with LIST_SINGLESEL.
extern GG_API const ListBoxStyle LIST_USERDELETE;     ///< Allows user to remove selected items by pressing the delete key.
extern GG_API const ListBoxStyle LIST_BROWSEUPDATES;  ///< Causes a signal to be emitted whenever the mouse moves over ("browses") a row.


/** \brief A flexible control that can contain rows and columns of other
    controls, even other ListBoxes.

    A ListBox consists of rows of controls, usually text or graphics.  Each
    row represents one item; rows can be added or removed, but not columns or
    individual controls (though the individual controls can be removed from a
    row by accessing it directly).  Each Row in a ListBox must have the same
    number of cells and the same cell widths as all the others.  If you add a
    row that has fewer cells than the ListBox you are adding it to, it will be
    padded with empty cells; likewise, if it has too many cells to fit into
    the Listbox, it will have cells removed.  ListBoxes are designed to be
    easy to use in common cases, and useful in uncommon cases with only a
    little work.  Adding a row to an empty ListBox will cause the ListBox to
    take on the number of columns that the row has cells, and each column will
    have an equal portion of the ListBox's width (any remainder is placed in
    the last column).  This allows you to just add rows to a ListBox without
    worrying about setting up the ListBox in any way ahead of time.  Use
    LockColWidths() to prevent empty ListBoxes from taking on a new row's
    number of columns.  To create a ListBox with user-defined widths, use the
    ctor designed for that, or call SetNumCols(), set individual widths with
    SetColWidth(), and lock the column widths with LockColWidths().

    <br>Note that Rows are stored by pointer.  If you want to move a Row from
    one ListBox to another, use GetRow() and Insert().

    <br>Note that drag-and-drop support is a key part of ListBox's
    functionality.  As such, special effort has been made to make its use as
    natural and flexible as possible.  This includes allowing arbitrary
    reordering of ListBox rows when the LIST_NOSORT is in effect, and includes
    the use of the DontAcceptDrop exception.  The DontAcceptDrop exception can
    be thrown by any client of the ListBox in response to its
    DropAcceptableSignal.  Such a throw will cause the drop to be refused.
    Note that a DropAcceptableSignal is emitted for each row dropped into the
    ListBox, so individual rows may be accepted or rejected from a single
    multi-row drop. */
class GG_API ListBox : public Control
{
public:
    /** \brief Sorts iterators to ListBox::Row*s from a container of
        ListBox::Row*s.

        For instance for use in a std::map<> or std::set<> (eg,
        ListBox::SelectionSet).  The iterators must refer to pointers to
        ListBox::Rows that are laid out vertically (as in a ListBox).  This
        layout is used to define a y-ordering that is used to sort the
        iterators. */
    template <class Cont>
    struct RowPtrIteratorLess
    {
        typedef typename Cont::iterator Iter;
        RowPtrIteratorLess();
        RowPtrIteratorLess(Cont* c);
        bool operator()(Iter lhs, Iter rhs) const;
        static bool LessThan(Iter lhs, Iter rhs, Iter end);
    private:
        Cont* m_container;
    };

    /** \brief A single item in a listbox.

        A Row is primarily a container for Controls.  Each cell in a Row
        contains pointer to a Control-derived object.  As always, each such
        Control can be connected to arbitrary functionality using signals and
        slots.  During dragging and dropping, the data type associated with a
        Row (DragDropDataType()) indicates to potential drop targets what type
        of data the Row represents; the target may accept or decline the drop
        based on the data type.  Rows are stored in ListBoxes by reference,
        not value; this means that you can subclass from Row to create your
        own custom Row types.  This is the recommended method for associating
        a row with the non-GUI object that it represents.  Note that all
        subclasses of Row must declare a SortKeyType, if it differs from
        std::string, and must provide a SortKey() method if it should differ
        from the default SortKey() that Row provides.  Note that SortKey is
        not virtual; this part of its interface is used for compile-time
        polymorphism -- whatever sorter is used with a Row subclass must know
        the most-derived type of the Row subclass.  \note The margin, column
        alignment, and width cell data are included so that each Row has all
        the necessary information with which to render itself (this is
        primarily needed to facilitate drag-and-drop); these data are
        duplicates of the margin, alignment, and column widths data found in
        the owning ListBox, and may be overwritten by the ListBox at any
        time. */
    struct GG_API Row : public Control
    {
        /** Allows multiple mutators on a Row to be called, with only one call
            to the sometimes-expensive Row::AdjustLayout made at the end.
            Declare a DeferAdjustLayout on the stack before calling Row
            mutators, and at the end of scope, when the DeferAdjustLayout is
            destructed, Row::AdjustLayout is called. */
        struct DeferAdjustLayout
        {
            DeferAdjustLayout(Row* row);
            ~DeferAdjustLayout();
            Row* const m_row;
        };

        /** the type of key used to sort rows */
        typedef std::string SortKeyType;

        /** \name Structors */ ///@{
        Row(); ///< default ctor
        Row(X w, Y h, const std::string& drag_drop_data_type, Alignment align = ALIGN_VCENTER, unsigned int margin = 2); ///< ctor
        virtual ~Row();
        //@}

        /** \name Accessors */ ///@{
        SortKeyType  SortKey(std::size_t column) const; ///< returns the string by which this row may be sorted
        std::size_t  size() const;               ///< returns the number of Controls in this Row
        bool         empty() const;              ///< returns true iff there are 0 Controls in this Row

        Control*     operator[](std::size_t n) const; ///< returns the Control in the \a nth cell of this Row; not range checked
        Control*     at(std::size_t n) const;    ///< returns the Control in the \a nth cell of this Row \throw std::range_error throws when size() <= \a n

        Alignment    RowAlignment() const;       ///< returns the vertical alignment of this Row
        Alignment    ColAlignment(std::size_t n) const; ///< returns the horizontal alignment of the Control in the \a nth cell of this Row; not range checked
        X            ColWidth(std::size_t n) const; ///< returns the width of the \a nth cell of this Row; not range checked
        unsigned int Margin() const;             ///< returns the amount of space left between the contents of adjacent cells, in pixels

        Control*     CreateControl(const std::string& str, const boost::shared_ptr<Font>& font, Clr color) const; ///< creates a "shrink-fit" TextControl from text, font, and color parameters
        Control*     CreateControl(const SubTexture& st) const; ///< creates a "shrink-fit" StaticGraphic Control from a SubTexture parameter
        //@}

        /** \name Mutators */ ///@{
        virtual void Render();

        void         push_back(Control* c); ///< adds a given Control to the end of the Row; this Control becomes property of the Row
        void         push_back(const std::string& str, const boost::shared_ptr<Font>& font, Clr color = CLR_BLACK); ///< overload of push_back that creates a TextControl and adds it to the Row
        void         push_back(const std::string& str, const std::string& font_filename, unsigned int pts, Clr color = CLR_BLACK); ///< overload of push_back that creates a TextControl and adds it to the Row
        void         push_back(const SubTexture& st); ///< overload of push_back that creates a StaticGraphic Control and adds it to the Row
        void         clear(); ///< removes and deletes all cells in this Row
        void         resize(std::size_t n); ///< resizes the Row to have \a n cells

        void         SetCell(std::size_t n, Control* c); ///< sets the Control in the \a nth cell of this Row, deleting any preexisting Control; not range checked
        Control*     RemoveCell(std::size_t n); ///< returns a pointer to the Control in the \a nth cell of this Row, and sets the contents of the cell to 0; not range checked
        void         SetRowAlignment(Alignment align); ///< sets the vertical alignment of this Row
        void         SetColAlignment(std::size_t n, Alignment align); ///< sets the horizontal alignment of the Control in the \a nth cell of this Row; not range checked
        void         SetColWidth(std::size_t n, X width); ///< sets the width of the \a nth cell of this Row; not range checked
        void         SetColAlignments(const std::vector<Alignment>& aligns); ///< sets the horizontal alignment of all the Controls in this Row; not range checked
        void         SetColWidths(const std::vector<X>& widths); ///< sets all the widths of the cells of this Row; not range checked
        void         SetMargin(unsigned int margin); ///< sets the amount of space left between the contents of adjacent cells, in pixels
        //@}

    private:
        void AdjustLayout(bool adjust_for_push_back = false);

        std::vector<Control*>  m_cells;          ///< the Controls in this Row (each may be null)
        Alignment              m_row_alignment;  ///< row alignment; one of ALIGN_TOP, ALIGN_VCENTER, or ALIGN_BOTTOM
        std::vector<Alignment> m_col_alignments; ///< column alignments; each is one of ALIGN_TOP, ALIGN_VCENTER, or ALIGN_BOTTOM
        std::vector<X>         m_col_widths;     ///< column widths
        unsigned int           m_margin;         ///< the amount of space left between the contents of adjacent cells, in pixels

        bool                   m_ignore_adjust_layout;

        friend class DeferAdjustLayout;

        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    typedef std::list<Row*>::iterator iterator;
    typedef std::list<Row*>::const_iterator const_iterator;
    typedef std::list<Row*>::reverse_iterator reverse_iterator;
    typedef std::list<Row*>::const_reverse_iterator const_reverse_iterator;

    typedef std::set<iterator, RowPtrIteratorLess<std::list<Row*> > > SelectionSet;

    /** \name Signal Types */ ///@{
    typedef boost::signal<void ()>                    ClearedSignalType;    ///< emitted when the list box is cleared
    typedef boost::signal<void (const SelectionSet&)> SelChangedSignalType; ///< emitted when one or more rows are selected or deselected
    typedef boost::signal<void (iterator)>            RowSignalType;        ///< the signature of row-change-notification signals
    typedef boost::signal<void (const_iterator)>      ConstRowSignalType;   ///< the signature of const row-change-notification signals
    typedef boost::signal<void (iterator, const Pt&)> RowClickSignalType;   ///< the signature of row-click-notification signals

    typedef RowSignalType      InsertedSignalType;       ///< emitted when a row is inserted into the list box
    typedef RowSignalType      DroppedSignalType;        ///< emitted when a row is inserted into the list box via drag-and-drop
    typedef ConstRowSignalType DropAcceptableSignalType; ///< emitted when a row may be inserted into the list box via drag-and-drop
    typedef RowClickSignalType LeftClickedSignalType;    ///< emitted when a row in the listbox is left-clicked; provides the row left-clicked and the clicked point
    typedef RowClickSignalType RightClickedSignalType;   ///< emitted when a row in the listbox is right-clicked; provides the row right-clicked and the clicked point
    typedef RowSignalType      DoubleClickedSignalType;  ///< emitted when a row in the listbox is left-double-clicked
    typedef RowSignalType      ErasedSignalType;         ///< emitted when a row in the listbox is erased; provides the deleted Row, and is emitted before the row is removed
    typedef RowSignalType      BrowsedSignalType;        ///< emitted when a row in the listbox is "browsed" (rolled over) by the cursor; provides the browsed row
    //@}

    /** \name Structors */ ///@{
    /** basic ctor */
    ListBox(X x, Y y, X w, Y h, Clr color, Clr interior = CLR_ZERO, Flags<WndFlag> flags = INTERACTIVE);

    virtual ~ListBox(); ///< virtual dtor
    //@}

    /** \name Accessors */ ///@{
    virtual void    DropsAcceptable(DropsAcceptableIter first,
                                    DropsAcceptableIter last,
                                    const Pt& pt) const;

    virtual Pt      MinUsableSize() const;
    virtual Pt      ClientUpperLeft() const;
    virtual Pt      ClientLowerRight() const;

    bool            Empty() const;          ///< returns true when the ListBox is empty
    const_iterator  begin() const;          ///< returns an iterator to the first list row
    const_iterator  end() const;            ///< returns an iterator to the imaginary row one past the last
    const_reverse_iterator
                    rbegin() const;         ///< returns an iterator to the last list row
    const_reverse_iterator
                    rend() const;           ///< returns an iterator to the imaginary row one past the first one
    const Row&      GetRow(std::size_t n) const; ///< returns a const reference to the row at index \a n; not range-checked.  \note This function is O(n).
    iterator        Caret() const;          ///< returns the row that has the caret
    const SelectionSet&
                    Selections() const;     ///< returns a const reference to the set row indexes that is currently selected
    bool            Selected(iterator it) const; ///< returns true if row \a it is selected
    Clr             InteriorColor() const;  ///< returns the color painted into the client area of the control
    Clr             HiliteColor() const;    ///< returns the color behind selected line items

    /** Returns the style flags of the listbox \see GG::ListBoxStyle */
    Flags<ListBoxStyle> Style() const;

    const Row&      ColHeaders() const;     ///< returns the row containing the headings for the columns, if any.  If undefined, the returned heading Row will have size() 0.
    iterator        FirstRowShown() const;  ///< returns the first row visible in the listbox
    std::size_t     FirstColShown() const;  ///< returns the index of the first column visible in the listbox

    iterator        LastVisibleRow() const; ///< returns the last row that could be drawn, taking into account the contents and the size of client area
    std::size_t     LastVisibleCol() const; ///< returns the index of the last column that could be drawn, taking into account the contents and the size of client area

    std::size_t     NumRows() const;        ///< returns the total number of rows in the ListBox
    std::size_t     NumCols() const;        ///< returns the total number of columns in the ListBox

    /** Returns true iff column widths are fixed \see LockColWidths() */
    bool            KeepColWidths() const;

    /** Returns the index of the column used to sort rows, when sorting is
        enabled.  \note The sort column is not range checked when it is set by
        the user; it may be < 0 or >= NumCols(). */
    std::size_t     SortCol() const;

    X               ColWidth(std::size_t n) const;     ///< returns the width of column \a n in pixels; not range-checked
    Alignment       ColAlignment(std::size_t n) const; ///< returns the alignment of column \a n; must be ALIGN_LEFT, ALIGN_CENTER, or ALIGN_RIGHT; not range-checked
    Alignment       RowAlignment(iterator it) const;   ///< returns the alignment of row \a it; must be ALIGN_TOP, ALIGN_VCENTER, or ALIGN_BOTTOM; not range-checked

    /** Returns the set of data types allowed to be dropped over this ListBox
        when drag-and-drop is enabled. \note If this set contains "", all drop
        types are allowed. */
    const std::set<std::string>& AllowedDropTypes() const;

    /** Whether the list should autoscroll when the user is attempting to drop
        an item into a location that is not currently visible. */
    bool            AutoScrollDuringDragDrops() const;

    /** The thickness of the area around the border of the client area that will
        provoke an auto-scroll, if AutoScrollDuringDragDrops() returns true. */
    unsigned int    AutoScrollMargin() const;

    /** The number of milliseconds that elapse between row/column scrolls when
        auto-scrolling. */
    unsigned int    AutoScrollInterval() const;

    /** The cleared signal object for this ListBox.  Note that this signal is
        never emitted by the ListBox.  It is provided here for convenience, to
        provide users of a ListBox with a canonical signal with which to
        communicate about ListBox clears. */
    mutable ClearedSignalType        ClearedSignal;

    /** The inserted signal object for this ListBox.  Note that this signal is
        never emitted by the ListBox.  It is provided here for convenience, to
        provide users of a ListBox with a canonical signal with which to
        communicate about ListBox insertions. */
    mutable InsertedSignalType       InsertedSignal;

    mutable SelChangedSignalType     SelChangedSignal;     ///< the selection change signal object for this ListBox
    mutable DroppedSignalType        DroppedSignal;        ///< the dropped signal object for this ListBox
    mutable DropAcceptableSignalType DropAcceptableSignal; ///< the drop-acceptability signal object for this ListBox
    mutable LeftClickedSignalType    LeftClickedSignal;    ///< the left click signal object for this ListBox
    mutable RightClickedSignalType   RightClickedSignal;   ///< the right click signal object for this ListBox
    mutable DoubleClickedSignalType  DoubleClickedSignal;  ///< the double click signal object for this ListBox
    mutable ErasedSignalType         ErasedSignal;         ///< the erased signal object for this ListBox
    mutable BrowsedSignalType        BrowsedSignal;        ///< the browsed signal object for this ListBox
    //@}

    /** \name Mutators */ ///@{
    virtual void   StartingChildDragDrop(const Wnd* wnd, const GG::Pt& offset);
    virtual void   AcceptDrops(const std::vector<Wnd*>& wnds, const Pt& pt);
    virtual void   ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination);
    virtual void   Render();

    virtual void   SizeMove(const Pt& ul, const Pt& lr);  ///< resizes the control, then resizes the scrollbars as needed

    virtual void   Disable(bool b = true);
    virtual void   SetColor(Clr c);

    /** Insertion sorts \a row into the ListBox if sorted, or inserts into an
        unsorted ListBox before \a it; returns insertion point.  This Row
        becomes the property of the ListBox. */
    iterator       Insert(Row* row, iterator it);

    /** Insertion sorts \a row into the ListBox if sorted, or inserts into an
        unsorted ListBox at the end of the list; returns insertion point.
        This Row becomes the property of the ListBox. */
    iterator       Insert(Row* row);

    Row*           Erase(iterator it);                    ///< removes and returns the row that \a it points to from the ListBox, or 0 if no such row exists
    void           Clear();                               ///< empties the ListBox
    void           SelectRow(iterator it);                ///< selects row \a it
    void           DeselectRow(iterator it);              ///< deselects row \a it
    void           SelectAll();                           ///< selects all rows
    void           DeselectAll();                         ///< deselects all rows
    iterator       begin();                               ///< returns an iterator to the first list row
    iterator       end();                                 ///< returns an iterator to the imaginary row one past the last one
    reverse_iterator
                   rbegin();                              ///< returns an iterator to the last list row
    reverse_iterator
                   rend();                                ///< returns an iterator to the imaginary row one past the first one
    Row&           GetRow(std::size_t n);                 ///< returns a reference to the Row at row index \a n; not range-checked.  \note This function is O(n).

    void           SetSelections(const SelectionSet& s);  ///< sets the set of selected rows to \a s
    void           SetCaret(iterator it);                 ///< sets the position of the caret to \a it
    void           BringRowIntoView(iterator it);         ///< moves the scrollbars so that row \a it is visible

    void           SetInteriorColor(Clr c);               ///< sets the color painted into the client area of the control
    void           SetHiliteColor(Clr c);                 ///< sets the color behind selected line items

    /** sets the style flags for the ListBox to \a s. \see GG::ListBoxStyle */
    void           SetStyle(Flags<ListBoxStyle> s);

    void           SetColHeaders(Row* r);                 ///< sets the row used as headings for the columns; this Row becomes property of the ListBox.
    void           RemoveColHeaders();                    ///< removes any columns headings set

    void           SetColWidth(std::size_t n, X w);  ///< sets the width of column \n to \a w; not range-checked
    void           SetNumCols(std::size_t n);             ///< sets the number of columns in the ListBox to \a n; if no column widths exist before this call, proportional widths are calulated and set, otherwise no column widths are set
    void           SetSortCol(std::size_t n);             ///< sets the index of the column used to sort rows when sorting is enabled; not range-checked

    /** Sets the comparison function used to sort a given pair of Rows during
        row sorting.  Note that \a sort_cmp is assumed to produce an ascending
        order when used to sort; setting the LIST_SORTDESCENDING style can be
        used to produce a reverse sort. */
    void           SetSortCmp(const boost::function<bool (const Row&, const Row&, std::size_t)>& sort_cmp);

    /** Fixes the column widths; by default, an empty ListBox will take on the
        number of columns of its first added row. \note The number of columns
        and their widths may still be set via SetNumCols() and SetColWidth()
        after this function has been called. */
    void           LockColWidths();

    /** Allows the number of columns to be determined by the first row added to
        an empty ListBox */
    void           UnLockColWidths();

    /** Sets the alignment of column \a n to \a align; not range-checked */
    void           SetColAlignment(std::size_t n, Alignment align);

    /** Sets the alignment of row \a it to \a align; not range-checked */
    void           SetRowAlignment(iterator it, Alignment align);

    /** Allows Rows with data type \a str to be dropped over this ListBox when
        drag-and-drop is enabled. \note Passing "" enables all drop types. */
    void           AllowDropType(const std::string& str);

    /** Disallows Rows with data type \a str to be dropped over this ListBox
        when drag-and-drop is enabled. \note If "" is still an allowed drop
        type, drops of type \a str will still be allowed, even after disallowed
        with a call to this function. */
    void           DisallowDropType(const std::string& str);

    /** Set this to determine whether the list should autoscroll when the user
        is attempting to drop an item into a location that is not currently
        visible. */
    void           AutoScrollDuringDragDrops(bool auto_scroll);

    /** Sets the thickness of the area around the border of the client area that
        will provoke an auto-scroll, if AutoScrollDuringDragDrops() returns
        true. */
    void           SetAutoScrollMargin(unsigned int margin);

    /** Sets the number of milliseconds that elapse between row/column scrolls
        when auto-scrolling. */
    void           SetAutoScrollInterval(unsigned int interval);

    virtual void   DefineAttributes(WndEditor* editor);
    //@}

    /** \brief Sorts two Rows of a ListBox using operator<() on the
        Row::SortKeyType provided by the rows' SortKey() methods.

        If you want to use operator<() with a Row subclass DerivedRow that has
        a custom SortKeyType, use DefaultRowCmp<DerivedRow>. */
    template <class RowType>
    struct DefaultRowCmp
    {
        /** Returns true iff lhs.SortKey( \a column ) < rhs.SortKey( \a column ). */
        bool operator()(const Row& lhs, const Row& rhs, std::size_t column) const;
    };

    static const unsigned int BORDER_THICK; ///< the thickness with which to render the border of the control

    /** \name Exceptions */ ///@{
    /** The base class for ListBox exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown by a ListBox that does not wish to accept a potential drop, for
        whatever reason. This may be throw by anyone -- even in client code
        activated by a DropAcceptableSignal. */
    GG_CONCRETE_EXCEPTION(DontAcceptDrop, GG::ListBox, Exception);
    //@}

protected:
    /** \name Structors */ ///@{
    ListBox(); ///< default ctor
    //@}

    /** \name Accessors */ ///@{
    X               RightMargin() const;     ///< space skipped at right of client area for vertical scroll bar
    Y               BottomMargin() const;    ///< space skipped at bottom of client area for horizontal scroll bar
    unsigned int    CellMargin() const;      ///< the number of pixels left between the contents of each cell and the cell boundary

    iterator        RowUnderPt(const Pt& pt) const; ///< returns row under pt, if any; value must be checked (i.e. it may be end())

    iterator        OldSelRow() const;   ///< returns the last row that was selected with a left-button mouse-down
    iterator        OldRDownRow() const; ///< returns the last row that was selected with a right-button mouse-down
    iterator        LClickRow() const;   ///< returns the last row that was left-clicked
    iterator        RClickRow() const;   ///< returns the last row that was right-clicked

    bool            AutoScrollingUp() const;    ///< returns true iff the list is being autoscrolled up due to drag-and-drop
    bool            AutoScrollingDown() const;  ///< returns true iff the list is being autoscrolled down due to drag-and-drop
    bool            AutoScrollingLeft() const;  ///< returns true iff the list is being autoscrolled left due to drag-and-drop
    bool            AutoScrollingRight() const; ///< returns true iff the list is being autoscrolled right due to drag-and-drop
    //@}

    /** \name Mutators */ ///@{
    virtual void    KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys);
    virtual void    MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys);
    virtual void    DragDropEnter(const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys);
    virtual void    DragDropHere(const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys);
    virtual void    DragDropLeave();
    virtual void    TimerFiring(unsigned int ticks, Timer* timer);

    virtual bool    EventFilter(Wnd* w, const WndEvent& event);

    iterator        Insert(Row* row, iterator it, bool dropped);  ///< insertion sorts into list, or inserts into an unsorted list before \a it; returns insertion point
    Row*            Erase(iterator it, bool removing_duplicate, bool signal); ///< erases the row at index \a idx, handling it as a duplicate removal (such as for drag-and-drops within a single ListBox) if indicated
    void            BringCaretIntoView();           ///< makes sure caret is visible when scrolling occurs due to keystrokes etc.
    void            RecreateScrolls();              ///< recreates the vertical and horizontal scrolls as needed.
    void            ResetAutoScrollVars();          ///< resets all variables related to auto-scroll to their initial values
    void            Resort();                       ///< performs a full resort of all rows, using the current sort functor.
    //@}

private:
    void            ConnectSignals();
    void            ValidateStyle(); ///< reconciles inconsistencies in the style flags
    void            AdjustScrolls(bool adjust_for_resize); ///< creates, destroys, or resizes scrolls to reflect size of data in listbox
    void            VScrolled(int tab_low, int tab_high, int low, int high); ///< signals from the vertical scroll bar are caught here
    void            HScrolled(int tab_low, int tab_high, int low, int high); ///< signals from the horizontal scroll bar are caught here
    void            ClickAtRow(iterator it, Flags<ModKey> mod_keys); ///< handles to a mouse-click or spacebar-click on \a it, modified by \a keys
    void            NormalizeRow(Row* row); ///< adjusts a Row so that it has the same number of cells as other rows, and that each cell has the correct width and alignment
    iterator        FirstRowShownWhenBottomIs(iterator bottom_row, Y client_height); ///< Returns the first row shown when the last row shown is \a bottom_row
    std::size_t     FirstColShownWhenRightIs(std::size_t right_col, X client_width); ///< Returns the index of the first column shown when the last column shown is \a right_col

    std::list<Row*> m_rows;             ///< line item data

    Scroll*         m_vscroll;          ///< vertical scroll bar on right
    Scroll*         m_hscroll;          ///< horizontal scroll bar at bottom
    iterator        m_caret;            ///< the item currently selected, or the last item selected by the user 
    SelectionSet    m_selections;       ///< vector of indexes of selected items
    iterator        m_old_sel_row;      ///< used to make sure clicks end on the same row where they started
    bool            m_old_sel_row_selected; ///< set to true if m_old_sel_row was selected at the point at which it was designated
    iterator        m_old_rdown_row;    ///< the row that most recently recieved a right button down message
    iterator        m_lclick_row;       ///< the row most recently left-clicked
    iterator        m_rclick_row;       ///< the row most recently right-clicked
    iterator        m_last_row_browsed; ///< the last row sent out as having been browsed (used to prevent duplicate browse signals)

    iterator        m_first_row_shown;  ///< index of row at top of visible area (always begin() for non-empty ListBox with LIST_NOSCROLL set)
    std::size_t     m_first_col_shown;  ///< like above, but index of column at left
    std::vector<X>  m_col_widths;       ///< the width of each of the columns goes here
    std::vector<Alignment> 
                    m_col_alignments;   ///< the horizontal alignment of each of the columns goes here
    unsigned int    m_cell_margin;      ///< the amount of space left between each edge of the cell and its contents, in pixels

    Clr             m_int_color;        ///< color painted into the client area of the control
    Clr             m_hilite_color;     ///< color behind selected line items
    Flags<ListBoxStyle>
                    m_style;            ///< composed of ListBoxStyles enums (see GUIBase.h)

    Row*            m_header_row;       ///< row of header text/graphics
    bool            m_keep_col_widths;  ///< should we keep the column widths, once set?
    bool            m_clip_cells;       ///< if true, the contents of each cell will be clipped to the visible area of that cell (TODO: currently unused)
    std::size_t     m_sort_col;         ///< the index of the column data used to sort the list
    boost::function<bool (const Row&, const Row&, std::size_t)>
                    m_sort_cmp;         ///< the predicate used to sort the values in the m_sort_col column of two rows
    std::set<std::string>
                    m_allowed_drop_types;///< the line item types allowed for use in this listbox

    bool            m_auto_scroll_during_drag_drops;
    unsigned int    m_auto_scroll_margin;
    bool            m_auto_scrolling_up;
    bool            m_auto_scrolling_down;
    bool            m_auto_scrolling_left;
    bool            m_auto_scrolling_right;
    Timer           m_auto_scroll_timer;

    iterator*       m_iterator_being_erased;

    friend class DropDownList; ///< allow complete access to DropDownList, which relies on ListBox to do its rendering

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG


// template implementations
template <class Cont>
GG::ListBox::RowPtrIteratorLess<Cont>::RowPtrIteratorLess() :
    m_container()
{ assert(m_container); }

template <class Cont>
GG::ListBox::RowPtrIteratorLess<Cont>::RowPtrIteratorLess(Cont* c) :
    m_container(c)
{ assert(m_container); }

template <class Cont>
bool GG::ListBox::RowPtrIteratorLess<Cont>::operator()(Iter lhs, Iter rhs) const
{
    // If you've seen an error message that lead you here, it is because you
    // are attempting to use RowPtrIteratorLess to sort a type that is not
    // iterators-to-pointers-to-ListBox::Rows!
    BOOST_MPL_ASSERT((boost::is_same<typename Iter::value_type, ::GG::ListBox::Row*>));
    return LessThan(lhs, rhs, m_container->end());
}

template <class Cont>
bool GG::ListBox::RowPtrIteratorLess<Cont>::LessThan(Iter lhs, Iter rhs, Iter end)
{
    return lhs == end ?
        false :
        (rhs == end ?
         true : (*lhs)->UpperLeft().y < (*rhs)->UpperLeft().y);
}

template <class Archive>
void GG::ListBox::Row::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_cells)
        & BOOST_SERIALIZATION_NVP(m_row_alignment)
        & BOOST_SERIALIZATION_NVP(m_col_alignments)
        & BOOST_SERIALIZATION_NVP(m_col_widths)
        & BOOST_SERIALIZATION_NVP(m_margin);
}

template <class RowType>
bool GG::ListBox::DefaultRowCmp<RowType>::operator()(const GG::ListBox::Row& lhs, const GG::ListBox::Row& rhs, std::size_t column) const
{
    return static_cast<const RowType&>(lhs).SortKey(column) < static_cast<const RowType&>(rhs).SortKey(column);
}

template <class Archive>
void GG::ListBox::serialize(Archive& ar, const unsigned int version)
{
    std::size_t caret_index = std::distance(m_rows.begin(), m_caret);
    std::size_t old_sel_row_index = std::distance(m_rows.begin(), m_old_sel_row);
    std::size_t old_rdown_row_index = std::distance(m_rows.begin(), m_old_rdown_row);
    std::size_t lclick_row_index = std::distance(m_rows.begin(), m_lclick_row);
    std::size_t rclick_row_index = std::distance(m_rows.begin(), m_rclick_row);
    std::size_t last_row_browsed_index = std::distance(m_rows.begin(), m_last_row_browsed);
    std::size_t first_row_shown_index = std::distance(m_rows.begin(), m_first_row_shown);
    std::set<std::size_t> selection_indices;

    if (Archive::is_saving::value) {
        for (SelectionSet::iterator it = m_selections.begin(); it != m_selections.end(); ++it) {
            selection_indices.insert(std::distance(m_rows.begin(), *it));
        }
    }

    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_rows)
        & BOOST_SERIALIZATION_NVP(m_vscroll)
        & BOOST_SERIALIZATION_NVP(m_hscroll)
        & BOOST_SERIALIZATION_NVP(caret_index)
        & BOOST_SERIALIZATION_NVP(selection_indices)
        & BOOST_SERIALIZATION_NVP(old_sel_row_index)
        & BOOST_SERIALIZATION_NVP(m_old_sel_row_selected)
        & BOOST_SERIALIZATION_NVP(old_rdown_row_index)
        & BOOST_SERIALIZATION_NVP(lclick_row_index)
        & BOOST_SERIALIZATION_NVP(rclick_row_index)
        & BOOST_SERIALIZATION_NVP(last_row_browsed_index)
        & BOOST_SERIALIZATION_NVP(first_row_shown_index)
        & BOOST_SERIALIZATION_NVP(m_first_col_shown)
        & BOOST_SERIALIZATION_NVP(m_col_widths)
        & BOOST_SERIALIZATION_NVP(m_col_alignments)
        & BOOST_SERIALIZATION_NVP(m_cell_margin)
        & BOOST_SERIALIZATION_NVP(m_int_color)
        & BOOST_SERIALIZATION_NVP(m_hilite_color)
        & BOOST_SERIALIZATION_NVP(m_style)
        & BOOST_SERIALIZATION_NVP(m_header_row)
        & BOOST_SERIALIZATION_NVP(m_keep_col_widths)
        & BOOST_SERIALIZATION_NVP(m_clip_cells)
        & BOOST_SERIALIZATION_NVP(m_sort_col)
        & BOOST_SERIALIZATION_NVP(m_allowed_drop_types)
        & BOOST_SERIALIZATION_NVP(m_auto_scroll_during_drag_drops)
        & BOOST_SERIALIZATION_NVP(m_auto_scroll_margin)
        & BOOST_SERIALIZATION_NVP(m_auto_scrolling_up)
        & BOOST_SERIALIZATION_NVP(m_auto_scrolling_down)
        & BOOST_SERIALIZATION_NVP(m_auto_scrolling_left)
        & BOOST_SERIALIZATION_NVP(m_auto_scrolling_right)
        & BOOST_SERIALIZATION_NVP(m_auto_scroll_timer);

    if (Archive::is_loading::value) {
        m_caret = boost::next(m_rows.begin(), caret_index);
        m_old_sel_row = boost::next(m_rows.begin(), old_sel_row_index);
        m_old_rdown_row = boost::next(m_rows.begin(), old_rdown_row_index);
        m_lclick_row = boost::next(m_rows.begin(), lclick_row_index);
        m_rclick_row = boost::next(m_rows.begin(), rclick_row_index);
        m_last_row_browsed = boost::next(m_rows.begin(), last_row_browsed_index);
        m_first_row_shown = boost::next(m_rows.begin(), first_row_shown_index);

        for (std::set<std::size_t>::iterator it = selection_indices.begin(); it != selection_indices.end(); ++it) {
            m_selections.insert(boost::next(m_rows.begin(), *it));
        }

        ValidateStyle();
        ConnectSignals();
    }
}

#endif // _GG_ListBox_h_
