/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#ifndef __DAVAENGINE_QT_LAYER_H__
#define __DAVAENGINE_QT_LAYER_H__

#include "DAVAEngine.h"

namespace DAVA 
{
class QtLayerDelegate
{
public:
    
    virtual void Quit() = 0;
    
};
    
class QtLayer: public Singleton<QtLayer>
{
public:

    QtLayer();
    virtual ~QtLayer() {};
    
    virtual void WidgetCreated() = 0;
    virtual void WidgetDestroyed() = 0;

    virtual void OnSuspend() = 0;
    virtual void OnResume() = 0;
	
    virtual void AppStarted() = 0;
    virtual void AppFinished() = 0;

	virtual void Resize(int32 width, int32 height) = 0;
	virtual void Move(int32 x, int32 y) = 0;
    
    virtual void ProcessFrame() = 0;

    virtual void LockKeyboardInput(bool locked) = 0;

    virtual void* CreateAutoreleasePool() { return NULL; };
    virtual void ReleaseAutoreleasePool(void *pool);

    void Quit();
    void SetDelegate(QtLayerDelegate *delegate);

	virtual void* GetOpenGLView() { return NULL; };
protected:
    
    QtLayerDelegate *delegate;
};	
};


#endif // __DAVAENGINE_QT_LAYER_H__
