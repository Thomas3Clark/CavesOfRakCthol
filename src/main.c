#include <pebble.h>
#include <stdlib.h>

#define LEVEL_NUM_PKEY			0
#define STAT_ATK_PKEY			1
#define STAT_DEF_PKEY			2
#define STAT_SPD_PKEY			3
#define STAT_MAG_PKEY			4
#define STAT_MDF_PKEY			5
#define UPGRADE_POINTS_PKEY		6
#define FLOOR_NUM_PKEY			7
#define HP_PKEY					8
#define EXP_PKEY				9

#define LEVEL_NUM				1
#define STAT_ATK				1
#define STAT_DEF				1
#define STAT_SPD				1
#define STAT_MAG				1
#define STAT_MDF				1
#define UPGRADE_POINTS			1
#define FLOOR_NUM				1
#define HP						20
#define EXP						0

static Window *s_window;

static GBitmap *s_go_forward, *s_go_left, *s_go_right, *s_go_up;
static int s_level, s_atk, s_def, s_spd, s_mag, s_mdf, s_upgrade_points, s_floor, s_hp, s_exp;

static BitmapLayer *s_floor_layer;
static TextLayer *s_hp_layer, *s_status_layer, *s_exp_layer;

static bool autoPlay = false;

// UGH MY GOD
static void newEvent();

static void down_button(ClickRecognizerRef recognizer, void *context) {
	autoPlay = !autoPlay;
}

static void select_button(ClickRecognizerRef recognizer, void *context) {
	newEvent();
}

static void click_prov(void *ctx) {
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 500, down_button);
	window_single_repeating_click_subscribe(BUTTON_ID_SELECT, 10, select_button);
}

static void window_load() {
	Layer *window_layer = window_get_root_layer(s_window);
	
	layer_add_child(window_layer, (Layer *)s_floor_layer);
	layer_add_child(window_layer, (Layer *)s_hp_layer);
	layer_add_child(window_layer, (Layer *)s_status_layer);
	layer_add_child(window_layer, (Layer *)s_exp_layer);
	
	window_set_click_config_provider(s_window, click_prov);
}

static void window_unload() {
	
}

static char statbuf[32];
static char hpbuf[16];
static char expbuf[8];

static void update_exp_text() {
	int exp_1000 = s_exp / 1000;
	int exp_100 = (s_exp % 1000) / 100;
	int exp_10 = (s_exp % 100) / 10;
	int exp_1 = (s_exp % 10);
	expbuf[0] = 48 + exp_1000;
	expbuf[1] = 48 + exp_100;
	expbuf[2] = 48 + exp_10;
	expbuf[3] = 48 + exp_1;
	text_layer_set_text(s_exp_layer, expbuf);
}

static void update_hp_text() {
	strcpy(hpbuf, "HP: 0000/0000");
	int hp_1000 = s_hp / 1000;
	int hp_100 = (s_hp % 1000) / 100;
	int hp_10 = (s_hp % 100) / 10;
	int hp_1 = (s_hp % 10);
	hpbuf[4] = 48 + hp_1000;
	hpbuf[5] = 48 + hp_100;
	hpbuf[6] = 48 + hp_10;
	hpbuf[7] = 48 + hp_1;
	int maxhp = 20 + (500 - 20) * (s_level - 1) / 100;
	int maxhp_1000 = maxhp / 1000;
	int maxhp_100 = (maxhp % 1000) / 100;
	int maxhp_10 = (maxhp % 100) / 10;
	int maxhp_1 = (maxhp % 10);
	hpbuf[9] = 48 + maxhp_1000;
	hpbuf[10] = 48 + maxhp_100;
	hpbuf[11] = 48 + maxhp_10;
	hpbuf[12] = 48 + maxhp_1;
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "Coded 0: %d", hpbuf[5]);
//	siprintf(tmpbuf, "HP: %d/%d", s_hp, 20 + (500 - 20) * s_level / 100);
	text_layer_set_text(s_hp_layer, hpbuf);
}

static void update_stat_text(int event) {
	switch(event) {
		case 0:
			strcpy(statbuf, "Attacked");
			break;
		case 1:
			strcpy(statbuf, "Healed");
			break;
		case 2:
			strcpy(statbuf, "New Floor");
			break;
		case 3:
			strcpy(statbuf, "You Died");
			break;
	}
	text_layer_set_text(s_status_layer, statbuf);
}

static void heal() {
	s_hp += 20 + (500 - 20) * (s_level - 1) / 1000;
	if(s_hp > 20 + (500 - 20) * (s_level - 1) / 100) {
		s_hp = 20 + (500 - 20) * (s_level - 1) / 100;
	}
	update_hp_text();
}

static int rnum;

static void spawnEnemy() {
	rnum = rand() % 6;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Encountered Enemy");
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Exp: %d", s_exp);
	s_exp += 1 + rnum * s_floor;
	int damage = 2 * (1 + rnum * s_floor);
	s_hp -= damage;
	if(s_exp > 20 + (500 - 20) * (s_level - 1) / 100) {
		s_level++;
		s_exp = (20 + (500 - 20) * (s_level - 1) / 100) - s_exp;
		s_upgrade_points += s_level;
		s_hp = 20 + (500 - 20) * (s_level - 1) / 100;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Level Up! %d", s_level);
	}
	update_exp_text();
	update_stat_text(0);
	if(s_hp <= 0) {
		s_atk				= STAT_ATK;
		s_def				= STAT_DEF;
		s_spd				= STAT_SPD;
		s_mag				= STAT_MAG;
		s_mdf				= STAT_MDF;
		s_level				= LEVEL_NUM;
		s_floor				= FLOOR_NUM;
		s_upgrade_points	= UPGRADE_POINTS;
		s_hp				= HP;
		s_exp				= EXP;
		update_stat_text(3);
	}
	update_hp_text();
}

static void newEvent() {
	rnum = rand() % 4;
	switch(rnum) {
		case 0:
			bitmap_layer_set_bitmap(s_floor_layer, s_go_forward);
			break;
		case 1:
			bitmap_layer_set_bitmap(s_floor_layer, s_go_left);
			break;
		case 2:
			bitmap_layer_set_bitmap(s_floor_layer, s_go_right);
			break;
		case 3:
			if(s_level > s_floor * 3) {
				bitmap_layer_set_bitmap(s_floor_layer, s_go_up);
			}
			break;
	}
	if(rnum % 2 == 0) {
		spawnEnemy();
	} else {
		heal();
		if(rnum == 3) {
			update_stat_text(s_level > s_floor * 3 ? 2 : 1);
		} else {
			update_stat_text(1);
		}
	}
}

static GFont s_pixel_operator;

static void every_second(struct tm* tick_time, TimeUnits units_changed) {
	if(tick_time->tm_sec % 2 == 0 && autoPlay) {
		newEvent();
	}
}

static void init() {
	s_go_forward	= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GO_FORWARD);
	s_go_left		= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GO_LEFT);
	s_go_right		= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GO_RIGHT);
	s_go_up			= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GO_UP);
	
	s_atk				= persist_exists(STAT_ATK_PKEY)			? persist_read_int(STAT_ATK_PKEY)			: STAT_ATK;
	s_def				= persist_exists(STAT_DEF_PKEY)			? persist_read_int(STAT_DEF_PKEY)			: STAT_DEF;
	s_spd				= persist_exists(STAT_SPD_PKEY)			? persist_read_int(STAT_SPD_PKEY)			: STAT_SPD;
	s_mag				= persist_exists(STAT_MAG_PKEY)			? persist_read_int(STAT_MAG_PKEY)			: STAT_MAG;
	s_mdf				= persist_exists(STAT_MDF_PKEY)			? persist_read_int(STAT_MDF_PKEY)			: STAT_MDF;
	s_level				= persist_exists(LEVEL_NUM_PKEY)		? persist_read_int(LEVEL_NUM_PKEY)			: LEVEL_NUM;
	s_floor				= persist_exists(FLOOR_NUM_PKEY)		? persist_read_int(FLOOR_NUM_PKEY)			: FLOOR_NUM;
	s_upgrade_points	= persist_exists(UPGRADE_POINTS_PKEY)	? persist_read_int(UPGRADE_POINTS_PKEY)		: UPGRADE_POINTS;
	s_hp				= persist_exists(HP_PKEY)				? persist_read_int(HP_PKEY)					: HP;
	s_exp				= persist_exists(EXP_PKEY)				? persist_read_int(EXP_PKEY)				: EXP;
	
	// DEBUG
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "%d", s_level);
	
	s_floor_layer = bitmap_layer_create(GRect(0, 12, 144, 144));
	bitmap_layer_set_bitmap(s_floor_layer, s_go_forward);
	
	s_pixel_operator = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PIXELOPERATOR_16));
	
	s_hp_layer = text_layer_create(GRect(0, 144, 144, 24));
	text_layer_set_font(s_hp_layer, s_pixel_operator);
	text_layer_set_text_color(s_hp_layer, GColorWhite);
	text_layer_set_background_color(s_hp_layer, GColorBlack);
	update_hp_text();
	
	s_status_layer = text_layer_create(GRect(0, 0, 144, 24));
	text_layer_set_font(s_status_layer, s_pixel_operator);
	text_layer_set_text_color(s_status_layer, GColorWhite);
	text_layer_set_background_color(s_status_layer, GColorBlack);
	
	s_exp_layer = text_layer_create(GRect(72, 0, 72, 24));
	text_layer_set_font(s_exp_layer, s_pixel_operator);
	text_layer_set_text_color(s_exp_layer, GColorWhite);
	text_layer_set_background_color(s_exp_layer, GColorClear);
	text_layer_set_text_alignment(s_exp_layer, GTextAlignmentRight);
	update_exp_text();
	
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(s_window, true);
	
	// DEBUG
	// s_level++;
	newEvent();
	tick_timer_service_subscribe(SECOND_UNIT, every_second);
}

static void deinit() {
	persist_write_int(STAT_ATK_PKEY,		s_atk);
	persist_write_int(STAT_DEF_PKEY,		s_def);
	persist_write_int(STAT_SPD_PKEY,		s_spd);
	persist_write_int(STAT_MAG_PKEY,		s_mag);
	persist_write_int(STAT_MDF_PKEY,		s_mdf);
	persist_write_int(LEVEL_NUM_PKEY,		s_level);
	persist_write_int(FLOOR_NUM_PKEY,		s_floor);
	persist_write_int(UPGRADE_POINTS_PKEY,	s_upgrade_points);
	persist_write_int(HP_PKEY,				s_hp);
	persist_write_int(EXP_PKEY, 			s_exp);
	
	gbitmap_destroy(s_go_forward);
	gbitmap_destroy(s_go_left);
	gbitmap_destroy(s_go_right);
	gbitmap_destroy(s_go_up);
	
	bitmap_layer_destroy(s_floor_layer);
	text_layer_destroy(s_hp_layer);
	
	window_destroy(s_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}