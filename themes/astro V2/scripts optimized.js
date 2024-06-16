let menuDialog;

function openMenu() {
	fetch(`/getFreeSpace`).then(response => response.text().then(text => {
		document.getElementById('freeSpace').textContent = text;
	}));
	
	fetch(`/lastWrite`).then(response => response.text().then(text => {
		document.getElementById('lastWrite').textContent = text;
	}));

	menuDialog.showModal();
}

function closeMenu() {
	menuDialog.close();
}

function toggleBlynk() {
	fetch(`/toggleBlynk`).then(response => response.text().then(text => {
		alert(text);
	}));
}

function deleteMessages() {
	if (window.confirm("Are you sure to delete all messages from server?")) {
		fetch(`/clear`).then(response => response.text().then(text => {
			alert(text);
		}));
	}
}

function onPostMessage() {
	if (this.nickname.value !== localStorage.getItem('nickname')) {
		localStorage.setItem('nickname', this.nickname.value);
	}
}

function changeChatText(text) {
	const messages = text.split('|');
	const ul = document.getElementById('chat');
	ul.innerHTML = '';

	const disclaimer = document.getElementById('disclaimer');
	if (messages.length - 1 === 0) {
		disclaimer.style.display = '';
		disclaimer.textContent = 'Be the first one to leave a message :)';
		return;
	} else {
		disclaimer.style.display = 'none';
	}

	for (let i = 0; i < messages.length - 1; i++) {
		const li = document.createElement('li');
		li.appendChild(document.createTextNode(messages[i]));
		ul.appendChild(li);
	}
}

function getText() {
	fetch(`/showText`).then(response => response.text().then(text => {
		changeChatText(text);
	}));
}

let cache = '';

function sync() {
	setInterval(() => {
		fetch(`/lastWrite`).then(response => response.text().then(text => {
			if (cache !== text) {
				cache = text;
				getText();
			}
		}));
	}, 1000);
}

window.onload = () => {
	if (localStorage.getItem('nickname') !== null) {
		document.getElementById('nickname').value = localStorage.getItem('nickname');
	}

	sync();

	menuDialog = document.querySelector(".dialog-menu");
};
