var context1;

if (typeof WinRTError !== 'undefined') {
    context1 = {context_application:1};
} else {
    context1 = {context_browser:1};
}