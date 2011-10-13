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

/** \file SDLGUI.h \brief Contains SDLGUI, the input driver for using SDL with
    GG. */

#ifndef _GG_SDLGUI_h_
#define _GG_SDLGUI_h_

#include <GG/GUI.h>

#include <SDL/SDL.h>


#ifdef _MSC_VER
# ifdef GiGiSDL_EXPORTS
#  define GG_SDL_API __declspec(dllexport)
# else
#  define GG_SDL_API __declspec(dllimport)
# endif
#else
# define GG_SDL_API
#endif

namespace GG {

/** \brief This is an abstract singleton class that represents the GUI
    framework of an SDL OpenGL application.

    <p>Usage:

    <br>Any application including an object of this class should declare that
    object as a local variable in main(). The name of this variable will
    herein be assumed to be "gui". It should be allocated on the stack; if it
    is created dynamically a leak may occur. SDLGUI is designed so the main()
    of the application can consist of just the one line "gui();".

    <p>To do this, the user needs only to override the Initialize() and
    FinalCleanup() methods, and ensure that the program does not terminate
    abnormally; this ensures FinalCleanup() is called when gui's destructor is
    invoked. Exit() can also perform cleanup and terminate the application
    cleanly.

    <p>Most of the member methods of SDLGUI have been declared virtual, to
    give the user great control when subclassing. The virtual function calls
    are usually not a performance issue, since none of the methods is called
    repeatedly, except HandleEvent(); if this is a problem, just create a new
    function in your subclass and call that from within Run() instead of
    HandleEvent(). Note that though the bulk of the program execution takes
    place within Run(), Run() itself is also only called once.

    <p>SDLGUI takes a two-tiered approach to event handling.  The event pump
    calls HandleSystemEvents(), which polls for SDL events and handles them by
    first determining whether the event is GG-related, or some other non-GG
    event, such as SDL_QUIT, etc.  GG events and non-GG events are passed to
    HandleGGEvent() and HandleNonGGEvent(), respectively.  For most uses,
    there should be no need to override the behavior of HandleSDLEvents().
    However, the HandleNonGGEvent() default implementation only responds to
    SDL_QUIT events, and so should be overridden in most cases. */
class GG_SDL_API SDLGUI : public GG::GUI
{
public:
    /** \name Structors */ ///@{
    explicit SDLGUI(int w = 1024, int h = 768, bool calc_FPS = false, const std::string& app_name = "GG"); ///< ctor
    virtual ~SDLGUI();
    //@}

    /** \name Accessors */ ///@{
    virtual X AppWidth() const;
    virtual Y AppHeight() const;
    virtual unsigned int Ticks() const;
    //@}

    /** \name Mutators */ ///@{
    void           operator()();      ///< external interface to Run()
    virtual void   Exit(int code);

    virtual void   Enter2DMode() = 0;
    virtual void   Exit2DMode() = 0;
    //@}

    static SDLGUI* GetGUI();                             ///< allows any code to access the gui framework by calling SDLGUI::GetGUI()
    static GG::Key GGKeyFromSDLKey(const SDL_keysym& key); ///< gives the GGKey equivalent of key

protected:
    void SetAppSize(const GG::Pt& size);

    // these are called at the beginning of the gui's execution
    virtual void   SDLInit();        ///< initializes SDL, FE, and SDL OpenGL functionality
    virtual void   GLInit();         ///< allows user to specify OpenGL initialization code; called at the end of SDLInit()
    virtual void   Initialize() = 0; ///< provides one-time gui initialization

    virtual void   HandleSystemEvents();
    virtual void   HandleNonGGEvent(const SDL_Event& event); ///< event handler for all SDL events that are not GG-related

    virtual void   RenderBegin();
    virtual void   RenderEnd();

    // these are called at the end of the gui's execution
    virtual void   FinalCleanup();   ///< provides one-time gui cleanup
    virtual void   SDLQuit();        ///< cleans up SDL and (if used) FE

    virtual void   Run();

private:
    X         m_app_width;      ///< application width and height (defaults to 1024 x 768)
    Y         m_app_height;
};

} // namespace GG

#endif // _GG_SDLGUI_h_

