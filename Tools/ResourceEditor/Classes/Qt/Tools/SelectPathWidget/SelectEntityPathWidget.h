/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __RESOURCEEDITORQT__SELECENTITYTPATHWIDGET__
#define __RESOURCEEDITORQT__SELECENTITYTPATHWIDGET__

#include <QWidget>
#include <QMimeData>
#include <qlineedit.h>
#include <qtoolbutton.h>
#include "SelectPathWidgetBase.h"

#include "DAVAEngine.h"
class SceneEditor2;

class SelectEntityPathWidget: public SelectPathWidgetBase
{
	Q_OBJECT

public:
	explicit SelectEntityPathWidget( QWidget* parent = 0, DAVA::String openDialoDefualtPath = "", DAVA::String relativPath = "");

	~SelectEntityPathWidget();
	
	DAVA::Entity* GetOutputEntity(SceneEditor2*);

protected:

	void dragEnterEvent(QDragEnterEvent* event);
	
	void ConvertQMimeDataFromSceneTree(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>&);
	
	void ConvertQMimeDataFromFilePath(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>&,
											 SceneEditor2* sceneEditor = NULL);
	
	void ConvertFromMimeData(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>& retList, SceneEditor2* sceneEditor);
	
	void SetEntities(const DAVA::List<DAVA::Entity*>& list, bool perfromRertain);
	
	//DAVA::Map<DAVA::String, void (*) (const QMimeData* mimeData, DAVA::List<DAVA::Entity*> & nameList, SceneEditor2* sceneEditor)> cornvertionFuncMap;
	
	
	DAVA::List<DAVA::Entity*> entitiesToHold;
};

#endif /* defined(__RESOURCEEDITORQT__SELECENTITYTPATHWIDGET__) */