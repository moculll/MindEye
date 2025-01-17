#include "Common.h"
#include "MindEyeApp.h"

MindEyeApp::MindEyeApp(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.isaacBeginBtn, &QPushButton::clicked, this, &MindEyeApp::onIsaacBeginBtnClicked);
    connect(ui.isaacTerminateBtn, &QPushButton::clicked, this, &MindEyeApp::onIsaacTerminateBtnClicked);
}

MindEyeApp::~MindEyeApp()
{
}

void MindEyeApp::onIsaacBeginBtnClicked()
{
    // 注入以撒辅助
    RemoteThreadInject32((LPWSTR)L"isaac-ng.exe", (LPWSTR)L"D:\\Workspace\\LaoQiongGui\\MindEye\\IsaacMindEye\\output\\x86\\Release\\IsaacMindEye.dll");
}

void MindEyeApp::onIsaacTerminateBtnClicked()
{
    DWORD dwProcessId = FindPIDByName((LPWSTR)L"isaac-ng.exe");
    HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    if (hProcess == NULL) {
        return;
    }
    ::TerminateProcess(hProcess, 0);
    ::CloseHandle(hProcess);
}
