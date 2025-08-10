#ifndef METADATA_H
#define METADATA_H

// WINDOW SETTINGS
#define WINDOW_RESIZABLE 1 // If you can resize the window, i might add a dynamic switch,
						   // but there are compile time ifs already so this will probably be 1 then you can set it using a variable.

// VULKAN APP SETTINGS
#define VULKAN_APP_NAME "SDL3VULKAN_TEMP"
#define VULKAN_APP_VER1 0
#define VULKAN_APP_VER2 0
#define VULKAN_APP_VER3 2

// VULKAN ENGINE SETTINGS
#define VULKAN_ENGINE_NAME "NONE"
#define VULKAN_ENGINE_VER1 0
#define VULKAN_ENGINE_VER2 0
#define VULKAN_ENGINE_VER3 1

// APP SETTINGS
#define NAME "SDL3+Vulkan"
#define BRANCH "main"
#define VERSION_STR "V" S(VULKAN_APP_VER1) "." S(VULKAN_APP_VER2) "." S(VULKAN_APP_VER3)
#define NAME_STR NAME " - " BRANCH " branch"

#endif // METADATA_H
