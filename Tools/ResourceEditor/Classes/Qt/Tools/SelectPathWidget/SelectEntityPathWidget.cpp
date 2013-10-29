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

#include "SelectEntityPathWidget.h"
#include "./../Qt/Tools/MimeDataHelper/MimeDataHelper.h"
#include "Scene/SceneEditor2.h"
#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QFileDialog>
#include <QStyle>

SelectEntityPathWidget::SelectEntityPathWidget(QWidget* _parent, DAVA::String _openDialogDefualtPath, DAVA::String _relativPath)
:	SelectPathWidgetBase(_parent, _openDialogDefualtPath, _relativPath, "Open Scene File", "Scene File (*.sc2)")
{
	cornvertionFuncMap["application/dava.entity"] = &SelectEntityPathWidget::ConvertQMimeDataFromSceneTree;
	cornvertionFuncMap["application/dava.emitter"] = &SelectEntityPathWidget::ConvertQMimeDataFromSceneTree;
	cornvertionFuncMap["text/uri-list"] = &SelectEntityPathWidget::ConvertQMimeDataFromFilePath;
	
	allowedFormatsList.push_back(".sc2");
}

void SelectEntityPathWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if(!DAVA::MimeDataHelper::IsMimeDataTypeSupported(event->mimeData()))
	{
		return;
	}
	bool isFormatSupported = true;
	
	if(event->mimeData()->hasFormat("text/uri-list"))
	{
		isFormatSupported= false;
		DAVA::FilePath path(event->mimeData()->urls().first().toString().toStdString());
		Q_FOREACH(DAVA::String item, allowedFormatsList)
		{
			if(path.IsEqualToExtension(item))
			{
				isFormatSupported = true;
				break;
			}
		}
	}
	if(isFormatSupported)
	{
		event->acceptProposedAction();
	}
}


DAVA::Entity* SelectEntityPathWidget::GetOutputEntity(SceneEditor2* editor)
{
	DAVA::List<DAVA::Entity*> retList;
	ConvertFromMimeData(&mimeData, retList, editor);
	DAVA::Entity* retEntity = retList.size() > 0 ? *retList.begin(): NULL;
	return retEntity;
}

void SelectEntityPathWidget::ConvertFromMimeData(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>& retList, SceneEditor2* sceneEditor)
{
	DAVA::Map<DAVA::String, void (*) (const QMimeData* mimeData, DAVA::List<DAVA::Entity*> & nameList, SceneEditor2* sceneEditor)>::iterator it;
	
	for(it = cornvertionFuncMap.begin(); it != cornvertionFuncMap.end(); ++it)
	{
		if(mimeData->hasFormat(QString(it->first.c_str())))
		{
			it->second(mimeData, retList, sceneEditor);
		}
	}
}

void SelectEntityPathWidget::ConvertQMimeDataFromSceneTree(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>& retList, SceneEditor2* sceneEditor)
{
	retList = MimeDataHelper::GetPointersFromSceneTreeMime(mimeData);
}

void SelectEntityPathWidget::ConvertQMimeDataFromFilePath(const QMimeData* mimeData,DAVA::List<DAVA::Entity*>& retList,
												  SceneEditor2* sceneEditor)
{
	if(mimeData == NULL || sceneEditor == NULL || !mimeData->hasUrls())
	{
		return;
	}
	
	retList.clear();
	
	QList<QUrl> droppedUrls = mimeData->urls();
	
	Q_FOREACH(QUrl url, droppedUrls)
	{
		DAVA::FilePath filePath( url.toLocalFile().toStdString());
		if(!(filePath.Exists() && filePath.GetExtension() == ".sc2"))
		{
			continue;
		}
		
		DAVA::Entity * entity = sceneEditor->GetRootNode(filePath);
		if(NULL != entity)
		{
			retList.push_back(entity);
		}
	}
}