/*
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 * Author: Rob Clark <rob@ti.com>
 *         Andy Gross <andy.gross@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef OMAP_DMM_TILER_H
#define OMAP_DMM_TILER_H

#include <plat/cpu.h>
#include "omap_drv.h"
#include "tcm.h"

enum tiler_fmt {
	TILFMT_8BIT = 0,
	TILFMT_16BIT,
	TILFMT_32BIT,
	TILFMT_PAGE,
	TILFMT_NFORMATS
};

struct pat_area {
	u32 x0:8;
	u32 y0:8;
	u32 x1:8;
	u32 y1:8;
};

struct tiler_block {
	struct list_head alloc_node;	/* node for global block list */
	struct tcm_area area;		/* area */
	enum tiler_fmt fmt;		/* format */
	uint32_t width;
	uint32_t height;
	uint32_t stride;		/* 2D: length of one line in pages
					   1D: length of buffer rounded to
						PAGE_SIZE */
};

/* bits representing the same slot in DMM-TILER hw-block */
#define SLOT_WIDTH_BITS         6
#define SLOT_HEIGHT_BITS        6

/* bits reserved to describe coordinates in DMM-TILER hw-block */
#define CONT_WIDTH_BITS         14
#define CONT_HEIGHT_BITS        13

/* calculated constants */
#define TILER_PAGE              (1 << (SLOT_WIDTH_BITS + SLOT_HEIGHT_BITS))
#define TILER_WIDTH             (1 << (CONT_WIDTH_BITS - SLOT_WIDTH_BITS))
#define TILER_HEIGHT            (1 << (CONT_HEIGHT_BITS - SLOT_HEIGHT_BITS))

/* tiler space addressing bitfields */
#define MASK_XY_FLIP		(1 << 31)
#define MASK_Y_INVERT		(1 << 30)
#define MASK_X_INVERT		(1 << 29)
#define SHIFT_ACC_MODE		27
#define MASK_ACC_MODE		3

#define VIEW_SIZE		(1u << (CONT_WIDTH_BITS + CONT_HEIGHT_BITS))
#define VIEW_MASK		(VIEW_SIZE - 1u)

#define MASK_VIEW		(MASK_X_INVERT | MASK_Y_INVERT | MASK_XY_FLIP)

#define TILER_FMT(x)	((enum tiler_fmt) \
		((x >> SHIFT_ACC_MODE) & MASK_ACC_MODE))

#define MASK(bits) ((1 << (bits)) - 1)

#define TILVIEW_8BIT    0x60000000u
#define TILVIEW_16BIT   (TILVIEW_8BIT  + VIEW_SIZE)
#define TILVIEW_32BIT   (TILVIEW_16BIT + VIEW_SIZE)
#define TILVIEW_PAGE    (TILVIEW_32BIT + VIEW_SIZE)
#define TILVIEW_END     (TILVIEW_PAGE  + VIEW_SIZE)

/* create tsptr by adding view orientation and access mode */
#define TIL_ADDR(x, orient, a)\
	((u32) (x) | (orient) | ((a) << SHIFT_ACC_MODE))

#ifdef CONFIG_DEBUG_FS
int tiler_map_show(struct seq_file *s, void *arg);
#endif

/* pin/unpin */
int tiler_pin(struct tiler_block *block, struct page **pages,
		uint32_t npages, uint32_t roll, bool wait);
int tiler_pin_phys(struct tiler_block *block, u32 *phys_addrs, u32 num_pages);
int tiler_unpin(struct tiler_block *block);

/* reserve/release */
struct tiler_block *tiler_reserve_2d(enum tiler_fmt fmt, uint16_t w, uint16_t h,
				uint16_t align);
struct tiler_block *tiler_reserve_1d(size_t size);
int tiler_release(struct tiler_block *block);

/* utilities */
dma_addr_t tiler_ssptr(struct tiler_block *block);
uint32_t tiler_stride(dma_addr_t tsptr);
size_t tiler_size(enum tiler_fmt fmt, uint16_t w, uint16_t h);
size_t tiler_vsize(enum tiler_fmt fmt, uint16_t w, uint16_t h);
void tiler_align(enum tiler_fmt fmt, uint16_t *w, uint16_t *h);
bool dmm_is_initialized(void);


/* rotation */
/* tiler (image/video frame) view */
struct tiler_view_t {
	uint32_t tsptr;		/* tiler space addr */
	uint32_t width;		/* width */
	uint32_t height;	/* height */
	uint32_t bpp;		/* bytes per pixel */
	int h_inc;		/* horizontal increment */
	int v_inc;		/* vertical increment */
};

bool is_tiler_addr(uint32_t phys);
int tiler_get_fmt(uint32_t phys, enum tiler_fmt *fmt);
void tilview_create(struct tiler_view_t *view, u32 phys, u32 width, u32 height);
void tilview_get(struct tiler_view_t *view, struct tiler_block *blk);
int tilview_crop(struct tiler_view_t *view, u32 left, u32 top, u32 width,
		u32 height);
int tilview_rotate(struct tiler_view_t *view, int rotation);
int tilview_flip(struct tiler_view_t *view, bool flip_x, bool flip_y);

extern struct platform_driver omap_dmm_driver;

/* GEM bo flags -> tiler fmt */
static inline enum tiler_fmt gem2fmt(uint32_t flags)
{
	switch (flags & OMAP_BO_TILED) {
	case OMAP_BO_TILED_8:
		return TILFMT_8BIT;
	case OMAP_BO_TILED_16:
		return TILFMT_16BIT;
	case OMAP_BO_TILED_32:
		return TILFMT_32BIT;
	default:
		return TILFMT_PAGE;
	}
}

static inline bool validfmt(enum tiler_fmt fmt)
{
	switch (fmt) {
	case TILFMT_8BIT:
	case TILFMT_16BIT:
	case TILFMT_32BIT:
	case TILFMT_PAGE:
		return true;
	default:
		return false;
	}
}

static inline int dmm_is_available(void)
{
	return cpu_is_omap44xx() || cpu_is_omap54xx();
}

#endif
