#ifndef ACTIONCOMPONENTEDITOR_H
#define ACTIONCOMPONENTEDITOR_H

#include "DAVAEngine.h"
#include "Scene3D/Components/ActionComponent.h"
#include <QDialog>
#include <QStyledItemDelegate>

namespace Ui {
class ActionComponentEditor;
}

class ActionComponentEditor;
class ActionItemEditDelegate : public QStyledItemDelegate
{
	Q_OBJECT
	
public:
	
	ActionItemEditDelegate(QObject *parent = 0);
	
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
						  const QModelIndex &index) const;
	
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
					  const QModelIndex &index) const;
	
	void updateEditorGeometry(QWidget *editor,
							  const QStyleOptionViewItem &option, const QModelIndex &index) const;
	
	void SetComponent(DAVA::ActionComponent* component);
	void SetComponentEditor(ActionComponentEditor* editor);
	
private:
	
	DAVA::ActionComponent* targetComponent;
	ActionComponentEditor* componentEditor;
};


class ActionComponentEditor : public QDialog
{
    Q_OBJECT
	
public:
    explicit ActionComponentEditor(QWidget *parent = 0);
    ~ActionComponentEditor();
	
	void SetComponent(DAVA::ActionComponent* component);
	void Update();
	
private:
	
	void UpdateTableFromComponent(DAVA::ActionComponent* component);
	DAVA::ActionComponent::Action GetDefaultAction();
	bool IsActionPresent(const DAVA::ActionComponent::Action action);
	
private slots:

	void OnAddAction();
	void OnRemoveAction();
	void OnSelectedItemChanged();
    
private:
    Ui::ActionComponentEditor *ui;
	
	DAVA::ActionComponent* targetComponent;
	ActionItemEditDelegate editDelegate;
};

#endif // ACTIONCOMPONENTEDITOR_H