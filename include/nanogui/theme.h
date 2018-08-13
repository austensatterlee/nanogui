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

    /// Retrieve a value using a json pointer. Create it first if it doesn't exist.
    template<typename value_type>
    value_type setDefault(const std::string& a_key, const value_type& a_default);

    /// Retrieve a value using a json pointer. If it doesn't exist, return a default.
    template<typename value_type>
    value_type get(const std::string& a_key, const value_type& a_default) const;

    template<typename value_type>
    value_type get(const std::string& a_key) const;

    /// Access the json object at a location specified by a json pointer.
    json& prop(const std::string& a_key="") {
        return mProperties[a_key];
    }
    const json& prop(const std::string& a_key = "") const {
        return mProperties[a_key];
    }

    /// Convert to a json object.
    operator json() const { return mProperties; }

    /**
     * Update with the items of `j`. If a key already exists, overwrite it with
     * the value from `j`.
     */
    void update(const json& j);

protected:
    json mProperties;
    NVGcontext *mCtx;

protected:
    virtual ~Theme() = default;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename value_type>
value_type Theme::setDefault(const std::string& a_key, const value_type& a_default) {
    try {
        return mProperties.at(a_key);
    } catch (std::out_of_range&) {
        mProperties[a_key] = a_default;
        return a_default;
    }
}

template <typename value_type>
value_type Theme::get(const std::string& a_key, const value_type& a_default) const {
    try {
        return mProperties.at(a_key);
    } catch (std::out_of_range&) {
        return a_default;
    }
}

template <typename value_type>
value_type Theme::get(const std::string& a_key) const {
    return get(a_key, value_type{});
}

NAMESPACE_END(nanogui)
