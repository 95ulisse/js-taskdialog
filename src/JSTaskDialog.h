#pragma once

#include "TaskDialog.h"

#include <node.h>
#include <v8.h>
#include "cvv8/v8-convert.hpp"
#include <uv.h>

#include <vector>
#include <queue>

using namespace v8;

// ************************************************
// JSTaskDialog - Class definition
// ************************************************

// Inherits Kerr::TaskDialog to forward events to a JS function
// and handle asynchronous comunication between the main thread (JS) and the worker thread.
class JSTaskDialog : public Kerr::TaskDialog {

    public:

        static void Initialize();

        JSTaskDialog(Persistent<Function> callback);
        ~JSTaskDialog();
        HRESULT DoModal(HWND parent = ::GetActiveWindow());
        void NavigatePage(TaskDialog& dest);

    private:

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

        Persistent<Function> _callbackFunction;
        static uv_async_t _async;
        static int _asyncReferences;
        static std::queue<AsyncMessageBaton*>* _asyncMessages;
        static uv_rwlock_t _asyncMessagesLock;
        static void AsyncMessageHandler(uv_async_t* handle, int status);

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
    _callbackFunction(callback)
{
    SetMainIcon((ATL::_U_STRINGorID)(UINT)0);
    SetFooterIcon((ATL::_U_STRINGorID)(UINT)0);
}

JSTaskDialog::~JSTaskDialog() {
    _callbackFunction.Dispose();
}

// Static initialization
void JSTaskDialog::Initialize() {
    _asyncMessages = new std::queue<AsyncMessageBaton*>();
    _asyncReferences = 0;
    uv_rwlock_init(&_asyncMessagesLock);
}

// Async watcher
uv_async_t JSTaskDialog::_async;
int JSTaskDialog::_asyncReferences;

// Message queue
std::queue<JSTaskDialog::AsyncMessageBaton*>* JSTaskDialog::_asyncMessages;
uv_rwlock_t JSTaskDialog::_asyncMessagesLock;

// This function is called on the main thread, and is the only one allowed to use v8
void JSTaskDialog::AsyncMessageHandler(uv_async_t* handle, int status) {
    HandleScope scope;

    // Copyies the contents of the queue to a temporary array.
    // We need to do this because we can't process the messages while keeping the read lock,
    // because a callback could place a new message on the queue, causing a deadlock
    std::vector<AsyncMessageBaton*> batons;
    uv_rwlock_rdlock(&_asyncMessagesLock);
        while (!_asyncMessages->empty()) {
            batons.push_back(_asyncMessages->front());
            _asyncMessages->pop();
        }
    uv_rwlock_rdunlock(&_asyncMessagesLock);

    // Processes all the messages on the queue
    for (auto it = batons.begin(); it < batons.end(); ++it) {
            
        AsyncMessageBaton* baton = *it;

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
}

void JSTaskDialog::RaiseJSEvent(const char* eventName, AsyncMessageDataBuilderBase* dataBuilder)
{
    AsyncMessageBaton* baton = new AsyncMessageBaton();
    baton->td = this;
    baton->eventName = eventName;
    baton->dataBuilder = dataBuilder;

    uv_rwlock_wrlock(&_asyncMessagesLock);
        _asyncMessages->push(baton);
    uv_rwlock_wrunlock(&_asyncMessagesLock);
    uv_async_send(&_async);
}

HRESULT JSTaskDialog::DoModal(HWND parent) {
    if (_asyncReferences == 0)
        uv_async_init(uv_default_loop(), &_async, JSTaskDialog::AsyncMessageHandler);

    _asyncReferences++;
    HRESULT res = Kerr::TaskDialog::DoModal(parent);
    _asyncReferences--;

    if (_asyncReferences == 0)
        uv_close((uv_handle_t*)&_async, NULL);

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