const disbl = "disabled";
const classHide = "visually-hidden";
const apiLink = "/api?action=";
const spiner = $('<span>', {
	"role": "status",
	"class": "spinner spinner-border text-primary spinner-border-sm"
}).css("margin-left", "10px");
const spiner2 = $("<span>", {
	"class": classHide
});

const pages = {
	API_PAGE_ROOT: {num: 0, str: "/", title: "Status"},
	API_PAGE_GENERAL: {num: 1, str: "/general", title: "General"},
	API_PAGE_ETHERNET: {num: 2, str: "/ethernet", title: "Config Ethernet"},
	API_PAGE_WIFI: {num: 3, str: "/wifi", title: "Config WiFi"},
	API_PAGE_SERIAL: {num: 4, str: "/zha-z2m", title: "Config ZHA and Z2M params"},
	API_PAGE_SECURITY: {num: 5, str: "/security", title: "Security"},
	API_PAGE_SYSTOOLS: {num: 6, str: "/sys-tools", title: "System and Tools"},
	API_PAGE_ABOUT: {num: 7, str: "/about", title: "About"}
}

const api = {
	actions: {
		API_GET_PAGE: 0, 
		API_GET_PARAM: 1,
		API_STARTWIFISCAN: 2,
		API_WIFISCANSTATUS: 3,
		API_GET_FILELIST: 4,
		API_GET_FILE: 5,
		API_SEND_HEX: 6,
		API_WIFICONNECTSTAT: 7,
		API_CMD: 8,
		API_GET_LOG: 9
	},
	pages: pages
}

$(document).ready(function () { //handle active nav
	$("a[href='" + document.location.pathname + "']").parent().addClass('nav-active'); //handle sidenav page selection on first load
	loadPage(document.location.pathname);

	if (!(localStorage.getItem('refresh_tip_got') == 1)) {//toast localStorage.setItem('refresh_tip_got', 1)
		toastConstructor("refreshTip");
	}

	if (isMobile()) {
		if (!(localStorage.getItem('shv_sdnv_frst_t') == 1)) {//show sidenav first time
			$("#sidenav").addClass("sidenav-active");
			localStorage.setItem('shv_sdnv_frst_t', 1);
			setTimeout(() => { $("#sidenav").removeClass("sidenav-active"); }, 2000);
		}
		setupSwipeHandler();
		$("#pageContent").removeClass("container");//no containers for mobile
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
		if (isMobile()) sidenavAutoclose(true);
	});
});

function name(params) {
	
}

function zbOta() {
	let file = $("#zbFirmware")[0].files[0];
	let reader = new FileReader();
	let text;
	let hex;
	reader.onload = function(e) {
		if(isHex(reader.result)){
			console.log("Starting parse .hex file");
			text = reader.result;
			
			text.split("\n").forEach(function(line, index, arr) {
				if (index === arr.length - 1 && line === "")  return;
				console.log("index:" + index);
				hex += text.slice(-(text.length - 9), -2).toUpperCase();
				// let hexSize = hex.split(" ").length;
    			// $.get(apiLink + api.actions.API_SEND_HEX + "&hex=" + hex + "&size=" + hexSize, function (data) {
				// });
			});
			console.log("hex len: " + hex.length);
			const hmax = 248;
			let pos = hmax;
			for (let index = 0; index < (hex.length / hmax); index++) {
				console.log(hex.slice(pos, hmax));
				pos+=hmax;
			}
		}else{
			alert("This file format not suported!");
		}
	}
	reader.readAsText(file);

}

function isHex(txt){
	var regex = /[0-9A-Fa-f]{21}/g;
	return regex.test(txt);
}

function copyCode() {
	let textArea = $("#generatedFile");
	if (!navigator.clipboard) {
		textArea.focus();
  		textArea.select();
  		try {
    		let successful = document.execCommand('copy');
    		let msg = successful ? 'successful' : 'unsuccessful';
    		console.log('Fallback: Copying text command was ' + msg);
  		} catch (err) {
    		console.error('Fallback: Oops, unable to copy', err);
  		}
	}else{
		navigator.clipboard.writeText(textArea.val()).then(function() {
			console.log('Async: Copying to clipboard was successful!');
		  }, function(err) {
			console.error('Async: Could not copy text: ', err);
		  });
	}
	$("#clipIco").attr("xlink:href", "icons.svg#clipboard2-check");
}

function generateConfig(params) {
	let result;
	const ip = window.location.host;
	const port = $("#port").val();
	if(ip == "192.168.1.1") $("#apAlert").removeClass(classHide);
	switch (params) {
		case "zha":
			result = "socket://" + ip + ":" + port;
		break;
		case "z2m":
			result = `# Serial settings
serial:
  # Location of SLZB-06
  port: tcp://${ip}:${port}
  baudrate: ${$("#baud").val()}
  # Disable green led?
  disable_led: false
# Set output power to max 20
advanced:
  transmit_power: 20`;
		break;
		case "usb":
			result = `# For homeassistant: Go to "Settings"→"System"→"Hardware"→Select the 3 dot menu in the upper right corner→"All Hardware"→Scroll to ttyUSB and find your adapter→Copy Device path like "/dev/ttyUSB0"
# List USB devices on Linux: ls  /dev/ttyUSB*
serial:
# Location of SLZB-06
  port: INSERT_DEVICE_PATCH_HERE
  baudrate: ${$("#baud").val()}
# Disable green led?
  disable_led: false
# Set output power to max 20
advanced:
  transmit_power: 20`;
		break;
	
		default:
			break;
	}
	$("#generatedFile").val(result);
}

function fillFileTable(files) {
	const icon = "<i class='bi bi-filetype-json'></i>";
	files.forEach((elem) => {
		let $row = $("<tr>").appendTo("#filelist");
		$("<td>" + icon + "<a href='#config_file' onClick=\"readfile('" + elem.filename + "');\">" + elem.filename + "</a></td>").appendTo($row);
		$("<td>" + elem.size + "B</td>").appendTo($row);
	});
}

function sendHex() {
	let hex = $("#sendHex").val().toUpperCase();
	let hexSize = hex.split(" ").length;
    $.get(apiLink + api.actions.API_SEND_HEX + "&hex=" + hex + "&size=" + hexSize, function (data) {
		$("#sendHex").val("");
	});
}

function loadPage(url) {
	window.history.pushState("", document.title, url); //fake location
	console.log("[loadPage] url: " + url);
	switch (url) {
		case api.pages.API_PAGE_ROOT.str:
			apiGetPage(api.pages.API_PAGE_ROOT);
		break;
		case api.pages.API_PAGE_GENERAL.str:
			apiGetPage(api.pages.API_PAGE_GENERAL, ()=>{
				if (!$("#usbMode").prop("checked")) {
					KeepWebDsbl(true);
				}
			});
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
				$.get(apiLink + api.actions.API_GET_PARAM + "&param=coordMode", function (data) {
					if (parseInt(data) != 1) {//not in wifi mode
						$(".card").addClass("card-disabled");
						toastConstructor("wifiDisabled");
					}
				});
				if ($("#dhcpWiFi").prop("checked")) {
					WifiDhcpDsbl(true);
				} else {
					WifiDhcpDsbl(false);
				}
			});
		break;
		case api.pages.API_PAGE_SERIAL.str:
			apiGetPage(api.pages.API_PAGE_SERIAL, ()=>{
				generateConfig("z2m");
			});
		break;
		case api.pages.API_PAGE_SECURITY.str:
			apiGetPage(api.pages.API_PAGE_SECURITY, ()=>{
				if ($("#webAuth").prop("checked")) {
					SeqInputDsbl(false);
				}
				if ($("#fwEnabled").prop("checked")) {
					SeqInputDsblFw(false);
				}
			});
		break;
		case api.pages.API_PAGE_SYSTOOLS.str:
			apiGetPage(api.pages.API_PAGE_SYSTOOLS, ()=>{
				$.get(apiLink + api.actions.API_GET_FILELIST, function (data) {
					fillFileTable(data.files);
				});
				$.get(apiLink + api.actions.API_GET_PARAM + "&param=refreshLogs", function (data) {
					if (parseInt(data) >= 1000) {
						logRefresh(parseInt(data));
					}else{
						logRefresh(1000);
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
	if(url != api.pages.API_PAGE_WIFI.str && $('.toast').hasClass("show")){
		if($('#toastBody').text().indexOf("Wi-Fi mode") > 0){
			$('.toast').toast('hide');
		}
	}
}

function espReboot() {
	$.get(apiLink + api.actions.API_CMD + "&cmd=3");
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
				const btn = $("form.saveParams button[type='submit']");
				$(':disabled').each(function (e) {
					$(this).removeAttr('disabled');
				});
				spiner.appendTo(btn);
				spiner2.appendTo(btn);
				btn.prop("disabled", true);
				const data = $(this).serialize() + "&pageId=" + page.num;//add page num
				$.ajax({
					type: "POST",
					url: e.currentTarget.action,
					data: data,
					success: function () {
						modalConstructor("saveOk");
				  	},
					error: function () {
						alert("Error saving settings. Check your network");
					},
					complete: function() {
						spiner.remove();
						spiner2.remove();
						btn.prop("disabled", false);
					}
				});
			});
			$("button").click(function(){
				const btnStndrt = "btn-outline-primary";
				const btnGrn = "btn-outline-success";
				const jbtn = $(this);
				const cmd = jbtn.attr("data-cmd");
				if(cmd){
					spiner.appendTo(jbtn);
					spiner2.appendTo(jbtn);
					jbtn.prop("disabled", true);
					$.get(apiLink + api.actions.API_CMD + "&cmd=" + cmd, function (data) {
						jbtn.removeClass(btnStndrt);
						jbtn.addClass(btnGrn);
						setTimeout(function(jbtn){
							jbtn.removeClass(btnGrn);
							jbtn.addClass(btnStndrt);
						},1000, jbtn);
					}).always(function() {
						setTimeout(function(jbtn){
							spiner.remove();
							spiner2.remove();
							jbtn.prop("disabled", false);
						},700, jbtn);
					  }).fail(function() {
						alert("Error. Check your network");
					  });
				}
			});

			$("[data-replace='pageName']").text(page.title);//update page name
			$("title[data-replace='pageName']").text(page.title + " - SLZB-06 Zigbee Ethernet POE USB Adapter");//update page title
			
			if(xhr.getResponseHeader("respValuesArr") === null) return;
			console.log("[apiGetPage] starting parse values");
			const values = JSON.parse(xhr.getResponseHeader("respValuesArr"));
			for (const property in values) {
				$("[data-replace='" + property + "']").map(function(){
					const elemType = $(this).prop('nodeName').toLowerCase();
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
	$("#toastButtons").html("");
	$("#toastHeaderText").text("");
	$("#toastBody").text("");
	switch (params) {
		case "refreshTip":
			$("#toastHeaderText").text("Tip");
			$("#toastBody").text("Statuses and other information in this window are updated when the page refreshes.");
			$('<button>', {
				type: "button",
				"class": "btn btn-outline-primary",
				text: "Got it!",
				click: function() {
					localStorage.setItem('refresh_tip_got', 1);
					$('.toast').toast('hide');
				}
			}).appendTo("#toastButtons");
		break;
		case "wifiDisabled":
			$("#toastHeaderText").text("Info");
			$("#toastBody").text(`Wi-Fi page is inactive because Wi-Fi mode is not activated.
			Activate Wi-Fi in the 'General' settings`);
			$('<button>', {
				type: "button",
				"class": "btn btn-outline-primary",
				text: "Go to 'General'",
				click: function() {
					window.location = "/general";
					$('.toast').toast('hide');
				}
			}).appendTo("#toastButtons");
			$('<button>', {
				type: "button",
				"class": "btn btn-outline-primary",
				text: "Close",
				click: function() {
					$('.toast').toast('hide');
				}
			}).appendTo("#toastButtons");
		break;
	
		default:
			break;
	}
	$('.toast').toast('show');
}

function closeModal() {
	$("#modal").modal("hide");
}

function rebootWait() {
	setTimeout(function () {
		modalConstructor("rebootWait");
		console.log("[rebootWait] start");
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
			$('<div>', {
				"role": "status",
				"class": "spinner-border text-primary",
				append: $("<span>", {
					"class": classHide
				})
			}).appendTo(modalBtns);
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
				$(modalBtns).html("");
				$(modalBody).text("No response from the device, this may happen if the device changed IP address or if the USB mode was selected.").css("color", "red");
				$('<button>', {
					type: "button",
					"class": "btn btn-warning",
					text: "Close",
					click: function() {
						closeModal();
					}
				}).appendTo(modalBtns);
			}, 20000);
		break;
		case "saveOk":
			if(window.location.pathname == "/wifi"){
				$(headerText).text("Wi-Fi network connection");
				$(modalBody).text(`Connecting to the network in progress...
				Wait for the result.`);
				$('<div>', {
					"role": "status",
					"class": "spinner-border text-primary",
					append: $("<span>", {
						"class": classHide
					})
				}).appendTo(modalBtns);
				let counter = 0;
				var getWifiIp = setInterval(function (params) {
					if(counter <= 15){
						$.get(apiLink + api.actions.API_WIFICONNECTSTAT, function(data) {
							if(data.connected){
								espReboot();
								clearInterval(getWifiIp);
								setTimeout(()=>{//5sec for reboot
									$(".modal-body").html(`<span style="color: green">Connected!</span><br>New IP address is ${data.ip}<br>Device will now reboot for the new settings to take effect.`);
									$(modalBtns).html("");
									$('<button>', {
										type: "button",
										"class": "btn btn-success",
										text: "Go to " + data.ip,
										click: function() {
											window.location = "http://"+ data.ip +"/";
										}
									}).appendTo(modalBtns);
								}, 5000);
							}else{
								counter++;
							}
						});
					}else{
						clearInterval(getWifiIp);
						$(modalBody).text("Connection error, check SSID, PASSWORD and try again").css("color", "red");
						$(modalBtns).html("");
						$('<button>', {
							type: "button",
							"class": "btn btn-success",
							text: "Close",
							click: function() {
								closeModal();
							}
						}).appendTo(modalBtns);
					}
				}, 1000);
			}else{
				let body = "New parameters saved. ";
				$(headerText).text("SETTINGS SAVED");
				if ($("#wifiMode").prop("checked")) {
					body += "You will be redirected to the Wi-Fi network selection page.";
					$('<button>', {
						type: "button",
						"class": "btn btn-success",
						text: "Select Wi-Fi network",
						click: function() {
							closeModal();
							loadPage("/wifi");
						}
					}).appendTo(modalBtns);
				} else {
					body += "Some settings require a reboot.";
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
							espReboot();
							rebootWait();
						}
					}).appendTo(modalBtns);
				}
			$(modalBody).text(body);
			}
		break;
		case "keepWeb":
			$(headerText).text("Keep network & web server");
			$(modalBody).text("This function allows you to leave one of the communication channels active and have access to the web interface. The device itself will select an available channel according to priority: WIFI, ETHERNET, WIFI AP");
			$('<button>', {
				type: "button",
				"class": "btn",
				text: "Ok",
				click: function() {
					closeModal();
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
		const tmrUpdWifi = setInterval(function(){
			$.get(apiLink + api.actions.API_WIFISCANSTATUS, function (data) {
				if(!data.scanDone) return;
				if(!data.wifi){
					alert("WiFi networks not found");	
				}else{
					if (data.wifi.length > 0) {
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
			let endPoint = event.changedTouches[0].clientX;
			if (startPoint < 30 && (endPoint - startPoint) > 50) {
				$("#sidenav").addClass("sidenav-active");//show sidenav
				sidenavAutoclose()//timeout hide sidenaw

			} else if (endPoint == startPoint) {
				$("#sidenav").removeClass("sidenav-active");//tap hide sidenav
			}
		} else {
			startPoint = event.touches[0].clientX;
		}
	}
}

function KeepWebDsbl(state) {
	$("#keepWeb").prop(disbl, state);
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

function SeqInputDsblFw(state) {
	$("#fwIp").prop(disbl, state);
}

function readfile(file) {
	$("#config_file").val("Loading file: " + file);
	$.get(apiLink + api.actions.API_GET_FILE + "&filename=" + file, function (data) {
		$("#title").text(file);
		$("#filename").val(file);
		$("#config_file").val(data);
	});
}

function logRefresh(ms) {
	var logUpd = setInterval(() => {
		$.get(apiLink + api.actions.API_GET_LOG, function (data) {
			if($("#console").length){//elem exists
				$("#console").val(data);
			}else{
				clearInterval(logUpd);
			}
		});
	}, ms);
}