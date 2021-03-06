//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer9.cpp: Defines the D3D9 VertexBuffer implementation.

#include "libANGLE/renderer/d3d/d3d9/VertexBuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/d3d/d3d9/vertexconversion.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/Buffer.h"

namespace rx
{

VertexBuffer9::VertexBuffer9(Renderer9 *renderer) : mRenderer(renderer)
{
    mVertexBuffer = nullptr;
    mBufferSize = 0;
    mDynamicUsage = false;
}

VertexBuffer9::~VertexBuffer9()
{
    SafeRelease(mVertexBuffer);
}

gl::Error VertexBuffer9::initialize(const gl::Context *context,
                                    unsigned int size,
                                    bool dynamicUsage)
{
    SafeRelease(mVertexBuffer);

    updateSerial();

    if (size > 0)
    {
        DWORD flags = D3DUSAGE_WRITEONLY;
        if (dynamicUsage)
        {
            flags |= D3DUSAGE_DYNAMIC;
        }

        HRESULT result = mRenderer->createVertexBuffer(size, flags, &mVertexBuffer);

        if (FAILED(result))
        {
            return gl::OutOfMemory()
                   << "Failed to allocate internal vertex buffer of size " << size;
        }
    }

    mBufferSize = size;
    mDynamicUsage = dynamicUsage;
    return gl::NoError();
}

gl::Error VertexBuffer9::storeVertexAttributes(const gl::Context *context,
                                               const gl::VertexAttribute &attrib,
                                               const gl::VertexBinding &binding,
                                               GLenum currentValueType,
                                               GLint start,
                                               size_t count,
                                               GLsizei instances,
                                               unsigned int offset,
                                               const uint8_t *sourceData)
{
    if (!mVertexBuffer)
    {
        return gl::OutOfMemory() << "Internal vertex buffer is not initialized.";
    }

    size_t inputStride = gl::ComputeVertexAttributeStride(attrib, binding);
    size_t elementSize = gl::ComputeVertexAttributeTypeSize(attrib);

    DWORD lockFlags = mDynamicUsage ? D3DLOCK_NOOVERWRITE : 0;

    uint8_t *mapPtr = nullptr;

    unsigned int mapSize = 0;
    ANGLE_TRY(
        mRenderer->getVertexSpaceRequired(context, attrib, binding, count, instances, &mapSize));

    HRESULT result = mVertexBuffer->Lock(offset, mapSize, reinterpret_cast<void**>(&mapPtr), lockFlags);
    if (FAILED(result))
    {
        return gl::OutOfMemory() << "Failed to lock internal vertex buffer, " << gl::FmtHR(result);
    }

    const uint8_t *input = sourceData;

    if (instances == 0 || binding.getDivisor() == 0)
    {
        input += inputStride * start;
    }

    gl::VertexFormatType vertexFormatType = gl::GetVertexFormatType(attrib, currentValueType);
    const d3d9::VertexFormat &d3dVertexInfo = d3d9::GetVertexFormatInfo(mRenderer->getCapsDeclTypes(), vertexFormatType);
    bool needsConversion = (d3dVertexInfo.conversionType & VERTEX_CONVERT_CPU) > 0;

    if (!needsConversion && inputStride == elementSize)
    {
        size_t copySize = count * inputStride;
        memcpy(mapPtr, input, copySize);
    }
    else
    {
        d3dVertexInfo.copyFunction(input, inputStride, count, mapPtr);
    }

    mVertexBuffer->Unlock();

    return gl::NoError();
}

unsigned int VertexBuffer9::getBufferSize() const
{
    return mBufferSize;
}

gl::Error VertexBuffer9::setBufferSize(const gl::Context *context, unsigned int size)
{
    if (size > mBufferSize)
    {
        return initialize(context, size, mDynamicUsage);
    }
    else
    {
        return gl::NoError();
    }
}

gl::Error VertexBuffer9::discard(const gl::Context *context)
{
    if (!mVertexBuffer)
    {
        return gl::OutOfMemory() << "Internal vertex buffer is not initialized.";
    }

    void *dummy;
    HRESULT result;

    result = mVertexBuffer->Lock(0, 1, &dummy, D3DLOCK_DISCARD);
    if (FAILED(result))
    {
        return gl::OutOfMemory() << "Failed to lock internal buffer for discarding, "
                                 << gl::FmtHR(result);
    }

    result = mVertexBuffer->Unlock();
    if (FAILED(result))
    {
        return gl::OutOfMemory() << "Failed to unlock internal buffer for discarding, "
                                 << gl::FmtHR(result);
    }

    return gl::NoError();
}

IDirect3DVertexBuffer9 * VertexBuffer9::getBuffer() const
{
    return mVertexBuffer;
}
}  // namespace rx
