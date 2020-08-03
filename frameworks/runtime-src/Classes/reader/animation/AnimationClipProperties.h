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
#include "Macros.h"

#include <vector>
#include <string>


NS_CCR_BEGIN



struct AnimPropRotation 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropPosition 
{
    float frame;
    cocos2d::Vec2 value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropPositionX 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropPositionY 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropScaleX 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropScaleY 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropWidth 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropHeight 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropColor 
{
    float frame;
    cocos2d::Color3B value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropOpacity 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropAnchorX 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropAnchorY 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropSkewX 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};

struct AnimPropSkewY 
{
    float frame;
    float value;
    std::vector<float> curveData;
    std::string curveType;
};


struct AnimPropSpriteFrame 
{
	float frame;
	std::string value;
	std::vector<float> curveData;
	std::string curveType;
};


class AnimProperties
{
public:

	AnimProperties() {
		m_target = nullptr;
	}

	~AnimProperties() {
		clearAnimMap();
	}

	void clearAnimMap();

	void updateAnimMap();
	bool isEmpty() {
		return m_sAnimMap.empty();
	}

	void updateAnimRotation(cocos2d::Node* target, float elapsed);
	void updateAnimPosition(cocos2d::Node* target, float elapsed);
	void updateAnimPositionX(cocos2d::Node* target, float elapsed);
	void updateAnimPositionY(cocos2d::Node* target, float elapsed);
	void updateAnimScaleX(cocos2d::Node* target, float elapsed);
	void updateAnimScaleY(cocos2d::Node* target, float elapsed);
	void updateAnimSkewX(cocos2d::Node* target, float elapsed);
	void updateAnimSkewY(cocos2d::Node* target, float elapsed);
	void updateAnimAnchorX(cocos2d::Node* target, float elapsed);
	void updateAnimAnchorY(cocos2d::Node* target, float elapsed);
	void updateAnimColor(cocos2d::Node* target, float elapsed);
	void updateAnimOpacity(cocos2d::Node* target, float elapsed);
	void updateAnimWidth(cocos2d::Node* target, float elapsed);
	void updateAnimHeight(cocos2d::Node* target, float elapsed);
	void updateAnimSpriteFrame(cocos2d::Node* target, float elapsed);

	void setTarget(cocos2d::Node* target) {
		m_target = target;
	}

	cocos2d::Node* getTarget() {
		return m_target;
	}


	std::vector<AnimPropRotation> animRotation;
	std::vector<AnimPropPosition> animPosition;
	std::vector<AnimPropPositionX> animPositionX;
	std::vector<AnimPropPositionY> animPositionY;
	std::vector<AnimPropScaleX> animScaleX;
	std::vector<AnimPropScaleY> animScaleY;
	std::vector<AnimPropSkewX> animSkewX;
	std::vector<AnimPropSkewY> animSkewY;
	std::vector<AnimPropAnchorX> animAnchorX;
	std::vector<AnimPropAnchorY> animAnchorY;
	std::vector<AnimPropColor> animColor;
	std::vector<AnimPropOpacity> animOpacity;
	std::vector<AnimPropWidth> animWidth;
	std::vector<AnimPropHeight> animHeight;
	std::vector<AnimPropSpriteFrame> animSpriteFrame;

    std::string path;

	//typedef std::function<void(cocos2d::Node* target, float elapsed)> AnimationUpdateFunc;

	std::list<std::function<void(cocos2d::Node* target, float elapsed)>> m_sAnimMap;
	cocos2d::Vec2 nextPos;
	float nextValue;
	cocos2d::Color3B nextColor;
	std::string nextPath;
	cocos2d::Node* m_target;
};

NS_CCR_END
