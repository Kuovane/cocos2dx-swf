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

class CreatorRadioButton : public ui::RadioButton
{
public :
	enum
	{
		TRANSITION_NONE = 0,
		TRANSITION_COLOR = 1,
		TRANSITION_SPRITE = 2,
		TRANSITION_SCALE = 3,
	};

	CreatorRadioButton();
	~CreatorRadioButton();

	static CreatorRadioButton* create();


	static CreatorRadioButton* create(const std::string& backGround,
		const std::string& backGroundSelected,
		const std::string& cross,
		const std::string& backGroundDisabled,
		const std::string& frontCrossDisabled,
		TextureResType texType = TextureResType::LOCAL);


	static CreatorRadioButton* create(const std::string& backGround,
		const std::string& cross,
		TextureResType texType = TextureResType::LOCAL);


	virtual void onTouchEnded(Touch *touch, Event *unusedEvent) override;
	virtual void releaseUpEvent() override;


	

	void addChild(Node *child) override;

	void backGroundTextureScaleChangedWithSize();
	void backGroundSelectedTextureScaleChangedWithSize();
	void frontCrossTextureScaleChangedWithSize();
	void backGroundDisabledTextureScaleChangedWithSize();
	void frontCrossDisabledTextureScaleChangedWithSize();


	virtual void onPressStateChangedToNormal() override;
	virtual void onPressStateChangedToPressed() override;
	virtual void onPressStateChangedToDisabled() override;

	inline void setActionDuration(float duration) { _actionDuration = duration; }
	float getActionDuration() const { return _actionDuration; }

	inline void setTransitionType(int transitionType) { _transitionType = transitionType; }
	int getTransitionType() const { return _transitionType; }

	void setNormalColor(Color4B& color);
	Color4B getNormalColor()const;

	void setPressedColor(Color4B& color);
	Color4B getPressedColor()const;

	void setDisableColor(Color4B& color);
	Color4B getDisableColor()const;

	inline void enableAutoGrayEffect(bool  enableAutoGrayEffect) { _enableAutoGrayEffect = enableAutoGrayEffect; }
	int isAutoGrayEffect() const { return _enableAutoGrayEffect; }

	inline void setToggleGroup(int isToggleGroup) { _isToggleGroup = isToggleGroup; }
	int getToggleGroup() const { return _isToggleGroup; }


	void setTouchEnabled(bool enable)override;


	void setNodeBgName(std::string sBgName);
	std::string getNodeBgName() { return _nodeBgName; }


	void setNodeCheckmarkName(std::string sName);
	std::string getNodeCheckmarkName() { return _nodeCheckmarkName; }


	void setCheckMarkNode(cocos2d::Sprite * pNode);
	void setBgNode(cocos2d::Sprite * pNode);


	protected:
	void dispatchSelectChangedEvent(bool selected)override;




public:
	float _actionDuration;
	int _transitionType;

	Color4B _normalColor;
	Color4B _pressedColor;
	Color4B _disableColor;

	bool _enableAutoGrayEffect;

	bool _isToggleGroup;

	std::string _nodeBgName;
	std::string _nodeCheckmarkName;

};

NS_CCR_END
