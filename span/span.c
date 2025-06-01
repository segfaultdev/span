#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <span.h>

static struct {
	// === RENDERING ===
	
	span_pipe_t pipe;
	
	// === ANIMATION ===
	
	uint32_t (*lerp)(uint32_t u);
	
	int *value;
	uint32_t time;
	
	int begin_value, end_value;
	uint32_t begin_time, end_time;
} span;

void span_init(span_pipe_t pipe) {
	// === RENDERING ===
	
	span.pipe = pipe;
	
	// === ANIMATION ===
	
	span.value = NULL;
}

int span_length(const char *text) {
	int n = 0;
	
	while (text[n] != '\x00') {
		n++;
	}
	
	return n;
}

bool span_equals(const char *text_0, const char *text_1) {
	int n = 0;
	
	while (true) {
		if (text_0[n] != text_1[n]) {
			return false;
		}
		
		if (text_0[n] == '\x00') {
			break;
		}
		
		n++;
	}
	
	return true;
}

int span_size_unit(const span_unit_t *unit) {
	if (unit->type == SPAN_UNIT_NONE) {
		return 0;
	}
	
	if (unit->type == SPAN_UNIT_TEXT) {
		return unit->font->h;
	}
	
	if (unit->type == SPAN_UNIT_ICON) {
		return unit->icon->h;
	}
	
	return 0;
}

void span_draw_unit_text(const span_unit_t *unit, int line) {
	if (unit->text == NULL) {
		span.pipe.span(unit->w, unit->color_0);
		return;
	}
	
	const span_font_t *font = unit->font;
	
	const int s = font->w * span_length(unit->text);
	const int l = (unit->align * (unit->w - s)) / 2;
	
	span.pipe.span(l, unit->color_0);
	
	for (int i = 0; unit->text[i] != '\x00'; i++) {
		const int c = (unsigned char)(unit->text[i]) - font->start;
		const int j = (line + c * font->h) * ((font->w + 15) / 16);
		
		span.pipe.mask(font->w, font->array + j, unit->color_0, unit->color_1);
	}
	
	span.pipe.span(unit->w - (l + s), unit->color_0);
}

void span_draw_unit_icon(const span_unit_t *unit, int line) {
	const span_icon_t *icon = unit->icon;
	
	const int s = icon->w;
	const int l = (unit->align * (unit->w - s)) / 2;
	
	span.pipe.span(l, unit->color_0);
	
	if (icon->type == SPAN_ICON_COPY) {
		const int j = line * icon->w;
		span.pipe.copy(icon->w, icon->array + j);
	}
	
	if (icon->type == SPAN_ICON_MASK) {
		const int j = line * ((icon->w + 15) / 16);
		span.pipe.mask(icon->w, icon->array + j, unit->color_0, unit->color_1);
	}
	
	span.pipe.span(unit->w - (l + s), unit->color_0);
}

void span_draw_unit(const span_unit_t *unit, int h, int line) {
	const int s = span_size_unit(unit);
	const int t = (h - s) / 2;
	
	if (line >= t && line < t + s) {
		if (unit->type == SPAN_UNIT_TEXT) {
			span_draw_unit_text(unit, line - t);
		}

		if (unit->type == SPAN_UNIT_ICON) {
			span_draw_unit_icon(unit, line - t);
		}
	} else {
		span.pipe.span(unit->w, unit->color_0);
	}
}

void span_draw_object(const span_object_t *object, int line) {
	for (int i = 0; i < object->n; i++) {
		const span_unit_t *unit = object->units + i;
		span_draw_unit(unit, object->h, line);
	}
}

void span_draw_window(const span_window_t *window, int line) {
	int j = 0;
	
	for (int i = 0; i < window->n; i++) {
		const span_object_t *object = window->items + i;
		const int scroll_line = (object->is_fixed ? 0 : window->line);
		
		if (line + scroll_line - j >= 0 && line + scroll_line - j < object->h) {
			span_draw_object(object, line + scroll_line - j);
			return;
		}
		
		j += object->h;
	}
	
	span.pipe.span(window->w, window->color);
}

int span_offset(const span_window_t *window, int n) {
	int j = 0;
	
	for (int i = 0; i < n; i++) {
		const span_object_t *object = window->items + i;
		j += object->h;
	}
	
	return j;
}

int span_midway(const span_window_t *window, int n) {
	const span_object_t *object = window->items + n;
	return span_offset(window, n) + (object->h / 2);
}

uint32_t span_linear(uint32_t u) {
	return u;
}

uint32_t span_poly_3(uint32_t u) {
	const uint32_t v = (SPAN_LERP_N - u) * (SPAN_LERP_N - u) * (SPAN_LERP_N - u);
	return SPAN_LERP_N - (v / (SPAN_LERP_N * SPAN_LERP_N));
}

uint32_t span_bounce(uint32_t u) {
	if (u < SPAN_LERP_N / 2) {
		u += SPAN_LERP_N / 2;
		return SPAN_LERP_N - ((4 * u) - (4 * u * u) / SPAN_LERP_N) / 1;
	}
	
	u -= SPAN_LERP_N / 2;
	
	if (u < SPAN_LERP_N / 4) {
		u *= 4;
		return SPAN_LERP_N - ((4 * u) - (4 * u * u) / SPAN_LERP_N) / 4;
	}
	
	u -= SPAN_LERP_N / 4;
	
	if (u < SPAN_LERP_N / 8) {
		u *= 8;
		return SPAN_LERP_N - ((4 * u) - (4 * u * u) / SPAN_LERP_N) / 16;
	}
	
	u -= SPAN_LERP_N / 8;
	u *= 8;
	
	return SPAN_LERP_N - ((4 * u) - (4 * u * u) / SPAN_LERP_N) / 64;
}

void span_update(uint32_t time) {
	span.time = time;
	
	if (span.value != NULL) {
		if (span.time >= span.end_time) {
			span.value[0] = span.end_value;
			span.value = NULL;
		} else {
			const uint32_t u = ((span.time - span.begin_time) * SPAN_LERP_N) / (span.end_time - span.begin_time);
			const uint32_t v = span.lerp(u);
			
			span.value[0] = span.begin_value + ((span.end_value - span.begin_value) * (int)(v)) / SPAN_LERP_N;
		}
	}
}

void span_lerp(int *value, int begin_value, int end_value, uint32_t time, uint32_t (*lerp)(uint32_t u)) {
	span.lerp = lerp;
	span.value = value;
	
	span.begin_value = begin_value;
	span.end_value = end_value;
	
	span.begin_time = span.time;
	span.end_time = span.time + time;
}

void span_goto(span_window_t *window, int i) {
	span_object_t *old_object = window->items + window->i;
	span_object_t *new_object = window->items + i;
	
	if (!new_object->is_hoverable) {
		return;
	}
	
	if (old_object->on_leave != NULL) {
		old_object->on_leave(window, window->i);
	}
	
	if (new_object->on_enter != NULL) {
		new_object->on_enter(window, i);
	}
	
	window->i = i;
}

void span_last(span_window_t *window) {
	for (int i = window->i - 1; i >= 0; i--) {
		const span_object_t *object = window->items + i;
		
		if (object->is_hoverable) {
			span_goto(window, i);
			return;
		}
	}
}

void span_next(span_window_t *window) {
	for (int i = window->i + 1; i < window->n; i++) {
		const span_object_t *object = window->items + i;
		
		if (object->is_hoverable) {
			span_goto(window, i);
			return;
		}
	}
}

void span_press(const span_window_t *window) {
	span_object_t *object = window->items + window->i;
	
	if (object->on_press) {
		object->on_press(window, window->i);
	}
}
