#include <pebble.h>

#define RADIUS_MAX 80

#define RADIUS_MIN 10

#define BORDER_OFFSET 10

static Window *s_window;
static Layer *s_layer;

static uint8_t s_hour;
static uint8_t s_minute;
static uint8_t s_second;

GPoint points[8*12];
int points_len;

/**
   Pass in the current x,y coordinates for the circle, the current
   radius, the change in radius, and the quadrant you are drawing an
   arc in (1, 2, 3, or 4).
   Returns a new center x,y coordinate and a new radius.
 */
static void create_spiral_coords(int cur_center_x,
                                 int cur_center_y,
                                 int radius,
                                 int radius_delta,
                                 int quadrant,
                                 int* new_center_x,
                                 int* new_center_y,
                                 int* new_radius)
{
  (*new_radius) = radius - radius_delta;
  switch (quadrant) {
  case 0:
    (*new_center_x) = cur_center_x;
    (*new_center_y) = cur_center_y - radius + *new_radius;
    break;
  case 1:
    (*new_center_x) = cur_center_x + radius - *new_radius;
    (*new_center_y) = cur_center_y;
    break;
  case 2:
    (*new_center_x) = cur_center_x;
    (*new_center_y) = cur_center_y + radius - (*new_radius);
    break;
  default:
    (*new_center_x) = cur_center_x - radius + (*new_radius);
    (*new_center_y) = cur_center_y;
  }
}

static inline GRect rect_from_center_and_radius(GPoint center, int radius){
  return GRect(center.x - radius, center.y - radius,
               radius * 2, radius * 2);
}

/**
   MAX_RADIUS is the maximum radius for the watch
   RADIUS_DELTA is the amount the radius decreases between each
   quadrant
   new_center_x/y is the new center x/y for the spiral
   new_radius is the new radius to use for drawing
   new_angle is the angle to start drawing the arc from
 */
static void
get_spiral_center_radius_start_angle_for_cur_time(int MAX_RADIUS,
                                                  int RADIUS_DELTA,
                                                  int* new_center_x,
                                                  int* new_center_y,
                                                  int* new_radius,
                                                  int* new_angle)
{
  int cur_hour = s_hour % 12;
  int cur_center_x = 90;
  int cur_center_y = 90;
  int cur_radius = MAX_RADIUS;
  int hour_quadrant = s_hour / 15;
  for (int i = 0; i < cur_hour; i++) {
    for (int j = 0; j < 4; j++) {
      if (i == 0 && j == 0) {
        // ignore the first arc
        continue;
      }
      create_spiral_coords(cur_center_x,
                           cur_center_y,
                           cur_radius,
                           RADIUS_DELTA,
                           j,
                           &cur_center_x,
                           &cur_center_y,
                           &cur_radius);
    }
  }
  for (int i = 0; i < hour_quadrant; i++) {
    if (cur_hour == 0 && i == 0) {
      // ignore first arc
      continue;
    }
    create_spiral_coords(cur_center_x,
                         cur_center_y,
                         cur_radius,
                         RADIUS_DELTA,
                         i,
                         &cur_center_x,
                         &cur_center_y,
                         &cur_radius);
  }

  (*new_center_x) = cur_center_x;
  (*new_center_y) = cur_center_y;
  (*new_radius) = cur_radius;
  (*new_angle) = cur_hour * 360 / 12;
}
                                                              

// Update the watchface display
static void update_display(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "update_display");

  int minute_hand_deg = s_minute * 360 / 60;
  int hour_hand_deg = s_hour * 360 / 60;

  GRect bounds = layer_get_bounds(layer);
  // GRect rect = rect_from_center_and_radius(GPoint(90, 90), RADIUS_MAX);
  // graphics_fill_radial(ctx, rect, GOvalScaleModeFitCircle,
  //                      1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(90));
  int curr_center_x = 90,
      curr_center_y = 90,
      radius = RADIUS_MAX,
      new_center_x, new_center_y, new_radius;

  GRect rect = rect_from_center_and_radius(
    GPoint(curr_center_x, curr_center_y), radius);
  graphics_fill_radial(ctx, rect, GOvalScaleModeFitCircle,
                       1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(90));
  for (int i = 1; i < 4*6; i++){
    create_spiral_coords(curr_center_x, curr_center_y, radius,
                         2, i%4, &new_center_x, &new_center_y,
                         &new_radius);
    APP_LOG(APP_LOG_LEVEL_INFO, "new_center_x: %d, new_center_y: %d, new_radius: %d", new_center_x, new_center_y, new_radius);

    rect = rect_from_center_and_radius(
        GPoint(new_center_x, new_center_y), new_radius);
    graphics_fill_radial(ctx, rect, GOvalScaleModeFitCircle,
                       1, DEG_TO_TRIGANGLE((i % 4)* 90),
                       DEG_TO_TRIGANGLE((i % 4) * 90 + 90));
    curr_center_x = new_center_x;
    curr_center_y = new_center_y;
    radius = new_radius;
  }

  curr_center_x = 90;
  curr_center_y = 90;
  radius = RADIUS_MAX - 4;

  graphics_context_set_fill_color(ctx, GColorRed);
  rect = rect_from_center_and_radius(
    GPoint(curr_center_x, curr_center_y), radius);
  graphics_fill_radial(ctx, rect, GOvalScaleModeFitCircle,
                       1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(90));

  for (int i = 1; i < 4*6; i++){
    create_spiral_coords(curr_center_x, curr_center_y, radius,
                         2, i%4, &new_center_x, &new_center_y,
                         &new_radius);
    APP_LOG(APP_LOG_LEVEL_INFO, "new_center_x: %d, new_center_y: %d, new_radius: %d", new_center_x, new_center_y, new_radius);

    rect = rect_from_center_and_radius(
        GPoint(new_center_x, new_center_y), new_radius);
    graphics_fill_radial(ctx, rect, GOvalScaleModeFitCircle,
                       1, DEG_TO_TRIGANGLE((i % 4)* 90),
                       DEG_TO_TRIGANGLE((i % 4) * 90 + 90));
    curr_center_x = new_center_x;
    curr_center_y = new_center_y;
    radius = new_radius;
  }

  // graphics_context_set_antialiased(ctx, true);

  GPoint minute_hand_point = gpoint_from_polar(GRect(10, 10, 160, 160),
                                               GOvalScaleModeFitCircle,
                                               minute_hand_deg);
  APP_LOG(APP_LOG_LEVEL_INFO, "minute_hand_point: x: %d, y: %d", minute_hand_point.x, minute_hand_point.y);
  graphics_context_set_fill_color(ctx, GColorYellow);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_pixel(ctx, minute_hand_point);




}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_layer = layer_create(layer_get_bounds(window_get_root_layer(s_window)));
  layer_add_child(window_get_root_layer(s_window), s_layer);
  layer_set_update_proc(s_layer, update_display);
  layer_mark_dirty(s_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_layer);
}

static void update_time(struct tm *tick_time) {
  s_hour = tick_time->tm_hour;
  s_minute = tick_time->tm_min;
  s_second = tick_time->tm_sec;
  layer_mark_dirty(s_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_INFO, "tick!");
  update_time(tick_time);
}

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);

  // Start the timer
  time_t start = time(NULL);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  update_time(localtime(&start));

}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}
