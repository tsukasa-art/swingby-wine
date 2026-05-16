/* Stub for i386: Vulkan is not supported on 32-bit. */
#include "wined3d_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(d3d);

struct wined3d_adapter *wined3d_adapter_vk_create(unsigned int ordinal,
        unsigned int wined3d_creation_flags)
{
    WARN("Vulkan not supported on i386.\n");
    return NULL;
}
