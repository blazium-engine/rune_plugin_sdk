/**
 * Example Environment Plugin for RUNE
 * 
 * Demonstrates:
 * - Reading environment variables via HostServices
 * - Accessing plugin settings
 * - Accessing RUNE application settings
 */

#define NODEPLUG_BUILDING
#include "rune_plugin.h"
#include <cstring>
#include <cstdio>

static HostServices* g_host = nullptr;
static const char* PLUGIN_ID = "com.rune.example.env";

/* ============================================================================
 * Settings Schema
 * ============================================================================ */

static const char* SETTINGS_SCHEMA = R"({
    "type": "object",
    "properties": {
        "default_env_var": {
            "type": "string",
            "title": "Default Environment Variable",
            "description": "Default environment variable name to look up"
        },
        "show_debug_info": {
            "type": "boolean",
            "title": "Show Debug Info",
            "description": "Log additional debug information"
        }
    }
})";

static const char* SETTINGS_DEFAULTS = R"({
    "default_env_var": "PATH",
    "show_debug_info": false
})";

static PluginSettingsSchema g_settingsSchema = {
    SETTINGS_SCHEMA,
    SETTINGS_DEFAULTS
};

static const PluginSettingsSchema* get_settings_schema(void) {
    return &g_settingsSchema;
}

static void on_settings_changed(const char* settings_json) {
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Env plugin settings changed");
        
        const char* show_debug = g_host->json_parse(settings_json, "show_debug_info");
        if (show_debug && strcmp(show_debug, "true") == 0) {
            g_host->log(LOG_LEVEL_DEBUG, "Debug mode enabled");
        }
    }
}

/* ============================================================================
 * Get Environment Variable Node
 * ============================================================================ */

static void* env_get_create(void) {
    return nullptr;
}

static void env_get_destroy(void* inst) {
    (void)inst;
}

static bool env_get_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    const char* var_name = ctx->get_input_string(ctx, "Name");
    
    HostServices* host = ctx->get_host_services(ctx);
    if (!host) {
        ctx->set_error(ctx, "Host services unavailable");
        return false;
    }
    
    // Check if variable exists
    bool exists = RUNE_ENV_HAS(host, var_name);
    ctx->set_output_bool(ctx, "Exists", exists);
    
    if (exists) {
        const char* value = RUNE_ENV_GET(host, var_name);
        ctx->set_output_string(ctx, "Value", value ? value : "");
    } else {
        ctx->set_output_string(ctx, "Value", "");
    }
    
    return true;
}

static NodeVTable env_get_vtable = {
    env_get_create,
    env_get_destroy,
    NULL, NULL,
    NULL, NULL,
    env_get_execute,
    NULL, NULL,
    NULL, NULL,
    NULL
};

static PinDesc env_get_pins[] = {
    RUNE_EXEC_PIN_IN("Execute"),
    RUNE_DATA_PIN_IN("Name", "string"),
    RUNE_EXEC_PIN_OUT("Done"),
    RUNE_DATA_PIN_OUT("Value", "string"),
    RUNE_DATA_PIN_OUT("Exists", "bool"),
};

static int env_color[] = {80, 160, 120};

static NodeDesc env_get_desc = {
    "Get Env Variable",
    "Environment",
    "com.rune.example.env.get_env",
    env_get_pins,
    5,
    NODE_FLAG_NONE,
    env_color,
    NULL,
    "Get environment variable value from .env files or flow environment"
};

/* ============================================================================
 * Get Plugin Settings Node
 * ============================================================================ */

static bool plugin_settings_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    const char* plugin_id = ctx->get_input_string(ctx, "PluginID");
    
    HostServices* host = ctx->get_host_services(ctx);
    if (!host) {
        ctx->set_error(ctx, "Host services unavailable");
        return false;
    }
    
    const char* settings = RUNE_GET_PLUGIN_SETTINGS(host, plugin_id);
    ctx->set_output_string(ctx, "Settings", settings ? settings : "{}");
    
    return true;
}

static NodeVTable plugin_settings_vtable = {
    env_get_create,
    env_get_destroy,
    NULL, NULL,
    NULL, NULL,
    plugin_settings_execute,
    NULL, NULL,
    NULL, NULL,
    NULL
};

static PinDesc plugin_settings_pins[] = {
    RUNE_EXEC_PIN_IN("Execute"),
    RUNE_DATA_PIN_IN("PluginID", "string"),
    RUNE_EXEC_PIN_OUT("Done"),
    RUNE_DATA_PIN_OUT("Settings", "json"),
};

static int settings_color[] = {120, 100, 180};

static NodeDesc plugin_settings_desc = {
    "Get Plugin Settings",
    "Environment",
    "com.rune.example.env.get_plugin_settings",
    plugin_settings_pins,
    4,
    NODE_FLAG_NONE,
    settings_color,
    NULL,
    "Get a plugin's current settings as JSON"
};

/* ============================================================================
 * Get RUNE Setting Node
 * ============================================================================ */

static bool rune_setting_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    const char* setting_name = ctx->get_input_string(ctx, "Setting");
    
    HostServices* host = ctx->get_host_services(ctx);
    if (!host) {
        ctx->set_error(ctx, "Host services unavailable");
        return false;
    }
    
    const char* value = RUNE_GET_SETTING(host, setting_name);
    ctx->set_output_string(ctx, "Value", value ? value : "");
    ctx->set_output_bool(ctx, "Found", value && value[0] != '\0');
    
    return true;
}

static NodeVTable rune_setting_vtable = {
    env_get_create,
    env_get_destroy,
    NULL, NULL,
    NULL, NULL,
    rune_setting_execute,
    NULL, NULL,
    NULL, NULL,
    NULL
};

static PinDesc rune_setting_pins[] = {
    RUNE_EXEC_PIN_IN("Execute"),
    RUNE_DATA_PIN_IN("Setting", "string"),
    RUNE_EXEC_PIN_OUT("Done"),
    RUNE_DATA_PIN_OUT("Value", "string"),
    RUNE_DATA_PIN_OUT("Found", "bool"),
};

static int rune_color[] = {180, 100, 100};

static NodeDesc rune_setting_desc = {
    "Get RUNE Setting",
    "Environment",
    "com.rune.example.env.get_rune_setting",
    rune_setting_pins,
    5,
    NODE_FLAG_NONE,
    rune_color,
    NULL,
    "Get a RUNE application setting (cache_directory, flows_directory, etc.)"
};

/* ============================================================================
 * Plugin Lifecycle
 * ============================================================================ */

static bool on_load(HostServices* host) {
    g_host = host;
    host->log(LOG_LEVEL_INFO, "Environment plugin loaded");
    
    // Demo: Check environment variable access
    if (host->env_has("PATH")) {
        host->log(LOG_LEVEL_DEBUG, "PATH environment variable is accessible");
    }
    
    // Demo: Read RUNE settings
    const char* cache_dir = host->get_rune_setting("cache_directory");
    if (cache_dir && cache_dir[0]) {
        host->log_formatted(LOG_LEVEL_DEBUG, "RUNE cache directory: %s", cache_dir);
    }
    
    // Demo: Read own plugin settings
    const char* settings = host->get_plugin_settings(PLUGIN_ID);
    if (settings) {
        host->log_formatted(LOG_LEVEL_DEBUG, "Plugin settings: %s", settings);
    }
    
    return true;
}

static void on_register(PluginNodeRegistry* reg, LuauRegistry* luau) {
    (void)luau;
    
    reg->register_node(&env_get_desc, &env_get_vtable);
    reg->register_node(&plugin_settings_desc, &plugin_settings_vtable);
    reg->register_node(&rune_setting_desc, &rune_setting_vtable);
    
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Environment plugin registered 3 nodes");
    }
}

static void on_unload(void) {
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Environment plugin unloaded");
    }
    g_host = nullptr;
}

static PluginAPI g_api = {
    {
        PLUGIN_ID,
        "Environment Plugin",
        "1.0.0",
        "RUNE Team",
        "Example plugin demonstrating environment variable and settings access",
        RUNE_PLUGIN_API_VERSION
    },
    on_load,
    on_register,
    on_unload,
    NULL,  // on_tick
    NULL,  // on_flow_loaded
    NULL,  // on_flow_unloaded
    get_settings_schema,
    on_settings_changed,
    NULL   // get_menus
};

NODEPLUG_EXPORT const PluginAPI* NodePlugin_GetAPI(void) {
    return &g_api;
}

