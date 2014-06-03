#pragma once

#include "TaskDialog.h"

#include <node.h>
#include <v8.h>

using namespace v8;

// ************************************************
// Class definition
// ************************************************

#define PROTOTYPE_PROP_IMPL(name) \
    static Handle<Value> Set##name(const Arguments& args);

class TaskDialogWrap : public node::ObjectWrap {

    public:
        static Handle<Function> Init();

    private:

        explicit TaskDialogWrap(Kerr::TaskDialog* td);
        ~TaskDialogWrap();

        // Instance members
        Kerr::TaskDialog* _taskDialog;

        // Constructor
        static Persistent<Function> _constructor;
        static Handle<Value> New(const Arguments& args);

        // Prototype properties implementation
        PROTOTYPE_PROP_IMPL(WindowTitle)
        PROTOTYPE_PROP_IMPL(MainInstruction)
        PROTOTYPE_PROP_IMPL(Content)
        PROTOTYPE_PROP_IMPL(CollapsedControlText)
        PROTOTYPE_PROP_IMPL(ExpandedControlText)
        PROTOTYPE_PROP_IMPL(ExpandedInformation)
        PROTOTYPE_PROP_IMPL(VerificationText)
        PROTOTYPE_PROP_IMPL(Footer)

        PROTOTYPE_PROP_IMPL(UseLinks)
        PROTOTYPE_PROP_IMPL(Cancelable)
        PROTOTYPE_PROP_IMPL(Minimizable)

        // Prototype methods
        static Handle<Value> Show(const Arguments& args);
};

// ************************************************
// Implementation
// ************************************************

#define PROTOTYPE_PROP(proto, name) \
    proto->Set(String::NewSymbol("Set" #name), FunctionTemplate::New(TaskDialogWrap::Set##name)->GetFunction());

#define PROTOTYPE_PROP_STRING_IMPL(name) \
    Handle<Value> TaskDialogWrap::Set##name(const Arguments& args) { \
        if (args.Length() != 1 || !args[0]->IsString()) \
            return ThrowException(Exception::TypeError(String::New("Expected only one string argument"))); \
        Kerr::TaskDialog* td = node::ObjectWrap::Unwrap<TaskDialogWrap>(args.This())->_taskDialog; \
        String::Utf8Value strJs(args[0]->ToString()); \
        td->Set##name(*strJs); \
        return Undefined(); \
    }

#define PROTOTYPE_PROP_BOOL_IMPL(name) \
    Handle<Value> TaskDialogWrap::Set##name(const Arguments& args) { \
        if (args.Length() != 1 || !args[0]->IsBoolean()) \
            return ThrowException(Exception::TypeError(String::New("Expected only one boolean argument"))); \
        Kerr::TaskDialog* td = node::ObjectWrap::Unwrap<TaskDialogWrap>(args.This())->_taskDialog; \
        td->Set##name(args[0]->ToBoolean()->BooleanValue()); \
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
    PROTOTYPE_PROP(proto, Cancelable)
    PROTOTYPE_PROP(proto, Minimizable)

    // Prototype methods
    proto->Set(String::NewSymbol("Show"), FunctionTemplate::New(Show)->GetFunction());

    // Actual constructor function
    _constructor = Persistent<Function>::New(tpl->GetFunction());
    return _constructor;
}


TaskDialogWrap::TaskDialogWrap(Kerr::TaskDialog* td):
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

    // Makes sure that it is used as a constructor
    if (!args.IsConstructCall())
        return _constructor->NewInstance();

    // Creates and wraps a TaskDialog
    Kerr::TaskDialog* td = new Kerr::TaskDialog();
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
PROTOTYPE_PROP_BOOL_IMPL(Cancelable)
PROTOTYPE_PROP_BOOL_IMPL(Minimizable)

// Prototype methods
Handle<Value> TaskDialogWrap::Show(const Arguments& args) {
    
    // Shows the modal
    Kerr::TaskDialog* td = node::ObjectWrap::Unwrap<TaskDialogWrap>(args.This())->_taskDialog;
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