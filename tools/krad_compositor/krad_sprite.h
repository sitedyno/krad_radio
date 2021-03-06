

#include "krad_radio.h"

#ifndef KRAD_SPRITE_H
#define KRAD_SPRITE_H

#define KRAD_SPRITE_DEFAULT_TICKRATE 4

typedef struct krad_sprite_St krad_sprite_t;

struct krad_sprite_St {

	int active;

	int x;
	int y;
	int z;	
	
	int tickrate;
	int tick;
	int frames;
	int frame;

	cairo_surface_t *sprite;
	cairo_pattern_t *sprite_pattern;

	int sheet_width;
	int sheet_height;
	int width;
	int height;

	float rotation;
	float opacity;
	float xscale;
	float yscale;
	
	int new_x;
	int new_y;

	float new_rotation;
	float new_opacity;
	float new_xscale;
	float new_yscale;
	
	
};


krad_sprite_t *krad_sprite_create ();
void krad_sprite_destroy (krad_sprite_t *krad_sprite);
krad_sprite_t *krad_sprite_create_from_file (char *filename);
void krad_sprite_reset (krad_sprite_t *krad_sprite);
void krad_sprite_open_file (krad_sprite_t *krad_sprite, char *filename);
void krad_sprite_set_xy (krad_sprite_t *krad_sprite, int x, int y);
void krad_sprite_set_new_xy (krad_sprite_t *krad_sprite, int x, int y);

void krad_sprite_set_scale (krad_sprite_t *krad_sprite, float scale);
void krad_sprite_set_xscale (krad_sprite_t *krad_sprite, float scale);
void krad_sprite_set_yscale (krad_sprite_t *krad_sprite, float scale);
void krad_sprite_set_opacity (krad_sprite_t *krad_sprite, float opacity);
void krad_sprite_set_rotation (krad_sprite_t *krad_sprite, float rotation);

void krad_sprite_set_new_scale (krad_sprite_t *krad_sprite, float scale);
void krad_sprite_set_new_xscale (krad_sprite_t *krad_sprite, float scale);
void krad_sprite_set_new_yscale (krad_sprite_t *krad_sprite, float scale);
void krad_sprite_set_new_opacity (krad_sprite_t *krad_sprite, float opacity);
void krad_sprite_set_new_rotation (krad_sprite_t *krad_sprite, float rotation);

void krad_sprite_set_tickrate (krad_sprite_t *krad_sprite, int tickrate);
void krad_sprite_render (krad_sprite_t *krad_sprite, cairo_t *cr);
void krad_sprite_tick (krad_sprite_t *krad_sprite);
void krad_sprite_render_xy (krad_sprite_t *krad_sprite, cairo_t *cr, int x, int y);


#endif
