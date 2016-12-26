/*
    src/colorpicker.cpp -- push button with a popup to tweak a color value

    This widget was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/colorpicker.h>
#include <nanogui/layout.h>
#include <nanogui/colorwheel.h>
#include "nanogui/slider.h"
#include "nanogui/textbox.h"
#include <iomanip>

NAMESPACE_BEGIN(nanogui)

ColorPicker::ColorPicker(Widget *parent, const Color& color, bool requireButtonClick) : PopupButton(parent, ""), mRequireButtonClick(requireButtonClick) {
    setBackgroundColor(color);
    Popup *popup = this->popup();
    popup->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Middle, 5, 5));

    mAlphaSlider = new Slider(popup);
    mAlphaSlider->setRange({ 0.0f,1.0f });
    mColorWheel = new ColorWheel(popup);
    mColorText = new TextBox(popup, "");
    mColorText->setEditable(true);
    mColorText->setAlignment(TextBox::Alignment::Center);
    mColorText->setFontSize(14);
    mColorText->setFormat("[0-9]*?\\.?[0-9]*?, *[0-9]*?\\.?[0-9]*?, *[0-9]*?\\.?[0-9]*?, *[0-9]*?\\.?[0-9]*?");
    mPickButton = new Button(popup, "Pick");
    mPickButton->setFixedSize(Vector2i(100, 25));

    PopupButton::setChangeCallback([&](bool) {
        setColor(backgroundColor());
        mCallback(backgroundColor());
    });

    mAlphaSlider->setCallback([&](const float f)
    {
        mColorWheel->callback()(mColorWheel->color());
    });

    mColorWheel->setCallback([&](const Color& c) {
        Color value = c;
        value.w() = mAlphaSlider->value();
        mPickButton->setBackgroundColor(value);
        mPickButton->setTextColor(value.contrastingColor());
        std::ostringstream os;
        os << std::setprecision(2) << value.r() << ", " << value.g() << ", " << value.b() << ", " << value.w();
        mColorText->setValue(os.str());
        if (!mRequireButtonClick)
            setColor(value);
        mCallback(value);
    });

    mColorText->setCallback([&](const std::string &str) {
        std::stringstream ss(str);
        std::string token;
        Color value = mColorWheel->color();
        value.w() = mAlphaSlider->value();
        int i = 0;
        while (i < 4)
        {
            if (std::getline(ss, token, ',')) {
                value[i] = strtof(token.c_str(), nullptr);
            }
            i++;
        }
        mColorWheel->callback()(value);
        return true;
    });

    mPickButton->setCallback([&]() {
        Color value = mColorWheel->color();
        value.w() = mAlphaSlider->value();
        setPushed(false);
        setColor(value);
        mCallback(value);
    });
}

Color ColorPicker::color() const {
    return backgroundColor();
}

void ColorPicker::setColor(const Color& color) {
    /* Ignore setColor() calls when the user is currently editing */
    if (!mPushed || !mRequireButtonClick) {
        Color fg = color.contrastingColor();
        setBackgroundColor(color);
        setTextColor(fg);
        mColorWheel->setColor(color);
        mAlphaSlider->setValue(color.w());
        std::ostringstream os;
        os << std::setprecision(2) << color.r() << ", " << color.g() << ", " << color.b() << ", " << color.w();
        mColorText->setValue(os.str());
        mPickButton->setBackgroundColor(color);
        mPickButton->setTextColor(fg);
    }
}

NAMESPACE_END(nanogui)
