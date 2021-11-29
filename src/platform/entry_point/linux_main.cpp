#include <types/types.h>
#include <editor/sources.cpp>

namespace Vultr
{
    namespace Platform
    {
        struct EntryArgs
        {
            int argc    = 0;
            char **argv = nullptr;
        };
    } // namespace Platform
} // namespace Vultr

using namespace Vultr;

int main(int argc, char **argv)
{
    Platform::EntryArgs args = {.argc = argc, .argv = argv};

    return Vultr::vultr_main(&args);
}
