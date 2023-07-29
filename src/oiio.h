// Copyright 2008-present Contributors to the OpenImageIO project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/OpenImageIO/oiio/blob/master/LICENSE.md


// clang-format off

#pragma once
#define OPENIMAGEIO_OIIO_H

#if defined(_MSC_VER)
// Ignore warnings about DLL exported classes with member variables that are template classes.
// This happens with the std::vector<T> members of PixelStats below.
#  pragma warning (disable : 4251)
#endif

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/fmath.h>
#include <OpenImageIO/parallel.h>
#include <OpenImageIO/span.h>

#include <limits>


OIIO_NAMESPACE_BEGIN

namespace ImageBufAlgo {


/// Render a text string (encoded as UTF-8) into image `dst`. If the `dst`
/// image is not yet initialized, it will be initialized to be a black
/// background exactly large enough to contain the rasterized text.  If
/// `dst` is already initialized, the text will be rendered into the
/// existing image by essentially doing an "over" of the character into the
/// existing pixel data.
///
/// @param dst
///             Destination ImageBuf -- text is rendered into this image.
/// @param x/y
///             The position to place the text.
/// @param text
///             The text to draw.
/// @param fontsize/fontname
///             Size and name of the font. If the name is not a full
///             pathname to a font file, it will search for a matching font,
///             defaulting to some reasonable system font if not supplied at
///             all), and with a nominal height of fontsize (in pixels).
/// @param textcolor
///             Color for drawing the text, defaulting to opaque white
///             (1.0,1.0,...) in all channels if not supplied. If provided,
///             it is expected to point to a float array of length at least
///             equal to `R.spec().nchannels`, or defaults will be chosen
///             for you).
/// @param alignx/aligny
///             The default behavior is to align the left edge of the
///             character baseline to (`x`,`y`). Optionally, `alignx` and
///             `aligny` can override the alignment behavior, with
///             horizontal alignment choices of TextAlignX::Left, Right, and
///             Center, and vertical alignment choices of Baseline, Top,
///             Bottom, or Center.
/// @param shadow
///             If nonzero, a "drop shadow" of this radius will be used to
///             make the text look more clear by dilating the alpha channel
///             of the composite (makes a black halo around the characters).
///
bool OIIO_API render_text_shadow(ImageBuf &dst, int x, int y, string_view text,
                           int fontsize=16, string_view fontname="",
                           cspan<float> textcolor = 1.0f,
                           TextAlignX alignx = TextAlignX::Left,
                           TextAlignY aligny = TextAlignY::Baseline,
						   int shadow = 0, 
						   ROI roi={}, int nthreads=0);

}  // end namespace ImageBufAlgo

OIIO_NAMESPACE_END

