var TaskDialog = require('../'),
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
        Buttons: [],
        UseProgressBar: true,
        UseTimer: true
    }),
    third = new TaskDialog({
        WindowTitle: 'Third step',
        MainInstruction: 'Third step',
        Content: 'Now choose:',
        Buttons: [
            [ 'close', 'Close' ],
            [ 'close-anyways', 'Close anyways' ]
        ],
        UseCommandLinks: true
    });

// Button handlers for first step
first.on('click:button', function (e) {
    if (e.data === 'next')
        first.Navigate(second);
});

// Configures the timer for the second step
second.on('timer', function (e) {
    second.ProgressBarPosition = Math.min(Math.round(100 * e.data / 5000), 100);
    second.MainInstruction = second.ProgressBarPosition + '%';
    if (e.data > 5000)
        second.Navigate(third);
});

// Shows the first step
first.Show(function (res) {
    new TaskDialog({ MainInstruction: 'You chose:', Content: res.button }).Show();
});