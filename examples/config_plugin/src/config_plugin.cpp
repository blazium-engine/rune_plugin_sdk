/**
 * Example Config Plugin for RUNE
 * 
 * Demonstrates:
 * - Plugin settings with JSON schema
 * - Nested menu items
 * - JSON, CSV, and INI parsing APIs
 */

#define NODEPLUG_BUILDING
#include "rune_plugin.h"
#include <cstring>
#include <cstdio>

static HostServices* g_host = nullptr;

/* ============================================================================
 * Settings Schema
 * ============================================================================ */

static const char* SETTINGS_SCHEMA = R"({
    "type": "object",
    "properties": {
        "enabled": {
            "type": "boolean",
            "title": "Enable Plugin",
            "description": "Enable or disable the plugin functionality"
        },
        "log_level": {
            "type": "string",
            "title": "Log Level",
            "enum": ["debug", "info", "warn", "error"],
            "default": "info"
        },
        "max_items": {
            "type": "integer",
            "title": "Maximum Items",
            "minimum": 1,
            "maximum": 1000,
            "default": 100
        },
        "api_key": {
            "type": "string",
            "title": "API Key",
            "description": "Optional API key for external services"
        }
    },
    "required": ["enabled", "log_level"]
})";

static const char* SETTINGS_DEFAULTS = R"({
    "enabled": true,
    "log_level": "info",
    "max_items": 100,
    "api_key": ""
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
        g_host->log(LOG_LEVEL_INFO, "Config plugin settings changed");
        
        // Parse the enabled flag
        const char* enabled = g_host->json_parse(settings_json, "enabled");
        if (enabled && enabled[0] != '\0') {
            g_host->log_formatted(LOG_LEVEL_DEBUG, "  enabled = %s", enabled);
        }
    }
}

/* ============================================================================
 * Menu Items
 * ============================================================================ */

static void on_menu_show_settings(void* user_data) {
    (void)user_data;
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Config Plugin: Show Settings clicked");
    }
}

static void on_menu_reload_config(void* user_data) {
    (void)user_data;
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Config Plugin: Reload Config clicked");
    }
}

static void on_menu_export_json(void* user_data) {
    (void)user_data;
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Config Plugin: Export as JSON clicked");
    }
}

static void on_menu_export_csv(void* user_data) {
    (void)user_data;
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Config Plugin: Export as CSV clicked");
    }
}

static void on_menu_export_ini(void* user_data) {
    (void)user_data;
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Config Plugin: Export as INI clicked");
    }
}

// Main plugin menu
static MenuItem g_pluginMenuItems[] = {
    {"Show Settings", NULL, on_menu_show_settings, NULL},
    {"Reload Config", NULL, on_menu_reload_config, NULL},
    {NULL, NULL, NULL, NULL},  // Separator
    {"Export", "Plugins/Config/Export", NULL, NULL},  // Submenu
};

// Export submenu
static MenuItem g_exportMenuItems[] = {
    {"As JSON...", NULL, on_menu_export_json, NULL},
    {"As CSV...", NULL, on_menu_export_csv, NULL},
    {"As INI...", NULL, on_menu_export_ini, NULL},
};

static MenuRegistration g_menus[] = {
    {"Plugins/Config", g_pluginMenuItems, 4},
    {"Plugins/Config/Export", g_exportMenuItems, 3},
};

static const MenuRegistration* get_menus(uint32_t* count) {
    *count = 2;
    return g_menus;
}

/* ============================================================================
 * JSON Parse Node
 * ============================================================================ */

static void* json_parse_create(void) {
    return nullptr;
}

static void json_parse_destroy(void* inst) {
    (void)inst;
}

static bool json_parse_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    const char* json_str = ctx->get_input_string(ctx, "JSON");
    const char* path = ctx->get_input_string(ctx, "Path");
    
    HostServices* host = ctx->get_host_services(ctx);
    if (!host) {
        ctx->set_error(ctx, "Host services unavailable");
        return false;
    }
    
    const char* result = host->json_parse(json_str, path);
    ctx->set_output_string(ctx, "Value", result ? result : "");
    ctx->set_output_bool(ctx, "Valid", result && result[0] != '\0');
    
    return true;
}

static NodeVTable json_parse_vtable = {
    json_parse_create,
    json_parse_destroy,
    NULL, NULL,
    NULL, NULL,
    json_parse_execute,
    NULL, NULL,
    NULL, NULL,
    NULL
};

static PinDesc json_parse_pins[] = {
    RUNE_EXEC_PIN_IN("Execute"),
    RUNE_DATA_PIN_IN("JSON", "string"),
    RUNE_DATA_PIN_IN("Path", "string"),
    RUNE_EXEC_PIN_OUT("Done"),
    RUNE_DATA_PIN_OUT("Value", "string"),
    RUNE_DATA_PIN_OUT("Valid", "bool"),
};

static int json_color[] = {100, 150, 200};

static NodeDesc json_parse_desc = {
    "Parse JSON",
    "Config",
    "com.rune.example.config.json_parse",
    json_parse_pins,
    6,
    NODE_FLAG_NONE,
    json_color,
    NULL,
    "Parse JSON and extract value at path"
};

/* ============================================================================
 * CSV Parse Node
 * ============================================================================ */

static bool csv_parse_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    const char* csv_str = ctx->get_input_string(ctx, "CSV");
    const char* delimiter_str = ctx->get_input_string(ctx, "Delimiter");
    char delimiter = (delimiter_str && delimiter_str[0]) ? delimiter_str[0] : ',';
    
    HostServices* host = ctx->get_host_services(ctx);
    if (!host) {
        ctx->set_error(ctx, "Host services unavailable");
        return false;
    }
    
    CsvData* data = host->csv_parse(csv_str, delimiter);
    if (!data) {
        ctx->set_output_int(ctx, "RowCount", 0);
        ctx->set_output_string(ctx, "FirstCell", "");
        return true;
    }
    
    ctx->set_output_int(ctx, "RowCount", data->row_count);
    
    if (data->row_count > 0 && data->rows[0].count > 0) {
        ctx->set_output_string(ctx, "FirstCell", data->rows[0].cells[0]);
    } else {
        ctx->set_output_string(ctx, "FirstCell", "");
    }
    
    host->csv_free(data);
    return true;
}

static NodeVTable csv_parse_vtable = {
    json_parse_create,
    json_parse_destroy,
    NULL, NULL,
    NULL, NULL,
    csv_parse_execute,
    NULL, NULL,
    NULL, NULL,
    NULL
};

static PinDesc csv_parse_pins[] = {
    RUNE_EXEC_PIN_IN("Execute"),
    RUNE_DATA_PIN_IN("CSV", "string"),
    RUNE_DATA_PIN_IN("Delimiter", "string"),
    RUNE_EXEC_PIN_OUT("Done"),
    RUNE_DATA_PIN_OUT("RowCount", "int"),
    RUNE_DATA_PIN_OUT("FirstCell", "string"),
};

static NodeDesc csv_parse_desc = {
    "Parse CSV",
    "Config",
    "com.rune.example.config.csv_parse",
    csv_parse_pins,
    6,
    NODE_FLAG_NONE,
    json_color,
    NULL,
    "Parse CSV data"
};

/* ============================================================================
 * INI Get Node
 * ============================================================================ */

static bool ini_get_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    const char* ini_str = ctx->get_input_string(ctx, "INI");
    const char* section = ctx->get_input_string(ctx, "Section");
    const char* key = ctx->get_input_string(ctx, "Key");
    
    HostServices* host = ctx->get_host_services(ctx);
    if (!host) {
        ctx->set_error(ctx, "Host services unavailable");
        return false;
    }
    
    const char* value = host->ini_get(ini_str, section, key);
    ctx->set_output_string(ctx, "Value", value ? value : "");
    ctx->set_output_bool(ctx, "Found", value && value[0] != '\0');
    
    return true;
}

static NodeVTable ini_get_vtable = {
    json_parse_create,
    json_parse_destroy,
    NULL, NULL,
    NULL, NULL,
    ini_get_execute,
    NULL, NULL,
    NULL, NULL,
    NULL
};

static PinDesc ini_get_pins[] = {
    RUNE_EXEC_PIN_IN("Execute"),
    RUNE_DATA_PIN_IN("INI", "string"),
    RUNE_DATA_PIN_IN("Section", "string"),
    RUNE_DATA_PIN_IN("Key", "string"),
    RUNE_EXEC_PIN_OUT("Done"),
    RUNE_DATA_PIN_OUT("Value", "string"),
    RUNE_DATA_PIN_OUT("Found", "bool"),
};

static int ini_color[] = {150, 120, 180};

static NodeDesc ini_get_desc = {
    "INI Get",
    "Config",
    "com.rune.example.config.ini_get",
    ini_get_pins,
    7,
    NODE_FLAG_NONE,
    ini_color,
    NULL,
    "Get value from INI configuration"
};

/* ============================================================================
 * Plugin Lifecycle
 * ============================================================================ */

static bool on_load(HostServices* host) {
    g_host = host;
    host->log(LOG_LEVEL_INFO, "Config plugin loaded");
    
    // Demo: Test JSON validation
    bool valid = host->json_validate("{\"test\": 123}");
    host->log_formatted(LOG_LEVEL_DEBUG, "JSON validation test: %s", valid ? "passed" : "failed");
    
    // Demo: Test INI parsing
    const char* test_ini = "[section]\nkey=value\n";
    const char* val = host->ini_get(test_ini, "section", "key");
    host->log_formatted(LOG_LEVEL_DEBUG, "INI get test: %s", val ? val : "(null)");
    
    return true;
}

static void on_register(PluginNodeRegistry* reg, LuauRegistry* luau) {
    (void)luau;
    
    reg->register_node(&json_parse_desc, &json_parse_vtable);
    reg->register_node(&csv_parse_desc, &csv_parse_vtable);
    reg->register_node(&ini_get_desc, &ini_get_vtable);
    
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Config plugin registered 3 nodes");
    }
}

static void on_unload(void) {
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Config plugin unloaded");
    }
    g_host = nullptr;
}

static PluginAPI g_api = {
    {
        "com.rune.example.config",
        "Config Plugin",
        "1.0.0",
        "RUNE Team",
        "Example plugin demonstrating settings, menus, and data formats",
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
    get_menus
};

NODEPLUG_EXPORT const PluginAPI* NodePlugin_GetAPI(void) {
    return &g_api;
}

