/*
    src/label.cpp -- Text label with an arbitrary font, color, and size

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/label.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>

NAMESPACE_BEGIN(nanogui)

Label::Label(Widget *parent, const std::string &caption, const std::string &font, int fontSize)
        : Widget(parent),
          mCaption(caption),
          mFont(font),
          mShowShadow(false),
          mHorizAlign(HAlign::Left)
    {
        if (mTheme)
            mColor = mTheme->get<Color>("/text-color");
        if (fontSize >= 0)
            mFontSize = fontSize;
    }

void Label::setTheme(Theme *theme) {
    Widget::setTheme(theme);
    if (mTheme)
        mColor = mTheme->get<Color>("/text-color");
}

Vector2i Label::preferredSize(NVGcontext *ctx) const {
    if (mCaption == "")
        return Vector2i::Zero();
    nvgFontFace(ctx, mFont.c_str());
    nvgFontSize(ctx, fontSize());
    if (mFixedSize.x() > 0) {
        nvgTextAlign(ctx, (int)mHorizAlign | NVG_ALIGN_TOP);
        float bounds[4];
        nvgTextBoxBounds(ctx, mPos.x(), mPos.y(), mFixedSize.x(), mCaption.c_str(), nullptr, bounds);
        return Vector2i(mFixedSize.x(), bounds[3] - bounds[1]);
    } else {
        nvgTextAlign(ctx, (int)mHorizAlign | NVG_ALIGN_MIDDLE);
        return Vector2i(
            nvgTextBounds(ctx, 0, 0, mCaption.c_str(), nullptr, nullptr) + 2,
            fontSize()
        );
    }
}

void Label::draw(NVGcontext* ctx) {
    Widget::draw(ctx);
    nvgFontFace(ctx, mFont.c_str());
    nvgFontSize(ctx, fontSize());
    if (mFixedSize.x() > 0) {
        nvgTextAlign(ctx, (int)mHorizAlign | NVG_ALIGN_TOP);
        if (mShowShadow) {
            nvgFillColor(ctx, mTheme->get<Color>("/shadow"));
            nvgTextBox(ctx, mPos.x(), mPos.y(), mFixedSize.x(), mCaption.c_str(), nullptr);
        }
        nvgFillColor(ctx, mColor);
        nvgTextBox(ctx, mPos.x(), mPos.y() + 1, mFixedSize.x(), mCaption.c_str(), nullptr);
    } else {
        nvgTextAlign(ctx, (int)mHorizAlign | NVG_ALIGN_MIDDLE);
        if (mShowShadow) {
            nvgFillColor(ctx, mTheme->get<Color>("/shadow"));
            nvgText(ctx, mPos.x(), mPos.y() + mSize.y() * 0.5f, mCaption.c_str(), nullptr);
        }
        nvgFillColor(ctx, mColor);
        nvgText(ctx, mPos.x(), mPos.y() + mSize.y() * 0.5f + 1, mCaption.c_str(), nullptr);
    }
}

void Label::save(Serializer &s) const {
    Widget::save(s);
    s.set("caption", mCaption);
    s.set("font", mFont);
    s.set("color", mColor);
}

bool Label::load(Serializer &s) {
    if (!Widget::load(s)) return false;
    if (!s.get("caption", mCaption)) return false;
    if (!s.get("font", mFont)) return false;
    if (!s.get("color", mColor)) return false;
    return true;
}

NAMESPACE_END(nanogui)
