/**
 * RUNE Plugin API - C ABI Interface
 * 
 * This header defines the C ABI boundary for RUNE plugins.
 * Plugins must implement the NodePlugin_GetAPI() function that returns
 * a pointer to a PluginAPI struct.
 * 
 * Example usage:
 *   NODEPLUG_EXPORT const PluginAPI* NodePlugin_GetAPI() {
 *       static PluginAPI api = { ... };
 *       return &api;
 *   }
 */

#ifndef RUNE_PLUGIN_API_H
#define RUNE_PLUGIN_API_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * Platform-specific export macros
 * ========================================================================== */

#ifdef __cplusplus
    #define NODEPLUG_EXTERN_C extern "C"
#else
    #define NODEPLUG_EXTERN_C
#endif

#if defined(_WIN32) || defined(_WIN64)
    #ifdef NODEPLUG_BUILDING
        #define NODEPLUG_EXPORT NODEPLUG_EXTERN_C __declspec(dllexport)
    #else
        #define NODEPLUG_EXPORT NODEPLUG_EXTERN_C __declspec(dllimport)
    #endif
#else
    #define NODEPLUG_EXPORT NODEPLUG_EXTERN_C __attribute__((visibility("default")))
#endif

/* ==========================================================================
 * API Version
 * ========================================================================== */

#define RUNE_PLUGIN_API_VERSION 1

/* ==========================================================================
 * Forward declarations
 * ========================================================================== */

typedef struct HostServices HostServices;
typedef struct PluginNodeRegistry PluginNodeRegistry;
typedef struct LuauRegistry LuauRegistry;
typedef struct ExecContext ExecContext;

/* ==========================================================================
 * Log Levels
 * ========================================================================== */

typedef enum PluginLogLevel {
    PLUGIN_LOG_LEVEL_DEBUG = 0,
    PLUGIN_LOG_LEVEL_INFO  = 1,
    PLUGIN_LOG_LEVEL_WARN  = 2,
    PLUGIN_LOG_LEVEL_ERROR = 3
} PluginLogLevel;

/* Compatibility aliases */
#define LOG_LEVEL_DEBUG PLUGIN_LOG_LEVEL_DEBUG
#define LOG_LEVEL_INFO  PLUGIN_LOG_LEVEL_INFO
#define LOG_LEVEL_WARN  PLUGIN_LOG_LEVEL_WARN
#define LOG_LEVEL_ERROR PLUGIN_LOG_LEVEL_ERROR

/* ==========================================================================
 * Node Flags - Classification of node types
 * ========================================================================== */

typedef enum NodeFlags {
    NODE_FLAG_NONE           = 0,
    NODE_FLAG_TRIGGER_EVENT  = 1 << 0,  /* Entry point node (no exec inputs, only exec outputs) */
    NODE_FLAG_PURE_DATA      = 1 << 1,  /* No execution pins, only data flow */
    NODE_FLAG_ASYNC          = 1 << 2,  /* Can run asynchronously */
    NODE_FLAG_STATEFUL       = 1 << 3,  /* Maintains state between executions */
    NODE_FLAG_HIDDEN         = 1 << 4   /* Not shown in node menu */
} NodeFlags;

/* ==========================================================================
 * Pin Types
 * ========================================================================== */

typedef enum PinDirection {
    PIN_IN  = 0,
    PIN_OUT = 1
} PinDirection;

typedef enum PinKind {
    PIN_KIND_DATA      = 0,  /* Data flow pin */
    PIN_KIND_EXECUTION = 1   /* Execution flow pin */
} PinKind;

typedef enum PinFlags {
    PIN_FLAG_NONE         = 0,
    PIN_FLAG_OPTIONAL     = 1 << 0,  /* Pin connection is optional */
    PIN_FLAG_MULTI_CONNECT = 1 << 1, /* Pin can have multiple connections */
    PIN_FLAG_HIDDEN       = 1 << 2   /* Pin is hidden in UI */
} PinFlags;

/* ==========================================================================
 * Pin Type IDs - Built-in types
 * ========================================================================== */

typedef uint64_t PinTypeId;

#define PIN_TYPE_STRING    ((PinTypeId)1)
#define PIN_TYPE_INT       ((PinTypeId)2)
#define PIN_TYPE_FLOAT     ((PinTypeId)3)
#define PIN_TYPE_BOOL      ((PinTypeId)4)
#define PIN_TYPE_JSON      ((PinTypeId)5)
#define PIN_TYPE_BLOB      ((PinTypeId)6)
#define PIN_TYPE_PATH      ((PinTypeId)7)
#define PIN_TYPE_EXECUTION ((PinTypeId)100)

/* Custom pin types start at this ID */
#define PIN_TYPE_CUSTOM_START ((PinTypeId)1000)

/* ==========================================================================
 * Pin Description
 * ========================================================================== */

typedef struct PinDesc {
    const char*   name;       /* Display name of the pin */
    const char*   type;       /* Type name: "string", "int", "float", "bool", "json", "execution", or custom */
    PinDirection  direction;  /* PIN_IN or PIN_OUT */
    PinKind       kind;       /* PIN_KIND_DATA or PIN_KIND_EXECUTION */
    uint32_t      flags;      /* Combination of PinFlags */
} PinDesc;

/* ==========================================================================
 * Node Type ID
 * ========================================================================== */

typedef uint64_t NodeTypeId;

/* ==========================================================================
 * Node Description
 * ========================================================================== */

typedef struct NodeDesc {
    const char*     name;         /* Display name */
    const char*     category;     /* Category for menu grouping (e.g., "Events", "Math", "IO") */
    const char*     unique_name;  /* Unique identifier for serialization (e.g., "com.example.mynode") */
    const PinDesc*  pins;         /* Array of pin descriptions */
    uint32_t        pin_count;    /* Number of pins */
    uint32_t        flags;        /* Combination of NodeFlags */
    const int*      color;        /* Optional RGB color (3 ints), NULL for default */
    const char*     icon;         /* Optional icon name, NULL for default */
    const char*     description;  /* Optional description for tooltip */
} NodeDesc;

/* ==========================================================================
 * Node VTable - Functions implemented by the plugin for each node type
 * ========================================================================== */

typedef struct NodeVTable {
    /* Instance lifecycle */
    void* (*create_instance)(void);
    void  (*destroy_instance)(void* inst);

    /* UI rendering (called on main thread) */
    void (*draw_inspector)(void* inst);    /* Right-side properties panel */
    void (*draw_node_body)(void* inst);    /* Optional custom node body UI */

    /* Serialization */
    bool (*serialize)(void* inst, uint8_t** out_data, uint32_t* out_size);
    bool (*deserialize)(void* inst, const uint8_t* data, uint32_t size);

    /* Execution */
    bool (*execute)(void* inst, ExecContext* ctx);

    /* Optional lifecycle hooks */
    void (*on_pre_execute)(void* inst, ExecContext* ctx);
    void (*on_post_execute)(void* inst, ExecContext* ctx, bool success);

    /* Event node specific - called when node should start/stop listening */
    bool (*start_listening)(void* inst, ExecContext* ctx);
    void (*stop_listening)(void* inst);
    
    /* Async node specific - poll for completion */
    bool (*is_complete)(void* inst);
    
} NodeVTable;

/* ==========================================================================
 * Execution Context - Passed to node execution
 * ========================================================================== */

struct ExecContext {
    /* Get input value by pin name */
    const char* (*get_input_string)(ExecContext* ctx, const char* pin_name);
    int64_t     (*get_input_int)(ExecContext* ctx, const char* pin_name);
    double      (*get_input_float)(ExecContext* ctx, const char* pin_name);
    bool        (*get_input_bool)(ExecContext* ctx, const char* pin_name);
    const char* (*get_input_json)(ExecContext* ctx, const char* pin_name);
    
    /* Set output value by pin name */
    void (*set_output_string)(ExecContext* ctx, const char* pin_name, const char* value);
    void (*set_output_int)(ExecContext* ctx, const char* pin_name, int64_t value);
    void (*set_output_float)(ExecContext* ctx, const char* pin_name, double value);
    void (*set_output_bool)(ExecContext* ctx, const char* pin_name, bool value);
    void (*set_output_json)(ExecContext* ctx, const char* pin_name, const char* json_str);
    
    /* Get node property value */
    const char* (*get_property)(ExecContext* ctx, const char* property_name);
    
    /* Set error message */
    void (*set_error)(ExecContext* ctx, const char* error_msg);
    
    /* Trigger execution output (for event nodes) */
    void (*trigger_output)(ExecContext* ctx, const char* exec_pin_name);
    
    /* Access to host services */
    HostServices* (*get_host_services)(ExecContext* ctx);
    
    /* Opaque context data - do not modify */
    void* _internal;
};

/* ==========================================================================
 * Job System Types
 * ========================================================================== */

typedef struct JobHandle {
    uint64_t id;
} JobHandle;

typedef void (*JobFunction)(void* user_data);
typedef void (*JobCompletionCallback)(void* user_data, bool success);

/* ==========================================================================
 * CSV Data Types
 * ========================================================================== */

typedef struct CsvRow {
    const char** cells;
    uint32_t count;
} CsvRow;

typedef struct CsvData {
    CsvRow* rows;
    uint32_t row_count;
} CsvData;

/* ==========================================================================
 * Host Services - Provided by RUNE to plugins
 * ========================================================================== */

struct HostServices {
    uint32_t api_version;
    
    /* Logging */
    void (*log)(PluginLogLevel level, const char* message);
    void (*log_formatted)(PluginLogLevel level, const char* format, ...);
    
    /* Threading / Job System */
    JobHandle (*submit_job)(JobFunction fn, void* user_data, JobCompletionCallback on_complete);
    bool      (*poll_job)(JobHandle handle);
    void      (*cancel_job)(JobHandle handle);
    
    /* Paths */
    const char* (*get_plugin_data_dir)(const char* plugin_id);
    const char* (*get_cache_dir)(void);
    const char* (*get_flows_dir)(void);
    
    /* Capabilities */
    bool (*has_capability)(const char* capability);
    
    /* Memory allocation (use these for data passed across DLL boundary) */
    void* (*alloc)(size_t size);
    void  (*free)(void* ptr);
    
    /* Timer for event nodes (returns timer ID, 0 on failure) */
    uint64_t (*create_timer)(uint32_t interval_ms, void (*callback)(void* user_data), void* user_data);
    void     (*destroy_timer)(uint64_t timer_id);
    
    /* JSON operations */
    const char* (*json_parse)(const char* json_str, const char* json_path);
    char* (*json_stringify)(const char* json_obj);
    bool (*json_validate)(const char* json_str);
    
    /* CSV operations */
    CsvData* (*csv_parse)(const char* csv_str, char delimiter);
    void (*csv_free)(CsvData* data);
    char* (*csv_stringify)(const CsvData* data, char delimiter);
    
    /* INI operations */
    const char* (*ini_get)(const char* ini_str, const char* section, const char* key);
    char* (*ini_set)(const char* ini_str, const char* section, const char* key, const char* value);
    char** (*ini_get_sections)(const char* ini_str, uint32_t* count);
    char** (*ini_get_keys)(const char* ini_str, const char* section, uint32_t* count);
    void (*ini_free_strings)(char** strings, uint32_t count);
};

/* ==========================================================================
 * Node Registry - For registering nodes and pin types
 * ========================================================================== */

struct PluginNodeRegistry {
    /* Register a custom pin type */
    PinTypeId (*register_pin_type)(const char* name, uint32_t size, uint32_t flags);
    
    /* Register a node type */
    NodeTypeId (*register_node)(const NodeDesc* desc, const NodeVTable* vtbl);
    
    /* Unregister a node type (for hot-reload) */
    void (*unregister_node)(NodeTypeId type_id);
    
    /* Get built-in pin type ID by name */
    PinTypeId (*get_pin_type_id)(const char* type_name);
};


/* ==========================================================================
 * Luau Registry - For registering Luau bindings
 * ========================================================================== */

typedef int (*LuaCFunction)(void* L);

struct LuauRegistry {
    /* Get the Luau state for this plugin (isolated environment) */
    void* (*get_plugin_state)(const char* plugin_id);
    
    /* Register a global function */
    void (*register_global)(void* L, const char* name, LuaCFunction fn);
    
    /* Register a library table (e.g., "myplugin" with functions) */
    void (*register_library)(void* L, const char* lib_name,
                            const char** fn_names, LuaCFunction* fn_ptrs, uint32_t count);
    
    /* Set sandbox policy for this plugin's Luau state */
    void (*set_sandbox_policy)(void* L, const char* policy_name);
};

/* ==========================================================================
 * Plugin Settings Schema (JSON Schema-based)
 * ========================================================================== */

typedef struct PluginSettingsSchema {
    const char* schema_json;   /* JSON schema defining settings structure */
    const char* defaults_json; /* Default values as JSON */
} PluginSettingsSchema;

/* ==========================================================================
 * Plugin Menubar Types
 * ========================================================================== */

typedef void (*MenuItemCallback)(void* user_data);

typedef struct MenuItem {
    const char* label;           /* Display label, NULL for separator */
    const char* submenu_id;      /* If non-NULL, this item opens a submenu */
    MenuItemCallback callback;   /* Callback when clicked (ignored if submenu) */
    void* user_data;             /* User data passed to callback */
} MenuItem;

typedef struct MenuRegistration {
    const char* menu_id;         /* Menu path, e.g. "Tools" or "Plugins/MyPlugin" */
    const MenuItem* items;       /* Array of menu items */
    uint32_t item_count;         /* Number of items */
} MenuRegistration;

/* ==========================================================================
 * Plugin Info
 * ========================================================================== */

typedef struct PluginInfo {
    const char* id;           /* Unique plugin ID (e.g., "com.example.myplugin") */
    const char* name;         /* Display name */
    const char* version;      /* Semantic version (e.g., "1.0.0") */
    const char* author;       /* Author name/organization */
    const char* description;  /* Brief description */
    uint32_t    api_version;  /* Must match RUNE_PLUGIN_API_VERSION */
} PluginInfo;

/* ==========================================================================
 * Plugin API - Main interface returned by NodePlugin_GetAPI()
 * ========================================================================== */

typedef struct PluginAPI {
    PluginInfo info;
    
    /* Lifecycle callbacks */
    bool (*on_load)(HostServices* host);
    void (*on_register)(PluginNodeRegistry* node_reg, LuauRegistry* luau_reg);
    void (*on_unload)(void);
    
    /* Optional: Called each frame (for event polling, etc.) */
    void (*on_tick)(float delta_time);
    
    /* Optional: Called when a flow is loaded/unloaded */
    void (*on_flow_loaded)(const char* flow_id);
    void (*on_flow_unloaded)(const char* flow_id);
    
    /* Optional: Settings support (JSON schema-based) */
    const PluginSettingsSchema* (*get_settings_schema)(void);
    void (*on_settings_changed)(const char* settings_json);
    
    /* Optional: Menubar integration */
    const MenuRegistration* (*get_menus)(uint32_t* count);
    
} PluginAPI;

/* ==========================================================================
 * Entry Point - Must be implemented by all plugins
 * ========================================================================== */

/**
 * Entry point function that plugins must export.
 * Returns a pointer to a static PluginAPI struct.
 */
typedef const PluginAPI* (*PluginGetAPIFunc)(void);

/* Default entry symbol name */
#define RUNE_PLUGIN_ENTRY_SYMBOL "NodePlugin_GetAPI"

#ifdef __cplusplus
}
#endif

#endif /* RUNE_PLUGIN_API_H */
