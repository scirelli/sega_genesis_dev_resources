/***************************************************************************

    png.h

    PNG file management.

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __PNG_H__
#define __PNG_H__

#include "mamecore.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define PNG_Signature       "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"
#define MNG_Signature       "\x8A\x4D\x4E\x47\x0D\x0A\x1A\x0A"

/* Chunk names */
#define PNG_CN_IHDR			0x49484452L
#define PNG_CN_PLTE			0x504C5445L
#define PNG_CN_IDAT			0x49444154L
#define PNG_CN_IEND			0x49454E44L
#define PNG_CN_gAMA			0x67414D41L
#define PNG_CN_sBIT			0x73424954L
#define PNG_CN_cHRM			0x6348524DL
#define PNG_CN_tRNS			0x74524E53L
#define PNG_CN_bKGD			0x624B4744L
#define PNG_CN_hIST			0x68495354L
#define PNG_CN_tEXt			0x74455874L
#define PNG_CN_zTXt			0x7A545874L
#define PNG_CN_pHYs			0x70485973L
#define PNG_CN_oFFs			0x6F464673L
#define PNG_CN_tIME			0x74494D45L
#define PNG_CN_sCAL			0x7343414CL

/* MNG Chunk names */
#define MNG_CN_MHDR			0x4D484452L
#define MNG_CN_MEND			0x4D454E44L
#define MNG_CN_TERM			0x5445524DL
#define MNG_CN_BACK			0x4241434BL

/* Prediction filters */
#define PNG_PF_None			0
#define PNG_PF_Sub			1
#define PNG_PF_Up			2
#define PNG_PF_Average		3
#define PNG_PF_Paeth		4

/* Error types */
#define PNGERR_NONE				0
#define PNGERR_OUT_OF_MEMORY	1
#define PNGERR_UNKNOWN_FILTER	2
#define PNGERR_FILE_ERROR		3
#define PNGERR_BAD_SIGNATURE	4
#define PNGERR_DECOMPRESS_ERROR	5
#define PNGERR_FILE_TRUNCATED	6
#define PNGERR_FILE_CORRUPT		7
#define PNGERR_UNKNOWN_CHUNK	8
#define PNGERR_COMPRESS_ERROR	9



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _png_text png_text;
struct _png_text
{
	png_text *		next;
	const char *	keyword;		/* this is allocated */
	const char *	text;			/* this points to a part of keyword */
};


typedef struct _png_info png_info;
struct _png_info
{
	UINT8 *			image;
	UINT32			width, height;
	UINT32			xres, yres;
	rectangle		screen;
	double			xscale, yscale;
	double			source_gamma;
	UINT32			resolution_unit;
	UINT8			bit_depth;
	UINT8			color_type;
	UINT8			compression_method;
	UINT8			filter_method;
	UINT8			interlace_method;

	UINT8 *			palette;
	UINT32			num_palette;

	UINT8 *			trans;
	UINT32			num_trans;

	png_text *		textlist;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void png_free(png_info *pnginfo);

int png_read_file(mame_file *fp, png_info *pnginfo);
int png_expand_buffer_8bit(png_info *p);

int png_add_text(const char *keyword, const char *text);
int png_filter(png_info *p);
int png_write_bitmap(mame_file *fp, mame_bitmap *bitmap);

int mng_capture_start(mame_file *fp, mame_bitmap *bitmap);
int mng_capture_frame(mame_file *fp, mame_bitmap *bitmap);
int mng_capture_stop(mame_file *fp);

#endif	/* __PNG_H__ */
