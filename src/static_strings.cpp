#include "static_strings.h"

using namespace gdluau;
using namespace godot;

StaticStrings *gdluau::static_strings = nullptr;

void gdluau::initialize_static_strings()
{
    static_strings = memnew(StaticStrings);
    static_strings->interrupt = StringName("interrupt");
    static_strings->debugbreak = StringName("debugbreak");
    static_strings->debugstep = StringName("debugstep");
}

void gdluau::uninitialize_static_strings()
{
    if (static_strings)
    {
        memdelete(static_strings);
        static_strings = nullptr;
    }
}
