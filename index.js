var TaskDialogNative = require('./build/Debug/TaskDialog'),
    ICONS = {
        'none': 0,
        'warning': -1,
        'error': -2,
        'info': -3,
        'shield': -4
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

// TaskDialog class
function TaskDialog(config) {
    defineHiddenProperty(this, '_native', new TaskDialogNative());
    if(config)
        for (var k in config)
            this[k] = config[k];
}

// Wraps the Set* methods of the native interface in a property-like interface
for (var k in TaskDialogNative.prototype)
    if (k.substring(0, 3) === 'Set')
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
        })(k.substring(3));

// Show method
TaskDialog.prototype.Show = function () {
    return this._native.Show();
};

// Freezes TaskDialog prototype
Object.freeze(TaskDialog.prototype);

// Exports the TaskDialog class
module.exports = TaskDialog;