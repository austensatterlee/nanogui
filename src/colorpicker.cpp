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
    Popup *popup = this->popup();
    popup->setLayout(new GridLayout(Orientation::Horizontal, 1, Alignment::Fill, 3, 3));

    mAlphaSlider = new Slider(popup);
    mAlphaSlider->setRange({ 0.0f,1.0f });

    mColorWheel = new ColorWheel(popup);

    mAlphaSlider->setCallback([this](const float f) {
        Color newColor = color();
        newColor.a() = f;
        setColor(newColor);
    });

    mColorWheel->setCallback([this](const Color& c) {
        Color newColor = c;
        newColor.a() = mAlphaSlider->value();
        setColor_(newColor);
    });

    auto rgb_cb = [this](int) {
        float r = static_cast<float>(mRGB[0]->value()) / 255.0f;
        float g = static_cast<float>(mRGB[1]->value()) / 255.0f;
        float b = static_cast<float>(mRGB[2]->value()) / 255.0f;
        float a = mAlphaSlider->value();
        mColorWheel->setColor({ r,g,b,a });
        setColor({ r,g,b,a });
    };
    auto hwb_cb = [this](int) {
        float h = static_cast<float>(mHWB[0]->value()) / 360.0f;
        float w = static_cast<float>(mHWB[1]->value()) / 100.0f;
        float b = static_cast<float>(mHWB[2]->value()) / 100.0f;
        float a = mAlphaSlider->value();
        if (w + b > 1) {
            float wbsum = w + b;
            w /= wbsum;
            b /= wbsum;
        }
        mColorWheel->setColorHWB({ h-0.25f,w,b,a });
        Color rgba = mColorWheel->color();
        rgba.a() = a;
        setColor(rgba);
    };

    // Set up RGB & HWB controls
    auto txtGrid = new Widget(popup);
    txtGrid->setLayout(new GridLayout(Orientation::Vertical, 2, Alignment::Fill, 3, 0));

    for(int i=0;i<3;i++) {
        mRGB[i] = new IntBox<int>(txtGrid, 0);
        mRGB[i]->setFontSize(12);
        mRGB[i]->setMinMaxValues(0, 255);
        mRGB[i]->setEditable(true);
        mRGB[i]->setSpinnable(true);
        mRGB[i]->setCallback(rgb_cb);

        mHWB[i] = new IntBox<int>(txtGrid, 0);
        mHWB[i]->setFontSize(12);
        mHWB[i]->setMinMaxValues(0, i == 0 ? 255 : 255);
        mHWB[i]->setEditable(true);
        mHWB[i]->setSpinnable(true);
        mHWB[i]->setCallback(hwb_cb);
    }
    mRGB[0]->setTooltip("Red");
    mRGB[0]->setUnits("R");
    mRGB[1]->setTooltip("Green");
    mRGB[1]->setUnits("G");
    mRGB[2]->setTooltip("Blue");
    mRGB[2]->setUnits("B");

    mHWB[0]->setTooltip("Hue");
    mHWB[0]->setUnits("H");
    mHWB[1]->setTooltip("White");
    mHWB[1]->setUnits("W");
    mHWB[2]->setTooltip("Black");
    mHWB[2]->setUnits("B");

    mPickButton = new Button(popup, "Pick");
    mPickButton->setCallback([this]() {
        setPushed(false);
        if (mCallback)
            mCallback(color());
    });

    PopupButton::setChangeCallback([this](bool pushed) {
        if (pushed) {
            // Update the displayed color with the saved color
            mSavedColor = color();
        } else if (mRequireButtonClick) {
            // Revert to the saved color
            setColor(mSavedColor);
            if (mCallback)
                mCallback(mSavedColor);
        }
    });
    setColor(c);
}

Color ColorPicker::color() const {
    return backgroundColor();
}

void ColorPicker::setColor(const Color& c) {
    mColorWheel->setColor(c);
    setColor_(c);
}

void ColorPicker::setColor_(const Color& c) {
    Color fg = c.contrastingColor();
    setBackgroundColor(c);
    setTextColor(fg);

    mAlphaSlider->setValue(c.a());

    mRGB[0]->setValue(c.r() * 255);
    mRGB[1]->setValue(c.g() * 255);
    mRGB[2]->setValue(c.b() * 255);

    Vector4f hwb = mColorWheel->colorHWB();
    mHWB[0]->setValue((hwb(0) + 0.25) * 255);
    mHWB[1]->setValue(hwb(1) * 255);
    mHWB[2]->setValue(hwb(2) * 255);

    mPickButton->setBackgroundColor(c);
    mPickButton->setTextColor(fg);

    if (mCallback)
        mCallback(color());
}
NAMESPACE_END(nanogui)
