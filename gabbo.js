function newMessage(msg) {
	var messagePane = document.getElementById("messagePane");
	var messages = document.getElementById("messages");
	var message = document.createElement("div");
	message.className = "message";
	message.textContent = "[" + msg + "]";

	var children = messages.childNodes;

	// TODO some option to scroll to bottom on output/keypress
	// FIXME only scroll to bottom if already at bottom
	var jumpToBottom = true;
	if (messages.scrollTop >= (messages.offsetHeight - messagePane.offsetHeight)) {
	    jumpToBottom = true;
	}
	for (var j=0 ; j<children.length ; j++) {
	    messages.appendChild(message);
	}
	if (jumpToBottom) {
		messagePane.scrollTop = messages.offsetHeight - messagePane.offsetHeight;
	}
    return;
}
