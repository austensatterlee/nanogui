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

ColorPicker::ColorPicker(Widget *parent, const Color& c, bool requireButtonClick) : PopupButton(parent, ""), mRequireButtonClick(requireButtonClick) {
    setBackgroundColor(c);
    Popup *popup = this->popup();
    popup->setLayout(new GridLayout(Orientation::Horizontal, 1, Alignment::Fill, 3, 3));

    mAlphaSlider = new Slider(popup);
    mAlphaSlider->setRange({ 0.0f,1.0f });

    mColorWheel = new ColorWheel(popup);

    // Set up RGB & HSL controls
    auto txtGrid = new Widget(popup);
    txtGrid->setLayout(new GridLayout(Orientation::Vertical, 2, Alignment::Fill, 3, 0));

    auto rgb_cb = [this](int) {
        float r = static_cast<float>(mRGB[0]->value()) / 255.0f;
        float g = static_cast<float>(mRGB[1]->value()) / 255.0f;
        float b = static_cast<float>(mRGB[2]->value()) / 255.0f;
        float a = mAlphaSlider->value();
        Color rgba{ r,g,b,a };
        previewColor(rgba);
    };
    auto hsl_cb = [this](int) {
        float h = static_cast<float>(mHSL[0]->value()) / 361.0f;
        float s = static_cast<float>(mHSL[1]->value()) / 101.0f;
        float l = static_cast<float>(mHSL[2]->value()) / 101.0f;
        float a = mAlphaSlider->value();
        Color rgba = Color::fromHSLA(Vector4f{h, s, l, a});
        previewColor(rgba);
    };

    for(int i=0;i<3;i++) {
        mRGB[i] = new IntBox<int>(txtGrid, 0);
        mRGB[i]->setFontSize(12);
        mRGB[i]->setMinMaxValues(0, 255);
        mRGB[i]->setEditable(true);
        mRGB[i]->setSpinnable(true);
        mRGB[i]->setCallback(rgb_cb);

        mHSL[i] = new IntBox<int>(txtGrid, 0);
        mHSL[i]->setFontSize(12);
        mHSL[i]->setMinMaxValues(0, i == 0 ? 360 : 100);
        mHSL[i]->setEditable(true);
        mHSL[i]->setSpinnable(true);
        mHSL[i]->setCallback(hsl_cb);
    }

    mPickButton = new Button(popup, "Pick");

    mAlphaSlider->setCallback([this](const float f)
    {
        previewColor(color());
    });

    mColorWheel->setCallback([this](const Color& c) {
        previewColor(c);
    });

    mPickButton->setCallback([this]() {
        Color value = color();
        setPushed(false); 
        setColor(value);
        if (mCallback)
            mCallback(color());
    });

    PopupButton::setChangeCallback([this](bool) {
        setColor(color());
        // Apply changes if they don't need explicit saving
        if (mCallback && !mRequireButtonClick)
            mCallback(color());
    });
}

Color ColorPicker::color() const {
    return backgroundColor();
}

void ColorPicker::setColor(const Color& color) {
    /* Ignore setColor() calls when the user is currently editing */
    if (!mPushed || !mRequireButtonClick) {
        mAlphaSlider->setValue(color.a());
        previewColor(color);
    }
}

void ColorPicker::previewColor(const Color& c) {
    Color rgba(c);
    rgba.a() = mAlphaSlider->value();

    mRGB[0]->setValue(rgba.r() * 255);
    mRGB[1]->setValue(rgba.g() * 255);
    mRGB[2]->setValue(rgba.b() * 255);

    Vector4f hsla = rgba.toHSLA();
    mHSL[0]->setValue(hsla(0) * 360);
    mHSL[1]->setValue(hsla(1) * 100);
    mHSL[2]->setValue(hsla(2) * 100);

    Color fg = rgba.contrastingColor();
    setBackgroundColor(rgba);
    setTextColor(fg);
    mColorWheel->setColor(rgba);
    mPickButton->setBackgroundColor(rgba);
    mPickButton->setTextColor(fg);
}
NAMESPACE_END(nanogui)
