#include "../util/Version.h"

namespace {
    static const std::string retval = "post-v0.3.17 [SVN 4386/GitHub Mirror] Xcode";
}

const std::string& FreeOrionVersionString()
{
    return retval;
}
