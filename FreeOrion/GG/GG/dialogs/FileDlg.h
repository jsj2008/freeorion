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

/** \file FileDlg.h \brief Contains the standard GG file dialog. */

#ifndef _GG_FileDlg_h_
#define _GG_FileDlg_h_

#include <GG/DropDownList.h>

#include <boost/filesystem/path.hpp>


namespace GG {

class TextControl;
class Edit;
class Button;
class Font;

/** \brief The default file open/save dialog box.

    This dialog, like all the common GG dialogs, is modal.  It asks the user
    for one or more filenames, which the caller may retrieve with a call to
    Result() after the dialog is closed.  Note that all strings displayed
    during the run of the FileDlg are customizable.  Sometimes, the FileDlg
    will pop up a message box (a ThreeButtonDlg) and notify the user of
    something or ask for input.  These message strings are also customizable.
    Some of these strings include the filename as part of the message.  When
    replacing these strings with your own, you need to include the placement
    of the filename in the message with the character sequence "%1%" (see
    boost.format for details). */
class GG_API FileDlg : public Wnd
{
public:
    /** \name Structors */ ///@{
    /** Basic ctor.  Parameters \a directory and \a filename pass an initial
        directory and filename to the dialog, if desired (such as when "Save
        As..." is selected in an app, and there is a current filename).  If \a
        directory is specified, it is taken as-is if it is absolute, or
        relative to boost::filesystem::initial_path() if it is relative.  If
        \a directory is "", the initial directory is WorkingDirectory().  \a
        save indicates whether this is a save or load dialog; \a multi
        indicates whether multiple file selections are allowed.  \throw
        GG::FileDlg::BadInitialDirectory Throws when \a directory is
        invalid. */
    FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi, const boost::shared_ptr<Font>& font,
            Clr color, Clr border_color, Clr text_color = CLR_BLACK);
    //@}

    /** \name Accessors */ ///@{
    std::set<std::string> Result() const; ///< returns a set of strings that contains the files chosen by the user; there will be only one file if \a multi == false was passed to the ctor

    /** Returns true iff this FileDlg will select directories instead of files. */
    bool SelectDirectories() const;

    /** Returns true iff this FileDlg will append the missing extension to a
        file when in save mode.  Note that action is only taken if there is a
        single file filter containing exactly one wildcard in its first
        position (i.e. it is of the form "*foo").  If precondition is
        satisfied, any filename the user selects that does not end in "foo"
        will have "foo" appended to it. */
    bool AppendMissingSaveExtension() const;

    const std::string& FilesString() const;                ///< returns the text label next to the files edit box to \a str Default: "File(s):"
    const std::string& FileTypesString() const;            ///< returns the text label next to the file types dropdown list to \a str Default: "Type(s):"
    const std::string& SaveString() const;                 ///< returns the text of the ok button in its "save" state to \a str Default: "Save"
    const std::string& OpenString() const;                 ///< returns the text of the ok button in its "open" state to \a str Default: "Open"
    const std::string& CancelString() const;               ///< returns the text of the cancel button to \a str Default: "Cancel"

    const std::string& MalformedFilenameString() const;    ///< returns the error message when a malformed filename is encountered to \a str Default: "Invalid file name."
    const std::string& OverwritePromptString() const;      ///< returns the prompt message when a malformed file-overwrite is requested to \a str Default: "%1% exists.\nOk to overwrite it?"
    const std::string& InvalidFilenameString() const;      ///< returns the error message when an invalid filename is encountered to \a str Default: "\"%1%\"\nis an invalid file name."
    const std::string& FilenameIsADirectoryString() const; ///< returns the error message when a directory instead of a filename is specified to \a str Default: "\"%1%\"\nis a directory."
    const std::string& FileDoesNotExistString() const;     ///< returns the error message when a nonexistent filename is encountered to \a str Default: "File \"%1%\"\ndoes not exist."
    const std::string& DeviceIsNotReadyString() const;     ///< returns the error message when an unmounted drive is encountered to \a str Default: "The device is not ready."
    const std::string& ThreeButtonDlgOKString() const;     ///< returns the text of the 3-button dialog's ok button to \a str Default: "Ok"
    const std::string& ThreeButtonDlgCancelString() const; ///< returns the text of the 3-button dialog's cancel button to \a str Default: "Cancel"
    //@}

    /** \name Mutators */ ///@{
    virtual void Render();
    virtual void KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys);

    /** Set this to true if this FileDlg should select directories instead of
        files.  Note that this will only have an effect in file-open mode. */
    void SelectDirectories(bool directories);

    /** Set this to true if this FileDlg should append the missing extension
        to a file when in save mode.  Note that action is only taken if there
        is a single file filter containing exactly one wildcard in its first
        position (i.e. it is of the form "*foo").  If precondition is
        satisfied, any filename the user selects that does not end in "foo"
        will have "foo" appended to it. */
    void AppendMissingSaveExtension(bool append);

    /** Sets the allowed file types.  Each pair in the \a types parameter
        contains a description of the file type in its .first member, and
        wildcarded file types in its .second member.  For example, an entry
        might be ("Text Files (*.txt)", "*.txt"). Only the '*' character is
        supported as a wildcard.  More than one wildcard expression can be
        specified in a filter; if so, they must be separated by a comma and
        exactly one space (", ").  Each filter is considered OR-ed together
        with the others, so passing "*.tga, *.png" specifies listing any file
        that is either a Targa or a PNG file.  Note that an empty filter is
        considered to match all files, so ("All Files", "") is perfectly
        correct. */
    void SetFileFilters(const std::vector<std::pair<std::string, std::string> >& filters);

    void SetFilesString(const std::string& str);                ///< sets the text label next to the files edit box to \a str Default: "File(s):"
    void SetFileTypesString(const std::string& str);            ///< sets the text label next to the file types dropdown list to \a str Default: "Type(s):"
    void SetSaveString(const std::string& str);                 ///< sets the text of the ok button in its "save" state to \a str Default: "Save"
    void SetOpenString(const std::string& str);                 ///< sets the text of the ok button in its "open" state to \a str Default: "Open"
    void SetCancelString(const std::string& str);               ///< sets the text of the cancel button to \a str Default: "Cancel"

    void SetMalformedFilenameString(const std::string& str);    ///< sets the error message when a malformed filename is encountered to \a str Default: "Invalid file name."
    void SetOverwritePromptString(const std::string& str);      ///< sets the prompt message when a malformed file-overwrite is requested to \a str Default: "%1% exists.\nOk to overwrite it?"
    void SetInvalidFilenameString(const std::string& str);      ///< sets the error message when an invalid filename is encountered to \a str Default: "\"%1%\"\nis an invalid file name."
    void SetFilenameIsADirectoryString(const std::string& str); ///< sets the error message when a directory instead of a filename is specified to \a str Default: "\"%1%\"\nis a directory."
    void SetFileDoesNotExistString(const std::string& str);     ///< sets the error message when a nonexistent filename is encountered to \a str Default: "File \"%1%\"\ndoes not exist."
    void SetDeviceIsNotReadyString(const std::string& str);     ///< sets the error message when an unmounted drive is encountered to \a str Default: "The device is not ready."
    void SetThreeButtonDlgOKString(const std::string& str);     ///< sets the text of the 3-button dialog's ok button to \a str Default: "Ok"
    void SetThreeButtonDlgCancelString(const std::string& str); ///< sets the text of the 3-button dialog's cancel button to \a str Default: "Cancel"
    //@}

    /** Returns the current directory (the one that will be used by default on
        the next invocation of FileDlg::Run()) */
    static const boost::filesystem::path& WorkingDirectory();

    /** \name Exceptions */ ///@{
    /** The base class for FileDlg exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when the initial directory for the dialog is bad. */
    GG_CONCRETE_EXCEPTION(BadInitialDirectory, GG::FileDlg, Exception);
    //@}

protected:
    static const X DEFAULT_WIDTH;  ///< default width for the dialog
    static const Y DEFAULT_HEIGHT; ///< default height for the dialog

    /** \name Structors */ ///@{
    FileDlg(); ///< default ctor
    //@}

private:
    void CreateChildren(const std::string& filename, bool multi);
    void PlaceLabelsAndEdits(X button_width, Y button_height);
    void AttachSignalChildren();
    void DetachSignalChildren();
    void Init(const std::string& directory);
    void ConnectSignals();
    void OkClicked();
    void OkHandler(bool double_click);
    void CancelClicked();
    void FileSetChanged(const ListBox::SelectionSet& files);
    void FileDoubleClicked(DropDownList::iterator it);
    void FilesEditChanged(const std::string& str);
    void FilterChanged(DropDownList::iterator it);
    void SetWorkingDirectory(const boost::filesystem::path& p);
    void PopulateFilters();
    void UpdateList();
    void UpdateDirectoryText();
    void OpenDirectory();

    Clr              m_color;
    Clr              m_border_color;
    Clr              m_text_color;
    boost::shared_ptr<Font>
                     m_font;

    bool             m_save;
    std::vector<std::pair<std::string, std::string> > 
                     m_file_filters;
    std::set<std::string>
                     m_result;
    bool             m_select_directories;
    bool             m_append_missing_save_extension;
    bool             m_in_win32_drive_selection;

    std::string      m_save_str;
    std::string      m_open_str;
    std::string      m_cancel_str;

    std::string      m_malformed_filename_str;
    std::string      m_overwrite_prompt_str;
    std::string      m_invalid_filename_str;
    std::string      m_filename_is_a_directory_str;
    std::string      m_file_does_not_exist_str;
    std::string      m_device_is_not_ready_str;
    std::string      m_three_button_dlg_ok_str;
    std::string      m_three_button_dlg_cancel_str;

    TextControl*     m_curr_dir_text;
    ListBox*         m_files_list;
    Edit*            m_files_edit;
    DropDownList*    m_filter_list;
    Button*          m_ok_button;
    Button*          m_cancel_button;
    TextControl*     m_files_label;
    TextControl*     m_file_types_label;

    static boost::filesystem::path s_working_dir; ///< declared static so each instance of FileDlg opens up the same directory

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG


// template implementations
template <class Archive>
void GG::FileDlg::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Wnd)
        & BOOST_SERIALIZATION_NVP(m_color)
        & BOOST_SERIALIZATION_NVP(m_border_color)
        & BOOST_SERIALIZATION_NVP(m_text_color)
        & BOOST_SERIALIZATION_NVP(m_font)
        & BOOST_SERIALIZATION_NVP(m_save)
        & BOOST_SERIALIZATION_NVP(m_file_filters)
        & BOOST_SERIALIZATION_NVP(m_result)
        & BOOST_SERIALIZATION_NVP(m_save_str)
        & BOOST_SERIALIZATION_NVP(m_open_str)
        & BOOST_SERIALIZATION_NVP(m_cancel_str)
        & BOOST_SERIALIZATION_NVP(m_malformed_filename_str)
        & BOOST_SERIALIZATION_NVP(m_overwrite_prompt_str)
        & BOOST_SERIALIZATION_NVP(m_invalid_filename_str)
        & BOOST_SERIALIZATION_NVP(m_filename_is_a_directory_str)
        & BOOST_SERIALIZATION_NVP(m_file_does_not_exist_str)
        & BOOST_SERIALIZATION_NVP(m_device_is_not_ready_str)
        & BOOST_SERIALIZATION_NVP(m_three_button_dlg_ok_str)
        & BOOST_SERIALIZATION_NVP(m_three_button_dlg_cancel_str)
        & BOOST_SERIALIZATION_NVP(m_curr_dir_text)
        & BOOST_SERIALIZATION_NVP(m_files_list)
        & BOOST_SERIALIZATION_NVP(m_files_edit)
        & BOOST_SERIALIZATION_NVP(m_filter_list)
        & BOOST_SERIALIZATION_NVP(m_ok_button)
        & BOOST_SERIALIZATION_NVP(m_cancel_button)
        & BOOST_SERIALIZATION_NVP(m_files_label)
        & BOOST_SERIALIZATION_NVP(m_file_types_label)
        & BOOST_SERIALIZATION_NVP(m_select_directories)
        & BOOST_SERIALIZATION_NVP(m_append_missing_save_extension);

    if (Archive::is_loading::value)
        ConnectSignals();
}

#endif // _GG_FileDlg_h_
