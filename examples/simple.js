var TaskDialog = require('../'),
    td = new TaskDialog({
        WindowTitle: 'WindowTitle',
        MainInstruction: 'MainInstruction',
        Content: 'Content',
        Footer: 'Footer',
        ExpandedControlText: 'ExpandedControlText',
        CollapsedControlText: 'CollapsedControlText',
        ExpandedInformation: 'ExpandedInformation',
        VerificationText: 'VerificationText',
        MainIcon: 'info',
        FooterIcon: 'shield',
        Cancelable: true
    });

td.Show(function (res) {
    new TaskDialog({
        Content: 'The previous TaskDialog returned: ' + JSON.stringify(res)
    }).Show();
});