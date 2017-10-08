/*
    nanogui/label.h -- Text label with an arbitrary font, color, and size

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <nanogui/widget.h>

NAMESPACE_BEGIN(nanogui)

/**
 * \class Label label.h nanogui/label.h
 *
 * \brief Text label widget.
 *
 * The font and color can be customized. When \ref Widget::setFixedWidth()
 * is used, the text is wrapped when it surpasses the specified width.
 */
class NANOGUI_EXPORT Label : public Widget {
public:

    /// How to align the text in the label.
    enum class HAlign {
        Left = 1 << 0,
        Center = 1 << 1,
        Right = 1 << 2
    };

    enum class VAlign {
        Top = 1 << 3,
        Middle = 1 << 4,
        Bottom = 1 << 5
    };

    Label(Widget *parent, const std::string &caption,
          const std::string &font = "sans", int fontSize = -1);

    /// Get the label's text caption
    const std::string &caption() const { return mCaption; }
    /// Set the label's text caption
    void setCaption(const std::string &caption) { mCaption = caption; }

    /// Set the currently active font (2 are available by default: 'sans' and 'sans-bold')
    void setFont(const std::string &font) { mFont = font; }
    /// Get the currently active font
    const std::string &font() const { return mFont; }

    /// Get the label color
    Color color() const { return mColor; }
    /// Set the label color
    void setColor(const Color& color) { mColor = color; }

    /// Check if the shadow is being drawn
    bool showShadow() const { return mShowShadow; }
    /// Specify if the shadow should be drawn
    void setShowShadow(bool showShadow) { mShowShadow = showShadow; }

    /// Set the label's horizontal text alignment
    void setHorizAlign(HAlign align) { mHorizAlign = align; }
    /// Get the label's horizontal text alignment
    HAlign horizAlign() const { return mHorizAlign; }
    /// Set the label's vertical text alignment
    void setVertAlign(VAlign align) { mVertAlign = align; }
    /// Get the label's vertical text alignment
    VAlign vertAlign() const { return mVertAlign; }

    /// Compute the size needed to fully display the label
    virtual Vector2i preferredSize(NVGcontext *ctx) const override;

    /// Draw the label
    virtual void draw(NVGcontext *ctx) override;

    virtual void save(Serializer &s) const override;
    virtual bool load(Serializer &s) override;
protected:
    std::string mCaption;
    std::string mFont;
    Color mColor;

    bool mShowShadow;
    HAlign mHorizAlign;
    VAlign mVertAlign;
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

NAMESPACE_END(nanogui)
