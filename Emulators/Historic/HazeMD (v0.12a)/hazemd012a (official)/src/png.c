/*********************************************************************

    png.c

    PNG reading functions.

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    07/15/1998 Created by Mathis Rosenhauer
    10/02/1998 Code clean up and abstraction by Mike Balfour
            and Mathis Rosenhauer
    10/15/1998 Image filtering. MLR
    11/09/1998 Bit depths 1-8 MLR
    11/10/1998 Some additional PNG chunks recognized MLR
    05/14/1999 Color type 2 and PNG save functions added
    05/15/1999 Handle RGB555 while saving, use mame_fxxx
            functions for writing MSH
    04/27/2001 Simple MNG support MLR

    TODO : Fully comply with the "Recommendations for Decoders"
        of the W3C

*********************************************************************/

#include <math.h>
#include <zlib.h>
#include "driver.h"
#include "png.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _image_data_chunk image_data_chunk;
struct _image_data_chunk
{
	image_data_chunk *	next;
	int					length;
	UINT8 *				data;
};


typedef struct _png_private png_private;
struct _png_private
{
	png_info *			pnginfo;
	image_data_chunk *	idata;
	image_data_chunk **	idata_next;
	UINT8				bpp;
	UINT32				rowbytes;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static const int samples[] = { 1, 0, 3, 1, 2, 0, 4 };



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE UINT8 fetch_8bit(UINT8 *v)
{
	return *v;
}


INLINE UINT16 fetch_16bit(UINT8 *v)
{
	return BIG_ENDIANIZE_INT16(*(UINT16 *)v);
}


INLINE UINT32 fetch_32bit(UINT8 *v)
{
	return BIG_ENDIANIZE_INT32(*(UINT32 *)v);
}


INLINE void put_8bit(UINT8 *v, UINT8 data)
{
	*v = data;
}


INLINE void put_16bit(UINT8 *v, UINT16 data)
{
	*(UINT16 *)v = BIG_ENDIANIZE_INT16(data);
}


INLINE void put_32bit(UINT8 *v, UINT32 data)
{
	*(UINT32 *)v = BIG_ENDIANIZE_INT32(data);
}


INLINE int compute_bpp(const png_info *pnginfo)
{
	return samples[pnginfo->color_type] * pnginfo->bit_depth / 8;
}


INLINE int compute_rowbytes(const png_info *pnginfo)
{
	return (pnginfo->width * samples[pnginfo->color_type] * pnginfo->bit_depth + 7) / 8;
}



/***************************************************************************
    GENERAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    png_free - free all memory allocated in a
    pnginfo structure
-------------------------------------------------*/

void png_free(png_info *pnginfo)
{
	while (pnginfo->textlist != NULL)
	{
		png_text *temp = pnginfo->textlist;
		pnginfo->textlist = temp->next;
		if (temp->keyword != NULL)
			free((void *)temp->keyword);
		free(temp);
	}

	if (pnginfo->palette != NULL)
		free(pnginfo->palette);
	pnginfo->palette = NULL;

	if (pnginfo->trans != NULL)
		free(pnginfo->trans);
	pnginfo->trans = NULL;

	if (pnginfo->image != NULL)
		free(pnginfo->image);
	pnginfo->image = NULL;
}



/***************************************************************************
    PNG READING FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    verify_header - verify the PNG
    header at the current file location
-------------------------------------------------*/

static int verify_header(mame_file *fp)
{
	UINT8 signature[8];

	/* read 8 bytes */
	if (mame_fread(fp, signature, 8) != 8)
		return PNGERR_FILE_TRUNCATED;

	/* return an error if we don't match */
	if (memcmp(signature, PNG_Signature, 8) != 0)
		return PNGERR_BAD_SIGNATURE;

	return PNGERR_NONE;
}


/*-------------------------------------------------
    read_chunk - read the next PNG chunk
-------------------------------------------------*/

static int read_chunk(mame_file *fp, UINT8 **data, UINT32 *type, UINT32 *length)
{
	UINT32 crc, chunk_crc;
	UINT8 tempbuff[4];

	/* fetch the length of this chunk */
	if (mame_fread(fp, tempbuff, 4) != 4)
		return PNGERR_FILE_TRUNCATED;
	*length = fetch_32bit(tempbuff);

	/* fetch the type of this chunk */
	if (mame_fread(fp, tempbuff, 4) != 4)
		return PNGERR_FILE_TRUNCATED;
	*type = fetch_32bit(tempbuff);

	/* stop when we hit an IEND chunk */
	if (*type == PNG_CN_IEND)
		return PNGERR_NONE;

	/* start the CRC with the chunk type (but not the length) */
	crc = crc32(0, tempbuff, 4);

	/* read the chunk itself into an allocated memory buffer */
	*data = NULL;
	if (*length != 0)
	{
		/* allocate memory for this chunk */
		*data = (UINT8 *)malloc(*length);
		if (*data == NULL)
			return PNGERR_OUT_OF_MEMORY;

		/* read the data from the file */
		if (mame_fread(fp, *data, *length) != *length)
		{
			free(*data);
			return PNGERR_FILE_TRUNCATED;
		}

		/* update the CRC */
		crc = crc32(crc, *data, *length);
	}

	/* read the CRC */
	if (mame_fread(fp, tempbuff, 4) != 4)
	{
		free(*data);
		return PNGERR_FILE_TRUNCATED;
	}
	chunk_crc = fetch_32bit(tempbuff);

	/* validate the CRC */
	if (crc != chunk_crc)
	{
		free(*data);
		return PNGERR_FILE_CORRUPT;
	}
	return PNGERR_NONE;
}


/*-------------------------------------------------
    process_chunk - process a PNG chunk
-------------------------------------------------*/

static int process_chunk(png_private *png, UINT8 *data, UINT32 type, UINT32 length, int *keepmem)
{
	/* default to not keeping memory */
	*keepmem = FALSE;

	/* switch off of the type */
	switch (type)
	{
		/* image header */
		case PNG_CN_IHDR:
			png->pnginfo->width = fetch_32bit(data);
			png->pnginfo->height = fetch_32bit(data + 4);
			png->pnginfo->bit_depth = fetch_8bit(data + 8);
			png->pnginfo->color_type = fetch_8bit(data + 9);
			png->pnginfo->compression_method = fetch_8bit(data + 10);
			png->pnginfo->filter_method = fetch_8bit(data + 11);
			png->pnginfo->interlace_method = fetch_8bit(data + 12);
			break;

		/* palette */
		case PNG_CN_PLTE:
			png->pnginfo->num_palette = length / 3;
			png->pnginfo->palette = data;
			*keepmem = TRUE;
			break;

		/* transparency information */
		case PNG_CN_tRNS:
			png->pnginfo->num_trans = length;
			png->pnginfo->trans = data;
			*keepmem = TRUE;
			break;

		/* image data */
		case PNG_CN_IDAT:

			/* allocate a new image data descriptor */
			*png->idata_next = malloc(sizeof(**png->idata_next));
			if (*png->idata_next == NULL)
				return PNGERR_OUT_OF_MEMORY;

			/* add it to the tail of the list */
			(*png->idata_next)->next = NULL;
			(*png->idata_next)->length = length;
			(*png->idata_next)->data = data;
			png->idata_next = &(*png->idata_next)->next;
			*keepmem = TRUE;
			break;

		/* gamma */
		case PNG_CN_gAMA:
			png->pnginfo->source_gamma = fetch_32bit(data) / 100000.0;
			break;

		/* physical information */
		case PNG_CN_pHYs:
			png->pnginfo->xres = fetch_32bit(data);
			png->pnginfo->yres = fetch_32bit(data + 4);
			png->pnginfo->resolution_unit = fetch_8bit(data + 8);
			break;

		/* text */
		case PNG_CN_tEXt:
		{
			png_text *text, *pt, *ct;

			/* allocate a new text item */
			text = malloc(sizeof(*text));
			if (text == NULL)
				return PNGERR_OUT_OF_MEMORY;

			/* set the elements */
			text->keyword = data;
			text->text = text->keyword + strlen(text->keyword) + 1;
			text->next = NULL;

			/* add to the end of the list */
			for (pt = NULL, ct = png->pnginfo->textlist; ct != NULL; pt = ct, ct = ct->next) ;
			if (pt == NULL)
				png->pnginfo->textlist = text;
			else
				pt->next = text;

			*keepmem = TRUE;
			break;
		}

		/* anything else */
		default:
			if ((type & 0x20000000) == 0)
				return PNGERR_UNKNOWN_CHUNK;
			break;
	}
	return PNGERR_NONE;
}


/*-------------------------------------------------
    unfilter_row - unfilter a single row of pixels
-------------------------------------------------*/

static int unfilter_row(int type, UINT8 *src, UINT8 *dst, UINT8 *dstprev, int bpp, int rowbytes)
{
	int x;

	/* switch off of it */
	switch (type)
	{
		/* no filter, just copy */
		case PNG_PF_None:
			for (x = 0; x < rowbytes; x++)
				*dst++ = *src++;
			break;

		/* SUB = previous pixel */
		case PNG_PF_Sub:
			for (x = 0; x < bpp; x++)
				*dst++ = *src++;
			for (x = bpp; x < rowbytes; x++, dst++)
				*dst = *src++ + dst[-bpp];
			break;

		/* UP = pixel above */
		case PNG_PF_Up:
			if (dstprev == NULL)
				return unfilter_row(PNG_PF_None, src, dst, dstprev, bpp, rowbytes);
			for (x = 0; x < rowbytes; x++, dst++)
				*dst = *src++ + *dstprev++;
			break;

		/* AVERAGE = average of pixel above and previous pixel */
		case PNG_PF_Average:
			if (dstprev == NULL)
			{
				for (x = 0; x < bpp; x++)
					*dst++ = *src++;
				for (x = bpp; x < rowbytes; x++, dst++)
					*dst = *src++ + dst[-bpp] / 2;
			}
			else
			{
				for (x = 0; x < bpp; x++, dst++)
					*dst = *src++ + *dstprev++ / 2;
				for (x = bpp; x < rowbytes; x++, dst++)
					*dst = *src++ + (*dstprev++ + dst[-bpp]) / 2;
			}
			break;

		/* PAETH = special filter */
		case PNG_PF_Paeth:
			for (x = 0; x < rowbytes; x++)
			{
				INT32 pa = (x < bpp) ? 0 : dst[-bpp];
				INT32 pc = (x < bpp || dstprev == NULL) ? 0 : dstprev[-bpp];
				INT32 pb = (dstprev == NULL) ? 0 : *dstprev++;
				INT32 prediction = pa + pb - pc;
				INT32 da = abs(prediction - pa);
				INT32 db = abs(prediction - pb);
				INT32 dc = abs(prediction - pc);
				if (da <= db && da <= dc)
					*dst++ = *src++ + pa;
				else if (db <= dc)
					*dst++ = *src++ + pb;
				else
					*dst++ = *src++ + pc;
			}
			break;

		/* unknown filter type */
		default:
			return PNGERR_UNKNOWN_FILTER;
	}

	return PNGERR_NONE;
}


/*-------------------------------------------------
    process_image - post-process a loaded iamge
-------------------------------------------------*/

static int process_image(png_private *png)
{
	int rowbytes, bpp, imagesize;
	int error = PNGERR_NONE;
	image_data_chunk *idat;
	UINT8 *src, *dst;
	z_stream stream;
	int zerr, y;

	/* compute some basic parameters */
	bpp = compute_bpp(png->pnginfo);
	rowbytes = compute_rowbytes(png->pnginfo);
	imagesize = png->pnginfo->height * (rowbytes + 1);

	/* allocate memory for the filtered image */
	png->pnginfo->image = (UINT8 *)malloc(imagesize);
	if (png->pnginfo->image == NULL)
		return PNGERR_OUT_OF_MEMORY;

	/* initialize the stream */
	memset(&stream, 0, sizeof(stream));
	stream.next_out = png->pnginfo->image;
	stream.avail_out = imagesize;
	zerr = inflateInit(&stream);
	if (zerr != Z_OK)
	{
		error = PNGERR_DECOMPRESS_ERROR;
		goto handle_error;
	}

	/* loop over IDAT and decompress each as part of a larger stream */
	for (idat = png->idata; idat != NULL; idat = idat->next)
	{
		/* decompress this chunk */
		stream.next_in = idat->data;
		stream.avail_in = idat->length;
		zerr = inflate(&stream, Z_NO_FLUSH);

		/* stop at the end of the stream */
		if (zerr == Z_STREAM_END)
			break;

		/* other errors are fatal */
		if (zerr != Z_OK)
		{
			error = PNGERR_DECOMPRESS_ERROR;
			goto handle_error;
		}
	}

	/* clean up */
	zerr = inflateEnd(&stream);
	if (zerr != Z_OK)
	{
		error = PNGERR_DECOMPRESS_ERROR;
		goto handle_error;
	}

	/* we de-filter in place */
	src = dst = png->pnginfo->image;

	/* iterate over rows */
	for (y = 0; y < png->pnginfo->height && error == PNGERR_NONE; y++)
	{
		/* first byte of each row is the filter type */
		int filter = *src++;
		error = unfilter_row(filter, src, dst, (y == 0) ? NULL : &dst[-rowbytes], bpp, rowbytes);
		src += rowbytes;
		dst += rowbytes;
	}

handle_error:
	/* if we errored, free the image data */
	if (error != PNGERR_NONE)
	{
		free(png->pnginfo->image);
		png->pnginfo->image = NULL;
	}
	return error;
}


/*-------------------------------------------------
    png_read_file - read a PNG from a MAME stream
-------------------------------------------------*/

int png_read_file(mame_file *fp, png_info *pnginfo)
{
	UINT8 *chunk_data = NULL;
	png_private png;
	int error;

	/* initialize the data structures */
	memset(&png, 0, sizeof(png));
	memset(pnginfo, 0, sizeof(*pnginfo));
	png.pnginfo = pnginfo;
	png.idata_next = &png.idata;

	/* verify the signature at the start of the file */
	error = verify_header(fp);
	if (error != PNGERR_NONE)
		goto handle_error;

	/* loop until we hit an IEND chunk */
	for ( ; ; )
	{
		UINT32 chunk_type, chunk_length;
		int keepmem;

		/* read a chunk */
		error = read_chunk(fp, &chunk_data, &chunk_type, &chunk_length);
		if (error != PNGERR_NONE)
			goto handle_error;

		/* stop when we hit an IEND chunk */
		if (chunk_type == PNG_CN_IEND)
			break;

		/* process the chunk */
		error = process_chunk(&png, chunk_data, chunk_type, chunk_length, &keepmem);
		if (error != PNGERR_NONE)
			goto handle_error;

		/* free memory if we didn't want to keep it */
		if (!keepmem)
			free(chunk_data);
		chunk_data = NULL;
	}

	/* finish processing the image */
	error = process_image(&png);
	if (error != PNGERR_NONE)
		goto handle_error;

handle_error:

	/* free all intermediate data */
	while (png.idata != NULL)
	{
		image_data_chunk *next = png.idata->next;
		if (png.idata->data != NULL)
			free(png.idata->data);
		free(png.idata);
		png.idata = next;
	}
	if (chunk_data != NULL)
		free(chunk_data);

	/* if we have an error, free all the other data as well */
	if (error != PNGERR_NONE)
	{
		png_free(pnginfo);
		memset(pnginfo, 0, sizeof(*pnginfo));
	}
	return error;
}


/*-------------------------------------------------
    png_expand_buffer_8bit - expand a buffer from
    sub 8-bit to 8-bit
-------------------------------------------------*/

int png_expand_buffer_8bit(png_info *pnginfo)
{
	int i,j, k;
	UINT8 *inp, *outp, *outbuf;

	/* nothing to do if we're at 8 or greater already */
	if (pnginfo->bit_depth >= 8)
		return PNGERR_NONE;

	/* allocate a new buffer at 8-bit */
	outbuf = malloc(pnginfo->width * pnginfo->height);
	if (outbuf == NULL)
		return PNGERR_OUT_OF_MEMORY;

	inp = pnginfo->image;
	outp = outbuf;

	for (i = 0; i < pnginfo->height; i++)
	{
		for(j = 0; j < pnginfo->width / ( 8 / pnginfo->bit_depth); j++)
		{
			for (k = 8 / pnginfo->bit_depth-1; k >= 0; k--)
				*outp++ = (*inp >> k * pnginfo->bit_depth) & (0xff >> (8 - pnginfo->bit_depth));
			inp++;
		}
		if (pnginfo->width % (8 / pnginfo->bit_depth))
		{
			for (k = pnginfo->width % (8 / pnginfo->bit_depth)-1; k >= 0; k--)
				*outp++ = (*inp >> k * pnginfo->bit_depth) & (0xff >> (8 - pnginfo->bit_depth));
			inp++;
		}
	}
	free (pnginfo->image);
	pnginfo->image = outbuf;

	return 1;
}



/***************************************************************************
    PNG WRITING FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    add_text - add a text entry to the png_info
-------------------------------------------------*/

static int add_text(png_info *pnginfo, const char *keyword, const char *text)
{
	png_text *newtext, *pt, *ct;
	char *textdata;
	int keylen;

	/* allocate a new text element */
	newtext = malloc(sizeof(*newtext));
	if (newtext == NULL)
		return PNGERR_OUT_OF_MEMORY;

	/* allocate a string long enough to hold both */
	keylen = strlen(keyword);
	textdata = malloc(keylen + 1 + strlen(text) + 1);
	if (textdata == NULL)
	{
		free(newtext);
		return PNGERR_OUT_OF_MEMORY;
	}

	/* copy in the data */
	strcpy(textdata, keyword);
	strcpy(textdata + keylen + 1, text);

	/* text follows a trailing NULL */
	newtext->keyword = textdata;
	newtext->text = textdata + keylen + 1;
	newtext->next = NULL;

	/* add us to the end of the linked list */
	for (pt = NULL, ct = pnginfo->textlist; ct != NULL; pt = ct, ct = ct->next) ;
	if (pt == NULL)
		pnginfo->textlist = newtext;
	else
		pt->next = newtext;

	return PNGERR_NONE;
}


/*-------------------------------------------------
    write_chunk - write an in-memory chunk to
    the given file
-------------------------------------------------*/

static int write_chunk(mame_file *fp, const UINT8 *data, UINT32 type, UINT32 length)
{
	UINT8 tempbuff[8];
	UINT32 crc;

	/* stuff the length/type into the buffer */
	put_32bit(tempbuff + 0, length);
	put_32bit(tempbuff + 4, type);
	crc = crc32(0, tempbuff + 4, 4);

	/* write that data */
	if (mame_fwrite(fp, tempbuff, 8) != 8)
		return PNGERR_FILE_ERROR;

	/* append the actual data */
	if (length > 0)
	{
		if (mame_fwrite(fp, data, length) != length)
			return PNGERR_FILE_ERROR;
		crc = crc32(crc, data, length);
	}

	/* write the CRC */
	put_32bit(tempbuff, crc);
	if (mame_fwrite(fp, tempbuff, 4) != 4)
		return PNGERR_FILE_ERROR;

	return PNGERR_NONE;
}


/*-------------------------------------------------
    write_deflated_chunk - write an in-memory
    chunk to the given file by deflating it
-------------------------------------------------*/

static int write_deflated_chunk(mame_file *fp, UINT8 *data, UINT32 type, UINT32 length)
{
	UINT64 lengthpos = mame_ftell(fp);
	UINT8 tempbuff[8192];
    UINT32 zlength = 0;
    z_stream stream;
	UINT32 crc;
	int zerr;

	/* stuff the length/type into the buffer */
	put_32bit(tempbuff + 0, length);
	put_32bit(tempbuff + 4, type);
	crc = crc32(0, tempbuff + 4, 4);

	/* write that data */
	if (mame_fwrite(fp, tempbuff, 8) != 8)
		return PNGERR_FILE_ERROR;

	/* initialize the stream */
	memset(&stream, 0, sizeof(stream));
	stream.next_in = data;
	stream.avail_in = length;
    zerr = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    if (zerr != Z_OK)
    	return PNGERR_COMPRESS_ERROR;

	/* now loop until we run out of data */
	for ( ; ; )
	{
		/* compress this chunk */
		stream.next_out = tempbuff;
		stream.avail_out = sizeof(tempbuff);
		zerr = deflate(&stream, Z_FINISH);

		/* if there's data to write, do it */
		if (stream.avail_out < sizeof(tempbuff))
		{
			int bytes = sizeof(tempbuff) - stream.avail_out;
			if (mame_fwrite(fp, tempbuff, bytes) != bytes)
			{
				deflateEnd(&stream);
				return PNGERR_FILE_ERROR;
			}
			crc = crc32(crc, tempbuff, bytes);
			zlength += bytes;
		}

		/* stop at the end of the stream */
		if (zerr == Z_STREAM_END)
			break;

		/* other errors are fatal */
		if (zerr != Z_OK)
		{
			deflateEnd(&stream);
			return PNGERR_COMPRESS_ERROR;
		}
	}

	/* clean up deflater(maus) */
	zerr = deflateEnd(&stream);
	if (zerr != Z_OK)
		return PNGERR_COMPRESS_ERROR;

	/* write the CRC */
	put_32bit(tempbuff, crc);
	if (mame_fwrite(fp, tempbuff, 4) != 4)
		return PNGERR_FILE_ERROR;

	/* seek back and update the length */
	mame_fseek(fp, lengthpos, SEEK_SET);
	put_32bit(tempbuff + 0, zlength);
	if (mame_fwrite(fp, tempbuff, 4) != 4)
		return PNGERR_FILE_ERROR;

	/* return to the end */
	mame_fseek(fp, lengthpos + 8 + zlength + 4, SEEK_SET);
	return PNGERR_NONE;
}


/*-------------------------------------------------
    convert_bitmap_to_image_palette - convert a
    bitmap to a palettized image
-------------------------------------------------*/

static int convert_bitmap_to_image_palette(png_info *pnginfo, const mame_bitmap *bitmap, int palette_length, const rgb_t *palette)
{
	int rowbytes;
	int x, y;

	/* set the common info */
	pnginfo->width = bitmap->width;
	pnginfo->height = bitmap->height;
	pnginfo->bit_depth = 8;
	pnginfo->color_type = 3;
	pnginfo->num_palette = 256;
	rowbytes = pnginfo->width;

	/* allocate memory for the palette */
	pnginfo->palette = malloc(3 * 256);
	if (pnginfo->palette == NULL)
		return PNGERR_OUT_OF_MEMORY;

	/* build the palette */
	memset(pnginfo->palette, 0, 3 * 256);
	for (x = 0; x < palette_length; x++)
	{
		rgb_t color = palette[x];
		pnginfo->palette[3 * x + 0] = RGB_RED(color);
		pnginfo->palette[3 * x + 1] = RGB_GREEN(color);
		pnginfo->palette[3 * x + 2] = RGB_BLUE(color);
	}

	/* allocate memory for the image */
	pnginfo->image = malloc(pnginfo->height * (rowbytes + 1));
	if (pnginfo->image == NULL)
	{
		free(pnginfo->palette);
		return PNGERR_OUT_OF_MEMORY;
	}

	/* copy in the pixels, specifying a NULL filter */
	for (y = 0; y < pnginfo->height; y++)
	{
		UINT16 *src = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
		UINT8 *dst = pnginfo->image + y * (rowbytes + 1);

		/* store the filter byte, then copy the data */
		*dst++ = 0;
		for (x = 0; x < pnginfo->width; x++)
			*dst++ = *src++;
	}

	return PNGERR_NONE;
}


/*-------------------------------------------------
    convert_bitmap_to_image_rgb - convert a
    bitmap to an RGB image
-------------------------------------------------*/

static int convert_bitmap_to_image_rgb(png_info *pnginfo, const mame_bitmap *bitmap, int palette_length, const rgb_t *palette)
{
	int rowbytes;
	int x, y;

	/* set the common info */
	pnginfo->width = bitmap->width;
	pnginfo->height = bitmap->height;
	pnginfo->bit_depth = 8;
	pnginfo->color_type = 2;
	rowbytes = pnginfo->width * 3;

	/* allocate memory for the image */
	pnginfo->image = malloc(pnginfo->height * (rowbytes + 1));
	if (pnginfo->image == NULL)
		return PNGERR_OUT_OF_MEMORY;

	/* copy in the pixels, specifying a NULL filter */
	for (y = 0; y < pnginfo->height; y++)
	{
		UINT32 *src32 = (UINT32 *)bitmap->base + y * bitmap->rowpixels;
		UINT16 *src16 = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
		UINT8 *dst = pnginfo->image + y * (rowbytes + 1);

		/* store the filter byte, then copy the data */
		*dst++ = 0;

		/* based on the bitmap depth, we need to translate */
		switch (bitmap->depth)
		{
			/* 16bpp palettized */
			case 16:
				for (x = 0; x < pnginfo->width; x++)
				{
					rgb_t color = palette[*src16++];
					*dst++ = RGB_RED(color);
					*dst++ = RGB_GREEN(color);
					*dst++ = RGB_BLUE(color);
				}
				break;

			/* 15-bit RGB direct */
			case 15:
				for (x = 0; x < pnginfo->width; x++)
				{
					UINT16 raw = *src16++;
					*dst++ = pal5bit(raw >> 10);
					*dst++ = pal5bit(raw >> 5);
					*dst++ = pal5bit(raw >> 0);
				}
				break;

			/* 32-bit RGB direct */
			case 32:
				for (x = 0; x < pnginfo->width; x++)
				{
					UINT32 raw = *src32++;
					*dst++ = RGB_RED(raw);
					*dst++ = RGB_GREEN(raw);
					*dst++ = RGB_BLUE(raw);
				}
				break;
		}
	}

	return PNGERR_NONE;
}


/*-------------------------------------------------
    write_png_stream - stream a series of PNG
    chunks to the given file
-------------------------------------------------*/

static int write_png_stream(void *fp, png_info *pnginfo, const mame_bitmap *bitmap, int palette_length, const rgb_t *palette)
{
	UINT8 tempbuff[16];
	png_text *text;
	int error;

	/* create an unfiltered image in either palette or RGB form */
	if (bitmap->depth == 16 && palette_length <= 256)
		error = convert_bitmap_to_image_palette(pnginfo, bitmap, palette_length, palette);
	else
		error = convert_bitmap_to_image_rgb(pnginfo, bitmap, palette_length, palette);
	if (error != PNGERR_NONE)
		goto handle_error;

	/* if we wanted to get clever and do filtering, we would do it here */

	/* write the IHDR chunk */
	put_32bit(tempbuff + 0, pnginfo->width);
	put_32bit(tempbuff + 4, pnginfo->height);
	put_8bit(tempbuff + 8, pnginfo->bit_depth);
	put_8bit(tempbuff + 9, pnginfo->color_type);
	put_8bit(tempbuff + 10, pnginfo->compression_method);
	put_8bit(tempbuff + 11, pnginfo->filter_method);
	put_8bit(tempbuff + 12, pnginfo->interlace_method);
	error = write_chunk(fp, tempbuff, PNG_CN_IHDR, 13);
	if (error != PNGERR_NONE)
		goto handle_error;

	/* write the PLTE chunk */
	if (pnginfo->num_palette > 0)
		error = write_chunk(fp, pnginfo->palette, PNG_CN_PLTE, pnginfo->num_palette * 3);
	if (error != PNGERR_NONE)
		goto handle_error;

	/* write a single IDAT chunk */
	error = write_deflated_chunk(fp, pnginfo->image, PNG_CN_IDAT, pnginfo->height * (compute_rowbytes(pnginfo) + 1));
	if (error != PNGERR_NONE)
		goto handle_error;

	/* write TEXT chunks */
	for (text = pnginfo->textlist; text != NULL; text = text->next)
	{
		error = write_chunk(fp, text->keyword, PNG_CN_tEXt, strlen(text->keyword) + 1 + strlen(text->text) + 1);
		if (error != PNGERR_NONE)
			goto handle_error;
	}

	/* write an IEND chunk */
	error = write_chunk(fp, NULL, PNG_CN_IEND, 0);

handle_error:
	return error;
}



int png_write_bitmap(mame_file *fp, mame_bitmap *bitmap)
{
	png_info pnginfo;
	char text[1024];
	int error;

	/* reset all entries in the PNG info */
	memset(&pnginfo, 0, sizeof(pnginfo));

	/* add two text entries describing the image */
	sprintf(text, APPNAME " %s", build_version);
	add_text(&pnginfo, "Software", text);
	sprintf(text, "%s %s", Machine->gamedrv->manufacturer, Machine->gamedrv->description);
	add_text(&pnginfo, "System", text);

	/* write the PNG signature */
	if (mame_fwrite(fp, PNG_Signature, 8) != 8)
	{
		png_free(&pnginfo);
		return PNGERR_FILE_ERROR;
	}

	/* write the rest of the PNG data */
	error = write_png_stream(fp, &pnginfo, bitmap, Machine->drv->total_colors, palette_get_adjusted_colors(Machine));
	png_free(&pnginfo);
	return error;
}

/********************************************************************************

  MNG write functions

********************************************************************************/

/* temporary hack for now */
static png_info mnginfo;

int mng_capture_start(mame_file *fp, mame_bitmap *bitmap)
{
	UINT8 mhdr[28];
	char text[1024];
	int error;

	memset(&mnginfo, 0, sizeof(mnginfo));

	sprintf (text, APPNAME " %s", build_version);
	add_text(&mnginfo, "Software", text);
	sprintf (text, "%s %s", Machine->gamedrv->manufacturer, Machine->gamedrv->description);
	add_text(&mnginfo, "System", text);

	if (mame_fwrite(fp, MNG_Signature, 8) != 8)
	{
		png_free(&mnginfo);
		return PNGERR_FILE_ERROR;
	}

	memset(mhdr, 0, 28);
	put_32bit(mhdr + 0, bitmap->width);
	put_32bit(mhdr + 4, bitmap->height);
	put_32bit(mhdr + 8, Machine->screen[0].refresh);
	put_32bit(mhdr + 24, 0x0041); /* Simplicity profile */
	/* frame count and play time unspecified because
       we don't know at this stage */
	error = write_chunk(fp, mhdr, MNG_CN_MHDR, 28);
	if (error != PNGERR_NONE)
		return error;

	return PNGERR_NONE;
}

int mng_capture_frame(mame_file *fp, mame_bitmap *bitmap)
{
	return write_png_stream(fp, &mnginfo, bitmap, Machine->drv->total_colors, palette_get_adjusted_colors(Machine));
}

int mng_capture_stop(mame_file *fp)
{
	return write_chunk(fp, NULL, MNG_CN_MEND, 0);
}

