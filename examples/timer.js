var TaskDialog = require('../'),
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

td.Show();