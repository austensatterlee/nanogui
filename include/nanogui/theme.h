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
    value_type setDefault(const std::string& json_ptr, const value_type& default_value);

    /// Retrieve a value using a json pointer. If it doesn't exist, return a default.
    template<typename value_type>
    value_type get(const std::string& json_ptr, const value_type& default_value) const;

    template<typename value_type>
    value_type get(const std::string& json_ptr) const;

    /// Access the json object at a location specified by a json pointer.
    json& prop(const std::string& json_ptr="") {
        return mProperties[json::json_pointer{ json_ptr }];
    }
    const json& prop(const std::string& json_ptr = "") const {
        return mProperties[json::json_pointer{ json_ptr }];
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
    virtual ~Theme() { };

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename value_type>
value_type Theme::setDefault(const std::string& json_ptr, const value_type& default_value) {
    try {
        return mProperties.at(json::json_pointer{ json_ptr });
    } catch (std::out_of_range) {
        mProperties[json::json_pointer{ json_ptr }] = default_value;
        return default_value;
    }
}

template <typename value_type>
value_type Theme::get(const std::string& json_ptr, const value_type& default_value) const {
    try {
        return mProperties.at(json::json_pointer{ json_ptr });
    } catch (std::out_of_range) {
        return default_value;
    }
}

template <typename value_type>
value_type Theme::get(const std::string& json_ptr) const {
    return get(json_ptr, value_type{});
}

NAMESPACE_END(nanogui)
