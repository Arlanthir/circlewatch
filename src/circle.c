#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x4E, 0xB1, 0xDB, 0x33, 0xDD, 0x5F, 0x47, 0xB6, 0xA1, 0x78, 0x4D, 0x13, 0x1F, 0xDF, 0x37, 0xD3 }
PBL_APP_INFO(MY_UUID,
             "Circle Watch", "Miguel Branco",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

Layer ring_display_layer;

TextLayer text_time_layer;
TextLayer text_date_layer;

const GPathInfo RING_SEGMENT_PATH_POINTS = {
	3,
	(GPoint []) {
		{0, 0},
		{-8, -75}, // 75 = radius + fudge; 8 = 75*tan(6 degrees); 6 degrees per minute;
		{8,  -75},
	}
};

GPath ring_segment_path;

void ring_display_layer_update_callback(Layer *me, GContext* ctx) {
	PblTm t;
	get_time(&t);
	unsigned int angle = t.tm_min * 6;
	GPoint center = grect_center_point(&me->frame);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, center, 70);
	graphics_context_set_fill_color(ctx, GColorBlack);

	for(; angle < 355; angle += 6) {
		gpath_rotate_to(&ring_segment_path, (TRIG_MAX_ANGLE / 360) * angle);
		gpath_draw_filled(ctx, &ring_segment_path);
	}

	graphics_fill_circle(ctx, center, 60);
}


void update() {

	layer_mark_dirty(&ring_display_layer);
  
	char timeText[] = "00:00:00";
	PblTm currentTime;
	get_time(&currentTime);
	if (clock_is_24h_style()) {
		string_format_time(timeText, sizeof(timeText), "%H:%M:%S", &currentTime);
	} else {
		string_format_time(timeText, sizeof(timeText), "%I:%M:%S", &currentTime);
	}

	static char hours[2] = "00";	// Used by the system later
	if (timeText[0] == '0') {
		hours[0] = timeText[1];
		hours[1] = '\0';
	} else {
		hours[0] = timeText[0];
		hours[1] = timeText[1];
	}
	
	text_layer_set_text(&text_time_layer, hours);

	static char date_text[] = "00 MMM";
	string_format_time(date_text, sizeof(date_text), "%e %b", &currentTime);

	text_layer_set_text(&text_date_layer, date_text);
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
	(void)ctx;
	(void)t;
	update();
}


void handle_init(AppContextRef ctx) {
	(void)ctx;

	window_init(&window, "Circle Watch");
	window_stack_push(&window, true);
	window_set_background_color(&window, GColorBlack);

	resource_init_current_app(&APP_RESOURCES);

	// Init the layer for the ring display
	layer_init(&ring_display_layer, window.layer.frame);
	ring_display_layer.update_proc = &ring_display_layer_update_callback;
	layer_add_child(&window.layer, &ring_display_layer);

	// Init the ring segment path
	gpath_init(&ring_segment_path, &RING_SEGMENT_PATH_POINTS);
	gpath_move_to(&ring_segment_path, grect_center_point(&ring_display_layer.frame));

	text_layer_init(&text_time_layer, window.layer.frame);
	text_layer_set_text_color(&text_time_layer, GColorWhite);
	text_layer_set_background_color(&text_time_layer, GColorClear);

	layer_set_frame(&text_time_layer.layer, GRect(0, 57, 144, 168-57));
	text_layer_set_text_alignment(&text_time_layer, GTextAlignmentCenter);

	text_layer_set_font(&text_time_layer, fonts_get_system_font(FONT_KEY_GOTHAM_42_BOLD));
	layer_add_child(&window.layer, &text_time_layer.layer);

	text_layer_init(&text_date_layer, window.layer.frame);
	text_layer_set_text_color(&text_date_layer, GColorWhite);
	text_layer_set_background_color(&text_date_layer, GColorClear);
    layer_set_frame(&text_date_layer.layer, GRect(0, 105, 144, 168-105));
    text_layer_set_text_alignment(&text_date_layer, GTextAlignmentCenter);
	text_layer_set_font(&text_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	layer_add_child(&window.layer, &text_date_layer.layer);

	update();
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    // Handle time updates
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = MINUTE_UNIT
    }

  };
  app_event_loop(params, &handlers);
}
