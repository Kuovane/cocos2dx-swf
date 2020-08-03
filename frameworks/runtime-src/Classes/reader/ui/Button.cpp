#include "Button.h"

NS_CCR_BEGIN



CreatorButton* CreatorButton::create()
{
	CreatorButton* widget = new (std::nothrow) CreatorButton();
	if (widget && widget->init())
	{
		widget->autorelease();
		return widget;
	}
	CC_SAFE_DELETE(widget);
	return nullptr;
}


CreatorButton* CreatorButton::create(const std::string &normalImage,
	const std::string& selectedImage,
	const std::string& disableImage,
	TextureResType texType)
{
	CreatorButton *btn = new (std::nothrow) CreatorButton;
	if (btn && btn->init(normalImage, selectedImage, disableImage, texType))
	{
		btn->autorelease();
		return btn;
	}
	CC_SAFE_DELETE(btn);
	return nullptr;
}



CreatorButton::CreatorButton()
{
	_actionDuration = 0.1;
	_transitionType = TransitionType::NONE;
	_enableAutoGrayEffect = false;
	_orignScale = 1.0;

}

CreatorButton::~CreatorButton()
{
	for (auto it = m_pRefrenceChildren.begin(); it != m_pRefrenceChildren.end(); it++)
	{
		(it->second)->release();
	}
}

void CreatorButton::setNodeBgName(std::string sBgName) {
	_nodeBgName = sBgName;
};

void CreatorButton::addChild(Node *child)
{

	std::string sChildName = child->getName();
	
	if (sChildName == _nodeBgName && !_nodeBgName.empty())
	{
		auto spirte = dynamic_cast<cocos2d::ui::Scale9Sprite*>(child);
		if (spirte)
		{
			removeProtectedChild(_buttonNormalRenderer);
			_buttonNormalRenderer = spirte;
			addProtectedChild(_buttonNormalRenderer, -2, -1);
			return;
		}
	}

	ui::Button::addChild(child);
}


void CreatorButton::onPressStateChangedToCancel()
{
	_buttonNormalRenderer->setVisible(true);
	_buttonClickedRenderer->setVisible(false);
	_buttonDisabledRenderer->setVisible(false);
	_buttonNormalRenderer->setState(ui::Scale9Sprite::State::NORMAL);

	if (_pressedTextureLoaded)
	{
		if (_transitionType == TransitionType::COLOR)
		{
			//this->stopAllActions();
			//this->setColor(Color3B(_normalColor.r, _normalColor.b, _normalColor.g));
			//this->setOpacity(_normalColor.a);


			this->stopAllActions();
			this->setColor(Color3B(_normalColor.r, _normalColor.b, _normalColor.g));
			this->setOpacity(_normalColor.a);

		}
		else if (_pressedActionEnabled)
		{

			float orignScale = this->getOrignScale();
			//float childScale = (orignScale == -1.0f ? this->getScale() : orignScale);
			//this->setScale(childScale);
			//this->setOrignScale(-1);

			this->stopAllActions();
			//this->runAction(EaseSineOut::create(ScaleTo::create(_actionDuration, orignScale, orignScale)));
			this->setScale(orignScale, orignScale);
		}


	}
	else
	{
		_buttonNormalRenderer->stopAllActions();
		_buttonNormalRenderer->setScale(1.0);

		if (nullptr != _titleRenderer)
		{
			_titleRenderer->stopAllActions();
			_titleRenderer->setScaleX(1.0f);
			_titleRenderer->setScaleY(1.0f);
		}

	}
}


void CreatorButton::onPressStateChangedToNormal()
{
	_buttonNormalRenderer->setVisible(true);
	_buttonClickedRenderer->setVisible(false);
	_buttonDisabledRenderer->setVisible(false);
	_buttonNormalRenderer->setState(ui::Scale9Sprite::State::NORMAL);

	if (_pressedTextureLoaded)
	{
		if (_transitionType == TransitionType::COLOR)
		{
			//this->stopAllActions();
			//this->setColor(Color3B(_normalColor.r, _normalColor.b, _normalColor.g));
			//this->setOpacity(_normalColor.a);


			this->stopAllActions();
			auto action = TintTo::create(_actionDuration, Color3B(_normalColor.r, _normalColor.b, _normalColor.g));
			auto fadeAction = FadeTo::create(_actionDuration, _normalColor.a);
			auto spwan = Spawn::create(action, fadeAction, nullptr);

			this->runAction(EaseSineOut::create(spwan));

		}
		else if (_pressedActionEnabled)
		{
			
			float orignScale = this->getOrignScale();
			//float childScale = (orignScale == -1.0f ? this->getScale() : orignScale);
			//this->setScale(childScale);
			//this->setOrignScale(-1);

			this->stopAllActions();
			this->runAction(EaseSineOut::create(ScaleTo::create(_actionDuration, orignScale, orignScale)));
		}


	}
	else
	{
		_buttonNormalRenderer->stopAllActions();
		_buttonNormalRenderer->setScale(1.0);

		if (nullptr != _titleRenderer)
		{
			_titleRenderer->stopAllActions();
			_titleRenderer->setScaleX(1.0f);
			_titleRenderer->setScaleY(1.0f);
		}

	}
}

void CreatorButton::onPressStateChangedToPressed()
{
	_buttonNormalRenderer->setState(ui::Scale9Sprite::State::NORMAL);

	if (_pressedTextureLoaded)
	{
		_buttonNormalRenderer->setVisible(false);
		_buttonClickedRenderer->setVisible(true);
		_buttonDisabledRenderer->setVisible(false);


		if (_transitionType == TransitionType::COLOR)
		{
			this->stopAllActions();


			auto action = TintTo::create(_actionDuration, Color3B(_pressedColor.r, _pressedColor.b, _pressedColor.g));
			auto fadeAction = FadeTo::create(_actionDuration, _pressedColor.a);
			auto spwan = Spawn::create(action, fadeAction, nullptr);

			this->runAction(EaseSineOut::create(spwan));
			//this->setColor(Color3B(_normalColor.r, _normalColor.b, _normalColor.g));
			//this->setOpacity(_normalColor.a);
		}
		else if (_pressedActionEnabled)
		{


			this->stopAllActions();
			float orignScale = this->getOrignScale();
			//float childScale = (orignScale == -1.0f ? this->getScale() : orignScale);
			//this->setOrignScale(childScale);

			ScaleTo *zoomChildAction = ScaleTo::create(_actionDuration,
				orignScale * (1.0f + _zoomScale),
				orignScale * (1.0f + _zoomScale));

			this->runAction(EaseSineOut::create(zoomChildAction));

		}
	}
	else
	{
		_buttonNormalRenderer->setVisible(true);
		_buttonClickedRenderer->setVisible(true);
		_buttonDisabledRenderer->setVisible(false);

		_buttonNormalRenderer->stopAllActions();
		_buttonNormalRenderer->setScale(1.0f + _zoomScale, 1.0f + _zoomScale);

		if (nullptr != _titleRenderer)
		{
			_titleRenderer->stopAllActions();
			_titleRenderer->setScaleX(1.0f + _zoomScale);
			_titleRenderer->setScaleY(1.0f + _zoomScale);
		}
	}
}

void CreatorButton::onPressStateChangedToDisabled()
{
	//if disable resource is null
	if (!_disabledTextureLoaded)
	{
		if (_normalTextureLoaded)
		{
			_buttonNormalRenderer->setState(ui::Scale9Sprite::State::GRAY);
		}
	}
	else
	{
		_buttonNormalRenderer->setVisible(false);
		_buttonDisabledRenderer->setVisible(true);
	}

	if (_transitionType == TransitionType::COLOR)
	{
		this->stopAllActions();


		auto action = TintTo::create(_actionDuration, Color3B(_disableColor.r, _disableColor.b, _disableColor.g));
		auto fadeAction = FadeTo::create(_actionDuration, _disableColor.a);
		auto spwan = Spawn::create(action, fadeAction, nullptr);

		this->runAction(EaseSineOut::create(spwan));
		//this->setColor(Color3B(_normalColor.r, _normalColor.b, _normalColor.g));
		//this->setOpacity(_normalColor.a);
	}
	
	if (_enableAutoGrayEffect)
	{
		_buttonDisabledRenderer->setState(ui::Scale9Sprite::State::GRAY);

	}
	

	_buttonClickedRenderer->setVisible(false);
	_buttonNormalRenderer->setScale(1.0);
	_buttonClickedRenderer->setScale(1.0);
}


void CreatorButton::setTransitionType(CreatorButton::TransitionType transitionType)
{
	_transitionType = transitionType;
	if (_transitionType == TransitionType::COLOR)
	{
		this->setCascadeColorEnabled(true);
		this->setCascadeOpacityEnabled(true);
	}
}

CreatorButton::TransitionType CreatorButton::getTransitionType()
{
	return _transitionType;
}


void CreatorButton::setNormalColor(Color4B& color)
{
	_normalColor = color;
}

Color4B CreatorButton::getNormalColor()const
{
	return _normalColor;
}

void CreatorButton::setPressedColor(Color4B& color)
{
	_pressedColor = color;
}

Color4B CreatorButton::getPressedColor()const
{
	return _pressedColor;
}

void CreatorButton::setDisableColor(Color4B& color)
{
	_disableColor = color;
}

Color4B CreatorButton::getDisableColor()const
{
	return _disableColor;
}



void CreatorButton::onTouchEnded(Touch *touch, Event* unusedEvent)
{
	ui::Button::onTouchEnded(touch, unusedEvent);
}


void CreatorButton::cancelUpEvent()
{
	this->retain();

	ui::Button::cancelUpEvent();
	
	if (isEnabled())
	{
		onPressStateChangedToCancel();
	}
	this->release();
}


void CreatorButton::setColor(const Color3B& color)
{
	Node::setColor(color);
	if (isCascadeColorEnabled()) {
		auto displayedColor = getDisplayedColor();
		for (auto it = m_pRefrenceChildren.begin(); it != m_pRefrenceChildren.end(); it++)
		{
			it->second->updateDisplayedColor(displayedColor);
		}
	}
}


void CreatorButton::addRefrenceChild(Node *child)
{
	auto it = m_pRefrenceChildren.find(child);
	if (it == m_pRefrenceChildren.end())
	{
		child->retain();
		m_pRefrenceChildren[child] = child;
	}
}


void CreatorButton::removeRefrenceChild(Node *child)
{
	auto it = m_pRefrenceChildren.find(child);
	if (it != m_pRefrenceChildren.end())
	{
		child->release();
		m_pRefrenceChildren.erase(it);
	}
}

void CreatorButton::setVisible(bool visible)
{
	Node::setVisible(visible);

	auto displayedColor = getDisplayedColor();
	for (auto it = m_pRefrenceChildren.begin(); it != m_pRefrenceChildren.end(); it++)
	{
		it->second->setVisible(visible);
	}

}


NS_CCR_END