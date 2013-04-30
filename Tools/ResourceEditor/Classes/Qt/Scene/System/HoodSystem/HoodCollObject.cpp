#include "Qt/Scene/System/HoodSystem/HoodCollObject.h"

HoodCollObject::HoodCollObject()
	 : btObject(NULL)
	 , btShape(NULL) 
	 , curScale(1.0)
{ }

HoodCollObject::~HoodCollObject()
{
	if(NULL != btObject)
	{
		delete btObject;
	}

	if(NULL != btShape)
	{
		delete btShape;
	}
}

void HoodCollObject::UpdatePos(const DAVA::Vector3 &pos)
{
	curPos = pos;
	curFrom = (baseFrom * curScale) + curPos;
	curTo = (baseTo * curScale) + curPos;

	// move bullet object to new pos
	btTransform transf = btObject->getWorldTransform();
	transf.setOrigin(btVector3(scaledOffset.x + curFrom.x, scaledOffset.y + curFrom.y, scaledOffset.z + curFrom.z));
	btObject->setWorldTransform(transf);
}

void HoodCollObject::UpdateScale(const DAVA::float32 &scale)
{
	this->curScale = scale;

	btShape->setLocalScaling(btVector3(scale, scale, scale));
	scaledOffset = DAVA::MultiplyVectorMat3x3(baseOffset * scale, baseRotate);

	UpdatePos(curPos);
}