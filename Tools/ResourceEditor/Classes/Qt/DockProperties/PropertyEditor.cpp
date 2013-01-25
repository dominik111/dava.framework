#include "DAVAEngine.h"
#include "Scene/SceneDataManager.h"
#include "Entity/Component.h"

#include "DockProperties/PropertyEditor.h"
#include "QtPropertyEditor/QtPropertyItem.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

PropertyEditor::PropertyEditor(QWidget *parent /* = 0 */)
	: QtPropertyEditor(parent)
	, curNode(NULL)
{
	// global scene manager signals
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData *)), this, SLOT(sceneActivated(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneChanged(SceneData *)), this, SLOT(sceneChanged(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData *)), this, SLOT(sceneReleased(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneNodeSelected(SceneData *, DAVA::SceneNode *)), this, SLOT(sceneNodeSelected(SceneData *, DAVA::SceneNode *)));
}

PropertyEditor::~PropertyEditor()
{
	SafeRelease(curNode);
}

void PropertyEditor::SetNode(DAVA::SceneNode *node)
{
	static bool sss = false;

	SafeRelease(curNode);
	curNode = SafeRetain(node);

	RemovePropertyAll();
	if(NULL != curNode)
	{
		if(!sss)
		{
			curNode->GetCustomProperties()->SetBool("111", true);
			curNode->GetCustomProperties()->SetArchive("subArchive", DAVA::Core::Instance()->GetOptions());
			sss = true;
		}

        AppendIntrospectionInfo(curNode, curNode->GetTypeInfo());
        
        for(int32 i = 0; i < Component::COMPONENT_COUNT; ++i)
        {
            Component *component = curNode->GetComponent(i);
            if(component)
            {
                AppendIntrospectionInfo(component, component->GetTypeInfo());
            }
        }
	}

	Test();

	expandToDepth(0);
}

void PropertyEditor::AppendIntrospectionInfo(void *object, const DAVA::IntrospectionInfo *info)
{
    const IntrospectionInfo *currentInfo = info;
    while(NULL != currentInfo)
    {
        //if(info->MembersCount())
        {
            QPair<QtPropertyItem*, QtPropertyItem*> prop = AppendProperty(currentInfo->Name(), new QtPropertyDataIntrospection(object, currentInfo));
            
            prop.first->setBackground(QBrush(QColor(Qt::lightGray)));
            prop.second->setBackground(QBrush(QColor(Qt::lightGray)));
        }
        
        currentInfo = currentInfo->BaseInfo();
    }
}

void PropertyEditor::sceneChanged(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertyEditor::sceneActivated(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertyEditor::sceneReleased(SceneData *sceneData)
{ }

void PropertyEditor::sceneNodeSelected(SceneData *sceneData, DAVA::SceneNode *node)
{
	SetNode(node);
}


void PropertyEditor::Test()
{
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);

	DAVA::VariantType v;
	v.SetColor(DAVA::Color(1.0, 0.5, 0, 1.0));
	AppendProperty("test color", new QtPropertyDataDavaVariant(v));

	//IntrospectionCollection<std::vector, int> col(vec);
	/*

	DAVA::IntrospectionCollectionBase *b = DAVA::CreateIntrospectionCollection(vec);
	printf("Collection type: %s\n", b->CollectionType()->GetTypeName());
	printf("Value type: %s\n", b->ValueType()->GetTypeName());

	if(b->Size() > 0)
	{
		void *i = b->Begin();
		while(NULL != i)
		{
			b->ValueType();
			int *p = (int *) b->ItemPointer(i);
			if(NULL != p)
			{
				printf("%d\n", *p);
			}
			i = b->Next(i);
		}
	}
	*/

	//aaaGetObjectsToContainer(vec);


	//DAVA::MetaInfo* info = DAVA::MetaInfo::Instance< DAVA::Vector<int> >();
	//printf("%s\n", info->GetTypeName());
}