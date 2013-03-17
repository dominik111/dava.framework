#include "Scene3D/Systems/UpdateSystem.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Entity.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{

UpdateSystem::UpdateSystem(Scene * scene)
:	SceneSystem(scene)
{

}

void UpdateSystem::AddEntity(Entity * entity)
{
	UpdatableComponent * component = (UpdatableComponent*)entity->GetComponent(Component::UPDATABLE_COMPONENT);
	IUpdatable * object = component->GetUpdatableObject();

	if(object)
	{
		IUpdatableBeforeTransform * updateBeforeTransform = dynamic_cast<IUpdatableBeforeTransform*>(object);
		if(updateBeforeTransform)
		{
			updatesBeforeTransform.push_back(updateBeforeTransform);
		}

		IUpdatableAfterTransform * updateAfterTransform = dynamic_cast<IUpdatableAfterTransform*>(object);
		if(updateAfterTransform)
		{
			updatesAfterTransform.push_back(updateAfterTransform);
		}
	}
}

void UpdateSystem::RemoveEntity(Entity * entity)
{
	UpdatableComponent * component = (UpdatableComponent*)entity->GetComponent(Component::UPDATABLE_COMPONENT);
	IUpdatable * object = component->GetUpdatableObject();

	if(object)
	{
		IUpdatableBeforeTransform * updateBeforeTransform = dynamic_cast<IUpdatableBeforeTransform*>(object);
		if(updateBeforeTransform)
		{
			uint32 size = updatesBeforeTransform.size();
			for(uint32 i = 0; i < size; ++i)
			{
				if(updatesBeforeTransform[i] == updateBeforeTransform)
				{
					updatesBeforeTransform[i] = updatesBeforeTransform[size-1];
					updatesBeforeTransform.pop_back();
					break;
				}
			}
		}

		IUpdatableAfterTransform * updateAfterTransform = dynamic_cast<IUpdatableAfterTransform*>(object);
		if(updateAfterTransform)
		{
			uint32 size = updatesAfterTransform.size();
			for(uint32 i = 0; i < size; ++i)
			{
				if(updatesAfterTransform[i] == updateAfterTransform)
				{
					updatesAfterTransform[i] = updatesAfterTransform[size-1];
					updatesAfterTransform.pop_back();
					break;
				}
			}
		}
	}
}

void UpdateSystem::Process()
{

}

void UpdateSystem::UpdatePreTransform()
{
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
	uint32 size = updatesBeforeTransform.size();
	for(uint32 i = 0; i < size; ++i)
	{
		updatesBeforeTransform[i]->UpdateBeforeTransform(timeElapsed);
	}
}

void UpdateSystem::UpdatePostTransform()
{
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
	uint32 size = updatesAfterTransform.size();
	for(uint32 i = 0; i < size; ++i)
	{
		updatesAfterTransform[i]->UpdateAfterTransform(timeElapsed);
	}
}

}