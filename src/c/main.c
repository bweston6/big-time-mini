#include <pebble.h>

static Window *s_main_window;
static Layer *s_canvas_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GBitmap *s_hour_hand_bitmap;
static GBitmap *s_minute_hand_bitmap;
static GPoint s_hour_hand_ic;
static GPoint s_minute_hand_ic;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);

    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    int32_t minute_angle = TRIG_MAX_ANGLE * tick_time->tm_min / 60;
    int32_t hour_angle = TRIG_MAX_ANGLE * (tick_time->tm_hour % 12) / 12 + minute_angle / 12;

    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    graphics_draw_rotated_bitmap(ctx, s_hour_hand_bitmap, s_hour_hand_ic, hour_angle, center);
    graphics_draw_rotated_bitmap(ctx, s_minute_hand_bitmap, s_minute_hand_ic, minute_angle, center);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    layer_mark_dirty(s_canvas_layer);
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    GPoint center = grect_center_point(&bounds);

    // bitmaps
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DIAL);
    s_hour_hand_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOUR_HAND);
    s_hour_hand_ic = GPoint(14, 59);
    s_minute_hand_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MINUTE_HAND);
    s_minute_hand_ic = GPoint(13, 79);

    // layers
    s_background_layer = bitmap_layer_create(bounds);
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    layer_add_child(window_layer, s_canvas_layer);

}

static void main_window_unload(Window *window) {
    layer_destroy(s_canvas_layer);
    bitmap_layer_destroy(s_background_layer);
    gbitmap_destroy(s_background_bitmap);
    gbitmap_destroy(s_hour_hand_bitmap);
    gbitmap_destroy(s_minute_hand_bitmap);
}

static void init() {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload
            });

    window_stack_push(s_main_window, true);

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
    tick_timer_service_unsubscribe();
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
