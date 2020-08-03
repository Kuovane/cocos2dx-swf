/****************************************************************************
 Copyright (c) 2017 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "CreatorReader_generated.h"

#include "Macros.h"
#include"ShaderCache.h"
using namespace cocos2d;
NS_CCR_BEGIN

class CreatorButton : public ui::Button
{
public :

	enum TransitionType
	{
		NONE,
		COLOR,
		SPRITE,
		SCALE
	};


	CreatorButton();
	~CreatorButton();
	/**
	* Create and return a empty CheckBox instance pointer.
	*/
	static CreatorButton* create();

	/**
	* Create a button with custom textures.
	* @param normalImage normal state texture name.
	* @param selectedImage  selected state texture name.
	* @param disableImage disabled state texture name.
	* @param texType    @see `TextureResType`
	* @return a Button instance.
	*/
	static CreatorButton* create(const std::string& normalImage,
		const std::string& selectedImage = "",
		const std::string& disableImage = "",
		TextureResType texType = TextureResType::LOCAL);

	void addChild(Node *child) override;

	

	virtual void onPressStateChangedToCancel();
	virtual void onPressStateChangedToNormal() override;
	virtual void onPressStateChangedToPressed() override;
	virtual void onPressStateChangedToDisabled() override;

	inline void setActionDuration(float duration) { _actionDuration = duration; }
	float getActionDuration() const { return _actionDuration; }

	void setTransitionType(TransitionType transitionType);
	TransitionType getTransitionType();

	void setNormalColor(Color4B& color);
	Color4B getNormalColor()const;

	void setPressedColor(Color4B& color);
	Color4B getPressedColor()const;

	void setDisableColor(Color4B& color);
	Color4B getDisableColor()const;

	inline void enableAutoGrayEffect(bool  enableAutoGrayEffect) { _enableAutoGrayEffect = enableAutoGrayEffect; }
	int isAutoGrayEffect() const { return _enableAutoGrayEffect; }

	virtual void onTouchEnded(Touch *touch, Event *unusedEvent)override;

	void cancelUpEvent()override;

	void setNodeBgName(std::string sBgName);
	std::string getNodeBgName() { return _nodeBgName; }

	virtual void setColor(const Color3B& color);
	void addRefrenceChild(Node *child);
	void removeRefrenceChild(Node *child);
	virtual void setVisible(bool visible);
public:
	float _actionDuration;
	TransitionType _transitionType;

	Color4B _normalColor;
	Color4B _pressedColor;
	Color4B _disableColor;

	bool _enableAutoGrayEffect;
	std::string _nodeBgName;

	std::map<Node *, Node *> m_pRefrenceChildren;
};

NS_CCR_END