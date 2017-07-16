/*
    src/popup.cpp -- Simple popup widget which is attached to another given
    window (can be nested)

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/popup.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>
#include "nanogui/popupbutton.h"
#include "nanogui/screen.h"

NAMESPACE_BEGIN(nanogui)

Popup::Popup(Widget *parent, Window *parentWindow, PopupButton *parentButton)
        : Window(parent, ""), mParentWindow(parentWindow), mParentButton(parentButton),
          mAnchorPos(Vector2i::Zero()), mAnchorHeight(30), mSide(Side::Right), mDisposable(false) {}

Popup::Popup(Widget *parent, Window *parentWindow) 
    : Popup(parent, parentWindow, nullptr) {
}

void Popup::performLayout(NVGcontext *ctx) {
    if (mLayout || mChildren.size() != 1) {
        Widget::performLayout(ctx);
    } else {
        mChildren[0]->setPosition(Vector2i::Zero());
        mChildren[0]->setSize(mSize);
        mChildren[0]->performLayout(ctx);
    }
    if (mSide == Side::Left)
        mAnchorPos[0] -= size()[0];
}

void Popup::refreshRelativePlacement() {
    mParentWindow->refreshRelativePlacement();
    mVisible &= mParentWindow->visibleRecursive();
    mPos = mParentWindow->position() + mAnchorPos - Vector2i{0, height()*0.5};
    Vector2i absPos = absolutePosition();
    Vector2i mBottom = absPos + mSize;
    if (mBottom.y() > screen()->height()) {
        mPos.y() -= mBottom.y() - screen()->height();
    }else if (absPos.y() < 0) {
        mPos.y() -= mPos.y();
    }
}

void Popup::draw(NVGcontext* ctx) {

    if (disposable() && !focused() && !mParentWindow->focused()) {
        setVisible(false);
        if (mParentButton) {
            mParentButton->setPushed(false);
            if (mParentButton->changeCallback())
                mParentButton->changeCallback()(false);
        } 
    }

    refreshRelativePlacement();

    if (!mVisible)
        return;

    int ds = mTheme->prop("/window/shadow-size"), cr = mTheme->prop("/window/corner-radius");

    nvgSave(ctx);
    nvgResetScissor(ctx);

    /* Draw a drop shadow */
    NVGpaint shadowPaint = nvgBoxGradient(
        ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr*2, ds*2,
        mTheme->get<Color>("/shadow"), mTheme->get<Color>("/transparent"));

    nvgBeginPath(ctx);
    nvgRect(ctx, mPos.x()-ds,mPos.y()-ds, mSize.x()+2*ds, mSize.y()+2*ds);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);
    nvgPathWinding(ctx, NVG_HOLE);
    nvgFillPaint(ctx, shadowPaint);
    nvgFill(ctx);

    /* Draw window */
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);

    Vector2i btnPos = mParentButton->absolutePosition();
    Vector2i base = mPos + Vector2i{ 0,height()*0.5f };
    int sign = (mSide == Left) ? 0 : 1;

    nvgMoveTo(ctx, btnPos.x() + sign*mParentButton->width(), btnPos.y()+mParentButton->height()*0.5);
    nvgLineTo(ctx, mPos.x() + (1-sign)*width(), std::min(base.y() + 15.0f, static_cast<float>(mPos.y() + height())));
    nvgLineTo(ctx, mPos.x() + (1-sign)*width(), std::max(base.y() - 15.0f, static_cast<float>(mPos.y())));

    nvgFillColor(ctx, mTheme->get<Color>("/popup/fill"));
    nvgFill(ctx);
    nvgRestore(ctx);    

    Widget::draw(ctx);
}

void Popup::save(Serializer &s) const {
    Window::save(s);
    s.set("anchorPos", mAnchorPos);
    s.set("anchorHeight", mAnchorHeight);
    s.set("side", mSide);
}

bool Popup::load(Serializer &s) {
    if (!Window::load(s)) return false;
    if (!s.get("anchorPos", mAnchorPos)) return false;
    if (!s.get("anchorHeight", mAnchorHeight)) return false;
    if (!s.get("side", mSide)) return false;
    return true;
}

NAMESPACE_END(nanogui)
