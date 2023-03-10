#include "imgui_impl_qt.h"
#include <QtCore/QObject>
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLWindow>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include <QtCore/QDateTime>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QInputMethodEvent>

#include "imgui.h"
#include <memory>

class ImGui_ImplQt_IWindow {
public:
    virtual ~ImGui_ImplQt_IWindow() = default;

    virtual void sizeInfo(int& w, int& h, int& display_w, int& display_h) const = 0;
    virtual bool  isActive() const = 0;
    virtual QObject* object() = 0;

    virtual void setCursor(Qt::CursorShape shape) = 0;
    virtual void setCursorPos(const QPoint& local_pos) = 0;
};

template<typename T>
class ImGui_ImplQt_Window :public ImGui_ImplQt_IWindow {
public:
    using Super = ImGui_ImplQt_Window;
public:
    explicit ImGui_ImplQt_Window(T* target)
        :window(target) {};

    void sizeInfo(int& w, int& h, int& display_w, int& display_h) const override
    {
        if (!window) return;
        auto size = window->size();
        w = size.width();
        h = size.height();

        display_w = window->devicePixelRatio() * w;
        display_h = window->devicePixelRatio() * h;
    }

    QObject* object() override {
        return window;
    }

    void setCursorPos(const QPoint& local_pos) override {
        const QPoint global_pos = window->mapToGlobal(local_pos);
        QCursor cursor = window->cursor();
        cursor.setPos(global_pos);
        window->setCursor(cursor);
    }

    void setCursor(Qt::CursorShape shape) override {
        window->setCursor(shape);
    }
protected:
    T* window{};
};

class ImGui_ImplQt_OpenGLWidget final :public ImGui_ImplQt_Window<QOpenGLWidget> {
public:
    using Super::Super;
    bool  isActive() const override {
        return window->isActiveWindow();
    }
};

class ImGui_ImplQt_OpenGLWindow final :public ImGui_ImplQt_Window<QOpenGLWindow> {
public:
    using Super::Super;

    bool  isActive() const override {
        return window->isActive();
    }
};

class ImGui_ImplQt :public QObject
{
public:
    std::unique_ptr<ImGui_ImplQt_IWindow> Window{};

    double         Time{};
    bool           WantUpdateMonitors{};
public:
    bool  Init(ImGuiIO& io, std::unique_ptr<ImGui_ImplQt_IWindow> window);
    void  NewFrame(ImGuiIO& io);
private:
    void  UpdateMouseData(ImGuiIO& io);
    void  UpdateCursorShape(ImGuiIO& io, ImGui_ImplQt_IWindow* window);
    bool  eventFilter(QObject* watched, QEvent* event) override;
private:
    ImGuiContext* Context{};
};

static ImGui_ImplQt* ImGui_ImplQt_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplQt*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

static const char* ImGui_ImplQt_GetClipboardText(void* user_data)
{
    QByteArray buffer = QGuiApplication::clipboard()->text().toUtf8();
    return (const char*)buffer.data();
}

static void ImGui_ImplQt_SetClipboardText(void* user_data, const char* text)
{
    QGuiApplication::clipboard()->setText(text);
}

bool ImGui_ImplQt_Init(QOpenGLWidget* window)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    ImGui_ImplQt* bd = IM_NEW(ImGui_ImplQt)();
    if (bd->Init(io, std::make_unique<ImGui_ImplQt_OpenGLWidget>(window))) {
        window->installEventFilter(bd);
        //设置为接收输入消息，鼠标追踪开启以正确更新鼠标位置
        window->setAttribute(Qt::WA_InputMethodEnabled);
        window->setMouseTracking(true);
        return true;
    }
    return false;
}

bool ImGui_ImplQt_Init(QOpenGLWindow* window)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    ImGui_ImplQt* bd = IM_NEW(ImGui_ImplQt)();
    if (bd->Init(io, std::make_unique<ImGui_ImplQt_OpenGLWindow>(window))) {
        window->installEventFilter(bd);
        //设置为接收输入消息，鼠标追踪开启以正确更新鼠标位置
        //window->setAttribute(Qt::WA_InputMethodEnabled);
        //window->setMouseTracking(true);
        return true;
    }
    return false;
}

void ImGui_ImplQt_Shutdown()
{
    ImGui_ImplQt* bd = ImGui_ImplQt_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");

    ImGuiIO& io = ImGui::GetIO();

    //ImGui_ImplQt_ShutdownPlatformInterface();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    IM_DELETE(bd);
}

void ImGui_ImplQt_NewFrame()
{
    ImGui_ImplQt* bd = ImGui_ImplQt_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplQt_Init()?");
    ImGuiIO& io = ImGui::GetIO();
    bd->NewFrame(io);
}

bool ImGui_ImplQt::Init(ImGuiIO& io, std::unique_ptr<ImGui_ImplQt_IWindow> window)
{
    {//记录Context以提供给eventFilter
        this->Context = ImGui::GetCurrentContext();
    }
    io.BackendPlatformUserData = (void*)this;
    io.BackendPlatformName = "imgui_impl_qt";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    //io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;

    Window = std::move(window);
    Time = 0.0;
    WantUpdateMonitors = true;

    io.SetClipboardTextFn = ImGui_ImplQt_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplQt_GetClipboardText;
    io.ClipboardUserData = Window.get();

    // Set platform dependent data in viewport
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = (void*)Window.get();

    //if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    //    ImGui_ImplQt_InitPlatformInterface();
    return true;
}

void ImGui_ImplQt::NewFrame(ImGuiIO& io)
{
    auto bd = this;
    ImGui_ImplQt_IWindow* window = Window.get();

    // Setup display size (every frame to accommodate for window resizing)
    int w{}, h{};
    int display_w{}, display_h{};
    if (window) {
        window->sizeInfo(w, h, display_w, display_h);
    }
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / (float)w, (float)display_h / (float)h);

    //if (bd->WantUpdateMonitors)
    //    ImGui_ImplQt_UpdateMonitors();

    double current_time = QDateTime::currentMSecsSinceEpoch() / double(1000);
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
    if (io.DeltaTime <= 0.0f) {
        io.DeltaTime = 0.00001f;
    }
    bd->Time = current_time;

    //设置光标位置
    UpdateMouseData(io);

    //设置光标形状
    UpdateCursorShape(io, window);
}

void ImGui_ImplQt::UpdateMouseData(ImGuiIO& io)
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    // If ImGui wants to set cursor position (for example, during navigation by using keyboard)
    // we need to do it here (before getting `QCursor::pos()` below).

    ImGuiID mouse_viewport_id = 0;
    const ImVec2 mouse_pos_prev = io.MousePos;

    for (int i = 0; i < platform_io.Viewports.size(); i++)
    {
        ImGuiViewport* viewport = platform_io.Viewports[i];
        ImGui_ImplQt_IWindow* window = (ImGui_ImplQt_IWindow*)viewport->PlatformHandle;
        if (window->isActive())
        {
            // NOTE: This code will be executed, only if the following flags have been set:
            // - backend flag: `ImGuiBackendFlags_HasSetMousePos`      - enabled
            // - config  flag: `ImGuiConfigFlags_NavEnableSetMousePos` - enabled
            if (io.WantSetMousePos) {
                QPoint local_pos{ (int)io.MousePos.x, (int)io.MousePos.y };
                // Convert position from widget-space into screen-space
                window->setCursorPos(local_pos);
            }
        }
    }
    if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)
        io.AddMouseViewportEvent(mouse_viewport_id);
}

void ImGui_ImplQt::UpdateCursorShape(ImGuiIO& io, ImGui_ImplQt_IWindow* window)
{
    // NOTE: This code will be executed, only if the following flags have been set:
    // - backend flag: `ImGuiBackendFlags_HasMouseCursors`    - enabled
    // - config  flag: `ImGuiConfigFlags_NoMouseCursorChange` - disabled
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;
    const ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    Qt::CursorShape cursorShape = Qt::CursorShape::ArrowCursor;
    if (io.MouseDrawCursor || (imgui_cursor == ImGuiMouseCursor_None))
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        cursorShape = Qt::CursorShape::BlankCursor;
    }
    else
    {
        // Show OS mouse cursor
        static std::vector<std::pair<ImGuiMouseCursor, Qt::CursorShape>>
            map{
                { ImGuiMouseCursor_Arrow,      Qt::CursorShape::ArrowCursor },
                { ImGuiMouseCursor_TextInput,  Qt::CursorShape::IBeamCursor },
                { ImGuiMouseCursor_ResizeAll,  Qt::CursorShape::SizeAllCursor },
                { ImGuiMouseCursor_ResizeNS,   Qt::CursorShape::SizeVerCursor },
                { ImGuiMouseCursor_ResizeEW,   Qt::CursorShape::SizeHorCursor },
                { ImGuiMouseCursor_ResizeNESW, Qt::CursorShape::SizeBDiagCursor },
                { ImGuiMouseCursor_ResizeNWSE, Qt::CursorShape::SizeFDiagCursor },
                { ImGuiMouseCursor_Hand,       Qt::CursorShape::PointingHandCursor },
                { ImGuiMouseCursor_NotAllowed, Qt::CursorShape::ForbiddenCursor }
        };
        for (auto& obj : map)
        {
            if (obj.first == imgui_cursor) {
                cursorShape = obj.second;
            }
        }
    }

    window->setCursor(cursorShape);
}

bool ImGui_ImplQt::eventFilter(QObject* watched, QEvent* event)
{
    bool flag = false;
    if (Window) {
        flag = (Window->object() == watched);
    }
    if (flag)
    {
        ImGui::SetCurrentContext(Context);
        ImGuiIO& io = ImGui::GetIO();

        switch (event->type())
        {
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        {
            if (auto e = dynamic_cast<QMouseEvent*>(event))
            {
                io.AddKeyEvent(ImGuiKey_ModCtrl,
                    e->modifiers().testFlag(
                        Qt::KeyboardModifier::ControlModifier));
                io.AddKeyEvent(ImGuiKey_ModShift,
                    e->modifiers().testFlag(
                        Qt::KeyboardModifier::ShiftModifier));
                io.AddKeyEvent(ImGuiKey_ModAlt,
                    e->modifiers().testFlag(
                        Qt::KeyboardModifier::AltModifier));
                io.AddKeyEvent(ImGuiKey_ModSuper,
                    e->modifiers().testFlag(
                        Qt::KeyboardModifier::MetaModifier));

                io.AddMouseButtonEvent(
                    ImGuiMouseButton_Left,
                    e->buttons().testFlag(
                        Qt::MouseButton::LeftButton)
                );
                io.AddMouseButtonEvent(
                    ImGuiMouseButton_Right,
                    e->buttons().testFlag(
                        Qt::MouseButton::RightButton)
                );
                io.AddMouseButtonEvent(
                    ImGuiMouseButton_Middle,
                    e->buttons().testFlag(
                        Qt::MouseButton::MiddleButton)
                );
            }
        }
        break;
        case QEvent::Wheel:
        {
            if (auto e = dynamic_cast<QWheelEvent*>(event))
            {
                float x{};
                float y{};
                // Handle horizontal component
                if (e->pixelDelta().x() != 0) {
                    x = e->pixelDelta().x() / (ImGui::GetTextLineHeight());
                }
                else {
                    // Magic number of 120 comes from Qt doc on QWheelEvent::pixelDelta()
                    x = e->angleDelta().x() / 120.0f;
                }

                // Handle vertical component
                if (e->pixelDelta().y() != 0) {
                    // 5 lines per unit
                    y = e->pixelDelta().y() / (5.0 * ImGui::GetTextLineHeight());
                }
                else {
                    // Magic number of 120 comes from Qt doc on QWheelEvent::pixelDelta()
                    y = e->angleDelta().y() / 120.0f;
                }
                io.AddMouseWheelEvent(x, y);
            }
        }
        break;
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        {
            if (auto e = dynamic_cast<QKeyEvent*>(event))
            {
                io.AddKeyEvent(ImGuiKey_ModCtrl,
                    e->modifiers().testFlag(
                        Qt::KeyboardModifier::ControlModifier));
                io.AddKeyEvent(ImGuiKey_ModShift,
                    e->modifiers().testFlag(
                        Qt::KeyboardModifier::ShiftModifier));
                io.AddKeyEvent(ImGuiKey_ModAlt,
                    e->modifiers().testFlag(
                        Qt::KeyboardModifier::AltModifier));
                io.AddKeyEvent(ImGuiKey_ModSuper,
                    e->modifiers().testFlag(
                        Qt::KeyboardModifier::MetaModifier));

                const bool key_pressed = (event->type() == QEvent::KeyPress);
                static std::vector<std::pair<Qt::Key, ImGuiKey>> map
                {
                    { Qt::Key_Tab, ImGuiKey_Tab },
                    { Qt::Key_Left, ImGuiKey_LeftArrow },
                    { Qt::Key_Right, ImGuiKey_RightArrow },
                    { Qt::Key_Up, ImGuiKey_UpArrow },
                    { Qt::Key_Down, ImGuiKey_DownArrow },
                    { Qt::Key_PageUp, ImGuiKey_PageUp },
                    { Qt::Key_PageDown, ImGuiKey_PageDown },
                    { Qt::Key_Home, ImGuiKey_Home },
                    { Qt::Key_End, ImGuiKey_End },
                    { Qt::Key_Insert, ImGuiKey_Insert },
                    { Qt::Key_Delete, ImGuiKey_Delete },
                    { Qt::Key_Backspace, ImGuiKey_Backspace },
                    { Qt::Key_Space, ImGuiKey_Space },
                    { Qt::Key_Enter, ImGuiKey_Enter },
                    { Qt::Key_Return, ImGuiKey_Enter },
                    { Qt::Key_Escape, ImGuiKey_Escape },
                    { Qt::Key_A, ImGuiKey_A },
                    { Qt::Key_C, ImGuiKey_C },
                    { Qt::Key_V, ImGuiKey_V },
                    { Qt::Key_X, ImGuiKey_X },
                    { Qt::Key_Y, ImGuiKey_Y },
                    { Qt::Key_Z, ImGuiKey_Z }
                };
                for (auto& obj : map)
                {
                    if (obj.first == e->key()) {
                        io.AddKeyEvent(obj.second, key_pressed);
                    }
                }
                if (key_pressed) {
                    const QString text = e->text();
                    if (text.size() == 1) {
                        io.AddInputCharacter(text.at(0).unicode());
                    }
                }
            }
        }
        break;
        case QEvent::InputMethod:
            if (auto e = dynamic_cast<QInputMethodEvent*>(event))
            {
                auto&& input = e->commitString();
                io.AddInputCharactersUTF8(input.toStdString().c_str());
            }
            break;
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        {
            if (auto e = dynamic_cast<QFocusEvent*>(event)) {
                io.AddFocusEvent(e->gotFocus());
            }
        }
        break;
        case QEvent::MouseMove:
        {
            //注意要开启鼠标追踪
            if (auto e = dynamic_cast<QMouseEvent*>(event)) {
                const QPoint pos = e->pos();
                io.AddMousePosEvent(pos.x(), pos.y());
            }
        }
        break;
        default:
            break;
        };
    }
    return QObject::eventFilter(watched, event);
}
