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



#include "SceneEditor/SceneEditorScreenMain.h"

#include "Main/mainwindow.h"
#include "Scene/SceneTabWidget.h"
#include "Scene/SceneEditor2.h"
#include "AppScreens.h"

#include "Classes/Deprecated/ScenePreviewDialog.h"
#include "Classes/Qt/Main/Request.h"

#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMessageBox>
#include <QFileInfo>

class QTransparentWidget: public QWidget
{
public:

	QTransparentWidget(QWidget *parent) : QWidget(parent)
	{
		setStyleSheet(QString::fromUtf8("background-color: rgba(175, 75, 75, 0);"));

//        setAttribute(Qt::WA_TranslucentBackground);
//        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	}
};

SceneTabWidget::SceneTabWidget(QWidget *parent)
	: QWidget(parent)
	, davaUIScreenID(SCREEN_MAIN)
	, dava3DViewMargin(3)
	, newSceneCounter(0)
	, curScene(NULL)
	, previewDialog(NULL)
	/*
	, curModifAxis(ST_AXIS_X)
	, curModifMode(ST_MODIF_MOVE)
	, curPivotPoint(ST_PIVOT_COMMON_CENTER)
	, curSelDrawMode(ST_SELDRAW_DRAW_CORNERS | ST_SELDRAW_FILL_SHAPE)
	, curColDrawMode(ST_COLL_DRAW_NOTHING)
	*/
{
	this->setMouseTracking(true);

	// create Qt controls and add them into layout
	// 
	// tab bar
	tabBar = new MainTabBar(this);
	tabBar->setTabsClosable(true);
	tabBar->setMovable(true);
	tabBar->setUsesScrollButtons(true);
	tabBar->setExpanding(false);
	tabBar->setMinimumHeight(tabBar->sizeHint().height());

	// davawidget to display DAVAEngine content
	davaWidget = new DavaGLWidget(this);
	davaWidget->setFocusPolicy(Qt::StrongFocus);
	davaWidget->installEventFilter(this);
    
	// put tab bar and davawidget into vertical layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(tabBar);
	layout->addWidget(davaWidget);
	layout->setMargin(0);
	layout->setSpacing(1);
	setLayout(layout);
	
	// create top widget for tool buttons
	topPlaceholder = new QTransparentWidget(this);

	topPlaceholderLayout = new QHBoxLayout(topPlaceholder);
	topPlaceholderLayout->setMargin(2);
	topPlaceholderLayout->setSpacing(1);
	topPlaceholderLayout->setAlignment(Qt::AlignLeft);
	topPlaceholder->setLayout(topPlaceholderLayout);

	setAcceptDrops(true);
    
	// create DAVA UI
	InitDAVAUI();

	QObject::connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(TabBarCurrentChanged(int)));
	QObject::connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(TabBarCloseRequest(int)));
	QObject::connect(tabBar, SIGNAL(OnDrop(const QMimeData *)), this, SLOT(TabBarDataDropped(const QMimeData *)));
	QObject::connect(davaWidget, SIGNAL(OnDrop(const QMimeData *)), this, SLOT(DAVAWidgetDataDropped(const QMimeData *)));

	QObject::connect(SceneSignals::Instance(), SIGNAL(MouseOverSelection(SceneEditor2*, const EntityGroup*)), this, SLOT(MouseOverSelectedEntities(SceneEditor2*, const EntityGroup*)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Saved(SceneEditor2*)), this, SLOT(SceneSaved(SceneEditor2*)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(ModifyStatusChanged(SceneEditor2 *, bool)), this, SLOT(SceneModifyStatusChanged(SceneEditor2 *, bool)));

	SetCurrentTab(0);
}

SceneTabWidget::~SceneTabWidget()
{
	davaWidget->removeEventFilter(this);
	SafeRelease(previewDialog);

	ReleaseDAVAUI();
}

void SceneTabWidget::InitDAVAUI()
{
	dava3DView = new DAVAUI3DView(this, DAVA::Rect(dava3DViewMargin, dava3DViewMargin, 0, 0));
	//dava3DView->SetDebugDraw(true);

	davaUIScreen = new DAVA::UIScreen();
	davaUIScreen->AddControl(dava3DView);
	davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	davaUIScreen->GetBackground()->SetColor(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f));

	UIScreenManager::Instance()->RegisterScreen(davaUIScreenID, davaUIScreen);
	UIScreenManager::Instance()->SetScreen(davaUIScreenID);
}

void SceneTabWidget::ReleaseDAVAUI()
{
	SafeRelease(davaUIScreen);
}

int SceneTabWidget::OpenTab()
{
	SceneEditor2 *scene = new SceneEditor2();

	DAVA::FilePath newScenePath = (QString("newscene") + QString::number(++newSceneCounter)).toStdString();
	newScenePath.ReplaceExtension(".sc2");

	scene->SetScenePath(newScenePath);

	int tabIndex = tabBar->addTab(newScenePath.GetFilename().c_str());
	SetTabScene(tabIndex, scene);

	SetCurrentTab(tabIndex);
	return tabIndex;
}

int SceneTabWidget::OpenTab(const DAVA::FilePath &scenePapth)
{
	HideScenePreview();

	int tabIndex = FindTab(scenePapth);
	if(tabIndex != -1)
	{
		SetCurrentTab(tabIndex);
		return tabIndex;
	}

	SceneEditor2 *scene = new SceneEditor2();

	QtMainWindow::Instance()->WaitStart("Opening scene...", scenePapth.GetAbsolutePathname().c_str());

	if(scene->Load(scenePapth))
	{
		tabIndex = tabBar->addTab(scenePapth.GetFilename().c_str());
		SetTabScene(tabIndex, scene);

		tabBar->setTabToolTip(tabIndex, scenePapth.GetAbsolutePathname().c_str());
	}
	else
	{
		// TODO:
		// message box about can't load scene
		// ...
		
        SafeRelease(scene);
	}

	if(tabBar->count() == 1)
	{
		SetCurrentTab(tabIndex);
	}

	QtMainWindow::Instance()->WaitStop();

	return tabIndex;
}

void SceneTabWidget::CloseTab(int index)
{
    Request request;
    
    emit CloseTabRequest(index, &request);
    
    if(!request.IsAccepted())
        return;
    
    
	SceneEditor2 *scene = GetTabScene(index);
    if(index == tabBar->currentIndex())
    {
        curScene = NULL;
        dava3DView->SetScene(NULL);
        SceneSignals::Instance()->EmitDeactivated(scene);
    }
    
    tabBar->removeTab(index);
    SafeRelease(scene);
}

int SceneTabWidget::GetCurrentTab() const
{
	return tabBar->currentIndex();
}

void SceneTabWidget::SetCurrentTab(int index)
{
	davaWidget->setEnabled(false);
 	topPlaceholder->setVisible(false);

	if(index >= 0 && index < tabBar->count())
	{
		SceneEditor2 *oldScene = curScene;
		curScene = GetTabScene(index);

		if(NULL != oldScene)
		{
			oldScene->selectionSystem->LockSelection(true);
			SceneSignals::Instance()->EmitDeactivated(oldScene);
		}

		tabBar->setCurrentIndex(index);

		if(NULL != curScene)
		{
			dava3DView->SetScene(curScene);
			curScene->SetViewportRect(dava3DView->GetRect());

			SceneSignals::Instance()->EmitActivated(curScene);
			curScene->selectionSystem->LockSelection(false);

			davaWidget->setEnabled(true);
//			topPlaceholder->setVisible(true); //VK: disabled for future.
		}
	}
}

SceneEditor2* SceneTabWidget::GetTabScene(int index) const
{
	SceneEditor2 *ret = NULL;

	if(index >= 0 && index < tabBar->count())
	{
		ret = tabBar->tabData(index).value<SceneEditor2 *>();
	}

	return ret;
}

void SceneTabWidget::SetTabScene(int index, SceneEditor2* scene)
{
	if(index >= 0 && index < tabBar->count())
	{
		tabBar->setTabData(index, qVariantFromValue(scene));
	}
}

int SceneTabWidget::GetTabCount() const
{
	return tabBar->count();
}

void SceneTabWidget::ProcessDAVAUIEvent(DAVA::UIEvent *event)
{
	SceneEditor2* scene = GetTabScene(tabBar->currentIndex());
	if(NULL != scene)
	{
		scene->PostUIEvent(event);
	}
}

void SceneTabWidget::TabBarCurrentChanged(int index)
{
	SetCurrentTab(index);
}

void SceneTabWidget::TabBarCloseRequest(int index)
{
	CloseTab(index);
}

void SceneTabWidget::TabBarDataDropped(const QMimeData *data)
{
	if(data->hasUrls())
	{
		QList<QUrl> urls = data->urls();
		for(int i = 0; i < urls.size(); ++i)
		{
            QtMainWindow::Instance()->OpenScene(urls[i].toLocalFile());
		}
	}
}

void SceneTabWidget::DAVAWidgetDataDropped(const QMimeData *data)
{
	if(NULL != curScene)
	{
		if(data->hasUrls())
		{
			DAVA::Vector3 pos;

			if(!curScene->collisionSystem->LandRayTestFromCamera(pos))
			{
				DAVA::Landscape *landscape = curScene->collisionSystem->GetLandscape();
				if( NULL != landscape && 
					NULL != landscape->GetHeightmap() &&
					landscape->GetHeightmap()->Size() > 0)
				{
					curScene->collisionSystem->GetLandscape()->PlacePoint(DAVA::Vector3(), pos);
				}
			}

			QString sc2path = data->urls().at(0).toLocalFile();
			
			QtMainWindow::Instance()->WaitStart("Adding object to scene", sc2path);
			curScene->structureSystem->Add(sc2path.toStdString(), pos);
			QtMainWindow::Instance()->WaitStop();
		}
	}
	else
	{
		TabBarDataDropped(data);
	}
}

void SceneTabWidget::MouseOverSelectedEntities(SceneEditor2* scene, const EntityGroup *entities)
{
	if(GetCurrentScene() == scene && NULL != entities)
	{
		switch(scene->modifSystem->GetModifMode())
		{
		case ST_MODIF_MOVE:
			setCursor(Qt::SizeAllCursor);
			break;
		case ST_MODIF_ROTATE:
			break;
		case ST_MODIF_SCALE:
			break;
		case ST_MODIF_OFF:
		default:
			setCursor(Qt::ArrowCursor);
			break;
		}
	}
	else
	{
		setCursor(Qt::ArrowCursor);
	}
}

void SceneTabWidget::SceneSaved(SceneEditor2 *scene)
{
	// update scene name on tabBar
	for(int i = 0; i < tabBar->count(); ++i)
	{
		SceneEditor2 *tabScene = GetTabScene(i);
		if(tabScene == scene)
		{
			UpdateTabName(i);
			break;
		}
	}

}

void SceneTabWidget::SceneModifyStatusChanged(SceneEditor2 *scene, bool modified)
{
	// update scene name on tabBar
	for(int i = 0; i < tabBar->count(); ++i)
	{
		SceneEditor2 *tabScene = GetTabScene(i);
		if(tabScene == scene)
		{
			UpdateTabName(i);
			break;
		}
	}
}

bool SceneTabWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == davaWidget && event->type() == QEvent::Resize)
	{
		QSize s = davaWidget->size();

		davaUIScreen->SetSize(DAVA::Vector2(s.width(), s.height()));
		dava3DView->SetSize(DAVA::Vector2(s.width() - 2 * dava3DViewMargin, s.height() - 2 * dava3DViewMargin));

		SceneEditor2* scene = GetTabScene(tabBar->currentIndex());
		if(NULL != scene)
		{
			scene->SetViewportRect(dava3DView->GetRect());
		}

		topPlaceholder->setGeometry(0, 25, width(), 20);
	}

	return QWidget::eventFilter(object, event);
}

void SceneTabWidget::dragEnterEvent(QDragEnterEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	if(mimeData->hasUrls()) 
	{
		event->acceptProposedAction();
	}
	else
	{
		event->setDropAction(Qt::IgnoreAction);
		event->accept();
	}
}

void SceneTabWidget::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	if(mimeData->hasUrls())
	{
		TabBarDataDropped(mimeData);
	}
}


void SceneTabWidget::UpdateTabName(int index)
{
	SceneEditor2 *scene = GetTabScene(index);
	if(NULL != scene)
	{
		DAVA::String tabName = scene->GetScenePath().GetFilename();
		DAVA::String tabTooltip = scene->GetScenePath().GetAbsolutePathname();

		if(scene->IsChanged())
		{
			tabName += "*";
		}

		tabBar->setTabText(index, tabName.c_str());
		tabBar->setTabToolTip(index, tabTooltip.c_str());
	}
}

SceneEditor2* SceneTabWidget::GetCurrentScene() const
{
	return curScene;
}

void SceneTabWidget::ShowScenePreview(const DAVA::FilePath &scenePath)
{
	if(!previewDialog)
    {
        previewDialog = new ScenePreviewDialog();
    }

	if(scenePath.IsEqualToExtension(".sc2"))
	{
		previewDialog->Show(scenePath);
	}
	else
	{
		previewDialog->Close();
	}
}

void SceneTabWidget::HideScenePreview()
{
	if(previewDialog && previewDialog->GetParent())
	{
		previewDialog->Close();
	}
}

void SceneTabWidget::AddTopToolWidget(QWidget *widget)
{
    if(widget)
    {
        widget->setParent(topPlaceholder);
		topPlaceholderLayout->addWidget(widget);
    }
}

DavaGLWidget * SceneTabWidget::GetDavaWidget() const
{
	return davaWidget;
}

int SceneTabWidget::FindTab( const DAVA::FilePath & scenePath )
{
	for(int i = 0; i < tabBar->count(); ++i)
	{
		SceneEditor2 *tabScene = GetTabScene(i);
		if(tabScene->GetScenePath() == scenePath)
		{
			return i;
		}
	}

	return -1;
}


MainTabBar::MainTabBar(QWidget* parent /* = 0 */)
	: QTabBar(parent)
{
	setAcceptDrops(true);
}

void MainTabBar::dragEnterEvent(QDragEnterEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	if(mimeData->hasUrls()) 
	{
		event->acceptProposedAction();
	}
	else
	{
		event->setDropAction(Qt::IgnoreAction);
		event->accept();
	}
}

void MainTabBar::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	if(mimeData->hasUrls())
	{
		emit OnDrop(mimeData);
	}
}


