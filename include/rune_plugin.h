/**
 * RUNE Plugin SDK - Main Header
 * 
 * Include this single header to get access to the entire RUNE Plugin API.
 * 
 * Example plugin:
 * 
 *   #define NODEPLUG_BUILDING
 *   #include <rune_plugin.h>
 *   
 *   static bool on_load(HostServices* host) {
 *       host->log(LOG_LEVEL_INFO, "My plugin loaded!");
 *       return true;
 *   }
 *   
 *   static void on_register(NodeRegistry* reg, LuauRegistry* luau) {
 *       // Register your nodes here
 *   }
 *   
 *   static void on_unload() {
 *       // Cleanup
 *   }
 *   
 *   static PluginAPI g_api = {
 *       {"com.example.myplugin", "My Plugin", "1.0.0", "Author", "Description", RUNE_PLUGIN_API_VERSION},
 *       on_load,
 *       on_register,
 *       on_unload,
 *       NULL, NULL, NULL
 *   };
 *   
 *   NODEPLUG_EXPORT const PluginAPI* NodePlugin_GetAPI() {
 *       return &g_api;
 *   }
 */

#ifndef RUNE_PLUGIN_SDK_H
#define RUNE_PLUGIN_SDK_H

/* Include the main plugin API */
#include "plugin_api.h"

/* Helper macros for plugin development */

/**
 * RUNE_DEFINE_PLUGIN - Helper macro to define a simple plugin
 * 
 * Usage:
 *   RUNE_DEFINE_PLUGIN("com.example.myplugin", "My Plugin", "1.0.0", "Author", "Description")
 *   
 * Then implement:
 *   - bool MyPlugin_OnLoad(HostServices* host)
 *   - void MyPlugin_OnRegister(NodeRegistry* reg, LuauRegistry* luau)
 *   - void MyPlugin_OnUnload()
 */
#define RUNE_DEFINE_PLUGIN(id, name, version, author, desc) \
    static bool MyPlugin_OnLoad(HostServices* host); \
    static void MyPlugin_OnRegister(PluginNodeRegistry* reg, LuauRegistry* luau); \
    static void MyPlugin_OnUnload(void); \
    static PluginAPI g_MyPluginAPI = { \
        {id, name, version, author, desc, RUNE_PLUGIN_API_VERSION}, \
        MyPlugin_OnLoad, \
        MyPlugin_OnRegister, \
        MyPlugin_OnUnload, \
        NULL, NULL, NULL \
    }; \
    NODEPLUG_EXPORT const PluginAPI* NodePlugin_GetAPI(void) { \
        return &g_MyPluginAPI; \
    }

/**
 * RUNE_DEFINE_NODE - Helper macro to define a simple node
 * 
 * Usage:
 *   static PinDesc my_node_pins[] = {
 *       {"Input", "string", PIN_IN, PIN_KIND_DATA, 0},
 *       {"Output", "string", PIN_OUT, PIN_KIND_DATA, 0},
 *   };
 *   
 *   RUNE_DEFINE_NODE("My Node", "Category", "com.example.mynode", my_node_pins, 2, NODE_FLAG_NONE)
 */
#define RUNE_DEFINE_NODE(name, category, unique_name, pins, pin_count, flags) \
    { \
        name, \
        category, \
        unique_name, \
        pins, \
        pin_count, \
        flags, \
        NULL, \
        NULL, \
        NULL \
    }

/**
 * RUNE_DATA_PIN_IN - Define a data input pin
 */
#define RUNE_DATA_PIN_IN(name, type) \
    {name, type, PIN_IN, PIN_KIND_DATA, 0}

/**
 * RUNE_DATA_PIN_OUT - Define a data output pin
 */
#define RUNE_DATA_PIN_OUT(name, type) \
    {name, type, PIN_OUT, PIN_KIND_DATA, 0}

/**
 * RUNE_EXEC_PIN_IN - Define an execution input pin
 */
#define RUNE_EXEC_PIN_IN(name) \
    {name, "execution", PIN_IN, PIN_KIND_EXECUTION, 0}

/**
 * RUNE_EXEC_PIN_OUT - Define an execution output pin
 */
#define RUNE_EXEC_PIN_OUT(name) \
    {name, "execution", PIN_OUT, PIN_KIND_EXECUTION, 0}

/**
 * RUNE_DEFINE_SETTINGS_SCHEMA - Define a plugin settings schema
 * 
 * Usage:
 *   RUNE_DEFINE_SETTINGS_SCHEMA(
 *       "{\"type\":\"object\",\"properties\":{\"enabled\":{\"type\":\"boolean\"}}}",
 *       "{\"enabled\":true}"
 *   )
 */
#define RUNE_DEFINE_SETTINGS_SCHEMA(schema, defaults) \
    static PluginSettingsSchema g_SettingsSchema = { schema, defaults }; \
    static const PluginSettingsSchema* GetSettingsSchema(void) { return &g_SettingsSchema; }

/**
 * RUNE_MENU_ITEM - Define a menu item with callback
 */
#define RUNE_MENU_ITEM(label, callback, user_data) \
    { label, NULL, callback, user_data }

/**
 * RUNE_MENU_SUBMENU - Define a submenu item
 */
#define RUNE_MENU_SUBMENU(label, submenu_id) \
    { label, submenu_id, NULL, NULL }

/**
 * RUNE_MENU_SEPARATOR - Define a menu separator
 */
#define RUNE_MENU_SEPARATOR \
    { NULL, NULL, NULL, NULL }

/**
 * RUNE_DEFINE_MENU - Define a menu registration
 */
#define RUNE_DEFINE_MENU(menu_id, items_array, count) \
    { menu_id, items_array, count }

#endif /* RUNE_PLUGIN_SDK_H */

