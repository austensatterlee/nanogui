/*
    nanogui/theme.h -- Storage class for basic theme-related properties

    The text box widget was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <nanogui/common.h>
#include <nanogui/object.h>
#include <json/json.hpp>

NAMESPACE_BEGIN(nanogui)

/**
 * \class Theme theme.h nanogui/theme.h
 *
 * \brief Storage class for basic theme-related properties.
 */
class NANOGUI_EXPORT Theme : public Object {
    using json = nlohmann::json;
public:
    Theme(NVGcontext *ctx);

    /// Retrieve a value using a json pointer. If it doesn't exist, return a default.
    template<typename value_type>
    value_type get(const std::string& json_ptr, const value_type& default_value) const;

    /// Access the json object at a location specified by a json pointer.
    json& prop(const std::string& json_ptr="") {
        return mProperties[json::json_pointer{ json_ptr }];
    };

    /* Fonts */
    int mFontMono;
    int mFontNormal;
    int mFontBold;
    int mFontIcons;

    /* Spacing-related parameters */
    int mStandardFontSize;
    int mButtonFontSize;
    int mTextBoxFontSize;
    int mWindowCornerRadius;
    int mWindowHeaderHeight;
    int mWindowDropShadowSize;
    int mButtonCornerRadius;
    float mTabBorderWidth;
    int mTabInnerMargin;
    int mTabMinButtonWidth;
    int mTabMaxButtonWidth;
    int mTabControlWidth;
    int mTabButtonHorizontalPadding;
    int mTabButtonVerticalPadding;

    /* Generic colors */
    Color mDropShadow;
    Color mTransparent;
    Color mBorderDark;
    Color mBorderLight;
    Color mBorderMedium;
    Color mTextColor;
    Color mDisabledTextColor;
    Color mTextColorShadow;
    Color mIconColor;

    /* Button colors */
    Color mButtonGradientTopFocused;
    Color mButtonGradientBotFocused;
    Color mButtonGradientTopUnfocused;
    Color mButtonGradientBotUnfocused;
    Color mButtonGradientTopPushed;
    Color mButtonGradientBotPushed;

    /* Window colors */
    Color mWindowFillUnfocused;
    Color mWindowFillFocused;
    Color mWindowTitleUnfocused;
    Color mWindowTitleFocused;

    Color mWindowHeaderGradientTop;
    Color mWindowHeaderGradientBot;
    Color mWindowHeaderSepTop;
    Color mWindowHeaderSepBot;

    Color mWindowPopup;
    Color mWindowPopupTransparent;

protected:
    json mProperties;

protected:
    virtual ~Theme() { };

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename value_type>
value_type Theme::get(const std::string& json_ptr, const value_type& default_value) const {
    try {
        return mProperties.at(json::json_pointer{ json_ptr });
    } catch (std::out_of_range) {
        return default_value;
    }
}

NAMESPACE_END(nanogui)
