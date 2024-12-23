#include "InterfaceApp.hpp"
#include "include/Serial.h"
#include <stdlib.h> 
#include <wx/spinctrl.h>

wxThread::ExitCode MyThread::Entry()
{
    try{
        Serial serial("COM8", 1000000);
        serial.init();

        uint8_t data_send[4] = {0xaa, 0x05, 0x17, 0x39};
        while (!TestDestroy()){
            if(serial.isConnect){
                serial.write( data_send, sizeof(data_send)/sizeof(*data_send) );
                DataRecv data_recv = serial.recvWithTimeout(1000);
                for(int i=0; i<data_recv.bytes_read; ++i){
                    fmt::print("{:x}", data_recv.data_recv[i]);
                }
                fmt::println("");
            }

        }
    }
    catch (const std::exception& e) {
        wxLogError("Exception in thread: %s", e.what());
    }
    catch (...) {
        wxLogError("Unknown exception occurred in thread.");
    }

    wxLogMessage("Thread is stopping.");
    return (wxThread::ExitCode)0; 
}

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();
    
    MyFrame* frame = new MyFrame();
    frame->SetInitialSize( wxSize(1200,800) );
    frame->Show(true);
    return true;
}

int MyApp::OnExit()
{
    wxLogMessage("Application is exiting...");
    return wxApp::OnExit();
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Motor Configuration Center"),
    m_timer(this, ID_TIMER),
    m_time(0.0)
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H", "Start a background task");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* controlbox = new wxBoxSizer(wxVERTICAL);

    wxPanel* motorPlot = createMotorDataPlot();
    ParentPanel motorStatusPanel = createMotorStatusPanel();
    ParentPanel controlPanel = createModePanel();

    PanelWithComponents posPidPanel = createMotorModeParamPanel<wxTextCtrl>( motorStatusPanel, 1 );
    PanelWithComponents velPidPanel = createMotorModeParamPanel<wxTextCtrl>( motorStatusPanel, 0 );
    createMotorVelocityPanel( controlPanel );
    createMotorPositionPanel( controlPanel );

    controlbox->Add(controlPanel.panel, 0, wxEXPAND | wxALL);
    controlbox->Add(motorStatusPanel.panel, 0, wxEXPAND | wxALL);

    hbox->Add(motorPlot, 2, wxEXPAND | wxLEFT);
    hbox->Add(controlbox, 0, wxEXPAND | wxRIGHT);

    this->SetSizerAndFit(hbox);

    // wxButton* setGain = new wxButton(this, ID_BUTTON, "set", wxPoint(900, 600), wxDefaultSize, 0);
    // wxArrayString ports({"COM1", "COM2", "COM3"});
    // wxComboBox* portList = new wxComboBox(this, ID_PORTLIST, 
    //         "select port", 
    //         wxPoint(900,100), 
    //         wxSize(80,320), 
    //         ports);
    // textBox = new wxTextCtrl(this, ID_TXTBOX,
    //         "",
    //         wxPoint(1000,100),
    //         wxDefaultSize,
    //         wxTE_PROCESS_ENTER);
    
    // Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    // Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    // Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_TIMER, &MyFrame::OnUpdatePlot, this, ID_TIMER);
    // Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
    //                         wxLogMessage("Button Clicked");
    //                     }, ID_BUTTON);
    // Bind(wxEVT_COMBOBOX, [&,this](wxCommandEvent& event) {
    //                         wxLogMessage( event.GetString() );
    //                         textBox->SetValue( event.GetString() );
    //                         }, ID_PORTLIST);
    // Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent& event) {
    //                         wxLogMessage( event.GetString() );
    //                         }, ID_TXTBOX);

    m_thread = new MyThread(this);
    if (m_thread->Run() != wxTHREAD_NO_ERROR)
    {
        wxLogError("Could not create the thread!");
        delete m_thread;
        m_thread = nullptr;
    }

    m_timer.Start(100);

    free(posPidPanel.components);
    free(velPidPanel.components);
}

MyFrame::~MyFrame()
{
    if (m_thread) 
    {
        m_thread->Delete();
        m_thread = nullptr;
    }
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example", "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Background thread is already running...");
}

ParentPanel MyFrame::createMotorStatusPanel(){
    ParentPanel parent;

    wxPanel* Panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxStaticBoxSizer* vbox = new wxStaticBoxSizer(wxVERTICAL, Panel, _T("Motor control paramters"));
    Panel->SetSizer(vbox);

    parent.panel = Panel;
    parent.box = vbox;

    return parent;
}

ParentPanel MyFrame::createModePanel(){
    ParentPanel parent;

    wxPanel* Panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxStaticBoxSizer* hbox = new wxStaticBoxSizer(wxHORIZONTAL, Panel, _T("Motor Control Mode"));
    Panel->SetSizer(hbox);

    parent.panel = Panel;
    parent.box = hbox;

    return parent;
}

template<typename T>
PanelWithComponents<T> MyFrame::createMotorModeParamPanel(ParentPanel Parent,
    int control_mode){
        
    PanelWithComponents<T> panelComponents;

    std::string name = (control_mode == 0) ? "Velocity mode" : "Position mode";

    panelComponents.panel = new wxPanel(Parent.panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    panelComponents.components = (T**)calloc(3, sizeof(*panelComponents.components));

    wxStaticBoxSizer* hbox = new wxStaticBoxSizer(wxHORIZONTAL, panelComponents.panel, name);

    std::vector<std::string> components = {"Kp", "Ki", "Kd"};

    wxBitmap playBitmap;
    playBitmap.LoadFile("./icons/playButton.png", wxBITMAP_TYPE_PNG);

    for(int i=0; i<components.size(); ++i){
        wxBoxSizer* vbox1 = new wxBoxSizer(wxVERTICAL);
        wxStaticText* label = new wxStaticText(panelComponents.panel, 
            wxID_ANY, 
            fmt::format("{}: ",components[i]) );
        wxBitmapButton* setButton = new wxBitmapButton(panelComponents.panel, 
            wxID_ANY, 
            playBitmap, 
            wxDefaultPosition, 
            wxDefaultSize);
        vbox1->Add(label, 0, wxEXPAND | wxALL);
        vbox1->Add(setButton, 0, wxEXPAND | wxALL);

        wxBoxSizer* vbox2 = new wxBoxSizer(wxVERTICAL);
        wxTextCtrl* vel_txtbox = new wxTextCtrl(panelComponents.panel, 
            wxID_ANY, 
            "0.0", 
            wxDefaultPosition, 
            wxDefaultSize, 
            wxTE_READONLY);
        wxSpinCtrlDouble* gainCtrl = new wxSpinCtrlDouble(panelComponents.panel, 
            wxID_ANY, 
            "", 
            wxDefaultPosition, 
            wxDefaultSize);
        gainCtrl->SetRange(-10000.0, 10000.0);
        gainCtrl->SetIncrement(0.1);
        gainCtrl->SetDigits(2);

        setButton->Bind(wxEVT_BUTTON, 
            [this, gainCtrl, control_mode, i](wxCommandEvent& event){
                if(control_mode == 0){
                    wxLogMessage( wxString::FromUTF8(fmt::format("Button Clicked {} and vel and {}", 
                        gainCtrl->GetValue(), i)) );
                }
                else{
                    wxLogMessage( wxString::FromUTF8(fmt::format("Button Clicked {} and pos and {}", 
                        gainCtrl->GetValue(), i)) );
                }
            });

        vbox2->Add(vel_txtbox, 0, wxEXPAND | wxALL);
        vbox2->Add(gainCtrl, 0, wxEXPAND | wxALL);

        hbox->Add(vbox1, 0, wxEXPAND | wxALL, 5);
        hbox->Add(vbox2, 0, wxEXPAND | wxALL, 5);
        panelComponents.components[i] = vel_txtbox;
    }
    
    panelComponents.panel->SetSizer(hbox);
    Parent.box->Add(panelComponents.panel);

    return panelComponents;
}

void MyFrame::createMotorVelocityPanel(ParentPanel Parent){
    wxStaticBoxSizer* vbox = new wxStaticBoxSizer(wxVERTICAL, Parent.panel, "Velocity Mode");

    wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* cspd_label = new wxStaticText(Parent.panel, 
            wxID_ANY, 
            "Current speed (rpm)");
    wxTextCtrl* vel_txtbox = new wxTextCtrl(Parent.panel, 
            wxID_ANY, 
            "0.0", 
            wxDefaultPosition, 
            wxSize(60, 20), 
            wxTE_READONLY);
    hbox1->Add(cspd_label, 0, wxRIGHT, 10);
    hbox1->Add(vel_txtbox);

    wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* cmdspd_label = new wxStaticText(Parent.panel, 
            wxID_ANY, 
            "Target speed (rpm)");
    wxSpinCtrlDouble* spdCtrl = new wxSpinCtrlDouble(Parent.panel, 
            wxID_ANY, 
            "", 
            wxDefaultPosition, 
            wxDefaultSize);
    hbox2->Add(cmdspd_label, 0, wxRIGHT, 17);
    hbox2->Add(spdCtrl);

    wxBoxSizer* hbox3 = new wxBoxSizer(wxHORIZONTAL);
    wxButton* runButton = new wxButton(Parent.panel, 
        wxID_ANY, 
        "run", 
        wxDefaultPosition, 
        wxDefaultSize);
    wxButton* stopButton = new wxButton(Parent.panel, 
        wxID_ANY, 
        "stop", 
        wxDefaultPosition, 
        wxDefaultSize);
    hbox3->Add(runButton, 0, wxLEFT | wxRIGHT, 10);
    hbox3->Add(stopButton, 0, wxLEFT | wxRIGHT, 10);

    vbox->Add(hbox1);
    vbox->Add(hbox2, 0, wxTOP, 3);
    vbox->Add(hbox3, 0, wxTOP, 3);

    Parent.box->Add(vbox);
}

void MyFrame::createMotorPositionPanel(ParentPanel Parent){
    wxStaticBoxSizer* vbox = new wxStaticBoxSizer(wxVERTICAL, Parent.panel, "Position Mode");

    wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* posLabel = new wxStaticText(Parent.panel, 
        wxID_ANY, 
        "Target (Deg): 0");
    wxSlider* posSlider = new wxSlider(Parent.panel, 
        wxID_ANY, 
        0, 
        0, 
        360, 
        wxDefaultPosition, 
        wxSize(300, 20), 
        wxSL_HORIZONTAL);
    hbox1->Add(posLabel, 0, wxRIGHT, 20);
    hbox1->Add(posSlider, 0, wxEXPAND | wxRIGHT);

    posSlider->Bind(wxEVT_SLIDER, [this, posLabel](wxCommandEvent& event){
        posLabel->SetLabel(wxString::Format("Target (Deg): %d", event.GetInt() ));
    });
    
    wxArrayString dirChoices;
    dirChoices.Add("CW");
    dirChoices.Add("CCW");
    wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);
    wxRadioBox* dirCtrl = new wxRadioBox(Parent.panel,
        wxID_ANY,
        "",
        wxDefaultPosition,
        wxSize(60, 60),
        dirChoices,
        wxCB_SORT);
    wxStaticText* cPos_label = new wxStaticText(Parent.panel, 
            wxID_ANY, 
            "Current pos (Deg)");
    wxTextCtrl* pos_txtbox = new wxTextCtrl(Parent.panel, 
            wxID_ANY, 
            "0.0", 
            wxDefaultPosition, 
            wxSize(60, 20), 
            wxTE_READONLY);
    wxStaticText* cmdPos_label = new wxStaticText(Parent.panel, 
            wxID_ANY, 
            "Target pos (Deg)");
    wxSpinCtrlDouble* posCtrl = new wxSpinCtrlDouble(Parent.panel, 
            wxID_ANY, 
            "", 
            wxDefaultPosition, 
            wxDefaultSize);
    hbox2->Add(dirCtrl, 0, wxCENTER);
    hbox2->Add(cPos_label, 0, wxRIGHT, 5);
    hbox2->Add(pos_txtbox, 0, wxRIGHT, 10);
    hbox2->Add(cmdPos_label, 0, wxRIGHT, 5);
    hbox2->Add(posCtrl);

    wxBoxSizer* hbox3 = new wxBoxSizer(wxHORIZONTAL);
    wxButton* runButton = new wxButton(Parent.panel, 
        wxID_ANY, 
        "run", 
        wxDefaultPosition, 
        wxDefaultSize);
    wxButton* stopButton = new wxButton(Parent.panel, 
        wxID_ANY, 
        "stop", 
        wxDefaultPosition, 
        wxDefaultSize);
    wxButton* setHomeButton = new wxButton(Parent.panel, 
        wxID_ANY, 
        "SetHome", 
        wxDefaultPosition, 
        wxDefaultSize);
    wxButton* goHomeButton = new wxButton(Parent.panel, 
        wxID_ANY, 
        "GoHome", 
        wxDefaultPosition, 
        wxDefaultSize);
    hbox3->Add(runButton, 0, wxRIGHT, 5);
    hbox3->Add(stopButton, 0, wxRIGHT, 15);
    hbox3->Add(setHomeButton, 0, wxRIGHT, 5);
    hbox3->Add(goHomeButton);

    vbox->Add(hbox1);
    vbox->Add(hbox2);
    vbox->Add(hbox3);

    Parent.box->Add(vbox, 0, wxLEFT, 5);
}

wxPanel* MyFrame::createMotorDataPlot(){
    wxPanel* motorPlot = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    m_plotWindow = new mpWindow(motorPlot, -1, wxPoint(0, 0), wxSize(800, 500), wxSUNKEN_BORDER);
    m_plotLayer = new mpFXYVector(wxT("motor speed"));
    m_plotLayer->SetPen(wxPen(*wxBLACK, 2));
    m_plotLayer->SetContinuity(true);

    m_plotLayer2 = new mpFXYVector(wxT("setpoint"));
    m_plotLayer2->SetPen(wxPen(*wxRED, 2));
    m_plotLayer2->SetContinuity(true);

    m_xData.reserve(500);
    m_yData.reserve(500);

    m_xData2.reserve(500);
    m_yData2.reserve(500);

    m_plotWindow->AddLayer(m_plotLayer);
    m_plotWindow->AddLayer(m_plotLayer2);
    m_plotWindow->EnableDoubleBuffer(true);
    m_plotWindow->SetMargins(30, 30, 50, 50);

    mpScaleX* xaxis = new mpScaleX(wxT("Time"), mpALIGN_CENTER, true);
    mpScaleY* yaxis = new mpScaleY(wxT("Amplitude"), mpALIGN_LEFT, true);
    m_plotWindow->AddLayer(xaxis);
    m_plotWindow->AddLayer(yaxis);

    vbox->Add(m_plotWindow, 1, wxEXPAND | wxALL, 5);
    motorPlot->SetSizer(vbox);

    return motorPlot;
}

void MyFrame::UpdateSineWave(){
    m_xData.clear();
    m_yData.clear();

    m_xData2.clear();
    m_yData2.clear();

    for(double t=m_time; t<m_time + 4*M_PI; t +=0.1){
        m_xData.push_back(t);
        m_yData.push_back( sin((float)t) );

        m_xData2.push_back(t);
        m_yData2.push_back( sin((float)(t + M_PI/2)) );
    }
    
    m_plotLayer->SetData(m_xData, m_yData);
    m_plotLayer2->SetData(m_xData2, m_yData2);
    
    m_plotWindow->Fit();
    m_plotWindow->UpdateAll();

    m_time += 0.1;
}

void MyFrame::OnUpdatePlot(wxTimerEvent& event){
    UpdateSineWave();
}