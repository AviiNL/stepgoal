#include <pebble.h>

static Window *window;
static TextLayer *time_layer;
static TextLayer *date_layer;
static TextLayer *charge_layer;
static TextLayer *step_layer;

bool showClock = false;
bool goalReached = false;
int goal = 10000;
int today = 0;

static const uint32_t const segments[] = {375, 250, 375};
VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
};

static void handle_battery_state(BatteryChargeState charge)
{
    static char charge_text[5];

    snprintf(charge_text, sizeof(charge_text), "%d%%", charge.charge_percent);

    text_layer_set_text(charge_layer, charge_text);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)
{
    if (!showClock)
        return;

    static char time_text[24];
    static char date_text[24];

    strftime(time_text, sizeof(time_text), "%H:%M:%S", tick_time);
    strftime(date_text, sizeof(date_text), "%d/%m/%y", tick_time);

    text_layer_set_text(time_layer, time_text);
    text_layer_set_text(date_layer, date_text);
}

static void updateStepCounter()
{
    static char step_text[48];
    HealthMetric metric = HealthMetricStepCount;
    time_t start = time_start_of_today();
    time_t end = time(NULL);

    // Reset if new day
    if ((int)start > today)
    {
        today = (int)time_start_of_today();
        goalReached = false;
    }

    // Check the metric has data available for today
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
    if (mask & HealthServiceAccessibilityMaskAvailable)
    {
        int current = (int)health_service_sum_today(metric);
        if (current >= goal)
        {
            if (goalReached == false)
            {
                goalReached = true;
                vibes_enqueue_custom_pattern(pat);
            }
            snprintf(step_text, sizeof(step_text), "%i/%i Reached!", current, goal);
        }
        else
        {
            goalReached = false;
            snprintf(step_text, sizeof(step_text), "%i/%i", current, goal);
        }

        if (showClock)
        {
            text_layer_set_text(step_layer, step_text);
        }
        else
        {
            static char time_text[24];
            static char date_text[24];

            snprintf(time_text, sizeof(time_text), "%i", current);
            snprintf(date_text, sizeof(date_text), "%i", goal);

            text_layer_set_text(time_layer, time_text);
            text_layer_set_text(date_layer, date_text);
            text_layer_set_text(step_layer, "");
        }
    }
}

static void health_handler(HealthEventType event, void *context)
{
    switch (event)
    {
    case HealthEventMovementUpdate:
        updateStepCounter();
        break;
    default:
        break;
    }
}

static void init_clock(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    time_layer = text_layer_create(GRect(0, (bounds.size.h / 2) - 30, bounds.size.w, bounds.size.h));
    text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
    text_layer_set_text_color(time_layer, GColorWhite);
    text_layer_set_background_color(time_layer, GColorClear);
    text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS));

    date_layer = text_layer_create(GRect(0, (bounds.size.h / 2), bounds.size.w, bounds.size.h));
    text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
    text_layer_set_text_color(date_layer, GColorWhite);
    text_layer_set_background_color(date_layer, GColorClear);
    text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));

    charge_layer = text_layer_create(GRect(0, (bounds.size.h - 17), bounds.size.w - 2, bounds.size.h));
    text_layer_set_text_alignment(charge_layer, GTextAlignmentRight);
    text_layer_set_text_color(charge_layer, GColorWhite);
    text_layer_set_background_color(charge_layer, GColorClear);
    text_layer_set_font(charge_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));

    step_layer = text_layer_create(GRect(0, (bounds.size.h - 17), bounds.size.w - 2, bounds.size.h));
    text_layer_set_text_alignment(step_layer, GTextAlignmentLeft);
    text_layer_set_text_color(step_layer, GColorWhite);
    text_layer_set_background_color(step_layer, GColorClear);
    text_layer_set_font(step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));

    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    handle_second_tick(current_time, SECOND_UNIT);
    tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

    handle_battery_state(battery_state_service_peek());
    battery_state_service_subscribe(&handle_battery_state);

#if defined(PBL_HEALTH)
    // Attempt to subscribe
    if (!health_service_events_subscribe(health_handler, NULL))
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    }
    else
    {
        layer_add_child(window_get_root_layer(window), text_layer_get_layer(step_layer));
        updateStepCounter();
    }
#else
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
#endif

    layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(charge_layer));
}

static void window_load(Window *window)
{
    init_clock(window);
}

static void window_unload(Window *window)
{
    text_layer_destroy(time_layer);
    text_layer_destroy(date_layer);
    text_layer_destroy(charge_layer);
    text_layer_destroy(step_layer);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context)
{
    Tuple *config_goal = dict_find(iter, 0);
    Tuple *config_show_clock = dict_find(iter, 1);

    if (config_goal)
    {
        goal = atoi(config_goal->value->cstring);
        if (!goal)
        {
            goal = 10000; // failed, revert to default value
        }
    }

    if (config_show_clock)
    {
        showClock = config_show_clock->value->int32 == 1;
    }

    updateStepCounter();
}

static void init(void)
{
    today = (int)time_start_of_today();
    app_message_register_inbox_received(inbox_received_handler);

    window = window_create();
    window_set_window_handlers(
        window,
        (WindowHandlers){
            .load = window_load,
            .unload = window_unload,
        });

    app_message_open(128, 128);

    window_stack_push(window, false);
    window_set_background_color(window, GColorBlack);
}

static void deinit(void)
{
    window_destroy(window);
}

int main(void)
{
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

    app_event_loop();
    deinit();
}
