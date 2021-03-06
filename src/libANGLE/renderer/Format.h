//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Format:
//   A universal description of typed GPU storage. Across multiple
//   renderer back-ends, there are common formats and some distinct
//   permutations, this enum encapsulates them all. Formats apply to
//   textures, but could also apply to any typed data.

#ifndef LIBANGLE_RENDERER_FORMAT_H_
#define LIBANGLE_RENDERER_FORMAT_H_

#include "libANGLE/renderer/renderer_utils.h"

namespace angle
{
enum class FormatID;

struct Format final : private angle::NonCopyable
{
    constexpr Format(FormatID id,
                     GLenum glFormat,
                     GLenum fboFormat,
                     rx::MipGenerationFunction mipGen,
                     const rx::FastCopyFunctionMap &fastCopyFunctions,
                     rx::PixelReadFunction colorRead,
                     rx::PixelWriteFunction colorWrite,
                     GLenum componentType,
                     GLuint redBits,
                     GLuint greenBits,
                     GLuint blueBits,
                     GLuint alphaBits,
                     GLuint depthBits,
                     GLuint stencilBits,
                     GLuint pixelBytes,
                     bool isBlock);

    static const Format &Get(FormatID id);
    static FormatID InternalFormatToID(GLenum internalFormat);

    constexpr bool hasDepthOrStencilBits() const;

    bool operator==(const Format &other) const { return this->id == other.id; }

    FormatID id;

    // The closest matching GL internal format for the storage this format uses. Note that this
    // may be a different internal format than the one this ANGLE format is used for.
    GLenum glInternalFormat;

    // The format we should report to the GL layer when querying implementation formats from a FBO.
    // This might not be the same as the glInternalFormat, since some DXGI formats don't have
    // matching GL format enums, like BGRA4, BGR5A1 and B5G6R6.
    GLenum fboImplementationInternalFormat;

    rx::MipGenerationFunction mipGenerationFunction;
    rx::PixelReadFunction pixelReadFunction;
    rx::PixelWriteFunction pixelWriteFunction;

    // A map from a gl::FormatType to a fast pixel copy function for this format.
    const rx::FastCopyFunctionMap &fastCopyFunctions;

    GLenum componentType;

    GLuint redBits;
    GLuint greenBits;
    GLuint blueBits;
    GLuint alphaBits;
    GLuint depthBits;
    GLuint stencilBits;

    GLuint pixelBytes;

    bool isBlock;
};

constexpr Format::Format(FormatID id,
                         GLenum glFormat,
                         GLenum fboFormat,
                         rx::MipGenerationFunction mipGen,
                         const rx::FastCopyFunctionMap &fastCopyFunctions,
                         rx::PixelReadFunction colorRead,
                         rx::PixelWriteFunction colorWrite,
                         GLenum componentType,
                         GLuint redBits,
                         GLuint greenBits,
                         GLuint blueBits,
                         GLuint alphaBits,
                         GLuint depthBits,
                         GLuint stencilBits,
                         GLuint pixelBytes,
                         bool isBlock)
    : id(id),
      glInternalFormat(glFormat),
      fboImplementationInternalFormat(fboFormat),
      mipGenerationFunction(mipGen),
      pixelReadFunction(colorRead),
      pixelWriteFunction(colorWrite),
      fastCopyFunctions(fastCopyFunctions),
      componentType(componentType),
      redBits(redBits),
      greenBits(greenBits),
      blueBits(blueBits),
      alphaBits(alphaBits),
      depthBits(depthBits),
      stencilBits(stencilBits),
      pixelBytes(pixelBytes),
      isBlock(isBlock)
{
}

constexpr bool Format::hasDepthOrStencilBits() const
{
    return depthBits > 0 || stencilBits > 0;
}
}  // namespace angle

#include "libANGLE/renderer/FormatID_autogen.inc"

#endif  // LIBANGLE_RENDERER_FORMAT_H_
