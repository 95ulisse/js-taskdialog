var TaskDialog = require('../'),
    td = new TaskDialog({
        WindowTitle: 'Link example',
        MainInstruction: 'Click a link',
        Content: 'Choose where to go: <a href="http://www.google.com">Google</a> or <a href="http://www.facebook.com">Facebook</a>.',
        UseLinks: true
    });

td.on('click:link', function (e) {
    console.log('Link clicked: ', e.data);
});

td.Show();