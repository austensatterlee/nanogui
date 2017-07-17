/*
    src/theme.cpp -- Storage class for basic theme-related properties

    The text box widget was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui_resources.h>

NAMESPACE_BEGIN(nanogui)

Theme::Theme(NVGcontext* ctx)
    : mCtx(ctx) {
    prop("/textbox/text-size") = 20;

    prop("/tab/border/width")     = 0.75f;
    prop("/tab/inner-margin")     = 5;
    prop("/tab/button/min-width") = 20;
    prop("/tab/button/max-width") = 160;
    prop("/tab/control/width")    = 20;
    prop("/tab/button/hpadding")  = 10;
    prop("/tab/button/vpadding")  = 2;

    prop("/text-size")            = 16;
    prop("/text-color")          = Color(255, 160);
    prop("/text-shadow")         = Color(0, 160);
    prop("/disabled-text-color") = Color(255, 80);
    prop("/shadow")              = Color(0, 128);
    prop("/transparent")         = Color(0, 0);
    prop("/icon-color")          = get<Color>("/text-color");

    prop("/border/dark")         = Color(29, 255);
    prop("/border/light")        = Color(92, 255);
    prop("/border/medium")       = Color(35, 255);

    prop("/button/text-size")     = 20;
    prop("/button/corner-radius") = 0;
    prop("/button/focused/grad-top")   = Color(64, 255);
    prop("/button/focused/grad-bot")   = Color(48, 255);
    prop("/button/unfocused/grad-top") = Color(74, 255);
    prop("/button/unfocused/grad-bot") = Color(58, 255);
    prop("/button/pushed/grad-top")    = Color(41, 255);
    prop("/button/pushed/grad-bot")    = Color(29, 255);

    /* Window-related */
    prop("/window/unfocused/fill")  = Color(43, 230);
    prop("/window/unfocused/title") = Color(220, 160);
    prop("/window/focused/fill")    = Color(45, 230);
    prop("/window/focused/title")   = Color(255, 190);

    prop("/window/corner-radius") = 0;
    prop("/window/shadow-size")   = 10;
    prop("/window/header/height") = 30;
    prop("/window/header/grad-top") = get<Color>("/button/unfocused/grad-top");
    prop("/window/header/grad-bot") = get<Color>("/button/unfocused/grad-bot");
    prop("/window/header/sep-top")  = get<Color>("/border/light");
    prop("/window/header/sep-bot")  = get<Color>("/border/dark");

    prop("/popup/fill")        = Color(50, 255);
    prop("/popup/transparent") = Color(50, 0);

    loadFonts();
}

Theme::Theme(NVGcontext* ctx, const json& j)
    : mCtx(ctx) {
    mProperties = j;
    loadFonts();
}

void Theme::loadFonts() {
    prop("/font/normal") = nvgCreateFontMem(mCtx, "sans", roboto_regular_ttf, roboto_regular_ttf_size, 0);
    prop("/font/bold") = nvgCreateFontMem(mCtx, "sans-bold", roboto_bold_ttf, roboto_bold_ttf_size, 0);
    prop("/font/mono") = nvgCreateFontMem(mCtx, "mono", droidsans_mono_ttf, droidsans_mono_ttf_size, 0);
    prop("/font/icons") = nvgCreateFontMem(mCtx, "icons", entypo_ttf, entypo_ttf_size, 0);

    for (const auto& f : prop("/font")) {
        if (f.get<int>() == -1)
            throw std::runtime_error("Could not load fonts!");
    }
}

NAMESPACE_END(nanogui)
