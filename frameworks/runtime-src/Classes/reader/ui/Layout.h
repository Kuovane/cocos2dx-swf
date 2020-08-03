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

#include "Macros.h"
#include"ShaderCache.h"
using namespace cocos2d;
NS_CCR_BEGIN

class CreatorLayout : public ui::Layout
{
public :

	enum class CreatorType
	{
		NONE,
		HORIZONAL,
		VERTICAL,
		GRID
	};

	enum class ResizeModeType
	{
		NONE,
		CONTAINER,
		CHILDREN,
	};

	enum  HDirectionType
	{
		LEFT_TO_RIGHT,
		RIGHT_TO_LEFT
	};

	enum  VDirectionType
	{
		BOTTOM_TO_TOP,
		TOP_TO_BOTTOM,
	
	};

	enum class StartAxis
	{
		HORIZONAL,
		VERTICAL,
	};

	CreatorLayout();
	~CreatorLayout();
	/**
	* Create and return a empty CreatorLayout instance pointer.
	*/
	static CreatorLayout* create();

	

	void addChild(Node *child, int zOrder, int tag)override;
	void addChild(Node* child, int zOrder, const std::string &name)override;

	void doResizeModeLayout();
	void doHorizonalLayout();
	void doVerticalLayout();
	void doGridLayout();
	
	cocos2d::Node* getFirstNode();
	void doLayout();



	cocos2d::Vec2 getGridStartPos();
	cocos2d::Vec2 setGridCellPos(Node* pNode, cocos2d::Vec2& desPos, cocos2d::Size &sCurLineSize);
	cocos2d::Vec2 setGridCellPosInHAxizAndNoneMode(Node* pNode, cocos2d::Vec2& desPos, cocos2d::Size &sCurLineSize);
	cocos2d::Vec2 setGridCellPosInVAxizAndNoneMode(Node* pNode, cocos2d::Vec2& desPos, cocos2d::Size &sCurLineSize);
	

	void setPaddingTop(float value) { _paddingTop = value; }
	void setPaddingBottom(float value) { _paddingBottom = value; }
	void setPaddingLeft(float value) { _paddingLeft = value; }
	void setPaddingRight(float value) { _paddingRight = value; }

	void setSpacingX(float value) { _spacingX = value; }
	void setSpacingY(float value) { _spacingY = value; }

	void setStartAxis(int value) { _startAxis = (StartAxis)value; }

	void setHorizonalDirection(int direction) { _horizontalDirection = direction; }

	void setVerticalDirection(int direction) { _verticalDirection = direction; }

	void setCreatorType(CreatorType type) { _CreatorType = type; }

	void setResizeModeType(ResizeModeType type) { _resizeModeType = type; }

	//void setCellSize(cocos2d::Size& cellSize) { _cellSize = cellSize; }

	void setCellSize(float w, float h);

public:
	float _paddingTop ;
	float _paddingBottom;
	float _paddingLeft;
	float _paddingRight;
	float _spacingX;
	float _spacingY;
	StartAxis	 _startAxis;
	int _horizontalDirection;
	int _verticalDirection;
	cocos2d::Size _cellSize;

	CreatorType _CreatorType;
	ResizeModeType _resizeModeType;

};

NS_CCR_END
