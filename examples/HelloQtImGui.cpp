#include <QtWidgets/QApplication>
#include <QtCore/QTimer>
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLWindow>
#include <QtCore/QFile>

#include "imgui_impl_qt.h"
#include "imgui_impl_qt_opengl3.h"

namespace
{
    struct ImDemo
    {
        void initialize()
        {
            QString font = QCoreApplication::applicationDirPath() + "/LXGWBright-Regular.ttf";
            if (!QFile::exists(font))
                return;
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->AddFontFromFileTTF(font.toStdString().c_str(),
                24.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
        }

        void render()
        {
            static bool show_imgui_demo_window = true;
            static ImVec4 clear_color = ImColor(114, 144, 154);

            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);

            static ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

            // 1. Show a simple window
            // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
            ImGui::Begin("Example:Fullscreen window", nullptr, flags);
            {
                static float f = 0.0f;
                ImGui::Text(u8"Hello, world!");
                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);
                if (ImGui::Button("ImGui Demo")) show_imgui_demo_window ^= 1;
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            }
            ImGui::End();

            // 2. Show another simple window, this time using an explicit Begin/End pair
            if (show_imgui_demo_window)
            {
                ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
                ImGui::ShowDemoWindow(&show_imgui_demo_window);
            }
        }
    };
}


class ApplicationView :public QOpenGLWidget
{
public:
    explicit ApplicationView(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags())
        :QOpenGLWidget(parent, f)
    {};
    ~ApplicationView()
    {
        ImGui::SetCurrentContext(m_ctx);
        ImGui_ImplQtOpenGL3_Shutdown();
        ImGui_ImplQt_Shutdown();
        ImGui::DestroyContext(m_ctx);
    }
protected:
    void initializeGL() override {
        m_ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(m_ctx);
        ImGui_ImplQt_Init(this);
        ImGui_ImplQtOpenGL3_Init(nullptr);

        demo.initialize();
    }

    void paintGL() override {
        ImGui::SetCurrentContext(m_ctx);
        ImGui_ImplQtOpenGL3_NewFrame();
        ImGui_ImplQt_NewFrame();
        ImGui::NewFrame();

        demo.render();

        ImGui::Render();
        ImGui_ImplQtOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
private:
    ImGuiContext* m_ctx{};
    ImDemo  demo{};
};

class ApplicationWindow :public QOpenGLWindow
{
public:
    ~ApplicationWindow()
    {
        ImGui::SetCurrentContext(m_ctx);
        ImGui_ImplQtOpenGL3_Shutdown();
        ImGui_ImplQt_Shutdown();
        ImGui::DestroyContext(m_ctx);
    }
protected:
    void initializeGL() override {
        m_ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(m_ctx);
        ImGui_ImplQt_Init(this);
        ImGui_ImplQtOpenGL3_Init(nullptr);

        demo.initialize();
    }

    void paintGL() override {
        ImGui::SetCurrentContext(m_ctx);
        ImGui_ImplQtOpenGL3_NewFrame();
        ImGui_ImplQt_NewFrame();
        ImGui::NewFrame();

        demo.render();

        ImGui::Render();
        ImGui_ImplQtOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
private:
    ImGuiContext* m_ctx{};
    ImDemo  demo{};
};


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    ApplicationView appView{};
    appView.setWindowTitle("ImGui Qt backend example - QOpenGLWidget");
    appView.resize(1280, 720);
    appView.show();

    ApplicationView appView1{};
    appView1.setWindowTitle("ImGui Qt backend example - QOpenGLWidget (other)");
    appView1.resize(1280, 720);
    appView1.show();

    ApplicationWindow appView2{};
    appView2.setTitle("ImGui Qt backend example - QOpenGLWindow");
    appView2.resize(1280, 720);
    appView2.show();

    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &appView, SLOT(update()));
    QObject::connect(&timer, SIGNAL(timeout()), &appView1, SLOT(update()));
    QObject::connect(&timer, SIGNAL(timeout()), &appView2, SLOT(update()));
    timer.start(16);

    return app.exec();
}
