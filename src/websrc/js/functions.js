const disbl = "disabled";
const classHide = "visually-hidden";
const apiLink = "/api?action=";
const pages = {
	API_PAGE_ROOT: {num: 0, str: "/", title: ""},
	API_PAGE_GENERAL: {num: 1, str: "/general", title: ""},
	API_PAGE_ETHERNET: {num: 2, str: "/ethernet", title: ""},
	API_PAGE_WIFI: {num: 3, str: "/wifi", title: ""},
	API_PAGE_SERIAL: {num: 4, str: "/serial", title: ""},
	API_PAGE_SECURITY: {num: 5, str: "/security", title: ""},
	API_PAGE_SYSTOOLS: {num: 6, str: "/sys-tools", title: ""},
	API_PAGE_ABOUT: {num: 7, str: "/about", title: ""}
}
const api = {
	actions: {
		API_GET_PAGE: 0, 
		API_GET_PARAM: 1,
		API_STARTWIFISCAN: 2,
		API_WIFISCANSTATUS: 3,
		API_GET_FILELIST: 4,
		API_GET_FILE: 5
	},
	pages: pages
}

$(document).ready(function () { //handle active nav
	$("a[href='" + document.location.pathname + "']").parent().addClass('nav-active'); //handle sidenav page selection on first load
	loadPage(document.location.pathname);

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

	$("a.nav-link").click(function(e){ //handle navigation
		e.preventDefault();
		const url = $(this).attr("href");
		if (url == "/logout") {
			window.location = "/logout";
			return;
		}
		loadPage(url);
		$(".nav-active").removeClass("nav-active");
		$(this).parent().addClass("nav-active");
		window.history.pushState("", document.title, url); //fake location
		if (isMobile()) sidenavAutoclose(true);
	});

	// setTimeout(()=> {
	// 	$('.masonry').masonry();
	// }, 500);
	//showPreloader(false);
});

function fillFileTable(files) {
	const icon = "<i class='bi bi-filetype-json'></i>";
	files.forEach((elem) => {
		let $row = $("<tr>").appendTo("#filelist");
		$("<td>" + icon + "<a href='#config_file' onClick=\"readfile('" + elem.filename + "');\">" + elem.filename + "</a></td>").appendTo($row);
		$("<td>" + elem.size + "B</td>").appendTo($row);
	});
}

function loadPage(url) {
	console.log("[loadPage] url: " + url);
	switch (url) {
		case api.pages.API_PAGE_ROOT.str:
			apiGetPage(api.pages.API_PAGE_ROOT);
		break;
		case api.pages.API_PAGE_GENERAL.str:
			apiGetPage(api.pages.API_PAGE_GENERAL);
		break;
		case api.pages.API_PAGE_ETHERNET.str:
			apiGetPage(api.pages.API_PAGE_ETHERNET, ()=>{
				if ($("#EthDhcpTog").prop("checked")) {
					EthInputDsbl(true);
				}
			});
		break;
		case api.pages.API_PAGE_WIFI.str:
			apiGetPage(api.pages.API_PAGE_WIFI, ()=>{
				if($("#WIFIssid").val().length > 1){
					setTimeout(() =>{
						$("#collapseWifiPass").collapse("show");
					}, 600);
				}
				if ($("#dhcpWiFi").prop("checked")) {
					WifiDhcpDsbl(true);
				} else {
					WifiDhcpDsbl(false);
				}
			});
		break;
		case api.pages.API_PAGE_SERIAL.str:
			apiGetPage(api.pages.API_PAGE_SERIAL);
		break;
		case api.pages.API_PAGE_SECURITY.str:
			apiGetPage(api.pages.API_PAGE_SECURITY, ()=>{
				if ($("#webAuth").prop("checked")) {
					SeqInputDsbl(false);
				}
			});
		break;
		case api.pages.API_PAGE_SYSTOOLS.str:
			apiGetPage(api.pages.API_PAGE_SYSTOOLS, ()=>{
				$.get(apiLink + api.actions.API_GET_FILELIST, function (data) {
					console.log(data);
					fillFileTable(data.files);
				});
				$.get(apiLink + api.actions.API_GET_PARAM + "&param=refreshLogs", function (data) {
					console.log(data);
					if (parseInt(data) >= 1000) {
						logRefresh(parseInt(data));
					}
				});
			});
		break;
		case api.pages.API_PAGE_ABOUT.str:
			apiGetPage(api.pages.API_PAGE_ABOUT);
		break;
		
		default:
			apiGetPage(api.pages.API_PAGE_ROOT);
		break;
	}
}

function apiGetPage(page, doneCall) {
	const animDuration = 200;
	const locCall = doneCall;
	showPreloader(true);
	$("#pageContent").fadeOut(animDuration).load( apiLink + api.actions.API_GET_PAGE + "&page=" + page.num, function(response, status, xhr) {
		if ( status == "error" ) {
		  const msg = "Page load error: ";
		  alert( msg + xhr.status + " " + xhr.statusText );
		}else{
			showPreloader(false);
			if(xhr.getResponseHeader("Authentication") == "ok") $(".logoutLink").removeClass(classHide);
			$("#pageContent").fadeIn(animDuration);
			$("form.saveParams").on("submit", function (e) {
				e.preventDefault();
				$(':disabled').each(function (e) {
					$(this).removeAttr('disabled');
				})		
				const data = $(this).serialize() + "&pageId=" + page.num;//add page num
				$.ajax({
				  type: "POST",
				  url: e.currentTarget.action,
				  data: data,
				  success: function () {
					modalConstructor("saveOk");
				  }
				});
			});
			if(xhr.getResponseHeader("respValuesArr") === null) return;
			console.log("[apiGetPage] starting parse values");
			const values = JSON.parse(xhr.getResponseHeader("respValuesArr"));
			//console.log(values);
			for (const property in values) {
				//console.log("[apiGetPage] property: " + property);
				$("[data-replace='" + property + "']").map(function(){
					const elemType = $(this).prop('nodeName').toLowerCase();
					//console.log("[apiGetPage] elemType: " + elemType);
					switch (elemType) {
						case "input" || "select" || "textarea":
							const type = $(this).prop('type').toLowerCase();
							if (elemType == "input" && (type == "checkbox" || type == "radio")) {
								$(this).prop( "checked", values[property]);
							} else {
								$(this).val(values[property]);
							}
						break;
						case "option":
							$(this).prop("selected", true);
						break;
						case "title":
							$(this).text(values[property] + " - SLZB-06 Zigbee Ethernet POE USB Adapter"); //handle title
						break;
					
						default:
							$(this).text(values[property]);
						break;
					}
				});
			}
			if(typeof (locCall) == "function") locCall();//callback
		}
	});
}

function showPreloader(state){
	if(state){
		$("#smPreloader").removeClass(classHide);
	}else{
		$("#smPreloader").addClass(classHide);
	}
}

function toastConstructor(params){
	if (params.headerIcon) {
		switch (params.headerIcon) {
			case "info":
				
				break;
		
			default:
				break;
		}
	}
	if (params.headerText) {
		
	}
	if (params.body) {
		
	}
	if (params.buttons) {
		
	}
}

function closeModal() {
	$("#modal").modal("hide");
}

function rebootWait() {
	setTimeout(function () {
		modalConstructor("rebootWait");
		console.log("[rebootWait] rebootWait start");
	}, 2000);
}

function modalConstructor(type, params) {
	console.log("[modalConstructor] start");
	const headerText = ".modal-title";
	const headerBtnClose = ".modal-btn-close";
	const modalBody = ".modal-body";
	const modalBtns = ".modal-footer";
	//$(".modal").css("display", "");
	$(headerText).text("").css("color", "");
	$(modalBody).text("").css("color", "");
	$(modalBtns).html("");
	switch (type) {
		case "flashWarning":
			$(headerText).text("WARNING").css("color", "red");
			$(modalBody).text("Flashing unofficial, incorrect or corrupted firmware can damage or brick your device!!!").css("color", "red");
			$('<button>', {
				type: "button",
				"class": "btn btn-success",
				text: "Close",
				click: function() {
					closeModal();
				}
			}).appendTo(modalBtns);
			$('<button>', {
				type: "button",
				"class": "btn btn-danger",
				text: "I agree and I am taking the risk",
				click: function() {
					closeModal();
					localStorage.setItem('flash_warning', 1);
					$('#updButton').removeAttr('disabled');
				}
			}).appendTo(modalBtns);
		break;
		case "rebootWait":
			$(headerText).text("DEVICE RESTART");
			$(modalBody).text("Waiting for device... This window will automatically close when the device reboots.");
			var waitTmr = setInterval(function (params) {
				$.get( "/", function() {
					clearInterval(waitTmr);
					clearTimeout(timeoutTmr);
					closeModal();
					console.log("[rebootWait] hide modal");
					window.location = "/";
				  });
			}, 1000);
			var timeoutTmr = setTimeout(function(){
				clearInterval(waitTmr);
				$(modalBody).text("No response from the device, this may happen if the device changed IP address or if the USB mode was selected.").css("color", "red");
				$('<button>', {
					type: "button",
					"class": "btn btn-warning",
					text: "Close",
					click: function() {
						closeModal();
					}
				}).appendTo(modalBtns);
			}, 15000);
		break;
		case "saveOk":
			$(headerText).text("SETTINGS SAVED");
			$(modalBody).text("New parameters saved. Some settings require a reboot.");
			$('<button>', {
				type: "button",
				"class": "btn btn-warning",
				text: "Restart manually later",
				click: function() {
					closeModal();
				}
			}).appendTo(modalBtns);
			$('<button>', {
				type: "button",
				"class": "btn btn-success",
				text: "Restart now",
				click: function() {
					closeModal();
					cmd('EspReboot');
					rebootWait();
				}
			}).appendTo(modalBtns);
		break;
	
		default:
		break;
	}
	$("#modal").modal("show");
}

function WifiDhcpDsbl(state) {
	$("#WifiIp").prop(disbl, state);
	$("#WifiMask").prop(disbl, state);
	$("#WifiGateway").prop(disbl, state);
}

function getWifiList() {
	$("#collapseWifiPass").collapse("hide");
	$("#wifiScanPreloader").removeClass(classHide);
	$("#wifiScanButton").addClass(classHide);
	$.get(apiLink + api.actions.API_STARTWIFISCAN, function (data) { //visually-hidden wifiLoadSpinner
		console.log(data);
		const tmrUpdWifi = setInterval(function(){
			$.get(apiLink + api.actions.API_WIFISCANSTATUS, function (data) {
				console.log(data);
				if(!data.scanDone) return;
				if(!data.wifi){
					//$("#WIFISSID").html("No networks found");
					alert("WiFi networks not found");
					
				}else{
					if (data.wifi.length > 0) {
						console.log("Start add elements to wifi table");
						data.wifi.forEach((elem) => {
							let $row = $("<tr class='ssidSelector' id='" + elem.ssid + "' >").appendTo("#wifiTable");
							$("<td>" + elem.ssid + "</td>").appendTo($row);
							
							let encryptType = "";
							switch (elem.secure) {
								case 2:
									encryptType = "WPA"
								break;

								case 3:
									encryptType = "WPA2"
								break;

								case 4:
									encryptType = "WPA2"
								break;

								case 5:
									encryptType = "WEP"
								break;

								case 7:
									encryptType = "OPEN"
								break;

								case 8:
									encryptType = "AUTO"
								break;
							
								default:
								break;
							}
							$("<td>" + encryptType + "</td>").appendTo($row);

							$("<td>" + elem.channel + "</td>").appendTo($row);
							$("<td>" + elem.rssi + "</td>").appendTo($row);
							
						});
						$("#wifiScanPreloader").addClass(classHide);
						clearInterval(tmrUpdWifi);
						$(".ssidSelector").click(function(elem){
							$("#WIFIssid").val(elem.currentTarget.id);
							$("#WIFIpassword").val("");
							$("#collapseWifiPass").collapse("show");
						});
					} else {
						$("#wifiScanPreloader").addClass(classHide);
						$("#wifiScanButton").removeClass(classHide);
					}
				}
			});
			
		}, 2000);
	});
}

function isMobile() {
	return ((window.innerWidth <= 767) && ('ontouchstart' in document.documentElement));
}

function sidenavAutoclose(now) {
	if (now) {
		$("#sidenav").removeClass("sidenav-active");
	} else {
		setTimeout(() => { $("#sidenav").removeClass("sidenav-active"); }, 5000);//timeout hide sidenaw
	}
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
				sidenavAutoclose()//timeout hide sidenaw

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
	$("#EthIp").prop(disbl, state);
	$("#EthMask").prop(disbl, state);
	$("#EthGateway").prop(disbl, state);
}

function SeqInputDsbl(state) {
	$("#webUser").prop(disbl, state);
	$("#webPass").prop(disbl, state);
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
	$("#config_file").val("Loading file: " + file);
	$.get(apiLink + api.actions.API_GET_FILE + "&filename=" + file, function (data) {
		console.log(data);
		$("#title").text(file);
		$("#filename").val(file);
		$("#config_file").val(data);
	});
}

function logRefresh(ms) {
	var refreshTmr = setInterval(() => {
		var xhr = getXhr();
		xhr.onreadystatechange = function () {
			if (xhr.readyState == 4) {
				leselect = xhr.responseText;
				let consoleElem = document.getElementById("console");
				if (consoleElem) {
					consoleElem.value = leselect;
				}else{
					clearInterval(refreshTmr);
				}
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

