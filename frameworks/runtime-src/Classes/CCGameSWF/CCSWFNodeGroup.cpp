//
//  CCSWFNodeGroup.cpp
//  TangooBaby
//
//  Created by chenee on 13-5-20.
//  Copyright chenee543216@gmail.com 2013. All rights reserved.
//

#include "CCSWFNodeGroup.h"

CCSWFNodeGroup::CCSWFNodeGroup()
{
    isSingleton = true;
}
CCSWFNodeGroup::~CCSWFNodeGroup()
{
    CCLOG("delete group");
   
    stopAction();
    
    this->removeAllChildrenWithCleanup(true);
    this->removeFromParentAndCleanup(true);
}
void CCSWFNodeGroup::addChild(cocos2d::Node *child,int zOrder)
{
    CCNode::addChild(child,zOrder);
    ((CCSWFNode*)child)->setGrouped(true);
}
void CCSWFNodeGroup::addChild(cocos2d::Node *child)
{
    this->addChild(child, 0);
}

void CCSWFNodeGroup::setFlipX(bool flipX)
{
	auto children = this->getChildren();
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		CCSWFNode* pNode = (CCSWFNode*)*it;
        pNode->setFlipX(flipX);
    }
}
void CCSWFNodeGroup::setFlipY(bool flipY)
{
	auto children = this->getChildren();
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		CCSWFNode* pNode = (CCSWFNode*)*it;
        pNode->setFlipX(flipY);
    }
}
float CCSWFNodeGroup::scale()
{
	return 1.0f;
}

void CCSWFNodeGroup::setScale(float scale)
{
	auto children = this->getChildren();
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		CCSWFNode* pNode = (CCSWFNode*)*it;
        pNode->setScale(scale);
    }
}

float CCSWFNodeGroup::scaleX()
{
	return 1.0f;
}

void CCSWFNodeGroup::setScaleX(float scaleX)
{
	auto children = this->getChildren();
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		CCSWFNode* pNode = (CCSWFNode*)*it;
        pNode->setScaleX(scaleX);
    }
}

float CCSWFNodeGroup::scaleY()
{
    return 1.0f;
}

void CCSWFNodeGroup::setScaleY(float scaleY)
{
	auto children = this->getChildren();
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		CCSWFNode* pNode = (CCSWFNode*)*it;
        pNode->setScaleY(scaleY);
    }
}

void CCSWFNodeGroup::runAction(CCSWFNode* swf)
{
    if (isSingleton) {
		auto children = this->getChildren();
		for (auto it = children.begin(); it != children.end(); ++it)
        {
            CCSWFNode* pNode = (CCSWFNode*) *it;
            if (pNode != swf) {
                pNode->stopAction();
            }
        }
    }
    
    swf->runAction();
}
void CCSWFNodeGroup::stopAction()
{
	auto children = this->getChildren();
	for(auto it = children.begin(); it != children.end(); ++it)
    {
		CCSWFNode* pNode = (CCSWFNode*)*it;//(CCSWFNode*) child;
        pNode->stopAction();
    }
}
bool CCSWFNodeGroup::Runing()
{
	auto children = this->getChildren();
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		CCSWFNode* pNode = (CCSWFNode*)*it;
        if(pNode->Runing()){
            return true;
        }
    }
    return false;
}


