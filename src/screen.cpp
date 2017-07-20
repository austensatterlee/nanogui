/*
    src/screen.cpp -- Top-level widget and interface between NanoGUI and GLFW

    A significant redesign of this code was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/screen.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/window.h>
#include <nanogui/popup.h>
#include <map>
#include <iostream>

#if defined(_WIN32)
#  define NOMINMAX
#  undef APIENTRY

#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#  define GLFW_EXPOSE_NATIVE_WGL
#  define GLFW_EXPOSE_NATIVE_WIN32
#  include <GLFW/glfw3native.h>
#endif

/* Allow enforcing the GL2 implementation of NanoVG */
#include <nanovg_gl.h>

NAMESPACE_BEGIN(nanogui)

std::map<GLFWwindow *, Screen *> __nanogui_screens;

#if defined(NANOGUI_GLAD)
static bool gladInitialized = false;
#endif

inline NVGcontext* nvgCreateContext(int flags) {
#if defined(NANOVG_GL2_IMPLEMENTATION)
    return nvgCreateGL2(flags);
#elif defined(NANOVG_GL3_IMPLEMENTATION)
    return nvgCreateGL3(flags);
#elif defined(NANOVG_GLES2_IMPLEMENTATION)
    return nvgCreateGLES2(flags);
#elif defined(NANOVG_GLES3_IMPLEMENTATION)
    return nvgCreateGLES3(flags);
#else
#error No NANOVG_GL*_IMPLEMENTATION macro defined
#endif
}
inline void nvgDeleteContext(NVGcontext* ctx) {
#if defined(NANOVG_GL2_IMPLEMENTATION)
    nvgDeleteGL2(ctx);
#elif defined(NANOVG_GL3_IMPLEMENTATION)
    nvgDeleteGL3(ctx);
#elif defined(NANOVG_GLES2_IMPLEMENTATION)
    nvgDeleteGLES2(ctx);
#elif defined(NANOVG_GLES3_IMPLEMENTATION)
    nvgDeleteGLES3(ctx);
#else
#error No NANOVG_GL*_IMPLEMENTATION macro defined
#endif
}

/* Calculate pixel ratio for hi-dpi devices. */
static float get_pixel_ratio(GLFWwindow *window) {
#if defined(_WIN32)
    HWND hWnd = glfwGetWin32Window(window);
    HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    /* The following function only exists on Windows 8.1+, but we don't want to make that a dependency */
    static HRESULT (WINAPI *GetDpiForMonitor_)(HMONITOR, UINT, UINT*, UINT*) = nullptr;
    static bool GetDpiForMonitor_tried = false;

    if (!GetDpiForMonitor_tried) {
        auto shcore = LoadLibrary(TEXT("shcore"));
        if (shcore)
            GetDpiForMonitor_ = (decltype(GetDpiForMonitor_)) GetProcAddress(shcore, "GetDpiForMonitor");
        GetDpiForMonitor_tried = true;
    }

    if (GetDpiForMonitor_) {
        uint32_t dpiX, dpiY;
        if (GetDpiForMonitor_(monitor, 0 /* effective DPI */, &dpiX, &dpiY) == S_OK)
            return std::round(dpiX / 96.0);
    }
    return 1.f;
#elif defined(__linux__)
    (void) window;

    /* Try to read the pixel ratio from GTK */
    FILE *fp = popen("gsettings get org.gnome.desktop.interface scaling-factor", "r");
    if (!fp)
        return 1;

    int ratio = 1;
    if (fscanf(fp, "uint32 %i", &ratio) != 1)
        return 1;

    if (pclose(fp) != 0)
        return 1;

    return ratio >= 1 ? ratio : 1;
#else
    Vector2i fbSize, size;
    glfwGetFramebufferSize(window, &fbSize[0], &fbSize[1]);
    glfwGetWindowSize(window, &size[0], &size[1]);
    return (float)fbSize[0] / (float)size[0];
#endif
}

Screen::Screen()
    : Widget(nullptr), mGLFWWindow(nullptr), mNVGContext(nullptr),
#if !defined(NANOGUI_CURSOR_DISABLED)
       mCursor(Cursor::Arrow),
#endif
       mBackground(0.3f, 0.3f, 0.32f, 1.f),
      mShutdownGLFWOnDestruct(false), mFullscreen(false), mFPS(0.0) {
#if !defined(NANOGUI_CURSOR_DISABLED)
    memset(mCursors, 0, sizeof(GLFWcursor *) * (int) Cursor::CursorCount);
#endif
}

Screen::Screen(const Vector2i &size, const std::string &caption, bool resizable,
               bool fullscreen, int colorBits, int alphaBits, int depthBits,
               int stencilBits, int nSamples,
               unsigned int glMajor, unsigned int glMinor)
    : Widget(nullptr), mGLFWWindow(nullptr), mNVGContext(nullptr),
#if !defined(NANOGUI_CURSOR_DISABLED)
      mCursor(Cursor::Arrow),
#endif
      mBackground(0.3f, 0.3f, 0.32f, 1.f), mCaption(caption),
      mShutdownGLFWOnDestruct(false), mFullscreen(fullscreen), mFPS(0.0) {
#if !defined(NANOGUI_CURSOR_DISABLED)
    memset(mCursors, 0, sizeof(GLFWcursor *) * (int) Cursor::CursorCount);
#endif

    /* Request a forward compatible OpenGL glMajor.glMinor core profile context.
       Default value is an OpenGL 3.3 core profile context. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, nSamples);
    glfwWindowHint(GLFW_RED_BITS, colorBits);
    glfwWindowHint(GLFW_GREEN_BITS, colorBits);
    glfwWindowHint(GLFW_BLUE_BITS, colorBits);
    glfwWindowHint(GLFW_ALPHA_BITS, alphaBits);
    glfwWindowHint(GLFW_STENCIL_BITS, stencilBits);
    glfwWindowHint(GLFW_DEPTH_BITS, depthBits);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);

    if (fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        mGLFWWindow = glfwCreateWindow(mode->width, mode->height,
                                       caption.c_str(), monitor, nullptr);
    } else {
        mGLFWWindow = glfwCreateWindow(size.x(), size.y(),
                                       caption.c_str(), nullptr, nullptr);
    }

    if (!mGLFWWindow)
        throw std::runtime_error("Could not create an OpenGL " +
                                 std::to_string(glMajor) + "." +
                                 std::to_string(glMinor) + " context!");

    glfwMakeContextCurrent(mGLFWWindow);

#if defined(NANOGUI_GLAD)
    if (!gladInitialized) {
        gladInitialized = true;
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
            throw std::runtime_error("Could not initialize GLAD!");
        glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
    }
#endif

    glfwGetFramebufferSize(mGLFWWindow, &mFBSize[0], &mFBSize[1]);
    glViewport(0, 0, mFBSize[0], mFBSize[1]);
    glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glfwSwapInterval(0);
    glfwSwapBuffers(mGLFWWindow);

#if defined(__APPLE__)
    /* Poll for events once before starting a potentially
       lengthy loading process. This is needed to be
       classified as "interactive" by other software such
       as iTerm2 */

    glfwPollEvents();
#endif

    /* Propagate GLFW events to the appropriate Screen instance */
    glfwSetCursorPosCallback(mGLFWWindow,
        [](GLFWwindow *w, double x, double y) {
            auto it = __nanogui_screens.find(w);
            if (it == __nanogui_screens.end())
                return;
            Screen *s = it->second;
            if (!s->mProcessEvents)
                return;
            s->cursorPosCallbackEvent(x, y);
        }
    );

    glfwSetMouseButtonCallback(mGLFWWindow,
        [](GLFWwindow *w, int button, int action, int modifiers) {
            auto it = __nanogui_screens.find(w);
            if (it == __nanogui_screens.end())
                return;
            Screen *s = it->second;
            if (!s->mProcessEvents)
                return;
            s->mouseButtonCallbackEvent(button, action, modifiers);
        }
    );

    glfwSetKeyCallback(mGLFWWindow,
        [](GLFWwindow *w, int key, int scancode, int action, int mods) {
            auto it = __nanogui_screens.find(w);
            if (it == __nanogui_screens.end())
                return;
            Screen *s = it->second;
            if (!s->mProcessEvents)
                return;
            s->keyCallbackEvent(key, scancode, action, mods);
        }
    );

    glfwSetCharCallback(mGLFWWindow,
        [](GLFWwindow *w, unsigned int codepoint) {
            auto it = __nanogui_screens.find(w);
            if (it == __nanogui_screens.end())
                return;
            Screen *s = it->second;
            if (!s->mProcessEvents)
                return;
            s->charCallbackEvent(codepoint);
        }
    );
 
#if !defined(NANOVG_GL2_IMPLEMENTATION) && !defined(NANOVG_GLES2_IMPLEMENTATION)
    glfwSetDropCallback(mGLFWWindow,
        [](GLFWwindow *w, int count, const char **filenames) {
            auto it = __nanogui_screens.find(w);
            if (it == __nanogui_screens.end())
                return;
            Screen *s = it->second;
            if (!s->mProcessEvents)
                return;
            s->dropCallbackEvent(count, filenames);
        }
    );
#endif

    glfwSetScrollCallback(mGLFWWindow,
        [](GLFWwindow *w, double x, double y) {
            auto it = __nanogui_screens.find(w);
            if (it == __nanogui_screens.end())
                return;
            Screen *s = it->second;
            if (!s->mProcessEvents)
                return;
            s->scrollCallbackEvent(x, y);
        }
    );

    /* React to framebuffer size events -- includes window
       size events and also catches things like dragging
       a window from a Retina-capable screen to a normal
       screen on Mac OS X */
    glfwSetFramebufferSizeCallback(mGLFWWindow,
        [](GLFWwindow* w, int width, int height) {
            auto it = __nanogui_screens.find(w);
            if (it == __nanogui_screens.end())
                return;
            Screen* s = it->second;

            if (!s->mProcessEvents)
                return;

            s->resizeCallbackEvent(width, height);
        }
    );

    initialize(mGLFWWindow, true);
}

void Screen::initialize(GLFWwindow *window, bool shutdownGLFWOnDestruct) {
    deinitialize();
    mGLFWWindow = window;
    mShutdownGLFWOnDestruct = shutdownGLFWOnDestruct;
    glfwGetWindowSize(mGLFWWindow, &mSize[0], &mSize[1]);
    glfwGetFramebufferSize(mGLFWWindow, &mFBSize[0], &mFBSize[1]);

    mPixelRatio = get_pixel_ratio(window);

#if defined(_WIN32) || defined(__linux__)
    if (mPixelRatio != 1 && !mFullscreen)
        glfwSetWindowSize(window, mSize.x() * mPixelRatio, mSize.y() * mPixelRatio);
#endif

#if defined(NANOGUI_GLAD)
    if (!gladInitialized) {
        gladInitialized = true;
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("Could not initialize GLAD!");
        glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
    }
#endif

    /* Detect framebuffer properties and set up compatible NanoVG context */
    GLint nStencilBits = 0, nSamples = 0;
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER,
        GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &nStencilBits);
    glGetIntegerv(GL_SAMPLES, &nSamples);

    int flags = 0;
    if (nStencilBits >= 8)
       flags |= NVG_STENCIL_STROKES;
    if (nSamples <= 1)
       flags |= NVG_ANTIALIAS;
#if !defined(NDEBUG)
    flags |= NVG_DEBUG;
#endif

    mNVGContext = nvgCreateContext(flags);
    if (mNVGContext == nullptr){
        throw std::runtime_error("Could not initialize NanoVG!");
    }

    mVisible = glfwGetWindowAttrib(window, GLFW_VISIBLE) != 0;
    setTheme(new Theme(mNVGContext));
    mMousePos = Vector2i::Zero();
    mMouseState = mModifiers = 0;
    mDragActive = false;
    mLastInteraction = glfwGetTime();
    mLastMouseDown = glfwGetTime();
    mProcessEvents = true;
    __nanogui_screens[mGLFWWindow] = this;

#if !defined(NANOGUI_CURSOR_DISABLED)
    for (int i=0; i < (int) Cursor::CursorCount; ++i)
        mCursors[i] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR + i);
#endif

    /// Fixes retina display-related font rendering issue (#185)
    nvgBeginFrame(mNVGContext, mSize[0], mSize[1], mPixelRatio);
    nvgEndFrame(mNVGContext);
}

void Screen::deinitialize() {
#if !defined(NANOGUI_CURSOR_DISABLED)
    for (int i=0; i < (int) Cursor::CursorCount; ++i) {
        if (mCursors[i]){
            glfwDestroyCursor(mCursors[i]);
            mCursors[i] = nullptr;
        }
    }
#endif
    if (mNVGContext){
        nvgDeleteContext(mNVGContext);
        mNVGContext = nullptr;
    }
    if(mGLFWWindow){
        __nanogui_screens.erase(mGLFWWindow);
        if (mShutdownGLFWOnDestruct)
            glfwDestroyWindow(mGLFWWindow);
        mGLFWWindow = nullptr;
    }
}

Screen::~Screen() {
    deinitialize();
}

void Screen::setVisible(bool visible) {
    if (mVisible != visible) {
        mVisible = visible;

        if (visible)
            glfwShowWindow(mGLFWWindow);
        else
            glfwHideWindow(mGLFWWindow);
    }
}

void Screen::setCaption(const std::string &caption) {
    if (caption != mCaption) {
        glfwSetWindowTitle(mGLFWWindow, caption.c_str());
        mCaption = caption;
    }
}

void Screen::setSize(const Vector2i &size) {
    Widget::setSize(size);

#if defined(_WIN32) || defined(__linux__)
    glfwSetWindowSize(mGLFWWindow, size.x() * mPixelRatio, size.y() * mPixelRatio);
#else
    glfwSetWindowSize(mGLFWWindow, size.x(), size.y());
#endif
}

void Screen::drawAll() {    
    double cpuStartTime = glfwGetTime();

    glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawContents();
    drawWidgets();

    glfwSwapBuffers(mGLFWWindow);
    
    float dCpuTime = glfwGetTime() - cpuStartTime;
    float fps = 1. / dCpuTime;
    mFPS = mFPS + 0.0175 * (fps - mFPS);
}

void Screen::drawWidgets() {
    if (!mVisible)
        return;

    glfwMakeContextCurrent(mGLFWWindow);

    glfwGetFramebufferSize(mGLFWWindow, &mFBSize[0], &mFBSize[1]);
    glfwGetWindowSize(mGLFWWindow, &mSize[0], &mSize[1]);

#if defined(_WIN32) || defined(__linux__)
    mSize = (mSize / mPixelRatio).cast<int>();
    mFBSize = (mSize * mPixelRatio).cast<int>();
#else
    /* Recompute pixel ratio on OSX */
    if (mSize[0])
        mPixelRatio = (float) mFBSize[0] / (float) mSize[0];
#endif

    glViewport(0, 0, mFBSize[0], mFBSize[1]);
#if !defined(NANOVG_GL2_IMPLEMENTATION) && !defined(NANOVG_GLES2_IMPLEMENTATION)
    glBindSampler(0, 0);
#endif
    nvgBeginFrame(mNVGContext, mSize[0], mSize[1], mPixelRatio);

    draw(mNVGContext);

    double elapsed = glfwGetTime() - mLastInteraction;

    if (elapsed > 0.0125f) {
        /* Draw tooltips */
        const Widget *widget = findWidget(mMousePos);
        if (widget && !widget->tooltip().empty()) {
            int tooltipWidth = 150;

            float bounds[4];
            nvgFontFace(mNVGContext, "sans");
            nvgFontSize(mNVGContext, 15.0f);
            nvgTextAlign(mNVGContext, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgTextLineHeight(mNVGContext, 1.1f);
            Vector2i pos = widget->absolutePosition() +
                           Vector2i(widget->width() / 2, widget->height() + 10);

            nvgTextBounds(mNVGContext, pos.x(), pos.y(),
                            widget->tooltip().c_str(), nullptr, bounds);
            int h = (bounds[2] - bounds[0]) / 2;
            if (h > tooltipWidth / 2) {
                nvgTextAlign(mNVGContext, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
                nvgTextBoxBounds(mNVGContext, pos.x(), pos.y(), tooltipWidth,
                                widget->tooltip().c_str(), nullptr, bounds);

                h = (bounds[2] - bounds[0]) / 2;
            }
            nvgGlobalAlpha(mNVGContext,
                           std::min(1.0, 2 * (elapsed - 0.5f)) * 0.8);

            nvgBeginPath(mNVGContext);
            nvgFillColor(mNVGContext, Color(0, 255));
            nvgRoundedRect(mNVGContext, bounds[0] - 4 - h, bounds[1] - 4,
                           (int) (bounds[2] - bounds[0]) + 8,
                           (int) (bounds[3] - bounds[1]) + 8, 3);

            int px = (int) ((bounds[2] + bounds[0]) / 2) - h;
            nvgMoveTo(mNVGContext, px, bounds[1] - 10);
            nvgLineTo(mNVGContext, px + 7, bounds[1] + 1);
            nvgLineTo(mNVGContext, px - 7, bounds[1] + 1);
            nvgFill(mNVGContext);

            nvgFillColor(mNVGContext, Color(255, 255));
            nvgFontBlur(mNVGContext, 0.0f);
            nvgTextBox(mNVGContext, pos.x() - h, pos.y(), tooltipWidth,
                       widget->tooltip().c_str(), nullptr);
        }
    }

    nvgEndFrame(mNVGContext);
}

bool Screen::keyboardEvent(int key, int scancode, int action, int modifiers) {
    if (mFocusPath.size() > 0) {
        for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
            if ((*it)->focused() && (*it)->keyboardEvent(key, scancode, action, modifiers))
                return true;
    }

    return false;
}

bool Screen::keyboardCharacterEvent(unsigned int codepoint) {
    if (mFocusPath.size() > 0) {
        for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
            if ((*it)->focused() && (*it)->keyboardCharacterEvent(codepoint))
                return true;
    }
    return false;
}

bool Screen::resizeEvent(const Vector2i& size) {
    if (mResizeCallback) {
        mResizeCallback(size);
        return true;
    }
    return false;
}

bool Screen::cursorPosCallbackEvent(double x, double y) {
    Vector2i p((int) x, (int) y);

#if defined(_WIN32) || defined(__linux__)
    p /= mPixelRatio;
#endif

    bool ret = false;
    mLastInteraction = glfwGetTime();
    try {
        p -= Vector2i(1, 2);

        if (!mDragActive) {
#if !defined(NANOGUI_CURSOR_DISABLED)
            Widget *widget = findWidget(p);
            if (widget != nullptr && widget->cursor() != mCursor) {
                mCursor = widget->cursor();
                glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
            }
#endif
        } else {
            ret = mDragWidget->mouseDragEvent(
                p - mDragWidget->parent()->absolutePosition(), p - mMousePos,
                mMouseState, mModifiers);
        }

        if (!ret) {
            updateMouseFocus(p);
            ret = mouseMotionEvent(p, p - mMousePos, mMouseState, mModifiers);
        }

        mMousePos = p;

        return ret;
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        return false;
    }
}

bool Screen::mouseButtonCallbackEvent(int button, int action, int modifiers) {
    mModifiers = modifiers;
    mLastInteraction = glfwGetTime();
    try {
        if (mFocusPath.size() > 1) {
            const Window *window =
                dynamic_cast<Window *>(mFocusPath[mFocusPath.size() - 2]);
            if (window && window->modal()) {
                if (!window->contains(mMousePos))
                    return false;
            }
        }

        if (action == GLFW_PRESS)
            mMouseState |= 1 << button;
        else
            mMouseState &= ~(1 << button);

        // Detect double clicks
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                if (mLastMouseDown >= 0 && (glfwGetTime() - mLastMouseDown) > 0.2)
                    mLastMouseDown = -1;
                if (glfwGetTime() - mLastMouseDown < 0.2) {
                    mModifiers |= GLFW_MOD_DOUBLE_CLICK;
                    mLastMouseDown = -1;
                } else {
                    mLastMouseDown = glfwGetTime();
                }
            }
        }

        auto dropWidget = findWidget(mMousePos, [](const Widget* w) { return w->draggable(); });
        if (mDragActive && action == GLFW_RELEASE && dropWidget!=mDragWidget)
            mDragWidget->mouseButtonEvent(
                mMousePos - mDragWidget->parent()->absolutePosition(), button,
                false, mModifiers);

        // refresh drop widget in case it was deleted during the mouse button event
#if !defined(NANOGUI_CURSOR_DISABLED)
        dropWidget = findWidget(mMousePos, [](const Widget* w) { return w->draggable(); });
        if (dropWidget != nullptr && dropWidget->cursor() != mCursor) {
            mCursor = dropWidget->cursor();
            glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
        }
#endif

        bool isDblClick = mModifiers & GLFW_MOD_DOUBLE_CLICK;
        if (!isDblClick && action == GLFW_PRESS && (button == GLFW_MOUSE_BUTTON_1 || button == GLFW_MOUSE_BUTTON_2)) {
            mDragWidget = findWidget(mMousePos, [](const Widget* w) { return w->draggable(); });
            if (mDragWidget == this)
                mDragWidget = nullptr;
            mDragActive = mDragWidget != nullptr;
            if (!mDragActive)
                updateFocus(nullptr);
        } else {
            mDragActive = false;
            mDragWidget = nullptr;
        }

        return mouseButtonEvent(mMousePos, button, action == GLFW_PRESS,
                                mModifiers);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        return false;
    }
}

bool Screen::keyCallbackEvent(int key, int scancode, int action, int mods) {
    mLastInteraction = glfwGetTime();
    try {
        return keyboardEvent(key, scancode, action, mods);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        return false;
    }
}

bool Screen::charCallbackEvent(unsigned int codepoint) {
    mLastInteraction = glfwGetTime();
    try {
        return keyboardCharacterEvent(codepoint);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        return false;
    }
}

bool Screen::dropCallbackEvent(int count, const char **filenames) {
    std::vector<std::string> arg(count);
    for (int i = 0; i < count; ++i)
        arg[i] = filenames[i];
    return dropEvent(arg);
}

bool Screen::scrollCallbackEvent(double x, double y) {
    mLastInteraction = glfwGetTime();
    try {
        if (mFocusPath.size() > 1) {
            const Window *window =
                dynamic_cast<Window *>(mFocusPath[mFocusPath.size() - 2]);
            if (window && window->modal()) {
                if (!window->contains(mMousePos))
                    return false;
            }
        }
        return scrollEvent(mMousePos, Vector2f(x, y));
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        return false;
    }
}

bool Screen::resizeCallbackEvent(int, int) {
    Vector2i fbSize, size;
    glfwGetFramebufferSize(mGLFWWindow, &fbSize[0], &fbSize[1]);
    glfwGetWindowSize(mGLFWWindow, &size[0], &size[1]);

#if defined(_WIN32) || defined(__linux__)
    size /= mPixelRatio;
#endif

    if (mFBSize == Vector2i(0, 0) || size == Vector2i(0, 0))
        return false;

    mFBSize = fbSize; mSize = size;
    mLastInteraction = glfwGetTime();

    try {
        return resizeEvent(mSize);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        return false;
    }
}

void Screen::updateFocus(Widget *widget) {
    // Construct new focus path
    Window *window = widget ? widget->window() : nullptr;
    std::vector<Widget*> newFocusPath;
    while (widget && widget->parent()) {
        newFocusPath.push_back(widget);
        widget = widget->parent();
    }

    std::vector<Widget*> oldFocusPath = mFocusPath;
    for (auto w : oldFocusPath) {
        // Don't send a de-focus event to widgets that are also in the new focus path.
        if (std::find(newFocusPath.begin(), newFocusPath.end(), w) != newFocusPath.end())
            continue;
        w->focusEvent(false);
    }

    for (auto it = newFocusPath.rbegin(); it != newFocusPath.rend(); ++it) {
        // Don't send a focus event to widgets that are already focused.
        Widget *w = *it;
        if (std::find(oldFocusPath.begin(), oldFocusPath.end(), w) != oldFocusPath.end())
            continue;
        w->focusEvent(true);
    }

    mFocusPath = newFocusPath;

    if (window && !window->isBackgroundWindow())
        moveWindowToFront(window);
}

void Screen::updateMouseFocus(const Vector2i& p) {
    std::vector<Widget*> newMouseFocusPath;

    Widget* widget = findWidget(p);
    while (widget) {
        newMouseFocusPath.push_back(widget);
        widget = widget->parent();
    }

    std::vector<Widget*> oldMouseFocusPath = mMouseFocusPath;
    for (auto w : oldMouseFocusPath) {
        // Don't send a de-focus event to widget's that are also in the new focus path.
        if (std::find(newMouseFocusPath.begin(), newMouseFocusPath.end(), w) != newMouseFocusPath.end())
            continue;
        w->mouseEnterEvent(p - w->absolutePosition(), false);
    }

    for (auto it = newMouseFocusPath.rbegin(); it != newMouseFocusPath.rend(); ++it) {
        // Don't send a focus event to widget's that were in the old focus path.
        Widget *w = *it;
        if (std::find(oldMouseFocusPath.begin(), oldMouseFocusPath.end(), w) != oldMouseFocusPath.end())
            continue;
        w->mouseEnterEvent(p - w->absolutePosition(), true);
    }

    mMouseFocusPath = newMouseFocusPath;
}

void Screen::disposeWindow(Window *window) {
    if (std::find(mFocusPath.begin(), mFocusPath.end(), window) != mFocusPath.end())
        mFocusPath.clear();
    if (std::find(mMouseFocusPath.begin(), mMouseFocusPath.end(), window) != mMouseFocusPath.end())
        mMouseFocusPath.clear();
    if (mDragWidget == window)
        mDragWidget = nullptr;
    removeChild(window);
}

void Screen::centerWindow(Window *window) {
    if (window->size() == Vector2i::Zero()) {
        window->setSize(window->preferredSize(mNVGContext));
        window->performLayout(mNVGContext);
    }
    window->setPosition((mSize - window->size()) / 2);
}

void Screen::moveWindowToFront(Window *window) {
    mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), window), mChildren.end());
    mChildren.push_back(window);
    /* Brute force topological sort (no problem for a few windows..) */
    bool changed = false;
    do {
        size_t baseIndex = 0;
        for (size_t index = 0; index < mChildren.size(); ++index)
            if (mChildren[index] == window)
                baseIndex = index;
        changed = false;
        for (size_t index = 0; index < mChildren.size(); ++index) {
            Popup *pw = dynamic_cast<Popup *>(mChildren[index]);
            if (pw && pw->parentWindow() == window && index < baseIndex) {
                moveWindowToFront(pw);
                changed = true;
                break;
            }
        }
    } while (changed);
}

NAMESPACE_END(nanogui)
