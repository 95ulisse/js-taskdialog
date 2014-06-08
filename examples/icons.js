var TaskDialog = require('../'),
    td = new TaskDialog({
        WindowTitle: 'Icons',
        MainInstruction: 'Select icon',
        Footer: 'Footer',
        RadioButtons: [
            [ 'none',    'None' ],
            [ 'warning', 'Warning' ],
            [ 'error',   'Error' ],
            [ 'info',    'Info' ],
            [ 'shield',  'Shield' ]
        ]
    });

td.on('click:radio', function (e) {
    td.MainIcon = e.data;
    td.FooterIcon = e.data;
});

td.Show();