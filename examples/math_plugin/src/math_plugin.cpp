/**
 * Example Math Plugin for RUNE
 * 
 * Demonstrates creating pure data nodes (NODE_FLAG_PURE_DATA).
 * These nodes perform calculations without execution flow.
 */

#define NODEPLUG_BUILDING
#include "rune_plugin.h"
#include <cmath>
#include <cstring>

static HostServices* g_host = nullptr;

/* ============================================================================
 * Add Node
 * ============================================================================ */

static void* add_create(void) {
    return nullptr; // No instance data needed
}

static void add_destroy(void* inst) {
    (void)inst;
}

static bool add_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    double a = ctx->get_input_float(ctx, "A");
    double b = ctx->get_input_float(ctx, "B");
    double result = a + b;
    
    ctx->set_output_float(ctx, "Result", result);
    return true;
}

static NodeVTable add_vtable = {
    add_create,
    add_destroy,
    NULL, NULL,     // draw_inspector, draw_node_body
    NULL, NULL,     // serialize, deserialize
    add_execute,
    NULL, NULL,     // on_pre_execute, on_post_execute
    NULL, NULL,     // start_listening, stop_listening
    NULL            // is_complete
};

static PinDesc add_pins[] = {
    {"A", "float", PIN_IN, PIN_KIND_DATA, 0},
    {"B", "float", PIN_IN, PIN_KIND_DATA, 0},
    {"Result", "float", PIN_OUT, PIN_KIND_DATA, 0},
};

static int add_color[] = {100, 200, 100};

static NodeDesc add_desc = {
    "Add",
    "Math",
    "com.rune.example.math.add",
    add_pins,
    3,
    NODE_FLAG_PURE_DATA,
    add_color,
    NULL,
    "Add two numbers together"
};

/* ============================================================================
 * Multiply Node
 * ============================================================================ */

static bool multiply_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    double a = ctx->get_input_float(ctx, "A");
    double b = ctx->get_input_float(ctx, "B");
    double result = a * b;
    
    ctx->set_output_float(ctx, "Result", result);
    return true;
}

static NodeVTable multiply_vtable = {
    add_create,
    add_destroy,
    NULL, NULL,
    NULL, NULL,
    multiply_execute,
    NULL, NULL,
    NULL, NULL,
    NULL
};

static PinDesc multiply_pins[] = {
    {"A", "float", PIN_IN, PIN_KIND_DATA, 0},
    {"B", "float", PIN_IN, PIN_KIND_DATA, 0},
    {"Result", "float", PIN_OUT, PIN_KIND_DATA, 0},
};

static NodeDesc multiply_desc = {
    "Multiply",
    "Math",
    "com.rune.example.math.multiply",
    multiply_pins,
    3,
    NODE_FLAG_PURE_DATA,
    add_color,
    NULL,
    "Multiply two numbers"
};

/* ============================================================================
 * Divide Node
 * ============================================================================ */

static bool divide_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    double a = ctx->get_input_float(ctx, "A");
    double b = ctx->get_input_float(ctx, "B");
    
    if (b == 0.0) {
        ctx->set_error(ctx, "Division by zero");
        return false;
    }
    
    double result = a / b;
    ctx->set_output_float(ctx, "Result", result);
    return true;
}

static NodeVTable divide_vtable = {
    add_create,
    add_destroy,
    NULL, NULL,
    NULL, NULL,
    divide_execute,
    NULL, NULL,
    NULL, NULL,
    NULL
};

static PinDesc divide_pins[] = {
    {"A", "float", PIN_IN, PIN_KIND_DATA, 0},
    {"B", "float", PIN_IN, PIN_KIND_DATA, 0},
    {"Result", "float", PIN_OUT, PIN_KIND_DATA, 0},
};

static NodeDesc divide_desc = {
    "Divide",
    "Math",
    "com.rune.example.math.divide",
    divide_pins,
    3,
    NODE_FLAG_PURE_DATA,
    add_color,
    NULL,
    "Divide A by B"
};

/* ============================================================================
 * Power Node
 * ============================================================================ */

static bool power_execute(void* inst, ExecContext* ctx) {
    (void)inst;
    
    double base = ctx->get_input_float(ctx, "Base");
    double exponent = ctx->get_input_float(ctx, "Exponent");
    double result = pow(base, exponent);
    
    ctx->set_output_float(ctx, "Result", result);
    return true;
}

static NodeVTable power_vtable = {
    add_create,
    add_destroy,
    NULL, NULL,
    NULL, NULL,
    power_execute,
    NULL, NULL,
    NULL, NULL,
    NULL
};

static PinDesc power_pins[] = {
    {"Base", "float", PIN_IN, PIN_KIND_DATA, 0},
    {"Exponent", "float", PIN_IN, PIN_KIND_DATA, 0},
    {"Result", "float", PIN_OUT, PIN_KIND_DATA, 0},
};

static NodeDesc power_desc = {
    "Power",
    "Math",
    "com.rune.example.math.power",
    power_pins,
    3,
    NODE_FLAG_PURE_DATA,
    add_color,
    NULL,
    "Raise Base to the power of Exponent"
};

/* ============================================================================
 * Plugin Lifecycle
 * ============================================================================ */

static bool on_load(HostServices* host) {
    g_host = host;
    host->log(LOG_LEVEL_INFO, "Math plugin loaded");
    return true;
}

static void on_register(PluginNodeRegistry* reg, LuauRegistry* luau) {
    (void)luau;
    
    reg->register_node(&add_desc, &add_vtable);
    reg->register_node(&multiply_desc, &multiply_vtable);
    reg->register_node(&divide_desc, &divide_vtable);
    reg->register_node(&power_desc, &power_vtable);
    
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Math plugin registered 4 nodes");
    }
}

static void on_unload(void) {
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Math plugin unloaded");
    }
    g_host = nullptr;
}

static PluginAPI g_api = {
    {
        "com.rune.example.math",
        "Math Plugin",
        "1.0.0",
        "RUNE Team",
        "Example plugin demonstrating pure data nodes",
        RUNE_PLUGIN_API_VERSION
    },
    on_load,
    on_register,
    on_unload,
    NULL, NULL, NULL
};

NODEPLUG_EXPORT const PluginAPI* NodePlugin_GetAPI(void) {
    return &g_api;
}

