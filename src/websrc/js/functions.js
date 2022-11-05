const disbl = "disabled";

$(document).ready(function () { //handle active nav
	$("a[href='" + document.location.pathname + "']").parent().addClass('nav-active'); //handle sidenav page selection

	if ($("#EthDhcpTog").attr("checked")) {
		EthInputDsbl(true);
	}

	if ($("#webAuth").attr("checked")) {
		SeqInputDsbl(false);
	}

	$('form').submit(function (e) {
		$(':disabled').each(function (e) {
			$(this).removeAttr(disbl);
		})
	});

	if (!(localStorage.getItem('refresh_tip_got') == 1)) {//toast localStorage.setItem('refresh_tip_got', 1)
		$('.toast').toast('show');
	}

	if (isMobile()) {
		if (!(localStorage.getItem('shv_sdnv_frst_t') == 1)) {//show sidenav first time
			$("#sidenav").addClass("sidenav-active");
			localStorage.setItem('shv_sdnv_frst_t', 1);
			setTimeout(() => { $("#sidenav").removeClass("sidenav-active"); }, 2000);
		}
		setupSwipeHandler();
	}

	// setTimeout(()=> {
	// 	$('.masonry').masonry();
	// }, 500);
});

function isMobile() {
	return ((window.innerWidth <= 767) && ('ontouchstart' in document.documentElement));
}

function setupSwipeHandler() {
	document.addEventListener("touchstart", handleSwipe, false);
	document.addEventListener("touchend", handleSwipe, false);
	var startPoint;

	function handleSwipe(event) {
		if (event.type == "touchend") {
			//console.log("end");
			//console.log(event.changedTouches[0].clientX);
			let endPoint = event.changedTouches[0].clientX;
			//console.log(endPoint);
			if (startPoint < 30 && (endPoint - startPoint) > 50) {
				//console.log("swiped right");
				$("#sidenav").addClass("sidenav-active");//show sidenav
				setTimeout(() => { $("#sidenav").removeClass("sidenav-active"); }, 5000);//timeout hide sidenaw

			} else if (endPoint == startPoint) {
				$("#sidenav").removeClass("sidenav-active");//tap hide sidenav
				//console.log("tap");
			}
		} else {
			//console.log("start");
			//console.log(event.touches[0].clientX);
			startPoint = event.touches[0].clientX;
		}
	}
}

function EthInputDsbl(state) {
	$("#EthIp").attr(disbl, state);
	$("#EthMask").attr(disbl, state);
	$("#EthGateway").attr(disbl, state);
}

function SeqInputDsbl(state) {
	$("#webUser").attr(disbl, state);
	$("#webPass").attr(disbl, state);
}


function getXhr() {
	var xhr = null;
	if (window.XMLHttpRequest) // Firefox et autres
		xhr = new XMLHttpRequest();
	else if (window.ActiveXObject) { // Internet Explorer
		try {
			xhr = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			xhr = new ActiveXObject("Microsoft.XMLHTTP");
		}
	}
	else { // XMLHttpRequest non supporté par le navigateur
		alert("Votre navigateur ne supporte pas les objets XMLHTTPRequest...");
		xhr = false;
	}
	return xhr;
}

function power(mac, cmd) {
	var xhr = getXhr();
	xhr.onreadystatechange = function () {
		if (xhr.readyState == 4) {
			leselect = xhr.responseText;
			document.getElementById(mac).innerHTML = leselect;
		}
	}
	xhr.open("GET", "SetPower?mac=" + escape(mac) + "&cmd=" + escape(cmd), true);
	xhr.setRequestHeader('Content-Type', 'application/html');
	xhr.send();

}
function GetGSMStatus() {
	var xhr = getXhr();
	xhr.onreadystatechange = function () {
		if (xhr.readyState == 4) {
			leselect = xhr.responseText;
			document.getElementById("gsmstatus").innerHTML = leselect;
		}
	}
	xhr.open("GET", "GetGSMStatus", true);
	xhr.setRequestHeader('Content-Type', 'application/html');
	xhr.send();

}
function GetThermostatStatus() {
	var xhr = getXhr();
	xhr.onreadystatechange = function () {
		if (xhr.readyState == 4) {
			leselect = xhr.responseText;
			document.getElementById("thermostat").innerHTML = leselect;
		}
	}
	xhr.open("GET", "GetThermostatStatus", true);
	xhr.setRequestHeader('Content-Type', 'application/html');
	xhr.send();
}

function GetAction(mac) {
	var xhr = getXhr();
	xhr.onreadystatechange = function () {
		if (xhr.readyState == 4) {
			leselect = xhr.responseText;
			document.getElementById(mac).innerHTML = leselect;
			setTimeout(function () { GetAction(mac); }, 3000);
		}
	}
	xhr.open("GET", "GetAction?mac=" + escape(mac), true);
	xhr.setRequestHeader('Content-Type', 'application/html');
	xhr.send();
}

function readfile(file) {
	var xhr = getXhr();
	xhr.onreadystatechange = function () {
		if (xhr.readyState == 4) {
			leselect = xhr.responseText;
			document.getElementById("title").innerHTML = file;
			document.getElementById("filename").value = file;
			document.getElementById("config_file").innerHTML = leselect;
		}
	}
	xhr.open("GET", "readFile?file=" + escape(file), true);
	xhr.setRequestHeader('Content-Type', 'application/html');
	xhr.send();
}

function logRefresh(ms) {
	ms = parseInt(ms);
	setInterval(() => {
		var xhr = getXhr();
		xhr.onreadystatechange = function () {
			if (xhr.readyState == 4) {
				leselect = xhr.responseText;
				document.getElementById("console").value = leselect;
				//setTimeout(function(){ logRefresh(); }, ms);
			}
		}
		xhr.open("GET", "getLogBuffer", true);
		xhr.setRequestHeader('Content-Type', 'application/html');
		xhr.send();
	}, ms);
}

function scanNetwork() {
	var xhr = getXhr();
	document.getElementById("networks").innerHTML = "<img src='/img/wait.gif'>";
	xhr.onreadystatechange = function () {
		if (xhr.readyState == 4) {
			leselect = xhr.responseText;
			document.getElementById("networks").innerHTML = leselect;
		}
	}
	xhr.open("GET", "scanNetwork", true);
	xhr.setRequestHeader('Content-Type', 'application/html');
	xhr.send();
}

function updateSSID(val) {
	document.getElementById("ssid").value = val;
}

function cmd(val) {

	var xhr = getXhr();
	xhr.open("GET", "cmd" + val, true);
	xhr.setRequestHeader('Content-Type', 'application/html');
	xhr.send();
}

