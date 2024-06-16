let menuDialog;

// update server stats and open the menu
function openMenu() {
	// get server stats before opening the menu
	let freeSpaceSpan = document.getElementById('freeSpace');
	fetch(`/getFreeSpace`).then(function (response) {
		response.text().then(function (text) {
			freeSpaceSpan.textContent = text;
		})
	});
	let lastWriteSpan = document.getElementById('lastWrite');
	fetch(`/lastWrite`).then(function (response) {
		response.text().then(function (text) {
			lastWriteSpan.textContent = text;
		})
	});

	// open the menu
	menuDialog.showModal();
}

function closeMenu() {
	// close the menu
	menuDialog.close();
}

// toogle led blynk
function toogleBlynk() {
	fetch(`/toggleBlynk`).then(function (response) {
		response.text().then(function (text) {
			alert(text);
		})
	});
}

// delete all messages from server cache
function deleteMessages() {
	if (window.confirm("Are you sure to delete all messages from server?"))
		fetch(`/clear`).then(function (response) {
			response.text().then(function (text) {
				alert(text);
			})
		});
}

// save the given name before form submit
function onPostMessage() {
	// if the nick name is not empty and this.nickname.value is not equal to the nickname stored in localStorage
	// then update the localStorage nickname
	if (this.nickname.value !== localStorage.getItem('nickname'))
		localStorage.setItem('nickname', this.nickname.value);
}

function changeChatText(text) {
	// split text into individual messages
	text = text.split('|')
	// load table
	let ul = document.getElementById('chat')
	// clear all messages
	ul.innerHTML = ''

	// set message if no text / hide message if there is
	let disclaimer = document.getElementById('disclaimer')
	if (text.length - 1 == 0) {
		disclaimer.style.display = ''
		disclaimer.textContent = 'Be the first one to leave a message :)'
		return
	} else {
		disclaimer.style.display = 'none'
	}
	// add messages one by one
	for (i = 0; i < text.length - 1; i++) {
		let li = document.createElement('li')
		li.appendChild(document.createTextNode(text[i]))
		ul.appendChild(li)
	}
}

// get message file contents and call changeChatText
function getText() {
	fetch(`/showText`).then(function (response) {
		response.text().then(function (text) {
			changeChatText(text)
		})
	});
}

// initialize global cache variable.
// we need this to compare write times of the message file
var cache = ''

// get last time chat was written to and syncronize if necessary.
// executed every second
function sync() {
	setInterval(function getLastWrite() {
		fetch(`/lastWrite`).then(function (response) {
			response.text().then(function (text) {
				if (cache !== text) {
					cache = text
					getText()
				}
			})
		})
	}, 1000);
}


window.onload = () => {
		// if local storage has nickname, set the nickname input value to it
		if (localStorage.getItem('nickname') !== null) {
			document.getElementById('nickname').value = localStorage.getItem('nickname')
		}

		// start syncing on page load
		sync();

		// initialize menu dialog variable for opening and closing the menu
		menuDialog = document.querySelector(".dialog-menu");
};
