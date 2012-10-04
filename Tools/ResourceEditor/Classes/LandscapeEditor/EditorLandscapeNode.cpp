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
#include "EditorLandscapeNode.h"
#include "LandscapeRenderer.h"
#include "EditorHeightmap.h"

using namespace DAVA;

EditorLandscapeNode::EditorLandscapeNode()
    : LandscapeNode()
{
    SetName(String("Landscape_EditorNode"));
    
    landscapeRenderer = NULL;
    nestedLandscape = NULL;
    parentLandscape = NULL;
}

EditorLandscapeNode::~EditorLandscapeNode()
{
    SafeRelease(landscapeRenderer);
    SafeRelease(nestedLandscape);
    SafeRelease(parentLandscape);
}

void EditorLandscapeNode::SetNestedLandscape(DAVA::LandscapeNode *landscapeNode)
{
    SafeRelease(nestedLandscape);
    nestedLandscape = SafeRetain(landscapeNode);
    
    EditorLandscapeNode *editorLandscape = dynamic_cast<EditorLandscapeNode *>(nestedLandscape);
    if(editorLandscape)
    {
        editorLandscape->SetParentLandscape(this);
    }
    
    SetDebugFlags(nestedLandscape->GetDebugFlags());
    
    SetHeightmap(nestedLandscape->GetHeightmap());
    heightmapPath = nestedLandscape->GetHeightmapPathname();

    SetTexture(TEXTURE_TILE_FULL, nestedLandscape->GetTexture(TEXTURE_TILE_FULL));
    
    box = nestedLandscape->GetBoundingBox();
    CopyCursorData(nestedLandscape, this);
    
    SetDisplayedTexture();
}


void EditorLandscapeNode::SetHeightmap(DAVA::Heightmap *height)
{
    SafeRelease(heightmap);
    heightmap = SafeRetain(height);
    
    EditorHeightmap *editorHeightmap = dynamic_cast<EditorHeightmap *>(height);
    if(editorHeightmap)
    {
        HeihghtmapUpdated(Rect(0, 0, (float32)editorHeightmap->Size() - 1.f, (float32)editorHeightmap->Size() - 1.f));
    }
}


void EditorLandscapeNode::SetRenderer(LandscapeRenderer *renderer)
{
    SafeRelease(landscapeRenderer);
    landscapeRenderer = SafeRetain(renderer);
}


void EditorLandscapeNode::Draw()
{
    if (!(flags & NODE_VISIBLE)) return;
    if(!landscapeRenderer) return;
    
    landscapeRenderer->BindMaterial(GetTexture(LandscapeNode::TEXTURE_TILE_FULL));
    
    landscapeRenderer->DrawLandscape();

    
#if defined(__DAVAENGINE_OPENGL__)
    if (debugFlags & DEBUG_DRAW_GRID)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        RenderManager::Instance()->SetColor(1.0f, 1.f, 1.f, 1.f);
        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
        RenderManager::Instance()->SetShader(0);
        RenderManager::Instance()->FlushState();

        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, landscapeRenderer->Indicies());

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
#endif //#if defined(__DAVAENGINE_OPENGL__)

    
	if(cursor)
	{
		RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
		eBlendMode src = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dst = RenderManager::Instance()->GetDestBlend();
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		RenderManager::Instance()->SetDepthFunc(CMP_LEQUAL);
		cursor->Prepare();
        
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, landscapeRenderer->Indicies());
        
		RenderManager::Instance()->SetDepthFunc(CMP_LESS);
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(src, dst);
	}

    landscapeRenderer->UnbindMaterial();
}
    

void EditorLandscapeNode::HeihghtmapUpdated(const DAVA::Rect &forRect)
{
    EditorLandscapeNode *editorLandscape = dynamic_cast<EditorLandscapeNode *>(nestedLandscape);
    if(editorLandscape)
    {
        editorLandscape->HeihghtmapUpdated(forRect);
    }
}

void EditorLandscapeNode::SetDisplayedTexture()
{
    
}

DAVA::LandscapeNode *EditorLandscapeNode::GetNestedLandscape()
{
    return nestedLandscape;
}


void EditorLandscapeNode::SetParentLandscape(EditorLandscapeNode *landscapeNode)
{
    SafeRelease(parentLandscape);
    parentLandscape = SafeRetain(landscapeNode);
}

EditorLandscapeNode *EditorLandscapeNode::GetParentLandscape()
{
    return parentLandscape;
}

LandscapeRenderer *EditorLandscapeNode::GetRenderer()
{
    return landscapeRenderer;
}

void EditorLandscapeNode::CopyCursorData(DAVA::LandscapeNode *sourceLandscape, DAVA::LandscapeNode *destinationLandscape)
{
    if(!sourceLandscape || !destinationLandscape)
        return;
    
    LandscapeCursor *sourceCursor = sourceLandscape->GetCursor();
    LandscapeCursor *destinationCursor = destinationLandscape->GetCursor();
    
    if(!sourceCursor && destinationCursor)
    {
        destinationLandscape->CursorDisable();
    }
    else if(sourceCursor)
    {
        if(!destinationCursor)
        {
            destinationLandscape->CursorEnable();
        }
        
        destinationLandscape->SetCursorTexture(sourceCursor->GetCursorTexture());
        destinationLandscape->SetBigTextureSize(sourceCursor->GetBigTextureSize());
		destinationLandscape->SetCursorPosition(sourceCursor->GetCursorPosition());
		destinationLandscape->SetCursorScale(sourceCursor->GetCursorScale());
    }
}

void EditorLandscapeNode::FlushChanges()
{
    if(parentLandscape)
    {
        parentLandscape->FlushChanges();
    }
    
    if(nestedLandscape)
    {
        CopyCursorData(this, nestedLandscape);
        nestedLandscape->SetDebugFlags(GetDebugFlags());
    }
}

void EditorLandscapeNode::DrawFullTiledTexture(DAVA::Texture *renderTarget, const DAVA::Rect &drawRect)
{
    Texture *fullTiledTexture = nestedLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_FULL);
    Sprite *background = Sprite::CreateFromTexture(fullTiledTexture, 0, 0, (float32)fullTiledTexture->GetWidth(), (float32)fullTiledTexture->GetHeight());
    background->SetPosition(0.f, 0.f);
    background->SetScaleSize((float32)renderTarget->GetWidth(), (float32)renderTarget->GetHeight());
    
    background->Draw();
}



