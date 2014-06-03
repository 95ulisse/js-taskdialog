#include "TaskDialogWrap.h"

#include <node.h>
#include <v8.h>

using namespace v8;

void InitNode(Handle<Object> exports, Handle<Object> module) {
    
    // Initializes the TaskDialogWrap and sets it as the module exports
    module->Set(String::NewSymbol("exports"), TaskDialogWrap::Init());

}

NODE_MODULE(TaskDialog, InitNode)