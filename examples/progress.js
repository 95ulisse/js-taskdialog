var TaskDialog = require('../'),
    td = new TaskDialog({
        WindowTitle: 'ProgressBar example',
        Buttons: [
            [ 'state', 'Cycle state', true ],
            [ 'marquee', 'Toggle marquee', true ]
        ],
        Cancelable: true,
        UseProgressBar: true,
        ProgressBarPosition: 25,
        ProgressBarMarquee: false
    }),
    states = [ 'normal', 'error', 'paused' ],
    i = 0;

td.on('click:button', function (e) {
    switch(e.data) {
        case 'state':
            i++;
            i = i % states.length;
            td.ProgressBarState = states[i];
            break;
        case 'marquee':
            td.ProgressBarMarquee = !td.ProgressBarMarquee;
            if (!td.ProgressBarMarquee)
                td.ProgressBarPosition = 25;
            break;
    }
    td.MainInstruction =
        'State: ' + states[i] + '\n' +
        'Marquee: ' + td.ProgressBarMarquee;
});

td.emit('click:button', {});

td.Show();