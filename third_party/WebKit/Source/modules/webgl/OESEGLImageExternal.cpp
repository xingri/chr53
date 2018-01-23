/*
 * Copyright (C) 2016-2018 LG Electronics Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "modules/webgl/OESEGLImageExternal.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/html/HTMLVideoElement.h"
#include "modules/webgl/WebGLTexture.h"

namespace blink {

OESEGLImageExternal::OESEGLImageExternal(WebGLRenderingContextBase* context)
    : WebGLExtension(context)
{
    context->extensionsUtil()->ensureExtensionEnabled("GL_OES_EGL_image_external");
}

OESEGLImageExternal::~OESEGLImageExternal()
{
}

WebGLExtensionName OESEGLImageExternal::name() const
{
    return OESEGLImageExternalName;
}

OESEGLImageExternal* OESEGLImageExternal::create(WebGLRenderingContextBase* context)
{
    return new OESEGLImageExternal(context);
}

bool OESEGLImageExternal::supported(WebGLRenderingContextBase* context)
{
    return context->extensionsUtil()->supportsExtension("GL_OES_EGL_image_external") &&
           context->extensionsUtil()->supportsExtension("GL_CHROMIUM_video_texture");
}

const char* OESEGLImageExternal::extensionName()
{
    return "OES_EGL_image_external";
}

void OESEGLImageExternal::EGLImageTargetTexture2DOES(GLenum target, HTMLVideoElement* video, ExceptionState& exceptionState)
{
    WebGLExtensionScopedContext scoped(this);
    if (scoped.isLost())
        return;

    if (target != GL_TEXTURE_EXTERNAL_OES) {
        scoped.context()->synthesizeGLError(GL_INVALID_ENUM, "EGLImageTargetTexture2DOES", "invalid texture target");
        return;
    }
    if (!scoped.context()->validateHTMLVideoElement("EGLImageTargetTexture2DOES", video, exceptionState) ||
        !scoped.context()->validateTexFuncDimensions("EGLImageTargetTexture2DOES", WebGLRenderingContextBase::BindTexImage, target, 0, video->videoWidth(), video->videoHeight(), 1))
        return;

    WebGLTexture* texture = scoped.context()->validateTextureBinding("EGLImageTargetTexture2DOES", target);
    if (!texture)
        return;

    if (video->bindVideoTextureToPlatformTexture(scoped.context()->contextGL(), texture->object()))
        return;
}

} // namespace blink
