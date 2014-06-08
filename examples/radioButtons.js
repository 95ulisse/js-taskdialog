var TaskDialog = require('../'),
    td = new TaskDialog({
        WindowTitle: 'Buttons example',
        MainInstruction: 'Choose an action',
        RadioButtons: [
            [ 'first', 'First action' ],
            [ 'second', 'Second action' ]
        ]
    });

td.Show(function (res) {
    switch(res.radio) {
        case 'first':
            new TaskDialog({ Content: 'The first action is always the good one!' }).Show();
            break;
        case 'second':
            new TaskDialog({ Content: 'The second action leads to the answer to the life, the universe and everything else...' }).Show();
            break;
    }
});