/*
    src/combobox.cpp -- simple combo box widget based on a popup button

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/combobox.h>
#include <nanogui/layout.h>
#include <nanogui/serializer/core.h>
#include <nanogui/vscrollpanel.h>
#include <cassert>

NAMESPACE_BEGIN(nanogui)

ComboBox::ComboBox(Widget *parent) : PopupButton(parent), mSelectedIndex(0) {
    mPopup->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Fill));
    mScrollPanel = new VScrollPanel(mPopup);
    mScrollPanel->setFixedHeight(200);
    mScrollPanelChild = new Widget(mScrollPanel);
    mScrollPanelChild->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Fill));
    mPopup->setDisposable(true);
}

ComboBox::ComboBox(Widget *parent, const std::vector<std::string> &items)
    : ComboBox(parent) {
    setItems(items);
}

ComboBox::ComboBox(Widget *parent, const std::vector<std::string> &items, const std::vector<std::string> &itemsShort)
    : PopupButton(parent), mSelectedIndex(0) {
    setItems(items, itemsShort);
}

void ComboBox::setSelectedIndex(int idx) {
    if (mItemsShort.empty())
        return;
    const std::vector<Widget *> &children = mScrollPanelChild->children();
    ((Button *) children[mSelectedIndex])->setPushed(false);
    ((Button *) children[idx])->setPushed(true);
    mSelectedIndex = idx;
    setCaption(mItemsShort[idx]);
}

void ComboBox::setItems(const std::vector<std::string> &items, const std::vector<std::string> &itemsShort) {
    assert(items.size() == itemsShort.size());
    mItems = items;
    mItemsShort = itemsShort;
    if (mSelectedIndex < 0 || mSelectedIndex >= (int) items.size())
        mSelectedIndex = 0;
    while (mScrollPanelChild->childCount() != 0)
        mScrollPanelChild->removeChild(mScrollPanelChild->childCount()-1);
    mScrollPanelChild->setLayout(new GroupLayout(10));
    int index = 0;
    for (const auto &str: items) {
        Button *button = new Button(mScrollPanelChild, str);
        button->setFlags(RadioButton);
        button->setCallback([&, index] {
            mSelectedIndex = index;
            setCaption(mItemsShort[index]);
            setPushed(false);
            popup()->setVisible(false);
            if (mCallback)
                mCallback(index);
        });
        index++;
    }
    setSelectedIndex(mSelectedIndex);
}

bool ComboBox::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    if (rel.y() < 0) {
        setSelectedIndex(std::min(mSelectedIndex+1, (int)(items().size()-1)));
        if (mCallback)
            mCallback(mSelectedIndex);
        return true;
    } else if (rel.y() > 0) {
        setSelectedIndex(std::max(mSelectedIndex-1, 0));
        if (mCallback)
            mCallback(mSelectedIndex);
        return true;
    }
    return Widget::scrollEvent(p, rel);
}

void ComboBox::save(Serializer &s) const {
    Widget::save(s);
    s.set("items", mItems);
    s.set("itemsShort", mItemsShort);
    s.set("selectedIndex", mSelectedIndex);
}

bool ComboBox::load(Serializer &s) {
    if (!Widget::load(s)) return false;
    if (!s.get("items", mItems)) return false;
    if (!s.get("itemsShort", mItemsShort)) return false;
    if (!s.get("selectedIndex", mSelectedIndex)) return false;
    return true;
}

NAMESPACE_END(nanogui)
