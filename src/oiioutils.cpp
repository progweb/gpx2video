#include <librsvg/rsvg.h>
#include <cairo.h>

#include "log_i.h"
#include "oiioutils.h"


VideoParams::Format OIIOUtils::getFormatFromOIIOBaseType(OIIO::TypeDesc::BASETYPE type) {
	switch (type) {
	case OIIO::TypeDesc::UNKNOWN:
	case OIIO::TypeDesc::NONE:
		break;

	case OIIO::TypeDesc::INT8:
	case OIIO::TypeDesc::INT16:
	case OIIO::TypeDesc::INT32:
	case OIIO::TypeDesc::UINT32:
	case OIIO::TypeDesc::INT64:
	case OIIO::TypeDesc::UINT64:
	case OIIO::TypeDesc::STRING:
#if OIIO_VERSION >= OIIO_MAKE_VERSION(2,5,0)
	case OIIO::TypeDesc::USTRINGHASH:
#endif
	case OIIO::TypeDesc::PTR:
	case OIIO::TypeDesc::LASTBASE:
	case OIIO::TypeDesc::DOUBLE:
		fprintf(stderr, "Tried to use unknown OIIO base type\n");
		break;

	case OIIO::TypeDesc::UINT8:
		return VideoParams::FormatUnsigned8;
	case OIIO::TypeDesc::UINT16:
		return VideoParams::FormatUnsigned16;
	case OIIO::TypeDesc::HALF:
		return VideoParams::FormatFloat16;
	case OIIO::TypeDesc::FLOAT:
		return VideoParams::FormatFloat32;
	}

	return VideoParams::FormatInvalid;
}


OIIO::TypeDesc::BASETYPE OIIOUtils::getOIIOBaseTypeFromFormat(enum VideoParams::Format format) {
	switch (format) {
	case VideoParams::FormatUnsigned8:
		return OIIO::TypeDesc::UINT8;
	case VideoParams::FormatUnsigned16:
		return OIIO::TypeDesc::UINT16;
	case VideoParams::FormatFloat16:
		return OIIO::TypeDesc::HALF;
	case VideoParams::FormatFloat32:
		return OIIO::TypeDesc::FLOAT;
	case VideoParams::FormatInvalid:
	case VideoParams::FormatCount:
		break;
	}

	return OIIO::TypeDesc::UNKNOWN;
}


void OIIOUtils::frameToBuffer(const Frame *frame, OIIO::ImageBuf *buf) {
	buf->set_pixels(OIIO::ROI(), 
			buf->spec().format, 
			frame->constData(), 
			OIIO::AutoStride, 
			frame->linesizeBytes());
}


void OIIOUtils::bufferToFrame(OIIO::ImageBuf *buf, const Frame *frame) {
	buf->get_pixels(OIIO::ROI(), 
			buf->spec().format, 
			frame->data(), 
			OIIO::AutoStride, 
			frame->linesizeBytes());
}


void OIIOUtils::bufferToFile(const OIIO::ImageBuf &buf, const std::string &filename) {
	std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename);

	if (out->open("/tmp/track.png", buf.spec()) == false) {
		log_error("Save OIIO buffer to file failure, can't open '%s' file", filename.c_str());
		goto error;
	}

	out->write_image(buf.spec().format, buf.localpixels());
	out->close();

error:
	return;
}


OIIO::ImageBuf * OIIOUtils::loadsvg(const char *filename, const double &size, const float *color) {
	bool apply_color;

	int stride;
	unsigned char *data;

	double width, height;

    GError *error = nullptr;

	cairo_t *cr = NULL;
	cairo_t *mask = NULL;
	cairo_surface_t *surface = NULL;
	cairo_surface_t *masksurface = NULL;

	int channelorder[] = { 2, 1, 0, 3 };
	float channelvalues[] = { };
	std::string channelnames[] = { "B", "G", "R", "A" };

	RsvgRectangle viewport;

	RsvgHandle *handle = NULL;

	OIIO::ImageBuf *buf = NULL;

	// Set color
	apply_color = (color != NULL) && (color[3] != 0);

	// load svg data
    handle = rsvg_handle_new_from_file(filename, &error);
    if (!handle) {
		log_error("Load svg image '%s' error: %s", 
				filename, 
				error ? error->message : "unknown error");

		if (error) 
        	g_error_free(error);
        goto error;
    }

    // svg dimensions
	rsvg_handle_get_intrinsic_size_in_pixels(handle, &width, &height);

	// Compute size
	width = width * size / height;
	height = size;

	// Create cairo surface (ARGB32)
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

	// Create cairo context
	cr = cairo_create(surface);

	if (apply_color) {
    	// Create cairo alpha only surface
		masksurface = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);

		// Create mask
		mask = cairo_create(masksurface);
	}

	// Render svg into cairo surface
	viewport = (RsvgRectangle) {
		.x = 0,
		.y = 0,
		.width = (double) width,
		.height = (double) height
	};

	if (!rsvg_handle_render_document(handle, apply_color ? mask : cr, &viewport, &error)) {
		log_error("Render svg '%s' error: %s", 
				filename, 
				error ? error->message : "unknown error");

		if (error) 
			g_error_free(error);
		goto error;
	}

	if (apply_color) {
		// Apply color
		cairo_set_source_rgba(cr, color[0], color[1], color[2], color[3]);

		// Paint mask
		cairo_mask_surface(cr, masksurface, 0, 0);
	}

    // Get raw pixel data from cairo
    data = cairo_image_surface_get_data(surface);
    stride = cairo_image_surface_get_stride(surface);

    // Create OpenImageIO ImageBuf (convert ARGB → RGBA if needed)
	buf = new OIIO::ImageBuf(OIIO::ImageSpec(width, height, 4, OIIO::TypeDesc::UINT8));

	// Cairo to OIIO
	if (data != NULL) {
		buf->set_pixels(OIIO::ROI(),
			buf->spec().format,
			data, 
			OIIO::AutoStride,
			stride);
	}

	// BGRA => RGBA
	OIIO::ImageBufAlgo::channels(*buf, *buf, 4, channelorder, channelvalues, channelnames);

error:
	// Free
	if (masksurface)
		cairo_surface_destroy(masksurface);
	if (surface)
		cairo_surface_destroy(surface);
	if (mask)
		cairo_destroy(mask);
	if (cr)
		cairo_destroy(cr);

	if (handle != NULL)
		g_object_unref(handle);

	return buf;
}
