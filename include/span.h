#ifndef __SPAN_H__
#define __SPAN_H__

#include <stdbool.h>
#include <stdint.h>

#define SPAN_COLOR_RGBA(r, g, b, a) ((uint16_t)(a << 12) | (uint16_t)(r << 8) | ((uint16_t)(g) << 4) | ((uint16_t)(b) << 0))
#define SPAN_COLOR(r, g, b)         SPAN_COLOR_RGBA(r, g, b, 0x0F)
#define SPAN_COLOR_HIGH(r, g, b)    SPAN_COLOR((uint16_t)(r + 1) >> 1, (uint16_t)(g + 1) >> 1, (uint16_t)(b + 1) >> 1)
#define SPAN_COLOR_TRUE(r, g, b)    SPAN_COLOR((uint16_t)(r + 8) >> 4, (uint16_t)(g + 8) >> 4, (uint16_t)(b + 8) >> 4)

#define SPAN_LERP_N 1024

typedef struct span_pipe_t span_pipe_t;

typedef struct span_icon_t span_icon_t;
typedef struct span_font_t span_font_t;

typedef struct span_unit_t span_unit_t;
typedef struct span_object_t span_object_t;
typedef struct span_window_t span_window_t;

struct span_pipe_t {
	void (*span)(int n, uint16_t color);
	void (*copy)(int n, const uint16_t *array);
	void (*mask)(int n, const uint16_t *array, uint16_t color_0, uint16_t color_1);
};

struct span_icon_t {
	enum {
		SPAN_ICON_COPY,
		SPAN_ICON_MASK,
	} type;
	
	const uint16_t *array;
	int w, h;
};

struct span_font_t {
	const uint16_t *array;
	int w, h;
	
	int start, n;
};

struct span_unit_t {
	enum {
		SPAN_UNIT_NONE,
		SPAN_UNIT_TEXT,
		SPAN_UNIT_ICON,
	} type;
	
	uint16_t color_0, color_1;
	int w;
	
	enum {
		SPAN_ALIGN_LEFT   = 0,
		SPAN_ALIGN_CENTER = 1,
		SPAN_ALIGN_RIGHT  = 2,
	} align;
	
	union {
		struct {
			char *text;
			const span_font_t *font;
		};
		
		struct {
			const span_icon_t *icon;
		};
	};
};

struct span_object_t {
	span_unit_t *units;
	int n;
	
	int h;
	
	bool is_hoverable;
	
	void (*on_enter)(const span_window_t *window, int i);
	void (*on_leave)(const span_window_t *window, int i);
	void (*on_press)(const span_window_t *window, int i);
};

struct span_window_t {
	span_object_t *items;
	int n;
	
	uint16_t color;
	int w;
	
	int line, i;
};

void span_init(span_pipe_t pipe);

int  span_length(const char *text);
bool span_equals(const char *text_0, const char *text_1);

// === RENDERING ===

int span_size_unit(const span_unit_t *unit);

void span_draw_unit_text(const span_unit_t *unit, int line);
void span_draw_unit_icon(const span_unit_t *unit, int line);

void span_draw_unit(const span_unit_t *unit, int h, int line);
void span_draw_object(const span_object_t *object, int line);
void span_draw_window(const span_window_t *window, int line);

int span_offset(const span_window_t *window, int n);

// === ANIMATION ===

uint32_t span_linear(uint32_t u);
uint32_t span_poly_3(uint32_t u);
uint32_t span_bounce(uint32_t u);

void span_update(uint32_t time);
void span_lerp(int *value, int begin_value, int end_value, uint32_t time, uint32_t (*lerp)(uint32_t u));

// === INTERACTS ===

void span_goto(span_window_t *window, int i);

void span_last(span_window_t *window);
void span_next(span_window_t *window);

void span_press(const span_window_t *window);

#endif
