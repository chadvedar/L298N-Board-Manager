#include <wx/wx.h>
#include <wx/thread.h>
#include "include/mathplot.h"
#include <fmt/core.h>
#include <cmath>
#include <stdarg.h>

enum
{
    ID_Hello = 1,
    ID_BUTTON,
    ID_TIMER,
    ID_PORTLIST,
    ID_TXTBOX
};

struct ParentPanel{
    wxPanel* panel;
    wxStaticBoxSizer* box;
};

template<typename T>
struct PanelWithComponents{
    wxPanel* panel;
    T** components = nullptr;
};

class MyApp : public wxApp
{
    public:
        bool OnInit() override;
        int OnExit() override;
};

class MyThread : public wxThread
{
    public:
        MyThread(wxFrame* handler) : wxThread(wxTHREAD_DETACHED), m_handler(handler) {}

    protected:
        ExitCode Entry() override;

    private:
        wxFrame* m_handler;
};

class MyFrame : public wxFrame
{
    public:
        MyFrame();
        ~MyFrame();

    private:
        void OnHello(wxCommandEvent& event);
        void OnExit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnUpdatePlot(wxTimerEvent& event);

        MyThread* m_thread;
        mpWindow* m_plotWindow;
        mpFXYVector* m_plotLayer;
        mpFXYVector* m_plotLayer2;
        wxTimer m_timer;

        wxTextCtrl* textBox;

        std::vector<double> m_xData;
        std::vector<double> m_yData;
        std::vector<double> m_xData2;
        std::vector<double> m_yData2;
        double m_time;

        wxPanel* createMotorDataPlot();
        ParentPanel createMotorStatusPanel();
        ParentPanel createModePanel();

        template<typename T>
        PanelWithComponents<T> createMotorModeParamPanel(ParentPanel Parent, 
            int control_mode);
        void createMotorVelocityPanel(ParentPanel Parent);
        void createMotorPositionPanel(ParentPanel Parent);

        void UpdateSineWave();
};