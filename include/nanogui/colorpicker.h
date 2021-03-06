/*
    nanogui/colorpicker.h -- push button with a popup to tweak a color value

    This widget was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <nanogui/popupbutton.h>
#include "textbox.h"

NAMESPACE_BEGIN(nanogui)

/**
 * \class ColorPicker colorpicker.h nanogui/colorpicker.h
 *
 * \brief Push button with a popup to tweak a color value.
 */
class NANOGUI_EXPORT ColorPicker : public PopupButton {
public:
    ColorPicker(Widget *parent, const Color& color = Color(1.0f, 0.0f, 0.0f, 1.0f), bool requireButtonClick=true);

    /// Set the change callback
    std::function<void(const Color &)> callback() const                  { return mCallback; }
    void setCallback(const std::function<void(const Color &)> &callback) { mCallback = callback; }

    /// Get the current preview color
    Color color() const;
    /// Set the current preview color
    void setColor(const Color& color);
protected:
    /**
     * Sets the color but does not update the color wheel.
     * 
     * Used internally to update the color without having the color wheel update itself.
     */
    void setColor_(const Color& color);
protected:
    std::function<void(const Color &)> mCallback;
    Slider *mAlphaSlider;
    ColorWheel *mColorWheel;
    std::array<IntBox<int>*, 3> mRGB;
    std::array<IntBox<int>*, 3> mHWB;
    Button *mPickButton;
    bool mRequireButtonClick;
    Color mSavedColor;
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

NAMESPACE_END(nanogui)
