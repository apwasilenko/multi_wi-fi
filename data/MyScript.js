var relay = document.getElementById("relay_button");
var mytext1 = document.getElementById("txt1");
var mytext2 = document.getElementById("txt2");
var relay_status = 0;
let interwal = 1000;
document.addEventListener('DOMContentLoaded', relay_state);

relay.addEventListener('click', relay_inverse);
	
setInterval(myAllert(), interwal);

function relay_state() {
	var request = new XMLHttpRequest();
	request.open('GET', './relay_status', true);
	request.onload = function(){
		if (request.readyState == 4 && request.status == 200) {
			var response = request.responseText;
			relay_status = Number.parseInt(response);
			if (relay_status == 0)
				relay.classList.add('relay_off');
			else
				relay.classList.add('relay_on');
		}
	}
	request.send();
	myAllert();
}

function relay_inverse() {
	myAllert();
	var request = new XMLHttpRequest();
	request.open("GET", "./relay_switch", false);
	request.send();
	if (request.readyState == 4 && request.status == 200){
		var response = request.responseText;
		if (response == '0'){
			relay.classList.remove('relay_on');
			relay.classList.add('relay_off');
			relay_status = 0;
		}
		else {
			relay.classList.remove('relay_off');
			relay.classList.add('relay_on');
			relay_status = 1;
		}
	}
}
function myAllert(){
	var request = new XMLHttpRequest();
	request.open("GET", "./mytime", false);
	request.send();
	console.log(request.status);
	console.log(request.readyState);
	if (request.readyState == 4 && request.status == 200){
		let a = request.responseText;
		console.log(a);
//		interwal = 1000 - (a % 1000);
		let nm_work = "NodeMCU в работе ";
		nm_work += Math.floor(a / 1000 / 60/ 60);
		let b = `<p class=".my_Text1">${convertTime(a)}</p>`;
		mytext1.innerHTML = b;
		mytext2.innerHTML = `<p class='.my_Text2'>${nm_work}</p>`;
		alert("myAlert");
	}
}

function convertTime(newTime){
	let my_day  = Math.floor(newTime / 1000 / 60 / 60 / 24)
	let my_hour = Math.floor((newTime - / 1000 / 60 / 60);
	let my_min  = Math.floor((newTime - my_hour * 1000 * 60 * 60) / 1000 / 60);
	let my_sec  = Math.floor((newTime - my_hour * 1000 * 60 * 60 - my_min * 1000 * 60) / 1000);
	let fulltime ="";
	fulltime += 
	fulltime += my_hour;
	fulltime += ":";
	fulltime += my_min;
  fulltime += ":";
	fulltime += my_sec;
	return fulltime;
}