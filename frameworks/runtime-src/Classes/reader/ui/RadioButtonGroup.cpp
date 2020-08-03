#include "RadioButtonGroup.h"
#include "RadioButton.h"

NS_CCR_BEGIN



CreatorRadioButtonGroup* CreatorRadioButtonGroup::create()
{
	CreatorRadioButtonGroup* widget = new (std::nothrow) CreatorRadioButtonGroup();
	if (widget && widget->init())
	{
		widget->autorelease();
		return widget;
	}
	CC_SAFE_DELETE(widget);
	return nullptr;
}


CreatorRadioButtonGroup::CreatorRadioButtonGroup()
{


}

CreatorRadioButtonGroup::~CreatorRadioButtonGroup()
{
}


void CreatorRadioButtonGroup::addRadioButton(ui::RadioButton* radioButton)
{
	ui::RadioButtonGroup::addRadioButton(radioButton);

	if (radioButton->isSelected())
	{
		if (radioButton != _selectedRadioButton)
		{
			radioButton->setSelected(false);
		}
	}
}


void CreatorRadioButtonGroup::callGroupCallback(CreatorRadioButton* pButton, int eventType)
{
	if (_radioButtonGroupEventCallback)
	{
		int index = (int)_radioButtons.getIndex(pButton);
		_radioButtonGroupEventCallback(pButton, index, EventType::SELECT_CHANGED);
	}
}

NS_CCR_END