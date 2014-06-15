# JS-TaskDialog

`js-taskdialog` is a wrapper around the native Windows TaskDialog API, which allows the creation of rich interactive dialogs directly from JavaScript (works from Windows Vista on).

**This project was born as an experiment to learn C++, so consider that there might be bugs, leaks and so on.**



## How to install

Just issue from the command line:

    npm install js-taskdialog

Note that since this project contains a native addon, you will need to satisfy other dependencies in order to compile the C++ source files:

* Microsoft Visual Studio C++ 2010 (at least) (express edition is fine)
* Python (v2.7.3 recommended, v3.x.x is not supported)
* `node-gyp` globally installed

With the newer versions of Visual Studio, `node-gyp` fails to recognize the correct version of the tools, so try this, if the above does not work:
    
    npm install js-taskdialog --msvs_version=2012   # 2012 if you have VS2012

If you want to know more about `node-gyp` visit its [repo](https://github.com/TooTallNate/node-gyp)!



## Getting started: a simple dialog

In your js file, simply require `js-taskdialog` and you get back the `TaskDialog` constructor, the main entry point for this library.

    var TaskDialog = require('js-taskdialog'),
        td = new TaskDialog({
            Content: 'Hello world!'
        });

    td.Show();

First of all, instantiate a new `TaskDialog`, passing to the constructor an object containing the options (for a full list of all the available options, scroll down below), then call the `Show()` function to show the dialog.

**Note**: the `Show()` function is asynchronous, this means that it will return immediately, but the process won't terminate until all the visible dialogs are closed.



## Getting started: buttons, radios, check boxes

The `Buttons` option holds an array where each element describes a button; each button is described by another array where the first element is the value that the button holds (can be any javascript value), and the second element is the caption text of the button.

    td = new TaskDialog({
        Content: 'Hello world!',
        Buttons: [
            [ 'first', 'First button' ],
            [ 'second', 'Second button' ]
        ]
    });

    td.Show(function (res) {
        console.log('You clicked the ' + res.button + ' button');
    });

*To show the buttons as command links (that giant buttons with a green arrow), simpy set `UseCommandLinks: true`*

When the callback function passed to `Show()` get called, the object `res` holds 3 properties:

* `button`: the value of the clicked button
* `radio`: the value of the selected radio
* `verification`: boolean representing the state of the checkbox

Radio buttions are implemented in the same way, but instead of `Buttons` the option is called `RadioButtons`:

    td = new TaskDialog({
        Content: 'Hello world!',
        RadioButtons: [
            [ 'first', 'First option' ],
            [ 'second', 'Second option' ]
        ]
    });

    td.Show(function (res) {
        console.log('You selected the ' + res.radio + ' option');
    });

To show the verification check, simply set the `VerificationText` property:

    td = new TaskDialog({
        Content: 'Hello world!',
        VerificationText: 'Remind me next time'
    });

    td.Show(function (res) {
        console.log('You ' + (res.verification ? 'WILL' : 'will NOT') + ' be reminded next time');
    });



## Interactivity and events

Clicking a button will close the dialog, but before the dialog is closed and the callback to the `Show` function gets called, the user can interact with the dialog in a lot of different ways. All the interaction made directly on the dialog is delivered to JavaScript through events (`TaskDialog` inherits node's `EventEmitter`). Let's make an example:

    td = new TaskDialog({
        Content: 'Hello world!',
        Buttons: [
            [ 'first', 'First button' ],
            [ 'second', 'Second button' ],
            [ 42, 'Special button', true ]
        ],
        UseCommandLinks: true
    });

    td.on('click:button', function (e) {
        console.log('[EVENT] Clicked button ' + e.data);
    });

    td.Show(function (res) {
        console.log('You clicked the ' + res.button + ' button');
    });

First of all, you can register an event listener using the method `on`, and passing the name of the event and an handler function. The event `click:button` is raised when a button is clicked (obviously), and the handler receives a parameter `e` which contains data specific to the raised event (in this case the button value). All the supported events are:

* `loaded`: Raised when the dialog is constructed and shown. No data.
* `click:button`: Raised when a button is clicked. Data: button value.
* `click:radio`: Raised when a radio button is clicked. Data: radio value.
* `click:verification`: Raised when the verification check box is clicked. Data: boolean representing the state of the check box.
* `click:expando`: Raised when the expando button is clicked. Data: boolean representing the state of the expando button.
* `click:link`: Raised when a link is clicked. Data: link url.
* `timer`: Scroll down below for more information
* `navigated`: Scroll down below for more information

So, in the example above, if we click the button *Special button* and then *Second button*, we see the following console output:

    [EVENT] Clicked button 42
    [EVENT] Clicked button second
    You clicked the second button

Two important things to notice:

* When the *Special button* gets clicked, it does not close the dialog, because it is marked as an event-only button. To make a button not to close the dialog, add a `true` as a third value of the array.
* Even if a closing-dialog button is clicked, before the `Show` callback is invoked, an event is fired.



## Timer and progress bar

    td = new TaskDialog({
        WindowTitle: 'Timer example',
        MainInstruction: '0',
        Buttons: [
            [ 'reset', 'Reset', true ],
            [ 'close', 'Close' ]
        ],
        UseTimer: true
    });

    td.on('timer', function (e) {
        td.MainInstruction = (e.data / 1000).toFixed(2) + ' seconds';
    });

    td.on('click:button', function (e) {
        if (e.data === 'reset')
            td.ResetTimer();
    });

First of all, to enable the timer, pass `true` to the `UseTimer` option, then register a listener for the `timer` event to get a notification every tick of the timer. The event data contains the number of milliseconds since the timer has started. To reset the timer simply call `ResetTimer` on the TaskDialog and, on the next tick, the timer will be reset. Nothing more.

Now, let's get to the progress bar.

    td = new TaskDialog({
        WindowTitle: 'ProgressBar example',
        UseProgressBar: true,
        ProgressBarPosition: 25,
        ProgressBarState: 'normal',
        ProgressBarMarquee: false
    })

Again, to enable the progress bar, pass `true` to the `UseProgressBar` option. The progress bar has range from 1 to 100, and its current position is controlled by the `ProgressBarPosition` property. Progress bars can have a "state", which is represented by the `ProgressBarState` property: this property can accept as values only `normal`, `error`, `paused` to get a green, red or yellow bar. If you instead don't have any precise position of the progress, enable the `ProgressBarMarquee` property to get an indefinite progress bar (note that the marquee works only if the progress bar is in `normal` state).



## Navigation

TaskDialogs provide a special interface to create wizard-like dialogs: you can navigate between different dialogs without closing and reopening any window; this is called **navigation**.

    first = new TaskDialog({
        WindowTitle: 'First step',
        MainInstruction: 'First step',
        Buttons: [
            [ 'next', 'Begin', true ]
        ],
        UseCommandLinks: true
    }),
    second = new TaskDialog({
        WindowTitle: 'Second step',
        MainInstruction: 'Second step',
        Content: 'Now choose:',
        Buttons: [
            [ '2-close', 'Close' ],
            [ '2-close-anyways', 'Close anyways' ]
        ],
        UseCommandLinks: true
    })

    first.on('click:button', function (e) {
        if (e.data === 'next')
            first.Navigate(second);
    });

    first.Show(function (res) {
        new TaskDialog({ MainInstruction: 'You chose:', Content: res.button }).Show();
    });

The above examples constructs two different dialogs, and makes the first one visible. When the user clicks on the *Begin* button, the first dialog navigates to the second one (`Navigate` method), and the latter becomes visible. Now the user can choose between two buttons (*Close* and *Close anyways*): both of them will close the dialog and invoke the callback passed to the `Show` method. Note that **even if the `Show` method was invoked on the first dialog, it gets the results of the second dialog**, since it is the last dialog that the user navigated to.



# Examples

Check all the examples in the `/examples/` directory: every file shows a single feature.



# Credits

Kudos to Kenny Kerr and his [TaskDialog](http://weblogs.asp.net/kennykerr/Windows-Vista-for-Developers-_1320_-Part-2-_1320_-Task-Dialogs-in-Depth) class.



# License

This project is released under the terms of the [MIT license](http://opensource.org/licenses/MIT).