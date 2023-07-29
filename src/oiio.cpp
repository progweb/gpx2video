#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <unordered_map>

//#include <OpenImageIO/half.h>

#include <OpenImageIO/filesystem.h>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imagebufalgo_util.h>
#include <OpenImageIO/sysutil.h>
#include <OpenImageIO/thread.h>

//#ifdef USE_FREETYPE
#    include <ft2build.h>
#    include FT_FREETYPE_H
//#endif

#include "oiio.h"



OIIO_NAMESPACE_BEGIN

namespace ImageBufAlgo {

namespace { // anon
static mutex ft_mutex;
static FT_Library ft_library = NULL;
static bool ft_broken        = false;
static std::vector<std::string> font_search_dirs;
static const char* default_font_name[] = { "DroidSans", "cour", "Courier New",
                                           "FreeMono", nullptr };

// Helper: given unicode and a font face, compute its size
static ROI
text_size_from_unicode(std::vector<uint32_t>& utext, FT_Face face)
{
    ROI size;
    size.xbegin = size.ybegin = std::numeric_limits<int>::max();
    size.xend = size.yend = std::numeric_limits<int>::min();
    FT_GlyphSlot slot     = face->glyph;
    int x                 = 0;
    for (auto ch : utext) {
        int error = FT_Load_Char(face, ch, FT_LOAD_RENDER);
        if (error)
            continue;  // ignore errors
        size.ybegin = std::min(size.ybegin, -slot->bitmap_top);
        size.yend   = std::max(size.yend, int(slot->bitmap.rows)
                                            - int(slot->bitmap_top) + 1);
        size.xbegin = std::min(size.xbegin, x + int(slot->bitmap_left));
        size.xend   = std::max(size.xend, x + int(slot->bitmap.width)
                                            + int(slot->bitmap_left) + 1);
        // increment pen position
        x += slot->advance.x >> 6;
    }
    return size;  // Font rendering not supported
}



// Given font name, resolve it to an existing font filename.
// If found, return true and put the resolved filename in result.
// If not found, return false and put an error message in result.
// Not thread-safe! The caller must use the mutex.
static bool
resolve_font(string_view font_, std::string& result)
{
    result.clear();

    // If we know FT is broken, don't bother trying again
    if (ft_broken)
        return false;

    // If FT not yet initialized, do it now.
    if (!ft_library) {
        if (FT_Init_FreeType(&ft_library)) {
            ft_broken = true;
            result    = "Could not initialize FreeType for font rendering";
            return false;
        }
    }

    // A set of likely directories for fonts to live, across several systems.
    // Fill out the list of search dirs if not yet done.
    if (font_search_dirs.size() == 0) {
        string_view home = Sysutil::getenv("HOME");
        if (home.size()) {
            std::string h(home);
            font_search_dirs.push_back(h + "/fonts");
            font_search_dirs.push_back(h + "/Fonts");
            font_search_dirs.push_back(h + "/Library/Fonts");
        }
        string_view systemRoot = Sysutil::getenv("SystemRoot");
        if (systemRoot.size())
            font_search_dirs.push_back(std::string(systemRoot) + "/Fonts");
        font_search_dirs.emplace_back("/usr/share/fonts");
        font_search_dirs.emplace_back("/usr/share/fonts/OpenImageIO");
        font_search_dirs.emplace_back("/Library/Fonts");
        font_search_dirs.emplace_back("/Library/Fonts/OpenImageIO");
        font_search_dirs.emplace_back("C:/Windows/Fonts");
        font_search_dirs.emplace_back("C:/Windows/Fonts/OpenImageIO");
        font_search_dirs.emplace_back("/usr/local/share/fonts");
        font_search_dirs.emplace_back("/usr/local/share/fonts/OpenImageIO");
        font_search_dirs.emplace_back("/opt/local/share/fonts");
        font_search_dirs.emplace_back("/opt/local/share/fonts/OpenImageIO");
        // Try $OPENIMAGEIO_ROOT_DIR/fonts
        string_view OpenImageIOrootdir = Sysutil::getenv("OpenImageIO_ROOT");
        if (OpenImageIOrootdir.size()) {
            font_search_dirs.push_back(std::string(OpenImageIOrootdir)
                                       + "/fonts");
            font_search_dirs.push_back(std::string(OpenImageIOrootdir)
                                       + "/share/fonts/OpenImageIO");
        }
        string_view oiiorootdir = Sysutil::getenv("OPENIMAGEIO_ROOT_DIR");
        if (oiiorootdir.size()) {
            font_search_dirs.push_back(std::string(oiiorootdir) + "/fonts");
            font_search_dirs.push_back(std::string(oiiorootdir)
                                       + "/share/fonts/OpenImageIO");
        }
        // Try $OPENIMAGEIOHOME/fonts -- deprecated (1.9)
        string_view oiiohomedir = Sysutil::getenv("OPENIMAGEIOHOME");
        if (oiiohomedir.size()) {
            font_search_dirs.push_back(std::string(oiiohomedir) + "/fonts");
            font_search_dirs.push_back(std::string(oiiohomedir)
                                       + "/share/fonts/OpenImageIO");
        }
        // Try ../fonts relative to where this executing binary came from
        std::string this_program = OIIO::Sysutil::this_program_path();
        if (this_program.size()) {
            std::string path = Filesystem::parent_path(this_program);
            path             = Filesystem::parent_path(path);
            font_search_dirs.push_back(path + "/fonts");
            font_search_dirs.push_back(path + "/share/fonts/OpenImageIO");
        }
    }

    // Try to find the font.  Experiment with several extensions
    std::string font = font_;
    if (font.empty()) {
        // nothing specified -- look for something to use as a default.
        for (int j = 0; default_font_name[j] && font.empty(); ++j) {
            static const char* extensions[] = { "", ".ttf", ".pfa", ".pfb",
                                                NULL };
            for (int i = 0; font.empty() && extensions[i]; ++i)
                font = Filesystem::searchpath_find(
                    std::string(default_font_name[j]) + extensions[i],
                    font_search_dirs, true, true);
        }
        if (font.empty()) {
            result = "Could not set default font face";
            return false;
        }
    } else if (Filesystem::is_regular(font)) {
        // directly specified a filename -- use it
    } else {
        // A font name was specified but it's not a full path, look for it
        std::string f;
        static const char* extensions[] = { "", ".ttf", ".pfa", ".pfb", NULL };
        for (int i = 0; f.empty() && extensions[i]; ++i)
            f = Filesystem::searchpath_find(font + extensions[i],
                                            font_search_dirs, true, true);
        if (f.empty()) {
            result = Strutil::sprintf("Could not set font face to \"%s\"",
                                      font);
            return false;
        }
        font = f;
    }

    if (!Filesystem::is_regular(font)) {
        result = Strutil::sprintf("Could not find font \"%s\"", font);
        return false;
    }

    // Success
    result = font;
    return true;
}

}


bool
render_text_shadow(ImageBuf& R, int x, int y, string_view text,
                          int fontsize, string_view font_,
                          cspan<float> textcolor, 
						  TextAlignX alignx, TextAlignY aligny, 
						  int shadow, 
						  ROI roi,
                          int /*nthreads*/) {
	if (R.spec().depth > 1) {
		return false;
	}

	// Thread safety
	lock_guard ft_lock(ft_mutex);

	std::string font;
	bool ok = resolve_font(font_, font);
    if (!ok) {
        std::string err = font.size() ? font : "Font error";
        R.errorf("%s", err);
        return false;
    }

    int error = 0;
    FT_Face face;  // handle to face object
    error = FT_New_Face(ft_library, font.c_str(), 0 /* face index */, &face);
    if (error) {
        R.errorf("Could not set font face to \"%s\"", font);
        return false;  // couldn't open the face
    }

    error = FT_Set_Pixel_Sizes(face /*handle*/, 0 /*width*/,
                               fontsize /*height*/);
    if (error) {
        FT_Done_Face(face);
        R.errorf("Could not set font size to %d", fontsize);
        return false;  // couldn't set the character size
    }

    FT_GlyphSlot slot = face->glyph;  // a small shortcut
    int nchannels(R.nchannels());
    IBA_FIX_PERCHAN_LEN_DEF(textcolor, nchannels);

    // Take into account the alpha of the requested text color. Slightly
    // complicated logic to try to make our best guess.
    int alpha_channel = R.spec().alpha_channel;
    float textalpha   = 1.0f;
    if (alpha_channel >= 0 && alpha_channel < int(textcolor.size())) {
        // If the image we're writing into has a designated alpha, use it.
        textalpha = textcolor[alpha_channel];
    } else if (alpha_channel < 0 && nchannels <= 4 && textcolor.size() == 4) {
        // If the buffer doesn't have an alpha, but the text color passed
        // has 4 values, assume the last value is supposed to be alpha.
        textalpha = textcolor[3];
		alpha_channel = 3;
    } else
		alpha_channel = -1;

    // Convert the UTF to 32 bit unicode
    std::vector<uint32_t> utext;
    utext.reserve(
        text.size());  //Possible overcommit, but most text will be ascii
    Strutil::utf8_to_unicode(text, utext);

    // Compute the size that the text will render as, into an ROI
    ROI textroi     = text_size_from_unicode(utext, face);
    textroi.zbegin  = 0;
    textroi.zend    = 1;
    textroi.chbegin = 0;
    textroi.chend   = 1;

    // Adjust position for alignment requests
    if (alignx == TextAlignX::Right)
        x -= textroi.width();
    if (alignx == TextAlignX::Center)
        x -= (textroi.width() / 2 + textroi.xbegin);
    if (aligny == TextAlignY::Top)
        y += textroi.height();
    if (aligny == TextAlignY::Bottom)
        y -= textroi.height();
    if (aligny == TextAlignY::Center)
        y -= (textroi.height() / 2 + textroi.ybegin);

    // Pad bounds for shadowing
    textroi.xbegin += x - shadow;
    textroi.xend += x + shadow;
    textroi.ybegin += y - shadow;
    textroi.yend += y + shadow;

    // Create a temp buffer of the right size and render the text into it.
    ImageBuf textimg(ImageSpec(textroi, TypeDesc::FLOAT));
    ImageBufAlgo::zero(textimg);

    // Glyph by glyph, fill in our txtimg buffer
    for (auto ch : utext) {
        int error = FT_Load_Char(face, ch, FT_LOAD_RENDER);
        if (error)
            continue;  // ignore errors
        // now, draw to our target surface
        for (int j = 0; j < static_cast<int>(slot->bitmap.rows); ++j) {
            int ry = y + j - slot->bitmap_top;
            for (int i = 0; i < static_cast<int>(slot->bitmap.width); ++i) {
                int rx  = x + i + slot->bitmap_left;
                float b = slot->bitmap.buffer[slot->bitmap.pitch * j + i]
                          / 255.0f;
                textimg.setpixel(rx, ry, &b, 1);
            }
        }
        // increment pen position
        x += slot->advance.x >> 6;
    }

    // Generate the alpha image -- if drop shadow is requested, dilate,
    // otherwise it's just a copy of the text image
    ImageBuf alphaimg;
    if (shadow)
        dilate(alphaimg, textimg, 2 * shadow + 1);
    else
        alphaimg.copy(textimg);

    if (!roi.defined())
        roi = textroi;
    if (!IBAprep(roi, &R))
        return false;
    roi = roi_intersection(textroi, R.roi());

    // Now fill in the pixels of our destination image
    float* pixelcolor = OIIO_ALLOCA(float, nchannels);
    ImageBuf::ConstIterator<float> t(textimg, roi, ImageBuf::WrapBlack);
    ImageBuf::ConstIterator<float> a(alphaimg, roi, ImageBuf::WrapBlack);
    ImageBuf::Iterator<float> r(R, roi);

    for (; !r.done(); ++r, ++t, ++a) {
        float val   = t[0];
		float alpha = a[0] * textalpha;

		R.getpixel(r.x(), r.y(), pixelcolor, nchannels);

		if (alpha == 0.0)
			continue;

        for (int c = 0; c < nchannels; ++c) {
			if (c == alpha_channel)
				pixelcolor[c] = alpha + (1.0f - alpha) * pixelcolor[c];
			else 
				pixelcolor[c] = (val * alpha * textcolor[c]) + (1.0f - alpha) * pixelcolor[c];
		}

        R.setpixel(r.x(), r.y(), pixelcolor);
    }

    FT_Done_Face(face);
    return true;
}

}

OIIO_NAMESPACE_END
