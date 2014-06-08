var TaskDialog = require('../'),

    // First dialog
    first = new TaskDialog({
        WindowTitle: 'Error',
        MainInstruction: 'An error has occurred',
        Content: 'Please, help improve the world: submit anonymous informations about the error.',
        Footer: 'This is a very severe error!',
        MainIcon: 'error',
        FooterIcon: 'error',
        Buttons: [
            [ 'yes', 'Yes, send\'em\nIt\'s your lucky day, man', true ],
            [ 'no', 'Nope!', true ]
        ],
        UseCommandLinks: true
    }),

    // Don't send dialog
    dontSendError = new TaskDialog({
        WindowTitle: 'Error',
        MainInstruction: 'I can\'t believe it',
        Content: 'You clicked no? You don\'t deserve anything.',
        Buttons: [
            [ 'never-again', 'I will never do it again' ]
        ]
    }),

    // Send error dialog
    sendError = new TaskDialog({
        WindowTitle: 'Error',
        MainInstruction: 'Sending...',
        Content: 'Contacting server...',
        UseProgressBar: true,
        ProgressBarMarquee: true,
        UseTimer: true
    }),

    // Thank you dialog
    thankYou = new TaskDialog({
        WindowTitle: 'Error',
        MainInstruction: 'Thank you',
        Content: 'The world is now a better place, thank you!\n\nNow choose what to do:',
        MainIcon: 'info',
        Buttons: [
            [ 'start-again', 'Start again' ],
            [ 'close', 'Close' ]
        ]
    });

first.on('click:button', function (e) {
    switch (e.data) {
        case 'yes':
            first.Navigate(sendError);
            break;
        case 'no':
            first.Navigate(dontSendError);
            break;
    }
});

sendError.on('timer', function (e) {

    // Makes sure that the marquee is enabled
    // (needed for the `start-again` button at the end)
    if (e.data < 3000 && !sendError.ProgressBarMarquee)
        sendError.ProgressBarMarquee = true;

    // Stops the marquee after 3 seconds
    if (e.data > 3000 && sendError.ProgressBarMarquee) {
        sendError.ProgressBarMarquee = false;
        sendError.Content = 'Sending...';
    }

    // Starts filling the progress bar
    if (e.data > 3000) {
        var percentage = Math.min(100, (e.data - 3000) / 5000 * 100);
        sendError.ProgressBarPosition = percentage;
        sendError.MainInstruction = 'Sending... ' + percentage.toFixed(2) + '%';
        if (percentage == 100)
            sendError.Navigate(thankYou);
    }

});

function showCallback(res) {
    if (res.button === 'start-again')
        first.Show(showCallback);
}
first.Show(showCallback);