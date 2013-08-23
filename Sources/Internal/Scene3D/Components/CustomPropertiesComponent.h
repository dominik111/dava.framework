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


#ifndef __DAVAENGINE_CUSTOM_PROPERTIES_COMPONENT_H__
#define __DAVAENGINE_CUSTOM_PROPERTIES_COMPONENT_H__

#include "Entity/Component.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{
	class CustomPropertiesComponent : public Component
	{
	public:
		CustomPropertiesComponent();
		virtual ~CustomPropertiesComponent();
		
		IMPLEMENT_COMPONENT_TYPE(CUSTOM_PROPERTIES_COMPONENT);
				
		virtual Component * Clone(Entity * toEntity);
		virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
		virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
		
		KeyedArchive* GetArchive();
				
		//this method helps to load data for older scene file version
		void LoadFromArchive(const KeyedArchive& srcProperties, SceneFileV2 *sceneFile);
		
	public:
		
		INTROSPECTION_EXTEND(CustomPropertiesComponent, Component,
							 MEMBER(properties, "Custom properties", I_SAVE | I_VIEW | I_EDIT)
		);
	private:
		
		CustomPropertiesComponent(const KeyedArchive& srcProperties);
		
	private:
		
		KeyedArchive* properties;
	};
};

#endif /* defined(__DAVAENGINE_CUSTOM_PROPERTIES_COMPONENT_H__) */