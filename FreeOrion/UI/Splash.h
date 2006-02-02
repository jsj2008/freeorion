// -*- C++ -*-
#ifndef _Splash_h_
#define _Splash_h_

#include <string>
#include <vector>

namespace GG {
    class StaticGraphic;
}

/** Loads a set of StaticGraphics that display the splash screen in chunks no larger than 512x512, to be friendly on
    minimal GL implementations. */
void LoadSplashGraphics(std::vector<std::vector<GG::StaticGraphic*> >& graphics);

inline std::string SplashRevision()
{return "$Id$";}

#endif // _Splash_h_
