var TaskDialogNative = require('./build/Debug/TaskDialog'),
    EventEmitter = require('events').EventEmitter,
    util = require('util');

// Helper function to define an hidden property (non enumerable, non configurable, but writable)
function defineHiddenProperty(obj, name, value) {
    Object.defineProperty(obj, name, {
        configurable: false,
        enumerable: false,
        writable: true,
        value: value
    });
}

// TaskDialog class
function TaskDialog(config) {

    // EventEmitter constructor
    EventEmitter.call(this);

    // Hidden property to store the native object
    defineHiddenProperty(this, '_native', new TaskDialogNative((function (eventName, eventData) {

        // Before passing the events to the user, we need to adjust the results
        // translating native IDs to meaningful data
        switch(eventName) {
            case 'click:button':
                if (eventData.data >= 101)
                    eventData.data = this.Buttons[eventData.data - 101][0];
                break;
            case 'click:radio':
                if (eventData.data >= 101)
                    eventData.data = this.RadioButtons[eventData.data - 101][0];
                break;
        }
        this.emit(eventName, eventData);

    }).bind(this)));

    // Collections
    this.Buttons = [];
    this.RadioButtons = [];

    // Shortcut properties via constructor
    if(config)
        for (var k in config)
            this[k] = config[k];
}

// Inherits EventEmitter
util.inherits(TaskDialog, EventEmitter);

// Wraps the Set* methods of the native interface in a property-like interface
var methods = [
    'WindowTitle',
    'MainInstruction',
    'Content',
    'CollapsedControlText',
    'ExpandedControlText',
    'ExpandedInformation',
    'VerificationText',
    'Footer',
    'UseLinks',
    'UseCommandLinks',
    'Cancelable',
    'Minimizable',
    'MainIcon',
    'FooterIcon'
];
for (var i = 0; i < methods.length; i++)
    (function (prop) {
        Object.defineProperty(TaskDialog.prototype, prop, {
            configurable: false,
            enumerable: true,
            get: function () {
                return this['_' + prop];
            },
            set: function (val) {
                if (!Object.prototype.hasOwnProperty.call(this, '_' + prop))
                    defineHiddenProperty(this, '_' + prop, val);
                else
                    this['_' + prop] = val;
                this._native['Set' + prop](val);
            }
        });
    })(methods[i]);

// Show method
TaskDialog.prototype.Show = function () {

    // Makes sure that buttons are up to date
    this._native.SetButtons(this.Buttons || []);
    this._native.SetRadioButtons(this.RadioButtons || []);

    // Shows the dialog
    var res = this._native.Show();

    // Maps the native results to meaningful data
    if (res.button >= 101)
        res.button = this.Buttons[res.button - 101][0];
    if (res.radio >= 101)
        res.radio = this.RadioButtons[res.radio - 101][0];

    // Returns the result
    return res;
};

// Freezes TaskDialog prototype
Object.freeze(TaskDialog.prototype);

// Exports the TaskDialog class
module.exports = TaskDialog;

/*ICONS = {
    'none': 0,
    'warning': -1,
    'error': -2,
    'info': -3,
    'shield': -4
};*/