window.showAlert = function (message) {
    alert(message);
};

window.showConfirm = function (message) {
    return confirm(message);
};

window.showPrompt = function (message, defaultValue) {
    return prompt(message, defaultValue);
};