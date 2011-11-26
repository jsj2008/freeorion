#include "../util/Version.h"

namespace {
    static const std::string retval = "post-v0.3.17 [GitMirror / SVN r4503] Xcode";
}

const std::string& FreeOrionVersionString()
{
    return retval;
}
