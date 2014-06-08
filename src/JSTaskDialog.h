#pragma once

#include "TaskDialog.h"

#include <node.h>
#include <v8.h>
#include "cvv8/v8-convert.hpp"
#include <uv.h>

using namespace v8;

// ************************************************
// JSTaskDialog - Class definition
// ************************************************

// Inherits Kerr::TaskDialog to forward events to a JS function
// and handle asynchronous comunication between the main thread (JS) and the worker thread.
class JSTaskDialog : public Kerr::TaskDialog {

    public:
        JSTaskDialog(Persistent<Function> callback);
        ~JSTaskDialog();
        HRESULT DoModal(HWND parent = ::GetActiveWindow());
        void NavigatePage(TaskDialog& dest);

    private:

        Persistent<Function> _callbackFunction;
        uv_async_t* _async;
        static void AsyncMessageHandler(uv_async_t* handle, int status);

        class AsyncMessageDataBuilderBase {
            public:
                virtual Handle<Value> Build() { throw "Build() method not implemented"; }
        };

        template<typename T>
        class AsyncMessageDataBuilder : public AsyncMessageDataBuilderBase {
            public:
                AsyncMessageDataBuilder(T value);
                Handle<Value> Build();
            private:
                T _value;
        };

        struct AsyncMessageBaton {
            JSTaskDialog* td;
            const char* eventName;
            AsyncMessageDataBuilderBase* dataBuilder;
        };

        void RaiseJSEvent(const char* eventName, AsyncMessageDataBuilderBase* dataBuilder);
        void OnDialogConstructed();
        void OnNavigated();
        void OnHyperlinkClicked(PCWSTR /*url*/);
        void OnButtonClicked(int /*buttonId*/, bool& closeDialog);
        void OnRadioButtonClicked(int /*buttonId*/);
        void OnVerificationClicked(bool /*checked*/);
        void OnExpandoButtonClicked(bool /*expanded*/);
        void OnTimer(DWORD /*milliseconds*/, bool& /*reset*/);
};

// ************************************************
// JSTaskDialog - Implementation
// ************************************************

// Implementation of the methods for the AsyncMessageDataBuilder.
// These classes are needed to provide a specific method to encapsulate values from the worker thread
// and a generic method to allow the main thread to construct the correct corresponding v8 types.

template<typename T>
JSTaskDialog::AsyncMessageDataBuilder<T>::AsyncMessageDataBuilder(T value) :
    _value(value)
{
}

template<typename T>
Handle<Value> JSTaskDialog::AsyncMessageDataBuilder<T>::Build() {
    return cvv8::CastToJS<T>(_value);
}

//-----------------

JSTaskDialog::JSTaskDialog(Persistent<Function> callback) :
    _callbackFunction(callback),
    _async(NULL)
{
    SetMainIcon((ATL::_U_STRINGorID)(UINT)0);
    SetFooterIcon((ATL::_U_STRINGorID)(UINT)0);
}

JSTaskDialog::~JSTaskDialog() {
    _callbackFunction.Dispose();
    if (_async)
        delete _async;
}

// This function is called on the main thread, and is the only one allowed to use v8
void JSTaskDialog::AsyncMessageHandler(uv_async_t* handle, int status) {
    AsyncMessageBaton* baton = (AsyncMessageBaton*)handle->data;
    
    HandleScope scope;

    Handle<Object> eventObject = Object::New();
    eventObject->Set(String::NewSymbol("data"), baton->dataBuilder ? baton->dataBuilder->Build() : Undefined());

    Handle<Value> arr[] = {
        String::New(baton->eventName),
        eventObject
    };
    baton->td->_callbackFunction->Call(Context::GetCurrent()->Global(), 2, arr);

    delete baton->dataBuilder;
    delete baton;
}

void JSTaskDialog::RaiseJSEvent(const char* eventName, AsyncMessageDataBuilderBase* dataBuilder)
{
    AsyncMessageBaton* baton = new AsyncMessageBaton();
    baton->td = this;
    baton->eventName = eventName;
    baton->dataBuilder = dataBuilder;
    _async->data = baton;
    uv_async_send(_async);
}

HRESULT JSTaskDialog::DoModal(HWND parent) {
    uv_async_t a;
    _async = &a; // We need to store a pointer to the uv_async_t because it must be shared between objects while navigating
    uv_async_init(uv_default_loop(), _async, JSTaskDialog::AsyncMessageHandler);
    HRESULT res = Kerr::TaskDialog::DoModal(parent);
    uv_close((uv_handle_t*)_async, NULL);
    return res;
}

void JSTaskDialog::NavigatePage(TaskDialog& dest) {
    ((JSTaskDialog&)dest)._async = this->_async;
    Kerr::TaskDialog::NavigatePage(dest);
}

void JSTaskDialog::OnDialogConstructed() {
    RaiseJSEvent("loaded", NULL);
}

void JSTaskDialog::OnNavigated() {
    RaiseJSEvent("navigated", NULL);
}

void JSTaskDialog::OnHyperlinkClicked(PCWSTR url) {
    char* str = "";
    Kerr::CopyWStrToStr(str, url);
    
    RaiseJSEvent("click:link", new AsyncMessageDataBuilder<const char*>(str));
}

void JSTaskDialog::OnButtonClicked(int buttonId, bool& closeDialog) {
    closeDialog = buttonId < 1000; // Conventionally, message-only buttons have an ID > 1000
    RaiseJSEvent("click:button", new AsyncMessageDataBuilder<int>(buttonId));
}

void JSTaskDialog::OnRadioButtonClicked(int buttonId) {
    RaiseJSEvent("click:radio", new AsyncMessageDataBuilder<int>(buttonId));
}

void JSTaskDialog::OnVerificationClicked(bool checked) {
    RaiseJSEvent("click:verification", new AsyncMessageDataBuilder<bool>(checked));
}

void JSTaskDialog::OnExpandoButtonClicked(bool expanded) {
    RaiseJSEvent("click:expando", new AsyncMessageDataBuilder<bool>(expanded));
}

void JSTaskDialog::OnTimer(DWORD milliseconds, bool& reset) {
    reset = false;
    RaiseJSEvent("timer", new AsyncMessageDataBuilder<unsigned long>(milliseconds));
}