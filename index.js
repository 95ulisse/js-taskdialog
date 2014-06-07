var TaskDialogNative = require('./build/Debug/TaskDialog'),
    EventEmitter = require('events').EventEmitter,
    util = require('util'),

    ICONS = {
        'none': 0,
        'warning': -1,
        'error': -2,
        'info': -3,
        'shield': -4
    },

    STANDARD_BUTTONS = {
        1: 'ok',
        2: 'cancel'
    },

    PROGRESSBAR_STATE = {
        'normal' : 1,
        'error': 2,
        'paused': 3
    };

// Helper function to define an hidden property (non enumerable, non configurable, but writable)
function defineHiddenProperty(obj, name, value) {
    Object.defineProperty(obj, name, {
        configurable: false,
        enumerable: false,
        writable: true,
        value: value
    });
}

// Helper function to wrap a native Set* method in a property-like interface
function wrapNativeMethod(prop, beforeSet, canNativeSet) {
    Object.defineProperty(TaskDialog.prototype, prop, {
        configurable: false,
        enumerable: true,
        get: function () {
            return this['_' + prop];
        },
        set: function (val) {
            if (beforeSet)
                val = beforeSet.call(this, val);
            if (!Object.prototype.hasOwnProperty.call(this, '_' + prop))
                defineHiddenProperty(this, '_' + prop, val);
            else
                this['_' + prop] = val;
            if (!canNativeSet || canNativeSet.call(this))
                this._native['Set' + prop](val);
        }
    });
}

// TaskDialog class
function TaskDialog(config) {

    // EventEmitter constructor
    EventEmitter.call(this);

    // Hidden property to store the native object
    defineHiddenProperty(this, '_native', new TaskDialogNative(function (eventName, eventData) {

        // Before passing the events to the user, we need to adjust the results
        // translating native IDs to meaningful data
        switch(eventName) {
            case 'click:button':
                if (eventData.data > 1000) // Removes the increment of 1000 for message-only buttons
                    eventData.data -= 1000;
                if (eventData.data >= 101)
                    eventData.data = this.Buttons[eventData.data - 101][0];
                if (eventData.data in STANDARD_BUTTONS)
                    eventData.data = STANDARD_BUTTONS[eventData.data];
                break;
            case 'click:radio':
                if (eventData.data >= 101)
                    eventData.data = this.RadioButtons[eventData.data - 101][0];
                break;
        }
        this.emit(eventName, eventData);

    }.bind(this)));

    // Collections
    this.Buttons = [];
    this.RadioButtons = [];

    // Shortcut properties via constructor
    if(config)
        for (var k in config)
            this[k] = config[k];

    // Registers an handler for the `loaded` event to make sure
    // that properties usable only when the dialog is visible
    // are re-set to their current value so that a message to the dialog is sent to update the UI
    this.on('loaded', function () {
        this.ProgressBarMarquee = this.ProgressBarMarquee;
        this.ProgressBarPosition = this.ProgressBarPosition;
        this.ProgressBarState = this.ProgressBarState;
    });
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
    'UseProgressBar',
    'Cancelable',
    'Minimizable'
];
for (var i = 0; i < methods.length; i++)
    wrapNativeMethod(methods[i]);

// Wraps the Set*Icon methods of the native interface in a property-like interface.
// This methods should translate icon indexes to a meaningful string,
// and must enure that the nerby text isn't null, otherwise the app will crash.
methods = [
    [ 'MainIcon', 'MainInstruction' ],
    [ 'FooterIcon', 'Footer' ]
];
for (var i = 0; i < methods.length; i++)
    (function (prop) {
        wrapNativeMethod(prop[0], function (val) {
            if (!Object.prototype.hasOwnProperty.call(this, '_' + prop[1]))
                throw new Error('Before setting ' + prop[0] + ', ensure that ' + prop[1] + ' has a value');
            if (!(val in ICONS))
                throw new Error('Unknown icon: ' + val);
            return ICONS[val];
        });
    })(methods[i]);

// Wraps the progress bar methods.
// These methods can be set at any time,
// but we must ensure that if the dialog isn't visible,
// the native method must not be called
wrapNativeMethod('ProgressBarMarquee', null, function () { return this.IsVisible; })
wrapNativeMethod('ProgressBarPosition', null, function () { return this.IsVisible; });
wrapNativeMethod(
    'ProgressBarState',
    function (val) {
        if (!(val in PROGRESSBAR_STATE))
            throw new Error('Unknown state: ' + val);
        return PROGRESSBAR_STATE[val];
    },
    function () {
        return this.IsVisible;
    }
);

// Show method
TaskDialog.prototype.Show = function (cb) {

    // Makes sure that buttons are up to date
    this._native.SetButtons(this.Buttons || []);
    this._native.SetRadioButtons(this.RadioButtons || []);

    // Shows the dialog
    this.IsVisible = true;
    this._native.Show(function (res) {

        this.IsVisible = false;

        // Maps the native results to meaningful data
        if (res.button > 1000) // Removes the increment of 1000 for message-only buttons
            res.button -= 1000;
        if (res.button >= 101)
            res.button = this.Buttons[res.button - 101][0];
        if (res.button in STANDARD_BUTTONS)
            res.button = STANDARD_BUTTONS[res.button];
        if (res.radio >= 101)
            res.radio = this.RadioButtons[res.radio - 101][0];

        // Calls the callback
        if (cb)
            cb(res);

    }.bind(this));

};

// Freezes TaskDialog prototype
Object.freeze(TaskDialog.prototype);

// Exports the TaskDialog class
module.exports = TaskDialog;