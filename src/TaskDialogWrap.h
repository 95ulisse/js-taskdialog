#pragma once

#include "TaskDialog.h"

#include <node.h>
#include <v8.h>

using namespace v8;

// ************************************************
// JSTaskDialog
// ************************************************

// Inherits Kerr::TaskDialog to forward events to a JS function
class JSTaskDialog : public Kerr::TaskDialog {
    public:
        JSTaskDialog(Persistent<Function> callback);
        ~JSTaskDialog();
    private:
        Persistent<Function> _callbackFunction;
        void RaiseJSEvent(const char* eventName, Handle<Value> e);
        void OnHyperlinkClicked(PCWSTR /*url*/);
        void OnButtonClicked(int /*buttonId*/, bool& closeDialog);
        void OnRadioButtonClicked(int /*buttonId*/);
        void OnVerificationClicked(bool /*checked*/);
        void OnExpandoButtonClicked(bool /*expanded*/);
};

JSTaskDialog::JSTaskDialog(Persistent<Function> callback) :
    _callbackFunction(callback)
{
    SetMainIcon((ATL::_U_STRINGorID)(UINT)0);
    SetFooterIcon((ATL::_U_STRINGorID)(UINT)0);
}

JSTaskDialog::~JSTaskDialog() {
    _callbackFunction.Dispose();
}

void JSTaskDialog::RaiseJSEvent(const char* eventName, Handle<Value> e)
{
    Handle<Value> arr[] = {
        String::New(eventName),
        e
    };
    _callbackFunction->Call(Context::GetCurrent()->Global(), 2, arr);
}

void JSTaskDialog::OnHyperlinkClicked(PCWSTR url) {
    char* str = "";
    Kerr::CopyWStrToStr(str, url);
    
    HandleScope scope;
    Handle<Object> e = Object::New();
    e->Set(String::NewSymbol("data"), String::New(str));
    RaiseJSEvent("click:link", e);
    scope.Close(Undefined());
}

void JSTaskDialog::OnButtonClicked(int buttonId, bool& closeDialog) {
    HandleScope scope;
    Handle<Object> e = Object::New();
    e->Set(String::NewSymbol("cancel"), Boolean::New(false));
    e->Set(String::NewSymbol("data"), Integer::New(buttonId));
    RaiseJSEvent("click:button", e);
    closeDialog = !e->Get(String::NewSymbol("cancel"))->BooleanValue();
    scope.Close(Undefined());
}

void JSTaskDialog::OnRadioButtonClicked(int buttonId) {
    HandleScope scope;
    Handle<Object> e = Object::New();
    e->Set(String::NewSymbol("data"), Integer::New(buttonId));
    RaiseJSEvent("click:radio", e);
    scope.Close(Undefined());
}

void JSTaskDialog::OnVerificationClicked(bool checked) {
    HandleScope scope;
    Handle<Object> e = Object::New();
    e->Set(String::NewSymbol("data"), Boolean::New(checked));
    RaiseJSEvent("click:verification", e);
    scope.Close(Undefined());
}

void JSTaskDialog::OnExpandoButtonClicked(bool expanded) {
    HandleScope scope;
    Handle<Object> e = Object::New();
    e->Set(String::NewSymbol("data"), Boolean::New(expanded));
    RaiseJSEvent("click:expando", e);
    scope.Close(Undefined());
}

// ************************************************
// TaskDialogWrap - Class definition
// ************************************************

#define PROTOTYPE_PROP_DEF(name) \
    static Handle<Value> Set##name(const Arguments& args);

class TaskDialogWrap : public node::ObjectWrap {

    public:
        static Handle<Function> Init();

    private:

        explicit TaskDialogWrap(JSTaskDialog* td);
        ~TaskDialogWrap();

        // Instance members
        JSTaskDialog* _taskDialog;

        // Private helper functions
        void RaiseEvent(const char* eventName, Handle<Value> eventData);

        // Constructor
        static Persistent<Function> _constructor;
        static Handle<Value> New(const Arguments& args);

        // Prototype properties implementation
        PROTOTYPE_PROP_DEF(WindowTitle)
        PROTOTYPE_PROP_DEF(MainInstruction)
        PROTOTYPE_PROP_DEF(Content)
        PROTOTYPE_PROP_DEF(CollapsedControlText)
        PROTOTYPE_PROP_DEF(ExpandedControlText)
        PROTOTYPE_PROP_DEF(ExpandedInformation)
        PROTOTYPE_PROP_DEF(VerificationText)
        PROTOTYPE_PROP_DEF(Footer)

        PROTOTYPE_PROP_DEF(UseLinks)
        PROTOTYPE_PROP_DEF(UseCommandLinks)
        PROTOTYPE_PROP_DEF(Cancelable)
        PROTOTYPE_PROP_DEF(Minimizable)

        PROTOTYPE_PROP_DEF(MainIcon)
        PROTOTYPE_PROP_DEF(FooterIcon)

        // Prototype methods
        static Handle<Value> Show(const Arguments& args);
        static Handle<Value> SetButtons(const Arguments& args);
        static Handle<Value> SetRadioButtons(const Arguments& args);
};

// ************************************************
// TaskDialogWrap - Implementation
// ************************************************

#define PROTOTYPE_PROP(proto, name) \
    proto->Set(String::NewSymbol("Set" #name), FunctionTemplate::New(TaskDialogWrap::Set##name)->GetFunction());

#define PROTOTYPE_PROP_STRING_IMPL(name) \
    Handle<Value> TaskDialogWrap::Set##name(const Arguments& args) { \
        if (args.Length() != 1 || !args[0]->IsString()) \
            return ThrowException(Exception::TypeError(String::New("Expected only one string argument"))); \
        JSTaskDialog* td = node::ObjectWrap::Unwrap<TaskDialogWrap>(args.This())->_taskDialog; \
        String::Utf8Value strJs(args[0]->ToString()); \
        td->Set##name(*strJs); \
        return Undefined(); \
    }

#define PROTOTYPE_PROP_BOOL_IMPL(name) \
    Handle<Value> TaskDialogWrap::Set##name(const Arguments& args) { \
        if (args.Length() != 1 || !args[0]->IsBoolean()) \
            return ThrowException(Exception::TypeError(String::New("Expected only one boolean argument"))); \
        JSTaskDialog* td = node::ObjectWrap::Unwrap<TaskDialogWrap>(args.This())->_taskDialog; \
        td->Set##name(args[0]->ToBoolean()->BooleanValue()); \
        return Undefined(); \
    }

#define PROTOTYPE_PROP_INT_IMPL(name) \
    Handle<Value> TaskDialogWrap::Set##name(const Arguments& args) { \
        if (args.Length() != 1 || !args[0]->IsNumber()) \
            return ThrowException(Exception::TypeError(String::New("Expected only one integer argument"))); \
        JSTaskDialog* td = node::ObjectWrap::Unwrap<TaskDialogWrap>(args.This())->_taskDialog; \
        td->Set##name(args[0]->ToNumber()->IntegerValue()); \
        return Undefined(); \
    }

Handle<Function> TaskDialogWrap::Init() {

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("TaskDialog"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype properties
    Handle<ObjectTemplate> proto = tpl->PrototypeTemplate();
    PROTOTYPE_PROP(proto, WindowTitle)
    PROTOTYPE_PROP(proto, MainInstruction)
    PROTOTYPE_PROP(proto, Content)
    PROTOTYPE_PROP(proto, CollapsedControlText)
    PROTOTYPE_PROP(proto, ExpandedControlText)
    PROTOTYPE_PROP(proto, ExpandedInformation)
    PROTOTYPE_PROP(proto, VerificationText)
    PROTOTYPE_PROP(proto, Footer)

    PROTOTYPE_PROP(proto, UseLinks)
    PROTOTYPE_PROP(proto, UseCommandLinks)
    PROTOTYPE_PROP(proto, Cancelable)
    PROTOTYPE_PROP(proto, Minimizable)
    
    PROTOTYPE_PROP(proto, MainIcon)
    PROTOTYPE_PROP(proto, FooterIcon)

    // Prototype methods
    proto->Set(String::NewSymbol("Show"), FunctionTemplate::New(Show)->GetFunction());
    proto->Set(String::NewSymbol("SetButtons"), FunctionTemplate::New(SetButtons)->GetFunction());
    proto->Set(String::NewSymbol("SetRadioButtons"), FunctionTemplate::New(SetRadioButtons)->GetFunction());

    // Actual constructor function
    _constructor = Persistent<Function>::New(tpl->GetFunction());
    return _constructor;
}


TaskDialogWrap::TaskDialogWrap(JSTaskDialog* td):
    _taskDialog(td)
{
}

TaskDialogWrap::~TaskDialogWrap() {
    if (_taskDialog)
        delete _taskDialog;
}

// Constructor
Persistent<Function> TaskDialogWrap::_constructor;
Handle<Value> TaskDialogWrap::New(const v8::Arguments& args) {

    // Makes sure that a single function is passed
    if (args.Length() != 1 || !args[0]->IsFunction())
        return ThrowException(Exception::TypeError(String::New("Expected only one function as parameter")));

    // Makes sure that it is used as a constructor
    if (!args.IsConstructCall()) {
        Handle<Value> arr[] = { args[0] };
        return _constructor->NewInstance(1, arr);
    }

    // Creates and wraps a TaskDialog
    JSTaskDialog* td = new JSTaskDialog(Persistent<Function>::New(Handle<Function>::Cast(args[0])));
    TaskDialogWrap* tdw = new TaskDialogWrap(td);
    tdw->Wrap(args.This());
    return args.This();

}

// Prototype properties implementation
PROTOTYPE_PROP_STRING_IMPL(WindowTitle)
PROTOTYPE_PROP_STRING_IMPL(MainInstruction)
PROTOTYPE_PROP_STRING_IMPL(Content)
PROTOTYPE_PROP_STRING_IMPL(CollapsedControlText)
PROTOTYPE_PROP_STRING_IMPL(ExpandedControlText)
PROTOTYPE_PROP_STRING_IMPL(ExpandedInformation)
PROTOTYPE_PROP_STRING_IMPL(VerificationText)
PROTOTYPE_PROP_STRING_IMPL(Footer)
PROTOTYPE_PROP_BOOL_IMPL(UseLinks)
PROTOTYPE_PROP_BOOL_IMPL(UseCommandLinks)
PROTOTYPE_PROP_BOOL_IMPL(Cancelable)
PROTOTYPE_PROP_BOOL_IMPL(Minimizable)
PROTOTYPE_PROP_INT_IMPL(MainIcon)
PROTOTYPE_PROP_INT_IMPL(FooterIcon)

// Prototype methods

Handle<Value> TaskDialogWrap::SetButtons(const Arguments& args) {
    
    // Checks arguments
    if (args.Length() != 1 || !args[0]->IsArray())
        return ThrowException(Exception::TypeError(String::New("Expected only one array as parameter")));
    
    // TaskDialog
    TaskDialogWrap* tdw = node::ObjectWrap::Unwrap<TaskDialogWrap>(args.This());
    JSTaskDialog* td = tdw->_taskDialog;

    HandleScope scope;

    // Builds the buttons list from the JS array
    Handle<Array> arr = Handle<Array>::Cast(args[0]);
    td->Buttons().RemoveAll();
    for (int i = 0; i < arr->Length(); i++) {
        if (!arr->Get(i)->IsArray())
            return ThrowException(Exception::TypeError(String::New("Parameter must be an array of arrays, where the first member is a custom value and the second one is the text to display")));
        Handle<Array> pair = Handle<Array>::Cast(arr->Get(i));
        if (pair->Length() != 2 || !pair->Get(1)->IsString())
            return ThrowException(Exception::TypeError(String::New("Parameter must be an array of arrays, where the first member is a custom value and the second one is the text to display")));
        String::Utf8Value strVal(pair->Get(1)->ToString());
        td->AddButton(*strVal, 101 + i);
    }

    return scope.Close(Undefined());

}

Handle<Value> TaskDialogWrap::SetRadioButtons(const Arguments& args) {
    
    // Checks arguments
    if (args.Length() != 1 || !args[0]->IsArray())
        return ThrowException(Exception::TypeError(String::New("Expected only one array as parameter")));
    
    // TaskDialog
    TaskDialogWrap* tdw = node::ObjectWrap::Unwrap<TaskDialogWrap>(args.This());
    JSTaskDialog* td = tdw->_taskDialog;

    HandleScope scope;

    // Builds the buttons list from the JS array
    Handle<Array> arr = Handle<Array>::Cast(args[0]);
    td->RadioButtons().RemoveAll();
    for (int i = 0; i < arr->Length(); i++) {
        if (!arr->Get(i)->IsArray())
            return ThrowException(Exception::TypeError(String::New("Parameter must be an array of arrays, where the first member is a custom value and the second one is the text to display")));
        Handle<Array> pair = Handle<Array>::Cast(arr->Get(i));
        if (pair->Length() != 2 || !pair->Get(1)->IsString())
            return ThrowException(Exception::TypeError(String::New("Parameter must be an array of arrays, where the first member is a custom value and the second one is the text to display")));
        String::Utf8Value strVal(pair->Get(1)->ToString());
        td->AddRadioButton(*strVal, 101 + i);
    }

    return scope.Close(Undefined());
}

Handle<Value> TaskDialogWrap::Show(const Arguments& args) {
    
    // Extracts the Task dialog
    TaskDialogWrap* tdw = node::ObjectWrap::Unwrap<TaskDialogWrap>(args.This());
    JSTaskDialog* td = tdw->_taskDialog;
    td->DoModal();

    // Creates a new object to hold the results
    HandleScope scope;
    Handle<Object> obj = Object::New();
    obj->Set(String::NewSymbol("button"), Integer::New(td->GetSelectedButtonId()));
    obj->Set(String::NewSymbol("radio"), Integer::New(td->GetSelectedRadioButtonId()));
    obj->Set(String::NewSymbol("verification"), Boolean::New(td->VerificiationChecked()));
    return scope.Close(obj);

}

#undef PROTOTYPE_PROP_STRING
#undef PROTOTYPE_PROP_STRING_IMPL