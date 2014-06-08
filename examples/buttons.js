var TaskDialog = require('../'),
    td = new TaskDialog({
        WindowTitle: 'Buttons example',
        MainInstruction: 'Choose an action',
        Buttons: [
            [ 'first', 'First action' ],
            [ 'second', 'Second action' ],
            [ 'never', 'This button does not close the dialog\nBut raises an event!', true ]
        ],
        UseCommandLinks: true
    });

td.on('click:button', function (e) {
    console.log('Button clicked: ', e.data);
    td.MainInstruction = td.WindowTitle = 'Hey!'
    td.Content = 'Look at the console';
});

td.Show(function (res) {
    switch(res.button) {
        case 'first':
            new TaskDialog({ Content: 'The first action is always the good one!' }).Show();
            break;
        case 'second':
            new TaskDialog({ Content: 'The second action leads to the answer to the life, the universe and everything else...' }).Show();
            break;
        case 'never':
            new TaskDialog({
                MainInstruction: 'This should never happen!',
                Content: 'The last button closed the dialog, ouch!',
                MainIcon: 'error'
            }).Show();
            break;
    }
});