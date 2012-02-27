/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/Material.h"
#include "Render/UberShader.h"
#include "Render/Texture.h"
#include "FileSystem/KeyedArchive.h"
#include "Utils/StringFormat.h"
#include "Render/Shader.h"
#include "Render/3D/PolygonGroup.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/LightNode.h"

namespace DAVA 
{
    
    
InstanceMaterialState::InstanceMaterialState()
{
    for (int32 k = 0; k < LIGHT_NODE_MAX_COUNT; ++k)
        lightNodes[k] = 0;
}

InstanceMaterialState::~InstanceMaterialState()
{
    for (int32 k = 0; k < LIGHT_NODE_MAX_COUNT; ++k)
        SafeRelease(lightNodes[k]);
}

void InstanceMaterialState::SetLight(int32 lightIndex, LightNode * lightNode)
{ 
    SafeRelease(lightNodes[lightIndex]);
    lightNodes[lightIndex] = SafeRetain(lightNode); 
}

LightNode * InstanceMaterialState::GetLight(int32 lightIndex) 
{ 
    return lightNodes[lightIndex]; 
}

    
REGISTER_CLASS(Material);
    
UberShader * Material::uberShader = 0;

Material::Material(Scene * _scene) 
    :   DataNode(_scene)
    ,   diffuseColor(1.0f, 1.0f, 1.0f, 1.0f)
    ,   specularColor(1.0f, 1.0f, 1.0f, 1.0f)
    ,   ambientColor(1.0f, 1.0f, 1.0f, 1.0f)
    ,   emissiveColor(1.0f, 1.0f, 1.0f, 1.0f)
    ,   shininess(1.0f)
    ,   isOpaque(false)
    ,   isTwoSided(false)
{
//    if (scene)
//    {
//        DataNode * materialsNode = scene->GetMaterials();
//        materialsNode->AddNode(this);
//    }
    
    if (!uberShader)
    {
        uberShader = new UberShader();
        uberShader->LoadShader("~res:/Shaders/Default/materials.shader");
        uberShader->CompileShaderCombination("MATERIAL_TEXTURE");
        uberShader->CompileShaderCombination("MATERIAL_DECAL");
        uberShader->CompileShaderCombination("MATERIAL_DETAIL");
        
        uberShader->CompileShaderCombination("MATERIAL_TEXTURE;VERTEX_LIT");
        uberShader->CompileShaderCombination("MATERIAL_DECAL;VERTEX_LIT");
        uberShader->CompileShaderCombination("MATERIAL_DETAIL;VERTEX_LIT");
    }
    
//    type = MATERIAL_UNLIT_TEXTURE;
//    shader = uberShader->GetShader("MATERIAL_TEXTURE");
    for (int32 tc = 0; tc < TEXTURE_COUNT; ++tc)
        textures[tc] = 0;

    SetType(MATERIAL_UNLIT_TEXTURE);
}
    
void Material::SetScene(Scene * _scene)
{
    DVASSERT(scene == 0);
    scene = _scene;
}


int32 Material::Release()
{
    int32 retainCount = BaseObject::Release();
    return retainCount;
}

Material::~Material()
{
}
    
void Material::RebuildShader()
{
    uniformTexture0 = -1;
    uniformTexture1 = -1;
    uniformLightPosition0 = -1;
    uniformMaterialDiffuseColor = -1;
    uniformMaterialSpecularColor = -1;
    
    String shaderCombileCombo = "MATERIAL_TEXTURE";
    
    switch (type) 
    {
        case MATERIAL_UNLIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE";
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
        case MATERIAL_UNLIT_TEXTURE_DECAL:
            shaderCombileCombo = "MATERIAL_DECAL";
            break;
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            shaderCombileCombo = "MATERIAL_DETAIL";
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE;VERTEX_LIT";
            break;
            
        default:
            break;
    };
    if (isOpaque)
    {
        shaderCombileCombo = shaderCombileCombo + ";OPAQUE";
    }
    

    
    // Get shader if combo unavailable compile it
    shader = uberShader->GetShader(shaderCombileCombo);
    
    switch (type) {
        case MATERIAL_UNLIT_TEXTURE:
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
        case MATERIAL_UNLIT_TEXTURE_DECAL:
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            //shader->SetT
            uniformTexture0 = shader->FindUniformLocationByName("texture0");
            uniformTexture1 = shader->FindUniformLocationByName("texture1");
            
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            //
            uniformLightPosition0 = shader->FindUniformLocationByName("lightPosition0");
            uniformMaterialDiffuseColor = shader->FindUniformLocationByName("materialDiffuseColor");
            uniformMaterialSpecularColor = shader->FindUniformLocationByName("materialSpecularColor");
            break;
        default:
            break;
    };
}
    
void Material::SetType(eType _type)
{
    type = _type;
    RebuildShader();
}
    
void Material::Save(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile)
{
    DataNode::Save(keyedArchive, sceneFile);
    
    keyedArchive->SetInt32("mat.texCount", TEXTURE_COUNT);
    for (int k = 0; k < TEXTURE_COUNT; ++k)
    {
        if (names[k].length() > 0)
        {
            String filename = sceneFile->AbsoluteToRelative(names[k]);
            keyedArchive->SetString(Format("mat.tex%d", k), filename);
            Logger::Debug("--- save material texture: %s", filename.c_str());
        }
    }
    
    keyedArchive->SetByteArrayAsType("mat.diffuse", diffuseColor);
    keyedArchive->SetByteArrayAsType("mat.ambient", ambientColor);
    keyedArchive->SetByteArrayAsType("mat.specular", specularColor);
    keyedArchive->SetByteArrayAsType("mat.emission", emissiveColor);
    keyedArchive->SetFloat("mat.shininess", shininess);
    
    keyedArchive->SetBool("mat.isOpaque", isOpaque);
    keyedArchive->SetBool("mat.isTwoSided", isTwoSided);
    
    keyedArchive->SetInt32("mat.type", type);
}
    
void Material::Load(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile)
{
    DataNode::Load(keyedArchive, sceneFile);

    int texCount = keyedArchive->GetInt32("mat.texCount");
    for (int k = 0; k < texCount; ++k)
    {
        String relativePathname = keyedArchive->GetString(Format("mat.tex%d", k));
        if (relativePathname.length() > 0)
        {
			String absolutePathname = relativePathname;
			if(!absolutePathname.empty() && absolutePathname[0] != '~') //not path like ~res:/Gfx...
			{
				absolutePathname = sceneFile->RelativeToAbsolute(relativePathname);
			}

            names[k] = absolutePathname;
            Logger::Debug("--- load material texture: %s abs:%s", relativePathname.c_str(), names[k].c_str());
            
            Texture::EnableMipmapGeneration();
            textures[k] = Texture::CreateFromFile(names[k]);
            if (textures[k])
            {
                textures[k]->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);
            }
            Texture::DisableMipmapGeneration();
        }
        
//        if (names[k].size())
//        {
//            Logger::Debug("- texture: %s index:%d", names[k].c_str(), index);
//        } 
    }
    
    diffuseColor = keyedArchive->GetByteArrayAsType("mat.diffuse", diffuseColor);
    ambientColor = keyedArchive->GetByteArrayAsType("mat.ambient", ambientColor);
    specularColor = keyedArchive->GetByteArrayAsType("mat.specular", specularColor);
    emissiveColor = keyedArchive->GetByteArrayAsType("mat.emission", emissiveColor);
    shininess = keyedArchive->GetFloat("mat.shininess", shininess);
    
    isOpaque = keyedArchive->GetBool("mat.isOpaque", isOpaque);
    isTwoSided = keyedArchive->GetBool("mat.isTwoSided", isTwoSided);

    eType mtype = (eType)keyedArchive->GetInt32("mat.type", type);
    SetType(mtype);
}

void Material::SetOpaque(bool _isOpaque)
{
    isOpaque = _isOpaque;
    RebuildShader();
}

bool Material::GetOpaque()
{
    return isOpaque;
}
void Material::SetTwoSided(bool _isTwoSided)
{
    isTwoSided = _isTwoSided;
}
    
bool Material::GetTwoSided()
{
    return isTwoSided;
}
    
void Material::SetAmbientColor(const Color & color)
{
    ambientColor = color;
}
void Material::SetDiffuseColor(const Color & color)
{
    diffuseColor = color;
}
void Material::SetSpecularColor(const Color & color)
{
    specularColor = color;
}
void Material::SetEmissiveColor(const Color & color)
{
    emissiveColor = color;
}

const Color & Material::GetAmbientColor() const
{
    return ambientColor;
}
const Color & Material::GetDiffuseColor() const
{
    return diffuseColor;
}
const Color & Material::GetSpecularColor() const
{
    return specularColor;
}
const Color & Material::GetEmissiveColor() const
{
    return emissiveColor;
}
    
void Material::SetShininess(float32 _shininess)
{
    shininess = _shininess;
}

float32 Material::GetShininess() const
{
    return shininess;
}

void Material::Draw(PolygonGroup * group, InstanceMaterialState * instanceMaterialState)
{
	RenderManager::Instance()->SetRenderData(group->renderDataObject);

    RenderManager::Instance()->SetShader(shader);
        
    if (textures[Material::TEXTURE_DIFFUSE])
    {
        RenderManager::Instance()->SetTexture(textures[Material::TEXTURE_DIFFUSE], 0);
    }
        
    if (textures[Material::TEXTURE_DECAL])
    {
        RenderManager::Instance()->SetTexture(textures[Material::TEXTURE_DECAL], 1);
    }
        
    if (isOpaque || isTwoSided)
    {
        RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE & (~RenderStateBlock::STATE_CULL));
    }else
    {
        RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
    }
    
    // render
    RenderManager::Instance()->FlushState();
    
    
    if (textures[Material::TEXTURE_DECAL])
    {
        shader->SetUniformValue(uniformTexture0, 0);
        shader->SetUniformValue(uniformTexture1, 1);
        
    }
    

    if (instanceMaterialState)
    {
        Camera * camera = scene->GetCurrentCamera();
        LightNode * lightNode0 = instanceMaterialState->GetLight(0);
        if (lightNode0 && camera)
        {
            if (uniformLightPosition0 != -1)
            {
                const Matrix4 & matrix = camera->GetMatrix();
                Vector3 lightPosition0InCameraSpace = lightNode0->GetPosition() * matrix;
                
                shader->SetUniformValue(uniformLightPosition0, lightPosition0InCameraSpace); 
            }
            if (uniformMaterialDiffuseColor != -1)
            {
                shader->SetUniformValue(uniformMaterialDiffuseColor, lightNode0->GetDiffuseColor() * GetDiffuseColor());
            }
            if (uniformMaterialSpecularColor != -1)
            {
                shader->SetUniformValue(uniformMaterialSpecularColor, lightNode0->GetSpecularColor() * GetSpecularColor());
            }
        }
    }
    
    // TODO: rethink this code
    if (group->renderDataObject->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, 0);
	}else
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, group->indexArray);
	}
    
	RenderManager::Instance()->SetTexture(0, 1); 
	RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);

}



};