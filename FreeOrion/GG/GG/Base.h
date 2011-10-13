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

/** \file Base.h \brief Contains enums, utility classes, and free functions
    used throughout GG. */

#ifndef _GG_Base_h_
#define _GG_Base_h_

#include <GG/Export.h>

// include OpenGL headers
#if defined(__APPLE__) && defined(__MACH__)
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
#endif
#include <GG/glext.h>

// include useful boost headers
#include <boost/cstdint.hpp>
#include <boost/signal.hpp>

// other headers
#include <GG/Enum.h>
#include <GG/Clr.h>
#include <GG/PtRect.h>
#include <GG/SignalsAndSlots.h>


/** \namespace GG \brief The namespace that encloses all GG classes,
    functions, typedefs, enums, etc. */
namespace GG {

extern GG_API const bool INSTRUMENT_ALL_SIGNALS;

template <class FlagType>
class Flags;
class ModKey;

template <class T>
class ScopedAssign
{
public:
    ScopedAssign(T& t, T val) :
        m_old_val(t),
        m_t(t)
        { m_t = val; }
    ~ScopedAssign()
        { m_t = m_old_val; }

private:
    T m_old_val;
    T& m_t;
};

/** "Regions" of a window; used e.g. to determine direction(s) of drag when a
    window that has a drag-frame is clicked. */
enum WndRegion {
    WR_NONE = -1, 
    WR_TOPLEFT = 0, 
    WR_TOP, 
    WR_TOPRIGHT, 
    WR_MIDLEFT, 
    WR_MIDDLE, 
    WR_MIDRIGHT, 
    WR_BOTTOMLEFT, 
    WR_BOTTOM, 
    WR_BOTTOMRIGHT
};

/** The orientations for scrollbars, sliders, etc. */
enum Orientation {
    VERTICAL,  ///< Vertical orientation.
    HORIZONTAL ///< Horizontal orientation.
};

/** The built-in visual styles of state buttons. */
enum StateButtonStyle {
    SBSTYLE_3D_XBOX,         ///< Draws a down-beveled box with a 3D x-mark inside.
    SBSTYLE_3D_CHECKBOX,     ///< Draws a down-beveled box with a 3D check-mark inside.
    SBSTYLE_3D_RADIO,        ///< Draws a down-beveled circle with a 3D "dot" or "bubble" inside.
    SBSTYLE_3D_BUTTON,       ///< Draws a button that toggles bewtween popped up and pushed down.
    SBSTYLE_3D_ROUND_BUTTON, ///< Draws a down-beveled circle with an up-beveled circle inside.

    /** Draws an up-beveled rectagular tab that is brighter and larger when
        pressed; it's bottom is unbeveled. */
    SBSTYLE_3D_TOP_ATTACHED_TAB,

    /** Draws an up-beveled rectagular tab that is brighter and larger when
        pressed; it's bottom is beveled. */
    SBSTYLE_3D_TOP_DETACHED_TAB
};

/** The rendering styles of the line the tab slides over in a Slider. */
enum SliderLineStyle {
    FLAT,
    RAISED,
    GROOVED
};

/** The styles of display for a TabBar. */
enum TabBarStyle {
    /** The currently active tab should appear to be connected to the area
        below the bar. */
    TAB_BAR_ATTACHED,

    /** The currently active tab should appear to be unconnected to the area
        below the bar. */
    TAB_BAR_DETACHED
};


/** Adpated from SDLKey enum in SDL_keysym.h of the SDL library; capital
    letter keys added. */
enum Key {
    // The keyboard symbols have been cleverly chosen to map to ASCII
    GGK_UNKNOWN      = 0,
    GGK_FIRST        = 0,
    GGK_BACKSPACE    = 8,
    GGK_TAB          = 9,
    GGK_CLEAR        = 12,
    GGK_RETURN       = 13,
    GGK_PAUSE        = 19,
    GGK_ESCAPE       = 27,
    GGK_SPACE        = 32,
    GGK_EXCLAIM      = 33,
    GGK_QUOTEDBL     = 34,
    GGK_HASH         = 35,
    GGK_DOLLAR       = 36,
    GGK_AMPERSAND    = 38,
    GGK_QUOTE        = 39,
    GGK_LEFTPAREN    = 40,
    GGK_RIGHTPAREN   = 41,
    GGK_ASTERISK     = 42,
    GGK_PLUS         = 43,
    GGK_COMMA        = 44,
    GGK_MINUS        = 45,
    GGK_PERIOD       = 46,
    GGK_SLASH        = 47,
    GGK_0            = 48,
    GGK_1            = 49,
    GGK_2            = 50,
    GGK_3            = 51,
    GGK_4            = 52,
    GGK_5            = 53,
    GGK_6            = 54,
    GGK_7            = 55,
    GGK_8            = 56,
    GGK_9            = 57,
    GGK_COLON        = 58,
    GGK_SEMICOLON    = 59,
    GGK_LESS         = 60,
    GGK_EQUALS       = 61,
    GGK_GREATER      = 62,
    GGK_QUESTION     = 63,
    GGK_AT           = 64,
    GGK_A            = 65,
    GGK_B            = 66,
    GGK_C            = 67,
    GGK_D            = 68,
    GGK_E            = 69,
    GGK_F            = 70,
    GGK_G            = 71,
    GGK_H            = 72,
    GGK_I            = 73,
    GGK_J            = 74,
    GGK_K            = 75,
    GGK_L            = 76,
    GGK_M            = 77,
    GGK_N            = 78,
    GGK_O            = 79,
    GGK_P            = 80,
    GGK_Q            = 81,
    GGK_R            = 82,
    GGK_S            = 83,
    GGK_T            = 84,
    GGK_U            = 85,
    GGK_V            = 86,
    GGK_W            = 87,
    GGK_X            = 88,
    GGK_Y            = 89,
    GGK_Z            = 90,
    GGK_LEFTBRACKET  = 91,
    GGK_BACKSLASH    = 92,
    GGK_RIGHTBRACKET = 93,
    GGK_CARET        = 94,
    GGK_UNDERSCORE   = 95,
    GGK_BACKQUOTE    = 96,
    GGK_a            = 97,
    GGK_b            = 98,
    GGK_c            = 99,
    GGK_d            = 100,
    GGK_e            = 101,
    GGK_f            = 102,
    GGK_g            = 103,
    GGK_h            = 104,
    GGK_i            = 105,
    GGK_j            = 106,
    GGK_k            = 107,
    GGK_l            = 108,
    GGK_m            = 109,
    GGK_n            = 110,
    GGK_o            = 111,
    GGK_p            = 112,
    GGK_q            = 113,
    GGK_r            = 114,
    GGK_s            = 115,
    GGK_t            = 116,
    GGK_u            = 117,
    GGK_v            = 118,
    GGK_w            = 119,
    GGK_x            = 120,
    GGK_y            = 121,
    GGK_z            = 122,
    GGK_DELETE       = 127,
    // End of ASCII mapped keysyms

    // International keyboard syms
    GGK_WORLD_0      = 160,      // 0xA0
    GGK_WORLD_1      = 161,
    GGK_WORLD_2      = 162,
    GGK_WORLD_3      = 163,
    GGK_WORLD_4      = 164,
    GGK_WORLD_5      = 165,
    GGK_WORLD_6      = 166,
    GGK_WORLD_7      = 167,
    GGK_WORLD_8      = 168,
    GGK_WORLD_9      = 169,
    GGK_WORLD_10     = 170,
    GGK_WORLD_11     = 171,
    GGK_WORLD_12     = 172,
    GGK_WORLD_13     = 173,
    GGK_WORLD_14     = 174,
    GGK_WORLD_15     = 175,
    GGK_WORLD_16     = 176,
    GGK_WORLD_17     = 177,
    GGK_WORLD_18     = 178,
    GGK_WORLD_19     = 179,
    GGK_WORLD_20     = 180,
    GGK_WORLD_21     = 181,
    GGK_WORLD_22     = 182,
    GGK_WORLD_23     = 183,
    GGK_WORLD_24     = 184,
    GGK_WORLD_25     = 185,
    GGK_WORLD_26     = 186,
    GGK_WORLD_27     = 187,
    GGK_WORLD_28     = 188,
    GGK_WORLD_29     = 189,
    GGK_WORLD_30     = 190,
    GGK_WORLD_31     = 191,
    GGK_WORLD_32     = 192,
    GGK_WORLD_33     = 193,
    GGK_WORLD_34     = 194,
    GGK_WORLD_35     = 195,
    GGK_WORLD_36     = 196,
    GGK_WORLD_37     = 197,
    GGK_WORLD_38     = 198,
    GGK_WORLD_39     = 199,
    GGK_WORLD_40     = 200,
    GGK_WORLD_41     = 201,
    GGK_WORLD_42     = 202,
    GGK_WORLD_43     = 203,
    GGK_WORLD_44     = 204,
    GGK_WORLD_45     = 205,
    GGK_WORLD_46     = 206,
    GGK_WORLD_47     = 207,
    GGK_WORLD_48     = 208,
    GGK_WORLD_49     = 209,
    GGK_WORLD_50     = 210,
    GGK_WORLD_51     = 211,
    GGK_WORLD_52     = 212,
    GGK_WORLD_53     = 213,
    GGK_WORLD_54     = 214,
    GGK_WORLD_55     = 215,
    GGK_WORLD_56     = 216,
    GGK_WORLD_57     = 217,
    GGK_WORLD_58     = 218,
    GGK_WORLD_59     = 219,
    GGK_WORLD_60     = 220,
    GGK_WORLD_61     = 221,
    GGK_WORLD_62     = 222,
    GGK_WORLD_63     = 223,
    GGK_WORLD_64     = 224,
    GGK_WORLD_65     = 225,
    GGK_WORLD_66     = 226,
    GGK_WORLD_67     = 227,
    GGK_WORLD_68     = 228,
    GGK_WORLD_69     = 229,
    GGK_WORLD_70     = 230,
    GGK_WORLD_71     = 231,
    GGK_WORLD_72     = 232,
    GGK_WORLD_73     = 233,
    GGK_WORLD_74     = 234,
    GGK_WORLD_75     = 235,
    GGK_WORLD_76     = 236,
    GGK_WORLD_77     = 237,
    GGK_WORLD_78     = 238,
    GGK_WORLD_79     = 239,
    GGK_WORLD_80     = 240,
    GGK_WORLD_81     = 241,
    GGK_WORLD_82     = 242,
    GGK_WORLD_83     = 243,
    GGK_WORLD_84     = 244,
    GGK_WORLD_85     = 245,
    GGK_WORLD_86     = 246,
    GGK_WORLD_87     = 247,
    GGK_WORLD_88     = 248,
    GGK_WORLD_89     = 249,
    GGK_WORLD_90     = 250,
    GGK_WORLD_91     = 251,
    GGK_WORLD_92     = 252,
    GGK_WORLD_93     = 253,
    GGK_WORLD_94     = 254,
    GGK_WORLD_95     = 255,      // 0xFF

    // Numeric keypad
    GGK_KP0          = 256,
    GGK_KP1          = 257,
    GGK_KP2          = 258,
    GGK_KP3          = 259,
    GGK_KP4          = 260,
    GGK_KP5          = 261,
    GGK_KP6          = 262,
    GGK_KP7          = 263,
    GGK_KP8          = 264,
    GGK_KP9          = 265,
    GGK_KP_PERIOD    = 266,
    GGK_KP_DIVIDE    = 267,
    GGK_KP_MULTIPLY  = 268,
    GGK_KP_MINUS     = 269,
    GGK_KP_PLUS      = 270,
    GGK_KP_ENTER     = 271,
    GGK_KP_EQUALS    = 272,

    // Arrows + Home/End pad
    GGK_UP           = 273,
    GGK_DOWN         = 274,
    GGK_RIGHT        = 275,
    GGK_LEFT         = 276,
    GGK_INSERT       = 277,
    GGK_HOME         = 278,
    GGK_END          = 279,
    GGK_PAGEUP       = 280,
    GGK_PAGEDOWN     = 281,

    // Function keys
    GGK_F1           = 282,
    GGK_F2           = 283,
    GGK_F3           = 284,
    GGK_F4           = 285,
    GGK_F5           = 286,
    GGK_F6           = 287,
    GGK_F7           = 288,
    GGK_F8           = 289,
    GGK_F9           = 290,
    GGK_F10          = 291,
    GGK_F11          = 292,
    GGK_F12          = 293,
    GGK_F13          = 294,
    GGK_F14          = 295,
    GGK_F15          = 296,

    // Key state modifier keys
    GGK_NUMLOCK      = 300,
    GGK_CAPSLOCK     = 301,
    GGK_SCROLLOCK    = 302,
    GGK_RSHIFT       = 303,
    GGK_LSHIFT       = 304,
    GGK_RCTRL        = 305,
    GGK_LCTRL        = 306,
    GGK_RALT         = 307,
    GGK_LALT         = 308,
    GGK_RMETA        = 309,
    GGK_LMETA        = 310,
    GGK_LSUPER       = 311,      // Left "Windows" key
    GGK_RSUPER       = 312,      // Right "Windows" key
    GGK_MODE         = 313,      // "Alt Gr" key
    GGK_COMPOSE      = 314,      // Multi-key compose key

    // Miscellaneous function keys
    GGK_HELP         = 315,
    GGK_PRINT        = 316,
    GGK_SYSREQ       = 317,
    GGK_BREAK        = 318,
    GGK_MENU         = 319,
    GGK_POWER        = 320,      // Power Macintosh power key
    GGK_EURO         = 321,      // Some european keyboards
    GGK_UNDO         = 322,      // Atari keyboard has Undo

    // Add any other keys here

    GGK_LAST
};

/** Translates a printable key combination from a keypad press to the
    equivalent main-keyboard press.  \a key is \a only modified if it is a
    keypad value.  NumLock is taken into account.  For instance, with numlock
    on, a GGK_KP7 (which is equal to a nonprintable char value) becomes a
    GGK_7 (which equals '7', and is printable). */
void KeypadKeyToPrintable(Key& key, Flags<ModKey> mod_keys);


////////////////////////////////////////
// EnumMaps and stream operators
////////////////////////////////////////

// define EnumMap and stream operators for Slider::LineStyleType
GG_ENUM_MAP_BEGIN(SliderLineStyle)
    GG_ENUM_MAP_INSERT(FLAT)
    GG_ENUM_MAP_INSERT(RAISED)
    GG_ENUM_MAP_INSERT(GROOVED)
GG_ENUM_MAP_END

GG_ENUM_STREAM_IN(SliderLineStyle)
GG_ENUM_STREAM_OUT(SliderLineStyle)

// define EnumMap and stream operators for Orientation
GG_ENUM_MAP_BEGIN(Orientation)
    GG_ENUM_MAP_INSERT(VERTICAL)
    GG_ENUM_MAP_INSERT(HORIZONTAL)
GG_ENUM_MAP_END

GG_ENUM_STREAM_IN(Orientation)
GG_ENUM_STREAM_OUT(Orientation)

// define EnumMap and stream operators for StateButtonStyle
GG_ENUM_MAP_BEGIN(StateButtonStyle)
    GG_ENUM_MAP_INSERT(SBSTYLE_3D_XBOX)
    GG_ENUM_MAP_INSERT(SBSTYLE_3D_CHECKBOX)
    GG_ENUM_MAP_INSERT(SBSTYLE_3D_RADIO)
    GG_ENUM_MAP_INSERT(SBSTYLE_3D_BUTTON)
    GG_ENUM_MAP_INSERT(SBSTYLE_3D_ROUND_BUTTON)
    GG_ENUM_MAP_INSERT(SBSTYLE_3D_TOP_ATTACHED_TAB)
    GG_ENUM_MAP_INSERT(SBSTYLE_3D_TOP_DETACHED_TAB)
GG_ENUM_MAP_END

GG_ENUM_STREAM_IN(StateButtonStyle)
GG_ENUM_STREAM_OUT(StateButtonStyle)

// define EnumMap and stream operators for TabBarStyle
GG_ENUM_MAP_BEGIN(TabBarStyle)
    GG_ENUM_MAP_INSERT(TAB_BAR_ATTACHED)
    GG_ENUM_MAP_INSERT(TAB_BAR_DETACHED)
GG_ENUM_MAP_END

GG_ENUM_STREAM_IN(TabBarStyle)
GG_ENUM_STREAM_OUT(TabBarStyle)

// define EnumMap and stream operators for Mod
GG_ENUM_MAP_BEGIN(Key)
    GG_ENUM_MAP_INSERT(GGK_UNKNOWN)
    GG_ENUM_MAP_INSERT(GGK_FIRST)
    GG_ENUM_MAP_INSERT(GGK_BACKSPACE)
    GG_ENUM_MAP_INSERT(GGK_TAB)
    GG_ENUM_MAP_INSERT(GGK_CLEAR)
    GG_ENUM_MAP_INSERT(GGK_RETURN)
    GG_ENUM_MAP_INSERT(GGK_PAUSE)
    GG_ENUM_MAP_INSERT(GGK_ESCAPE)
    GG_ENUM_MAP_INSERT(GGK_SPACE)
    GG_ENUM_MAP_INSERT(GGK_EXCLAIM)
    GG_ENUM_MAP_INSERT(GGK_QUOTEDBL)
    GG_ENUM_MAP_INSERT(GGK_HASH)
    GG_ENUM_MAP_INSERT(GGK_DOLLAR)
    GG_ENUM_MAP_INSERT(GGK_AMPERSAND)
    GG_ENUM_MAP_INSERT(GGK_QUOTE)
    GG_ENUM_MAP_INSERT(GGK_LEFTPAREN)
    GG_ENUM_MAP_INSERT(GGK_RIGHTPAREN)
    GG_ENUM_MAP_INSERT(GGK_ASTERISK)
    GG_ENUM_MAP_INSERT(GGK_PLUS)
    GG_ENUM_MAP_INSERT(GGK_COMMA)
    GG_ENUM_MAP_INSERT(GGK_MINUS)
    GG_ENUM_MAP_INSERT(GGK_PERIOD)
    GG_ENUM_MAP_INSERT(GGK_SLASH)
    GG_ENUM_MAP_INSERT(GGK_0)
    GG_ENUM_MAP_INSERT(GGK_1)
    GG_ENUM_MAP_INSERT(GGK_2)
    GG_ENUM_MAP_INSERT(GGK_3)
    GG_ENUM_MAP_INSERT(GGK_4)
    GG_ENUM_MAP_INSERT(GGK_5)
    GG_ENUM_MAP_INSERT(GGK_6)
    GG_ENUM_MAP_INSERT(GGK_7)
    GG_ENUM_MAP_INSERT(GGK_8)
    GG_ENUM_MAP_INSERT(GGK_9)
    GG_ENUM_MAP_INSERT(GGK_COLON)
    GG_ENUM_MAP_INSERT(GGK_SEMICOLON)
    GG_ENUM_MAP_INSERT(GGK_LESS)
    GG_ENUM_MAP_INSERT(GGK_EQUALS)
    GG_ENUM_MAP_INSERT(GGK_GREATER)
    GG_ENUM_MAP_INSERT(GGK_QUESTION)
    GG_ENUM_MAP_INSERT(GGK_AT)
    GG_ENUM_MAP_INSERT(GGK_A)
    GG_ENUM_MAP_INSERT(GGK_B)
    GG_ENUM_MAP_INSERT(GGK_C)
    GG_ENUM_MAP_INSERT(GGK_D)
    GG_ENUM_MAP_INSERT(GGK_E)
    GG_ENUM_MAP_INSERT(GGK_F)
    GG_ENUM_MAP_INSERT(GGK_G)
    GG_ENUM_MAP_INSERT(GGK_H)
    GG_ENUM_MAP_INSERT(GGK_I)
    GG_ENUM_MAP_INSERT(GGK_J)
    GG_ENUM_MAP_INSERT(GGK_K)
    GG_ENUM_MAP_INSERT(GGK_L)
    GG_ENUM_MAP_INSERT(GGK_M)
    GG_ENUM_MAP_INSERT(GGK_N)
    GG_ENUM_MAP_INSERT(GGK_O)
    GG_ENUM_MAP_INSERT(GGK_P)
    GG_ENUM_MAP_INSERT(GGK_Q)
    GG_ENUM_MAP_INSERT(GGK_R)
    GG_ENUM_MAP_INSERT(GGK_S)
    GG_ENUM_MAP_INSERT(GGK_T)
    GG_ENUM_MAP_INSERT(GGK_U)
    GG_ENUM_MAP_INSERT(GGK_V)
    GG_ENUM_MAP_INSERT(GGK_W)
    GG_ENUM_MAP_INSERT(GGK_X)
    GG_ENUM_MAP_INSERT(GGK_Y)
    GG_ENUM_MAP_INSERT(GGK_Z)
    GG_ENUM_MAP_INSERT(GGK_LEFTBRACKET)
    GG_ENUM_MAP_INSERT(GGK_BACKSLASH)
    GG_ENUM_MAP_INSERT(GGK_RIGHTBRACKET)
    GG_ENUM_MAP_INSERT(GGK_CARET)
    GG_ENUM_MAP_INSERT(GGK_UNDERSCORE)
    GG_ENUM_MAP_INSERT(GGK_BACKQUOTE)
    GG_ENUM_MAP_INSERT(GGK_a)
    GG_ENUM_MAP_INSERT(GGK_b)
    GG_ENUM_MAP_INSERT(GGK_c)
    GG_ENUM_MAP_INSERT(GGK_d)
    GG_ENUM_MAP_INSERT(GGK_e)
    GG_ENUM_MAP_INSERT(GGK_f)
    GG_ENUM_MAP_INSERT(GGK_g)
    GG_ENUM_MAP_INSERT(GGK_h)
    GG_ENUM_MAP_INSERT(GGK_i)
    GG_ENUM_MAP_INSERT(GGK_j)
    GG_ENUM_MAP_INSERT(GGK_k)
    GG_ENUM_MAP_INSERT(GGK_l)
    GG_ENUM_MAP_INSERT(GGK_m)
    GG_ENUM_MAP_INSERT(GGK_n)
    GG_ENUM_MAP_INSERT(GGK_o)
    GG_ENUM_MAP_INSERT(GGK_p)
    GG_ENUM_MAP_INSERT(GGK_q)
    GG_ENUM_MAP_INSERT(GGK_r)
    GG_ENUM_MAP_INSERT(GGK_s)
    GG_ENUM_MAP_INSERT(GGK_t)
    GG_ENUM_MAP_INSERT(GGK_u)
    GG_ENUM_MAP_INSERT(GGK_v)
    GG_ENUM_MAP_INSERT(GGK_w)
    GG_ENUM_MAP_INSERT(GGK_x)
    GG_ENUM_MAP_INSERT(GGK_y)
    GG_ENUM_MAP_INSERT(GGK_z)
    GG_ENUM_MAP_INSERT(GGK_DELETE)

    GG_ENUM_MAP_INSERT(GGK_WORLD_0)
    GG_ENUM_MAP_INSERT(GGK_WORLD_1)
    GG_ENUM_MAP_INSERT(GGK_WORLD_2)
    GG_ENUM_MAP_INSERT(GGK_WORLD_3)
    GG_ENUM_MAP_INSERT(GGK_WORLD_4)
    GG_ENUM_MAP_INSERT(GGK_WORLD_5)
    GG_ENUM_MAP_INSERT(GGK_WORLD_6)
    GG_ENUM_MAP_INSERT(GGK_WORLD_7)
    GG_ENUM_MAP_INSERT(GGK_WORLD_8)
    GG_ENUM_MAP_INSERT(GGK_WORLD_9)
    GG_ENUM_MAP_INSERT(GGK_WORLD_10)
    GG_ENUM_MAP_INSERT(GGK_WORLD_11)
    GG_ENUM_MAP_INSERT(GGK_WORLD_12)
    GG_ENUM_MAP_INSERT(GGK_WORLD_13)
    GG_ENUM_MAP_INSERT(GGK_WORLD_14)
    GG_ENUM_MAP_INSERT(GGK_WORLD_15)
    GG_ENUM_MAP_INSERT(GGK_WORLD_16)
    GG_ENUM_MAP_INSERT(GGK_WORLD_17)
    GG_ENUM_MAP_INSERT(GGK_WORLD_18)
    GG_ENUM_MAP_INSERT(GGK_WORLD_19)
    GG_ENUM_MAP_INSERT(GGK_WORLD_20)
    GG_ENUM_MAP_INSERT(GGK_WORLD_21)
    GG_ENUM_MAP_INSERT(GGK_WORLD_22)
    GG_ENUM_MAP_INSERT(GGK_WORLD_23)
    GG_ENUM_MAP_INSERT(GGK_WORLD_24)
    GG_ENUM_MAP_INSERT(GGK_WORLD_25)
    GG_ENUM_MAP_INSERT(GGK_WORLD_26)
    GG_ENUM_MAP_INSERT(GGK_WORLD_27)
    GG_ENUM_MAP_INSERT(GGK_WORLD_28)
    GG_ENUM_MAP_INSERT(GGK_WORLD_29)
    GG_ENUM_MAP_INSERT(GGK_WORLD_30)
    GG_ENUM_MAP_INSERT(GGK_WORLD_31)
    GG_ENUM_MAP_INSERT(GGK_WORLD_32)
    GG_ENUM_MAP_INSERT(GGK_WORLD_33)
    GG_ENUM_MAP_INSERT(GGK_WORLD_34)
    GG_ENUM_MAP_INSERT(GGK_WORLD_35)
    GG_ENUM_MAP_INSERT(GGK_WORLD_36)
    GG_ENUM_MAP_INSERT(GGK_WORLD_37)
    GG_ENUM_MAP_INSERT(GGK_WORLD_38)
    GG_ENUM_MAP_INSERT(GGK_WORLD_39)
    GG_ENUM_MAP_INSERT(GGK_WORLD_40)
    GG_ENUM_MAP_INSERT(GGK_WORLD_41)
    GG_ENUM_MAP_INSERT(GGK_WORLD_42)
    GG_ENUM_MAP_INSERT(GGK_WORLD_43)
    GG_ENUM_MAP_INSERT(GGK_WORLD_44)
    GG_ENUM_MAP_INSERT(GGK_WORLD_45)
    GG_ENUM_MAP_INSERT(GGK_WORLD_46)
    GG_ENUM_MAP_INSERT(GGK_WORLD_47)
    GG_ENUM_MAP_INSERT(GGK_WORLD_48)
    GG_ENUM_MAP_INSERT(GGK_WORLD_49)
    GG_ENUM_MAP_INSERT(GGK_WORLD_50)
    GG_ENUM_MAP_INSERT(GGK_WORLD_51)
    GG_ENUM_MAP_INSERT(GGK_WORLD_52)
    GG_ENUM_MAP_INSERT(GGK_WORLD_53)
    GG_ENUM_MAP_INSERT(GGK_WORLD_54)
    GG_ENUM_MAP_INSERT(GGK_WORLD_55)
    GG_ENUM_MAP_INSERT(GGK_WORLD_56)
    GG_ENUM_MAP_INSERT(GGK_WORLD_57)
    GG_ENUM_MAP_INSERT(GGK_WORLD_58)
    GG_ENUM_MAP_INSERT(GGK_WORLD_59)
    GG_ENUM_MAP_INSERT(GGK_WORLD_60)
    GG_ENUM_MAP_INSERT(GGK_WORLD_61)
    GG_ENUM_MAP_INSERT(GGK_WORLD_62)
    GG_ENUM_MAP_INSERT(GGK_WORLD_63)
    GG_ENUM_MAP_INSERT(GGK_WORLD_64)
    GG_ENUM_MAP_INSERT(GGK_WORLD_65)
    GG_ENUM_MAP_INSERT(GGK_WORLD_66)
    GG_ENUM_MAP_INSERT(GGK_WORLD_67)
    GG_ENUM_MAP_INSERT(GGK_WORLD_68)
    GG_ENUM_MAP_INSERT(GGK_WORLD_69)
    GG_ENUM_MAP_INSERT(GGK_WORLD_70)
    GG_ENUM_MAP_INSERT(GGK_WORLD_71)
    GG_ENUM_MAP_INSERT(GGK_WORLD_72)
    GG_ENUM_MAP_INSERT(GGK_WORLD_73)
    GG_ENUM_MAP_INSERT(GGK_WORLD_74)
    GG_ENUM_MAP_INSERT(GGK_WORLD_75)
    GG_ENUM_MAP_INSERT(GGK_WORLD_76)
    GG_ENUM_MAP_INSERT(GGK_WORLD_77)
    GG_ENUM_MAP_INSERT(GGK_WORLD_78)
    GG_ENUM_MAP_INSERT(GGK_WORLD_79)
    GG_ENUM_MAP_INSERT(GGK_WORLD_80)
    GG_ENUM_MAP_INSERT(GGK_WORLD_81)
    GG_ENUM_MAP_INSERT(GGK_WORLD_82)
    GG_ENUM_MAP_INSERT(GGK_WORLD_83)
    GG_ENUM_MAP_INSERT(GGK_WORLD_84)
    GG_ENUM_MAP_INSERT(GGK_WORLD_85)
    GG_ENUM_MAP_INSERT(GGK_WORLD_86)
    GG_ENUM_MAP_INSERT(GGK_WORLD_87)
    GG_ENUM_MAP_INSERT(GGK_WORLD_88)
    GG_ENUM_MAP_INSERT(GGK_WORLD_89)
    GG_ENUM_MAP_INSERT(GGK_WORLD_90)
    GG_ENUM_MAP_INSERT(GGK_WORLD_91)
    GG_ENUM_MAP_INSERT(GGK_WORLD_92)
    GG_ENUM_MAP_INSERT(GGK_WORLD_93)
    GG_ENUM_MAP_INSERT(GGK_WORLD_94)
    GG_ENUM_MAP_INSERT(GGK_WORLD_95)

    GG_ENUM_MAP_INSERT(GGK_KP0)
    GG_ENUM_MAP_INSERT(GGK_KP1)
    GG_ENUM_MAP_INSERT(GGK_KP2)
    GG_ENUM_MAP_INSERT(GGK_KP3)
    GG_ENUM_MAP_INSERT(GGK_KP4)
    GG_ENUM_MAP_INSERT(GGK_KP5)
    GG_ENUM_MAP_INSERT(GGK_KP6)
    GG_ENUM_MAP_INSERT(GGK_KP7)
    GG_ENUM_MAP_INSERT(GGK_KP8)
    GG_ENUM_MAP_INSERT(GGK_KP9)
    GG_ENUM_MAP_INSERT(GGK_KP_PERIOD)
    GG_ENUM_MAP_INSERT(GGK_KP_DIVIDE)
    GG_ENUM_MAP_INSERT(GGK_KP_MULTIPLY)
    GG_ENUM_MAP_INSERT(GGK_KP_MINUS)
    GG_ENUM_MAP_INSERT(GGK_KP_PLUS)
    GG_ENUM_MAP_INSERT(GGK_KP_ENTER)
    GG_ENUM_MAP_INSERT(GGK_KP_EQUALS)

    GG_ENUM_MAP_INSERT(GGK_UP)
    GG_ENUM_MAP_INSERT(GGK_DOWN)
    GG_ENUM_MAP_INSERT(GGK_RIGHT)
    GG_ENUM_MAP_INSERT(GGK_LEFT)
    GG_ENUM_MAP_INSERT(GGK_INSERT)
    GG_ENUM_MAP_INSERT(GGK_HOME)
    GG_ENUM_MAP_INSERT(GGK_END)
    GG_ENUM_MAP_INSERT(GGK_PAGEUP)
    GG_ENUM_MAP_INSERT(GGK_PAGEDOWN)

    GG_ENUM_MAP_INSERT(GGK_F1)
    GG_ENUM_MAP_INSERT(GGK_F2)
    GG_ENUM_MAP_INSERT(GGK_F3)
    GG_ENUM_MAP_INSERT(GGK_F4)
    GG_ENUM_MAP_INSERT(GGK_F5)
    GG_ENUM_MAP_INSERT(GGK_F6)
    GG_ENUM_MAP_INSERT(GGK_F7)
    GG_ENUM_MAP_INSERT(GGK_F8)
    GG_ENUM_MAP_INSERT(GGK_F9)
    GG_ENUM_MAP_INSERT(GGK_F10)
    GG_ENUM_MAP_INSERT(GGK_F11)
    GG_ENUM_MAP_INSERT(GGK_F12)
    GG_ENUM_MAP_INSERT(GGK_F13)
    GG_ENUM_MAP_INSERT(GGK_F14)
    GG_ENUM_MAP_INSERT(GGK_F15)

    GG_ENUM_MAP_INSERT(GGK_NUMLOCK)
    GG_ENUM_MAP_INSERT(GGK_CAPSLOCK)
    GG_ENUM_MAP_INSERT(GGK_SCROLLOCK)
    GG_ENUM_MAP_INSERT(GGK_RSHIFT)
    GG_ENUM_MAP_INSERT(GGK_LSHIFT)
    GG_ENUM_MAP_INSERT(GGK_RCTRL)
    GG_ENUM_MAP_INSERT(GGK_LCTRL)
    GG_ENUM_MAP_INSERT(GGK_RALT)
    GG_ENUM_MAP_INSERT(GGK_LALT)
    GG_ENUM_MAP_INSERT(GGK_RMETA)
    GG_ENUM_MAP_INSERT(GGK_LMETA)
    GG_ENUM_MAP_INSERT(GGK_LSUPER)
    GG_ENUM_MAP_INSERT(GGK_RSUPER)
    GG_ENUM_MAP_INSERT(GGK_MODE)
    GG_ENUM_MAP_INSERT(GGK_COMPOSE)

    GG_ENUM_MAP_INSERT(GGK_HELP)
    GG_ENUM_MAP_INSERT(GGK_PRINT)
    GG_ENUM_MAP_INSERT(GGK_SYSREQ)
    GG_ENUM_MAP_INSERT(GGK_BREAK)
    GG_ENUM_MAP_INSERT(GGK_MENU)
    GG_ENUM_MAP_INSERT(GGK_POWER)
    GG_ENUM_MAP_INSERT(GGK_EURO)
    GG_ENUM_MAP_INSERT(GGK_UNDO)

    GG_ENUM_MAP_INSERT(GGK_LAST)
GG_ENUM_MAP_END

GG_ENUM_STREAM_IN(Key)
GG_ENUM_STREAM_OUT(Key)

} // namespace GG

#endif // _GG_Base_h_

