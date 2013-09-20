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



#include "SelectPathWidgetBase.h"
#include "./../Qt/Tools/MimeDataHelper/MimeDataHelper.h"


#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QFileDialog>
#include <QStyle>

SelectPathWidgetBase::SelectPathWidgetBase(QWidget* _parent, DAVA::String _openDialogDefualtPath, DAVA::String _relativPath, DAVA::String _openFileDialogTitle, DAVA::String _fileFormatDescriotion)
:	QLineEdit(_parent)
{
	Init(_openDialogDefualtPath, _relativPath, _openFileDialogTitle, _fileFormatDescriotion);
}

SelectPathWidgetBase::~SelectPathWidgetBase()
{
	delete openButton;
	delete clearButton;
}

void SelectPathWidgetBase::Init(DAVA::String& _openDialogDefualtPath, DAVA::String& _relativPath,DAVA::String _openFileDialogTitle, DAVA::String _fileFormatFilter)
{
	setAcceptDrops(true);
	
	relativePath = DAVA::FilePath(_relativPath);
	openDialogDefaultPath = _openDialogDefualtPath;
	openFileDialogTitle = _openFileDialogTitle;
	fileFormatFilter = _fileFormatFilter;
	
	clearButton = CreateToolButton(":/QtIcons/ccancel.png");
	openButton = CreateToolButton(":/QtIcons/openscene.png");
		
	connect(clearButton, SIGNAL(clicked()), this, SLOT(EraseClicked()));
	connect(openButton, SIGNAL(clicked()), this, SLOT(OpenClicked()));
	connect(this, SIGNAL(editingFinished()), this, SLOT(acceptEditing()));
}

void SelectPathWidgetBase::resizeEvent(QResizeEvent *)
{
	QSize sz = clearButton->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	clearButton->move(rect().right() - frameWidth - sz.width(),(rect().bottom() + 1 - sz.height())/2);
	
	QSize szOpenBtn = openButton->sizeHint();
	openButton->move(rect().right() - sz.width() - frameWidth - szOpenBtn.width(),(rect().bottom() + 1 - szOpenBtn.height())/2);
}

QToolButton* SelectPathWidgetBase::CreateToolButton(const DAVA::String& iconPath)
{
	QToolButton* retButton;
	
	retButton = new QToolButton(this);
	QIcon icon(iconPath.c_str());
	retButton->setIcon(icon);
	retButton->setCursor(Qt::ArrowCursor);
	retButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	QSize msz = minimumSizeHint();
	setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(retButton->sizeHint().width() + frameWidth + msz.width()));
	setMinimumSize(qMax(msz.width(), retButton->sizeHint().height() + frameWidth * 2 + 2),
				   qMax(msz.height(), retButton->sizeHint().height() + frameWidth * 2 + 2));
	
	return retButton;
}

void SelectPathWidgetBase::EraseWidget()
{
	setText(QString(""));
}

void SelectPathWidgetBase::EraseClicked()
{
	EraseWidget();
}

void SelectPathWidgetBase::acceptEditing()
{
	this->setText(getText());
}

void SelectPathWidgetBase::OpenClicked()
{
	DAVA::FilePath presentPath(text().toStdString());
	DAVA::FilePath dialogString(openDialogDefaultPath);
	if(presentPath.GetDirectory().Exists())//check if file text box clean
	{
		dialogString = presentPath.GetDirectory();
	}
	
	DAVA::String retString = QFileDialog::getOpenFileName(this, openFileDialogTitle.c_str(),
                                                    QString(dialogString.GetAbsolutePathname().c_str()),
                                                    fileFormatFilter.c_str()).toStdString();

	if(!retString.empty())
	{
		HandlePathSelected(retString);
	}
}

void SelectPathWidgetBase::HandlePathSelected(DAVA::String name)
{
	DAVA::FilePath fullPath(name);
	
	DVASSERT(fullPath.Exists());
	
	setText(name);
}

void SelectPathWidgetBase::setText(const QString& filePath)
{
	QLineEdit::setText(filePath);
	emit PathSelected(filePath.toStdString());
}

void SelectPathWidgetBase::setText(const DAVA::String &filePath)
{
	setText(QString(filePath.c_str()));
}

DAVA::String SelectPathWidgetBase::getText()
{
	return text().toStdString();
}

void SelectPathWidgetBase::SetRelativePath(const DAVA::String& newRelativPath)
{
	relativePath = DAVA::FilePath(newRelativPath);
	DAVA::String existingPath = text().toStdString();
	if(!existingPath.empty())
	{
		setText(QString(ConvertToRelativPath(existingPath).c_str()));
	}
}

DAVA::String SelectPathWidgetBase::ConvertToRelativPath(const DAVA::String& path)
{
	DAVA::FilePath fullPath(path);
	if(fullPath.Exists())
	{
		return fullPath.GetRelativePathname(relativePath);
	}
	else
	{
		return path;
	}
}
