//*****************************************************************************
//
//  Author:         Kenny Kerr
//  Date created:   16 July 2006
//
//  Description:    Implements a C++ wrapper for the TaskDialogIndirect 
//                  function.
//
// (Sligthly modified by Marco Cameriero to fit in this project)
// 
//*****************************************************************************

#pragma once

// Use explicit constructors for CString
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

// Use non-restrictive notify handlers
#define _WTL_NEW_PAGE_NOTIFY_HANDLERS

// From ATL
#include <atlbase.h>
#include <atlcoll.h>
#include <atlstr.h>

// From WTL
#include "wtl/atlapp.h"

// Includes some headers to manage Utf8-Utf16 conversion
#include <locale>
#include <codecvt>
#include <string>

// Links to Common Controls 6 library
#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// Diagnostics macros

#define ASSERT(x) ATLASSERT(x)
#define TRACE ATLTRACE

#if defined _DEBUG
#define VERIFY(x) ASSERT(x)
#else
#define VERIFY(x) ((void)(x))
#endif

#if defined _DEBUG
#define COM_VERIFY(x) ASSERT(SUCCEEDED(x))
#else
#define COM_VERIFY(x) ((void)(x))
#endif


namespace Kerr
{
    void CopyStrToWStr(PCWSTR& dest, const char* source)
    {
        #pragma warning(push)
        #pragma warning(disable:4996)

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wsource = converter.from_bytes(source);
        wchar_t* wdest = new wchar_t[wsource.length() + 1];
        std::size_t charsCopied = wsource.copy(wdest, wsource.length());
        wdest[charsCopied] = L'\0';
        if (dest)
            delete[] dest;
        dest = wdest;

        #pragma warning(pop)
    }

    void CopyWStrToStr(char*& dest, PCWSTR wsource)
    {
        #pragma warning(push)
        #pragma warning(disable:4996)

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::string source = converter.to_bytes(wsource);
        dest = new char[source.length() + 1];
        std::size_t charsCopied = source.copy(dest, source.length());
        dest[charsCopied] = '\0';

        #pragma warning(pop)
    }

    class TaskDialog : public CWindow
    {
    public:

        explicit TaskDialog();
        virtual ~TaskDialog() {}

        // Set text captions
        void SetWindowTitle(ATL::_U_STRINGorID text);
        void SetMainInstruction(ATL::_U_STRINGorID text);
        void SetContent(ATL::_U_STRINGorID text);
        void SetVerificationText(ATL::_U_STRINGorID text);
        void SetExpandedInformation(ATL::_U_STRINGorID text);
        void SetExpandedControlText(ATL::_U_STRINGorID text);
        void SetCollapsedControlText(ATL::_U_STRINGorID text);
        void SetFooter(ATL::_U_STRINGorID text);

        // Set icons
        void SetMainIcon(HICON handle);
        void SetMainIcon(ATL::_U_STRINGorID resource);
        void SetFooterIcon(HICON handle);
        void SetFooterIcon(ATL::_U_STRINGorID resource);

        // Buttons
        CAtlArray<TASKDIALOG_BUTTON>& Buttons();
        CAtlArray<TASKDIALOG_BUTTON>& RadioButtons();
        void AddButton(ATL::_U_STRINGorID text,
                       int id);

        void AddRadioButton(ATL::_U_STRINGorID text,
                            int id);

        // Flags
        void SetCommonButtons(TASKDIALOG_COMMON_BUTTON_FLAGS commonButtons);
        void SetUseLinks(bool useLinks = true);
        void SetUseCommandLinks(bool useCommandLinks = true);
        void SetUseProgressBar(bool useProgressBar = true);
        void SetUseTimer(bool useTimer = true);
        void SetCancelable(bool cancelable = true);
        void SetMinimizable(bool minimizable = true);

        virtual HRESULT DoModal(HWND parent = ::GetActiveWindow());
        int GetSelectedButtonId() const;
        int GetSelectedRadioButtonId() const;
        bool VerificiationChecked() const;

        // Messages
        virtual void ClickButton(int buttonId);
        virtual void ClickRadioButton(int buttonId);
        virtual void ClickVerification(bool checked, bool setKeyFocus);
        virtual void EnableButton(int buttonId, bool enable = true);
        virtual void EnableRadioButton(int buttonId, bool enable = true);
        virtual void SetProgressBarMarquee(bool marquee = true);
        virtual void SetProgressBarState(int state);
        virtual void SetProgressBarRange(WORD minRange = 0, WORD maxRange = 100);
        virtual void SetProgressBarPosition(int position);
        virtual void SetProgressBarMarquee(bool marquee, DWORD speed);
        virtual void SetButtonElevationRequired(int buttonId, bool required = true);
        virtual void NavigatePage(TaskDialog& newDialog);
        virtual void ResetTimer();

    protected:

        // Events
        virtual void OnDialogConstructed() {}
        virtual void OnHyperlinkClicked(PCWSTR /*url*/) {}
        virtual void OnButtonClicked(int /*buttonId*/, bool& closeDialog) { closeDialog = true; }
        virtual void OnRadioButtonClicked(int /*buttonId*/) {}
        virtual void OnVerificationClicked(bool /*checked*/) {}
        virtual void OnHelp() {}
        virtual void OnExpandoButtonClicked(bool /*expanded*/) {}
        virtual void OnTimer(DWORD /*milliseconds*/, bool& /*reset*/) {}
        virtual void OnNavigated() {}

        static HRESULT CALLBACK Callback(HWND handle, 
                                         UINT notification, 
                                         WPARAM wParam, 
                                         LPARAM lParam, 
                                         LONG_PTR data);

        TASKDIALOGCONFIG m_config;
        CAtlArray<TASKDIALOG_BUTTON> m_buttons;
        CAtlArray<TASKDIALOG_BUTTON> m_radioButtons;
        int m_selectedButtonId;
        int m_selectedRadioButtonId;
        BOOL m_verificationChecked;
        BOOL m_resetTimer;
    };
}

Kerr::TaskDialog::TaskDialog() :
    m_selectedButtonId(0),
    m_selectedRadioButtonId(0),
    m_verificationChecked(FALSE),
    m_resetTimer(FALSE)
{
    ::ZeroMemory(&m_config, 
                 sizeof (TASKDIALOGCONFIG));

    m_config.cbSize = sizeof (TASKDIALOGCONFIG);
    m_config.hInstance = ATL::_AtlBaseModule.GetResourceInstance();
    m_config.pfCallback = Callback;
    m_config.lpCallbackData = reinterpret_cast<LONG_PTR>(this);
    m_config.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW;
}

void Kerr::TaskDialog::SetWindowTitle(ATL::_U_STRINGorID text)
{

    if (0 == m_hWnd)
    {
        CopyStrToWStr(m_config.pszWindowTitle, text.m_lpstr);
    }
    else if (IS_INTRESOURCE(text.m_lpstr))
    {
        CString string;

        // Since we know that text is actually a resource Id we can ignore the pointer truncation warning.
        #pragma warning(push)
        #pragma warning(disable: 4311)

        VERIFY(string.LoadString(m_config.hInstance,
                                 reinterpret_cast<UINT>(text.m_lpstr)));

        #pragma warning(pop)

        VERIFY(SetWindowText(string));
    }
    else
    {
        VERIFY(SetWindowText(text.m_lpstr));
    }
}

void Kerr::TaskDialog::SetMainInstruction(ATL::_U_STRINGorID text)
{
    PCWSTR wstr = NULL;
    CopyStrToWStr(wstr, text.m_lpstr);
    if (0 == m_hWnd)
    {
        m_config.pszMainInstruction = wstr;
    }
    else
    {
        SendMessage(TDM_SET_ELEMENT_TEXT,
                    TDE_MAIN_INSTRUCTION,
                    reinterpret_cast<LPARAM>(wstr));
    }
}

void Kerr::TaskDialog::SetContent(ATL::_U_STRINGorID text)
{
    PCWSTR wstr = NULL;
    CopyStrToWStr(wstr, text.m_lpstr);
    if (0 == m_hWnd)
    {
        m_config.pszContent = wstr;
    }
    else
    {
        SendMessage(TDM_SET_ELEMENT_TEXT,
                    TDE_CONTENT,
                    reinterpret_cast<LPARAM>(wstr));
    }
}

void Kerr::TaskDialog::SetVerificationText(ATL::_U_STRINGorID text)
{
    CopyStrToWStr(m_config.pszVerificationText, text.m_lpstr);
}

void Kerr::TaskDialog::SetExpandedInformation(ATL::_U_STRINGorID text)
{
    PCWSTR wstr = NULL;
    CopyStrToWStr(wstr, text.m_lpstr);
    if (0 == m_hWnd)
    {
        m_config.pszExpandedInformation = wstr;
    }
    else
    {
        SendMessage(TDM_SET_ELEMENT_TEXT,
                    TDE_EXPANDED_INFORMATION,
                    reinterpret_cast<LPARAM>(wstr));
    }
}

void Kerr::TaskDialog::SetExpandedControlText(ATL::_U_STRINGorID text)
{
    CopyStrToWStr(m_config.pszExpandedControlText, text.m_lpstr);
}

void Kerr::TaskDialog::SetCollapsedControlText(ATL::_U_STRINGorID text)
{
    CopyStrToWStr(m_config.pszCollapsedControlText, text.m_lpstr);
}

void Kerr::TaskDialog::SetFooter(ATL::_U_STRINGorID text)
{
    PCWSTR wstr = NULL;
    CopyStrToWStr(wstr, text.m_lpstr);
    if (0 == m_hWnd)
    {
        m_config.pszFooter = wstr;
    }
    else
    {
        SendMessage(TDM_SET_ELEMENT_TEXT,
                    TDE_FOOTER,
                    reinterpret_cast<LPARAM>(wstr));
    }
}

void Kerr::TaskDialog::SetMainIcon(HICON handle)
{
    if (0 == m_hWnd)
    {
        m_config.hMainIcon = handle;
        m_config.dwFlags |= TDF_USE_HICON_MAIN;
    }
    else
    {
        ASSERT(TDF_USE_HICON_MAIN & m_config.dwFlags);

        SendMessage(TDM_UPDATE_ICON,
                    TDIE_ICON_MAIN,
                    reinterpret_cast<LPARAM>(handle));
    }
}

void Kerr::TaskDialog::SetMainIcon(ATL::_U_STRINGorID resource)
{
    if (0 == m_hWnd)
    {
        if (IS_INTRESOURCE(resource.m_lpstr))
            m_config.pszMainIcon = (PCWSTR)resource.m_lpstr;
        else
            CopyStrToWStr(m_config.pszMainIcon, resource.m_lpstr);
        m_config.dwFlags &= ~TDF_USE_HICON_MAIN;
    }
    else
    {
        ASSERT(0 == (TDF_USE_HICON_MAIN & m_config.dwFlags));

        SendMessage(TDM_UPDATE_ICON,
                    TDIE_ICON_MAIN,
                    reinterpret_cast<LPARAM>(resource.m_lpstr));
    }
}

void Kerr::TaskDialog::SetFooterIcon(HICON handle)
{
    if (0 == m_hWnd)
    {
        m_config.hFooterIcon = handle;
        m_config.dwFlags |= TDF_USE_HICON_FOOTER;
    }
    else
    {
        ASSERT(TDF_USE_HICON_FOOTER & m_config.dwFlags);

        SendMessage(TDM_UPDATE_ICON,
                    TDIE_ICON_FOOTER,
                    reinterpret_cast<LPARAM>(handle));
    }
}

void Kerr::TaskDialog::SetFooterIcon(ATL::_U_STRINGorID resource)
{
    if (0 == m_hWnd)
    {
        if (IS_INTRESOURCE(resource.m_lpstr))
            m_config.pszFooterIcon = (PCWSTR)resource.m_lpstr;
        else
            CopyStrToWStr(m_config.pszFooterIcon, resource.m_lpstr);
        m_config.dwFlags &= ~TDF_USE_HICON_FOOTER;
    }
    else
    {
        ASSERT(0 == (TDF_USE_HICON_FOOTER & m_config.dwFlags));

        SendMessage(TDM_UPDATE_ICON,
                    TDIE_ICON_FOOTER,
                    reinterpret_cast<LPARAM>(resource.m_lpstr));
    }
}

CAtlArray<TASKDIALOG_BUTTON>& Kerr::TaskDialog::Buttons()
{
    return m_buttons;
}

CAtlArray<TASKDIALOG_BUTTON>& Kerr::TaskDialog::RadioButtons()
{
    return m_radioButtons;
}

void Kerr::TaskDialog::AddButton(ATL::_U_STRINGorID text,
                                 int id)
{
    size_t index = m_buttons.Add();
    CopyStrToWStr(m_buttons[index].pszButtonText, text.m_lpstr);
    m_buttons[index].nButtonID = id;
}

void Kerr::TaskDialog::AddRadioButton(ATL::_U_STRINGorID text,
                                      int id)
{
    size_t index = m_radioButtons.Add();
    CopyStrToWStr(m_radioButtons[index].pszButtonText, text.m_lpstr);
    m_radioButtons[index].nButtonID = id;
}

void Kerr::TaskDialog::SetCommonButtons(TASKDIALOG_COMMON_BUTTON_FLAGS commonButtons) {
    ASSERT(m_hWnd == 0);
    m_config.dwCommonButtons = commonButtons;
}

void Kerr::TaskDialog::SetUseLinks(bool useLinks)
{
    ASSERT(m_hWnd == 0);
    if (useLinks)
        m_config.dwFlags |= TDF_ENABLE_HYPERLINKS;
    else
        m_config.dwFlags &= ~TDF_ENABLE_HYPERLINKS;
}

void Kerr::TaskDialog::SetUseCommandLinks(bool useCommandLinks)
{
    ASSERT(m_hWnd == 0);
    if (useCommandLinks)
        m_config.dwFlags |= TDF_USE_COMMAND_LINKS;
    else
        m_config.dwFlags &= ~TDF_USE_COMMAND_LINKS;
}

void Kerr::TaskDialog::SetUseProgressBar(bool useProgressBar)
{
    ASSERT(m_hWnd == 0);
    if (useProgressBar)
        m_config.dwFlags |= TDF_SHOW_PROGRESS_BAR;
    else
        m_config.dwFlags &= ~TDF_SHOW_PROGRESS_BAR;
}

void Kerr::TaskDialog::SetUseTimer(bool useTimer)
{
    ASSERT(m_hWnd == 0);
    if (useTimer)
        m_config.dwFlags |= TDF_CALLBACK_TIMER;
    else
        m_config.dwFlags &= ~TDF_CALLBACK_TIMER;
}

void Kerr::TaskDialog::SetCancelable(bool cancelable)
{
    ASSERT(m_hWnd == 0);
    if (cancelable)
        m_config.dwFlags |= TDF_ALLOW_DIALOG_CANCELLATION;
    else
        m_config.dwFlags &= ~TDF_ALLOW_DIALOG_CANCELLATION;
}

void Kerr::TaskDialog::SetMinimizable(bool minimizable)
{
    ASSERT(m_hWnd == 0);
    if (minimizable)
        m_config.dwFlags |= TDF_CAN_BE_MINIMIZED;
    else
        m_config.dwFlags &= ~TDF_CAN_BE_MINIMIZED;
}

HRESULT Kerr::TaskDialog::DoModal(HWND parent)
{
    ASSERT(0 == m_hWnd);

    m_config.hwndParent = parent;
    m_config.pButtons = m_buttons.GetData();
    m_config.cButtons = static_cast<UINT>(m_buttons.GetCount());
    m_config.pRadioButtons = m_radioButtons.GetData();
    m_config.cRadioButtons = static_cast<UINT>(m_radioButtons.GetCount());

    return ::TaskDialogIndirect(&m_config,
                                &m_selectedButtonId,
                                &m_selectedRadioButtonId,
                                &m_verificationChecked);
}

int Kerr::TaskDialog::GetSelectedButtonId() const
{
    return m_selectedButtonId;
}

int Kerr::TaskDialog::GetSelectedRadioButtonId() const
{
    return m_selectedRadioButtonId;
}

bool Kerr::TaskDialog::VerificiationChecked() const
{
    return FALSE != m_verificationChecked;
}

void Kerr::TaskDialog::ClickButton(int buttonId)
{
    SendMessage(TDM_CLICK_BUTTON,
                buttonId);
}

void Kerr::TaskDialog::ClickRadioButton(int buttonId)
{
    SendMessage(TDM_CLICK_RADIO_BUTTON,
                buttonId);
}

void Kerr::TaskDialog::ClickVerification(bool checked, 
                                         bool setKeyFocus)
{
    SendMessage(TDM_CLICK_VERIFICATION,
                checked,
                setKeyFocus);
}

void Kerr::TaskDialog::EnableButton(int buttonId, 
                                    bool enable)
{
    SendMessage(TDM_ENABLE_BUTTON,
                buttonId,
                enable);
}

void Kerr::TaskDialog::EnableRadioButton(int buttonId, 
                                         bool enable)
{
    SendMessage(TDM_ENABLE_RADIO_BUTTON,
                buttonId,
                enable);
}

void Kerr::TaskDialog::SetProgressBarMarquee(bool marquee)
{
    SetProgressBarMarquee(marquee, 0);
}

void Kerr::TaskDialog::SetProgressBarMarquee(bool marquee,
                                             DWORD milliseconds)
{
    SendMessage(TDM_SET_MARQUEE_PROGRESS_BAR,
                marquee);
    SendMessage(TDM_SET_PROGRESS_BAR_MARQUEE,
                marquee,
                milliseconds);
}

void Kerr::TaskDialog::SetProgressBarState(int state)
{
    SendMessage(TDM_SET_PROGRESS_BAR_STATE,
                state);
}

void Kerr::TaskDialog::SetProgressBarPosition(int position)
{
    SendMessage(TDM_SET_PROGRESS_BAR_POS,
                position);
}

void Kerr::TaskDialog::SetProgressBarRange(WORD minRange, 
                                           WORD maxRange)
{
    SendMessage(TDM_SET_PROGRESS_BAR_RANGE,
                0,
                MAKELPARAM(minRange, maxRange));
}

void Kerr::TaskDialog::SetButtonElevationRequired(int buttonId, 
                                                  bool required)
{
    SendMessage(TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE,
                buttonId,
                required);
}

void Kerr::TaskDialog::NavigatePage(TaskDialog& newDialog)
{
    ASSERT(0 == newDialog.m_hWnd);

    newDialog.m_config.pButtons = newDialog.m_buttons.GetData();
    newDialog.m_config.cButtons = static_cast<UINT>(newDialog.m_buttons.GetCount());
    newDialog.m_config.pRadioButtons = newDialog.m_radioButtons.GetData();
    newDialog.m_config.cRadioButtons = static_cast<UINT>(newDialog.m_radioButtons.GetCount());

    SendMessage(TDM_NAVIGATE_PAGE,
                0,
                reinterpret_cast<LPARAM>(&newDialog.m_config));

    this->Detach();
}

void Kerr::TaskDialog::ResetTimer() {
    m_resetTimer = true;
}

HRESULT Kerr::TaskDialog::Callback(HWND handle, 
                                   UINT notification, 
                                   WPARAM wParam, 
                                   LPARAM lParam, 
                                   LONG_PTR data)
{
    ASSERT(0 != handle);
    ASSERT(0 != data);

    TaskDialog* pThis = reinterpret_cast<TaskDialog*>(data);
    HRESULT result = S_OK;

    switch (notification)
    {
        case TDN_NAVIGATED:
        {
            pThis->OnNavigated();
            break;
        }
        case TDN_BUTTON_CLICKED:
        {
            bool closeDialog = false;
            pThis->OnButtonClicked(static_cast<int>(wParam), closeDialog);
            result = !closeDialog;
            break;
        }
        case TDN_HYPERLINK_CLICKED:
        {
            pThis->OnHyperlinkClicked(reinterpret_cast<PCWSTR>(lParam));
            break;
        }
        case TDN_TIMER:
        {
            ASSERT(UINT_MAX >= wParam);
            bool reset = pThis->m_resetTimer;
            if (!reset) {
                pThis->OnTimer(static_cast<DWORD>(wParam), reset);
            } else {
                pThis->OnTimer(0, reset);
                pThis->m_resetTimer = FALSE;
                reset = true;
            }
            result = reset;
            break;
        }
        case TDN_DESTROYED:
        {
            pThis->Detach();
            break;
        }
        case TDN_RADIO_BUTTON_CLICKED:
        {
            pThis->OnRadioButtonClicked(static_cast<int>(wParam));
            break;
        }
        case TDN_CREATED:
        {
            break;
        }
        case TDN_DIALOG_CONSTRUCTED:
        {
            pThis->Attach(handle);
            pThis->OnDialogConstructed();
            break;
        }
        case TDN_VERIFICATION_CLICKED:
        {
            pThis->OnVerificationClicked(0 != wParam);
            break;
        }
        case TDN_HELP:
        {
            pThis->OnHelp();
            break;
        }
        case TDN_EXPANDO_BUTTON_CLICKED:
        {
            pThis->OnExpandoButtonClicked(0 != wParam);
            break;
        }
        default:
        {
            TRACE(L"TaskDialog::Callback -- Unhandled notification encountered\n");
            break;
        }

    }

    return result;
}
