/**
 * Example Timer Plugin for RUNE
 * 
 * Demonstrates creating trigger/event nodes (NODE_FLAG_TRIGGER_EVENT).
 * These nodes serve as entry points that fire based on external events.
 */

#define NODEPLUG_BUILDING
#include "rune_plugin.h"
#include <cstring>
#include <cstdlib>
#include <stdexcept>

static HostServices* g_host = nullptr;

// Helper: check if a given application environment flag is set to a truthy value.
// This is used for crash-testing the host's plugin safety guards. In normal
// operation these flags are unset, and the plugin behaves as usual.
static bool IsTestFlagEnabled(HostServices* host, const char* key)
{
    if (!host || !host->app_env_get || !key)
        return false;

    const char* value = host->app_env_get(key);
    if (!value || !value[0])
        return false;

    if (std::strcmp(value, "1") == 0)
        return true;

    if (std::strcmp(value, "true") == 0 || std::strcmp(value, "TRUE") == 0)
        return true;

    return false;
}

/* ============================================================================
 * Timer Event Node
 * 
 * Fires at a configurable interval (in milliseconds).
 * ============================================================================ */

typedef struct TimerInstance {
    uint64_t timer_id;
    ExecContext* ctx;
    bool active;
    uint32_t interval_ms;
    uint64_t tick_count;
} TimerInstance;

static void timer_callback(void* user_data) {
    TimerInstance* inst = (TimerInstance*)user_data;
    if (!inst || !inst->active || !inst->ctx) {
        return;
    }
    
    inst->tick_count++;
    
    // Set output values
    inst->ctx->set_output_int(inst->ctx, "TickCount", (int64_t)inst->tick_count);
    
    // Trigger the execution output
    inst->ctx->trigger_output(inst->ctx, "OnTimer");
}

static void* timer_create(void) {
    TimerInstance* inst = (TimerInstance*)malloc(sizeof(TimerInstance));
    if (inst) {
        inst->timer_id = 0;
        inst->ctx = NULL;
        inst->active = false;
        inst->interval_ms = 1000;
        inst->tick_count = 0;
    }
    return inst;
}

static void timer_destroy(void* inst_ptr) {
    TimerInstance* inst = (TimerInstance*)inst_ptr;
    if (inst) {
        if (inst->timer_id && g_host) {
            g_host->destroy_timer(inst->timer_id);
        }
        free(inst);
    }
}

static bool timer_start_listening(void* inst_ptr, ExecContext* ctx) {
    TimerInstance* inst = (TimerInstance*)inst_ptr;
    if (!inst || !ctx || !g_host) {
        return false;
    }
    
    // Get interval from property
    const char* interval_str = ctx->get_property(ctx, "IntervalMs");
    if (interval_str && interval_str[0]) {
        inst->interval_ms = (uint32_t)atoi(interval_str);
    }
    
    // Default to 1000ms if invalid
    if (inst->interval_ms == 0) {
        inst->interval_ms = 1000;
    }
    
    // Store context for callback
    inst->ctx = ctx;
    inst->active = true;
    inst->tick_count = 0;
    
    // Create timer
    inst->timer_id = g_host->create_timer(inst->interval_ms, timer_callback, inst);
    
    if (inst->timer_id == 0) {
        g_host->log(LOG_LEVEL_ERROR, "Failed to create timer");
        return false;
    }
    
    g_host->log_formatted(LOG_LEVEL_INFO, "Timer started with interval %u ms", inst->interval_ms);
    return true;
}

static void timer_stop_listening(void* inst_ptr) {
    TimerInstance* inst = (TimerInstance*)inst_ptr;
    if (!inst) {
        return;
    }
    
    inst->active = false;
    
    if (inst->timer_id && g_host) {
        g_host->destroy_timer(inst->timer_id);
        inst->timer_id = 0;
        g_host->log(LOG_LEVEL_INFO, "Timer stopped");
    }
}

static bool timer_execute(void* inst_ptr, ExecContext* ctx) {
    (void)inst_ptr;
    (void)ctx;
    // Timer event nodes don't execute directly - they trigger via callback
    return true;
}

static NodeVTable timer_vtable = {
    timer_create,
    timer_destroy,
    NULL, NULL,     // draw_inspector, draw_node_body
    NULL, NULL,     // serialize, deserialize
    timer_execute,
    NULL, NULL,     // on_pre_execute, on_post_execute
    timer_start_listening,
    timer_stop_listening,
    NULL            // is_complete
};

static PinDesc timer_pins[] = {
    {"IntervalMs", "int", PIN_IN, PIN_KIND_DATA, 0},
    {"OnTimer", "execution", PIN_OUT, PIN_KIND_EXECUTION, 0},
    {"TickCount", "int", PIN_OUT, PIN_KIND_DATA, 0},
};

static int timer_color[] = {200, 150, 100};

static NodeDesc timer_desc = {
    "Timer Event",
    "Events",
    "com.rune.example.timer.event",
    timer_pins,
    3,
    NODE_FLAG_TRIGGER_EVENT,
    timer_color,
    NULL,
    "Fires at a configurable interval"
};

/* ============================================================================
 * Delay Node
 * 
 * Delays execution by a specified amount of time.
 * ============================================================================ */

typedef struct DelayInstance {
    uint64_t timer_id;
    ExecContext* ctx;
    bool completed;
} DelayInstance;

static void delay_callback(void* user_data) {
    DelayInstance* inst = (DelayInstance*)user_data;
    if (!inst || !inst->ctx) {
        return;
    }
    
    inst->completed = true;
    
    // Trigger the execution output
    inst->ctx->trigger_output(inst->ctx, "OnComplete");
    
    // Destroy the one-shot timer
    if (inst->timer_id && g_host) {
        g_host->destroy_timer(inst->timer_id);
        inst->timer_id = 0;
    }
}

static void* delay_create(void) {
    DelayInstance* inst = (DelayInstance*)malloc(sizeof(DelayInstance));
    if (inst) {
        inst->timer_id = 0;
        inst->ctx = NULL;
        inst->completed = false;
    }
    return inst;
}

static void delay_destroy(void* inst_ptr) {
    DelayInstance* inst = (DelayInstance*)inst_ptr;
    if (inst) {
        if (inst->timer_id && g_host) {
            g_host->destroy_timer(inst->timer_id);
        }
        free(inst);
    }
}

static bool delay_execute(void* inst_ptr, ExecContext* ctx) {
    DelayInstance* inst = (DelayInstance*)inst_ptr;
    if (!inst || !ctx || !g_host) {
        return false;
    }

    // Crash-testing hook for node execution: when the flag is enabled, this
    // node will deliberately throw so the host can confirm that plugin node
    // exceptions are contained and reported without crashing the app.
    if (IsTestFlagEnabled(g_host, "RUNE_TEST_TIMER_THROW_IN_DELAY_EXECUTE")) {
        throw std::runtime_error("Timer plugin test exception in delay_execute");
    }
    
    // Get delay from input
    int64_t delay_ms = ctx->get_input_int(ctx, "DelayMs");
    if (delay_ms <= 0) {
        delay_ms = 1000; // Default 1 second
    }
    
    inst->ctx = ctx;
    inst->completed = false;
    
    // Create one-shot timer
    inst->timer_id = g_host->create_timer((uint32_t)delay_ms, delay_callback, inst);
    
    if (inst->timer_id == 0) {
        g_host->log(LOG_LEVEL_ERROR, "Failed to create delay timer");
        return false;
    }
    
    g_host->log_formatted(LOG_LEVEL_DEBUG, "Delay started: %lld ms", (long long)delay_ms);
    return true;
}

static bool delay_is_complete(void* inst_ptr) {
    DelayInstance* inst = (DelayInstance*)inst_ptr;
    return inst ? inst->completed : true;
}

static NodeVTable delay_vtable = {
    delay_create,
    delay_destroy,
    NULL, NULL,
    NULL, NULL,
    delay_execute,
    NULL, NULL,
    NULL, NULL,
    delay_is_complete
};

static PinDesc delay_pins[] = {
    {"Execute", "execution", PIN_IN, PIN_KIND_EXECUTION, 0},
    {"DelayMs", "int", PIN_IN, PIN_KIND_DATA, 0},
    {"OnComplete", "execution", PIN_OUT, PIN_KIND_EXECUTION, 0},
};

static int delay_color[] = {150, 150, 200};

static NodeDesc delay_desc = {
    "Delay",
    "Flow Control",
    "com.rune.example.timer.delay",
    delay_pins,
    3,
    NODE_FLAG_ASYNC,
    delay_color,
    NULL,
    "Delays execution by specified milliseconds"
};

/* ============================================================================
 * Plugin Lifecycle
 * ============================================================================ */

static bool on_load(HostServices* host) {
    g_host = host;

    // Crash-testing hook: when RUNE_TEST_TIMER_THROW_ON_LOAD is set in the
    // application environment, deliberately throw here so the host can verify
    // that plugin on_load exceptions are caught and handled safely.
    if (IsTestFlagEnabled(host, "RUNE_TEST_TIMER_THROW_ON_LOAD")) {
        throw std::runtime_error("Timer plugin test exception in on_load");
    }

    host->log(LOG_LEVEL_INFO, "Timer plugin loaded");
    return true;
}

static void on_register(PluginNodeRegistry* reg, LuauRegistry* luau) {
    (void)luau;

    if (g_host && IsTestFlagEnabled(g_host, "RUNE_TEST_TIMER_THROW_ON_REGISTER")) {
        throw std::runtime_error("Timer plugin test exception in on_register");
    }

    reg->register_node(&timer_desc, &timer_vtable);
    reg->register_node(&delay_desc, &delay_vtable);
    
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Timer plugin registered 2 nodes");
    }
}

static void on_unload(void) {
    if (g_host) {
        g_host->log(LOG_LEVEL_INFO, "Timer plugin unloaded");
    }
    g_host = NULL;
}

static PluginAPI g_api = {
    {
        "com.rune.example.timer",
        "Timer Plugin",
        "1.0.0",
        "RUNE Team",
        "Example plugin demonstrating trigger/event nodes",
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

