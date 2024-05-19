const disbl = "disabled";
const chck = "checked";
const classHide = "visually-hidden";
const apiLink = "/api?action=";
const spiner = $('<span>', {
	"role": "status",
	"class": "spinner spinner-border spinner-color spinner-border-sm"
}).css("margin-left", "10px");
const spiner2 = $("<span>", {
	"class": classHide
});

const statusOk = $('<span>', {
	"role": "status",
}).css("margin-left", "10px").text("✅");

const statusFail = $('<span>', {
	"role": "status",
}).css("margin-left", "10px").text("❌");

const zbFwInfoUrl = "https://raw.githubusercontent.com/xyzroe/XZG/zb_fws/ti/manifest.json";

const headerText = ".modal-title";
const headerBtnClose = ".modal-btn-close";
const modalBody = ".modal-body";
const modalBtns = ".modal-footer";

const pages = {
	API_PAGE_ROOT: { num: 0, str: "/" },
	API_PAGE_GENERAL: { num: 1, str: "/general" },
	API_PAGE_ETHERNET: { num: 2, str: "/ethernet" },
	API_PAGE_NETWORK: { num: 3, str: "/network" },
	API_PAGE_ZIGBEE: { num: 4, str: "/zigbee" },
	API_PAGE_SECURITY: { num: 5, str: "/security" },
	API_PAGE_TOOLS: { num: 6, str: "/tools" },
	API_PAGE_ABOUT: { num: 7, str: "/about" },
	API_PAGE_MQTT: { num: 8, str: "/mqtt" },
	API_PAGE_VPN: { num: 9, str: "/vpn" }
}

const commands = {
	CMD_ZB_ROUTER_RECON: 0,
	CMD_ZB_RST: 1,
	CMD_ZB_BSL: 2,
	CMD_ESP_RES: 3,
	CMD_ADAP_LAN: 4,
	CMD_ADAP_USB: 5,
	CMD_LED_ACT: 6,
	CMD_ZB_FLASH: 7,
	CMD_CLEAR_LOG: 8,
	CMD_ESP_UPD_URL: 9,
	CMD_ZB_CHK_FW: 10,
	CMD_ZB_CHK_HW: 11,
	CMD_ZB_LED_TOG: 12,
	CMD_ESP_FAC_RES: 13
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
		API_GET_LOG: 9,
		API_DEL_FILE: 10,
		API_FLASH_ZB: 11
	},
	pages: pages,
	commands: commands
}

const IconsStatusCodes = {
	OK: 1,
	WARN: 2,
	ERROR: 3
};

let intervalIdUpdateRoot;
let intervalTimeUpdateRoot;

let updateValues = {};


function applyScale(baseScale) {
	const switchScale = 0.8 + baseScale * 0.4; // 0.8 min > 1.2 max > 0.4 diff
	const selectScale = 0.7 + baseScale * 0.3; // 0.7 min > 1.0 max > 0.3 diff

	document.querySelectorAll('label[for="toggleBtn"]').forEach(function (element) {
		element.style.transform = `scale(${switchScale})`;
	});

	const langSelect = document.getElementById('langSel');
	if (langSelect) {
		langSelect.style.transform = `scale(${selectScale})`;
	}
}


function handleResize() {
	if (window.innerWidth <= 767) {
		applyScale(0.8);
		$(".ui_set").removeClass("hstack");
		$(".ui_set").addClass("vstack");
		$(".switch-container").css("margin-left", "");
		$("#pageName").hide();
	} else {
		applyScale(1.0);
		$(".ui_set").removeClass("vstack");
		$(".ui_set").addClass("hstack");
		$(".switch-container").css("margin-left", "0.3em");
		$("#pageName").show();
	}
}

window.addEventListener('resize', handleResize);
document.addEventListener("scroll", function () {
	var credits = document.getElementById("credits");
	if ((window.innerHeight + window.scrollY) >= document.body.offsetHeight) {
		credits.style.display = "block";
	} else {
		credits.style.display = "none";
	}
});

document.addEventListener('DOMContentLoaded', function () {
	setTimeout(updateRootEvents(function (connected) {
		if (connected) {
			//console.log("ok");
		}
	}), 300);
	const savedLang = localStorage.getItem("selected-lang");
	const browserLang = navigator.language ? navigator.language.substring(0, 2) : navigator.userLanguage;
	let preferredLang = savedLang || (languages.some(lang => lang.value === browserLang) ? browserLang : 'en'); // 'en' как fallback

	changeLanguage(preferredLang);
});

function identifyLed(event, element, led) {
	event.preventDefault();

	const offLed = '💡';
	const onLed = '⭕';
	let count = 0;
	let blinkingText = element.nextElementSibling;

	blinkingText.textContent = i18next.t('p.to.ls');

	function toggleEmoji() {
		element.innerHTML = element.innerHTML === offLed ? onLed : offLed;
		count++;

		if (count < 11) { // Needed changes x 2 + 1
			setTimeout(toggleEmoji, 500);
		}
		else {
			element.innerHTML = offLed;
			blinkingText.textContent = '';
		}
	}

	//const item = $(this);
	//const led = item.attr("data-led");


	console.log(led);
	//14&led=1&act=2"
	//let led = -1;
	/*if (event.target.href == "power") {
		led = 0;
	}
	else if (event.target.href == "mode") {
		led = 1;
	}
	if (led > -1) {*/
	$.get(apiLink + api.actions.API_CMD + "&cmd=" + api.commands.CMD_LED_ACT + "&act=3&led=" + led, function (data) {
		blinkingText.textContent = i18next.t('p.to.lb');
		toggleEmoji();
	}).fail(function () {
		alert(i18next.t('c.ercn'));
	});






}

$(document).ready(function () { //handle active nav
	$("a[href='" + document.location.pathname + "']").parent().addClass('nav-active'); //handle sidenav page selection on first load
	loadPage(document.location.pathname);

	if (isMobile()) {
		if (!(localStorage.getItem('shv_sdnv_frst_t') == 1)) {//show sidenav first time
			$("#sidenav").addClass("sidenav-active");
			localStorage.setItem('shv_sdnv_frst_t', 1);
			//setTimeout(() => { $("#sidenav").removeClass("sidenav-active"); }, 2000);
		}
		setupSwipeHandler();
		$("#pageContent").removeClass("container");//no containers for mobile 
	}

	handleResize();

	handleClicks();

	handleMsg();
});

function name(params) {

}

function zbOta() {
	let file = $("#zbFirmware")[0].files[0];
	let reader = new FileReader();
	let text;
	let hex;
	reader.onload = function (e) {
		if (isHex(reader.result)) {
			console.log("Starting parse .hex file");
			text = reader.result;

			text.split("\n").forEach(function (line, index, arr) {
				if (index === arr.length - 1 && line === "") return;
				console.log("index:" + index);
				hex += text.slice(-(text.length - 9), -2).toUpperCase();
				let hexSize = hex.split(" ").length;
				$.get(apiLink + api.actions.API_SEND_HEX + "&hex=" + hex + "&size=" + hexSize, function (data) {
				});
			});
			console.log("hex len: " + hex.length);
			const hmax = 248;
			let pos = hmax;
			for (let index = 0; index < (hex.length / hmax); index++) {
				console.log(hex.slice(pos, hmax));
				pos += hmax;
			}
		} else {
			alert("This file format not suported!");
		}
	}
	reader.readAsText(file);

}

function isHex(txt) {
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
	} else {
		navigator.clipboard.writeText(textArea.val()).then(function () {
			console.log('Async: Copying to clipboard was successful!');
		}, function (err) {
			console.error('Async: Could not copy text: ', err);
		});
	}
	$("#clipIco").attr("xlink:href", "icons.svg#clipboard2-check");
}

function generateConfig(params) {
	let result;
	const mist_cfg_txt = `baudrate: ${$("#baud").val()}
# ${i18next.t('p.zi.cfg.dzl')}
	disable_led: false
# ${i18next.t('p.zi.cfg.sopm')}
advanced:
	transmit_power: 20`;
	const ip = window.location.host;
	const port = $("#port").val();
	if (ip == "192.168.1.1") $("#apAlert").removeClass(classHide);
	switch (params) {
		case "zha":
			result = "socket://" + ip + ":" + port;
			break;
		case "z2m":
			result = `# ${i18next.t('p.zi.cfg.ss')}
serial:
# ${i18next.t('p.zi.cfg.lxzg')}
  port: tcp://${ip}:${port}
  ${mist_cfg_txt}`;
			break;
		case "usb":
			result = `# ${i18next.t('p.zi.cfg.ha')}
# ${i18next.t('p.zi.cfg.lin')}
# ${i18next.t('p.zi.cfg.ss')}
serial:
# ${i18next.t('p.zi.cfg.lxzg')}
  port: ${i18next.t('p.zi.cfg.dp')}
  ${mist_cfg_txt}`;
			break;

		default:
			break;
	}
	$("#generatedFile").val(result);
}

function fillFileTable(files) {
	const icon_file = `<svg class="card_icon file_icon" viewBox="0 0 16 16"><use xlink:href="icons.svg#file" /></svg>`;
	const icon_del = `<svg class="card_icon del_icon" viewBox="0 0 16 16"><use xlink:href="icons.svg#magic" /></svg>`;
	files.forEach((elem) => { //.slice(0, files.length - 1)
		if (elem.size > 0) {
			let $row = $("<tr>").appendTo("#filelist")
			$("<td class='col-min-width'>" + icon_file + "</td>").appendTo($row);
			$("<td><a href='#config_file' onClick=\"readFile(event, '" + elem.filename + "');\">" + elem.filename + "</a></td>").appendTo($row);
			$("<td>" + elem.size + "B</td>").appendTo($row);
			$("<td class='text-end col-min-width'><a href='' onClick=\"delFile(event, '" + elem.filename + "');\">" + icon_del + "</a></td>").appendTo($row);
		}
	});
}

function sendHex() {
	let hex = $("#sendHex").val().toUpperCase();
	let hexSize = hex.split(" ").length;
	$.get(apiLink + api.actions.API_SEND_HEX + "&hex=" + hex + "&size=" + hexSize, function (data) {
		$("#sendHex").val("");
	});
}

function setIconGlow(iconId, state, show = true) {
	const icon = document.getElementById(iconId);
	if (icon) {
		switch (state) {
			case IconsStatusCodes.ERROR:
				color = '#fc0500'; // danger
				break;
			case IconsStatusCodes.WARN:
				color = '#e9cf01'; // warn
				break;
			case IconsStatusCodes.OK:
				color = '#01b464'; // success
				break;
			default:
				color = '#000000'; // black
		}
		icon.style.filter = color ? `drop-shadow(0 0 1px ${color})
                                    drop-shadow(0 0 2px ${color})
                                    drop-shadow(0 0 4px ${color})` : 'none';
		icon.style.backgroundColor = color ? color : 'transparent';
		icon.style.border = color ? `2px solid ${color}` : 'none';
		if (show) {
			icon.classList.remove(classHide);
		} else {
			icon.classList.add(classHide);
		}
	}
}

function loadPage(url) {


	if (window.location.pathname !== url) {
		window.history.pushState("", document.title, url);
	}
	//console.log("[loadPage] url: " + url);


	/*if (url == "/") {
		$.get(apiLink + api.actions.API_GET_PARAM + "&param=refreshLogs", function (data) {
			if (parseInt(data) >= 1) {
				intervalTimeUpdateRoot = parseInt(data) * 1000;
			} else {
				intervalTimeUpdateRoot = 1000;
			}
		});
		//intervalIdUpdateRoot = setInterval(updateRoot, 1000);
		intervalIdUpdateRoot = setTimeout(updateRoot, intervalTimeUpdateRoot);
	}*/

	//else {
	//	clearInterval(intervalIdUpdateRoot);
	//}

	switch (url) {
		case api.pages.API_PAGE_ROOT.str:
			apiGetPage(api.pages.API_PAGE_ROOT);
			break;
		case api.pages.API_PAGE_GENERAL.str:
			apiGetPage(api.pages.API_PAGE_GENERAL, () => {
				if (!$("#usbMode").prop(chck)) {
					KeepWebDsbl(true);
				}
			});
			break;
		case api.pages.API_PAGE_MQTT.str:
			apiGetPage(api.pages.API_PAGE_MQTT, () => {
				if ($("#MqttEnable").prop(chck) == false) {
					MqttInputDsbl(true);
				}
			});
			break;
		case api.pages.API_PAGE_VPN.str:
			apiGetPage(api.pages.API_PAGE_VPN, () => {
				if ($("#wgEnable").prop(chck) == false) {
					WgInputDsbl(true);
				}
				if ($("#hnEnable").prop(chck) == false) {
					HnInputDsbl(true);
				}
			});
			break;
		case api.pages.API_PAGE_NETWORK.str:
			apiGetPage(api.pages.API_PAGE_NETWORK, () => {

				if ($("#ethEnbl").prop(chck)) {
					EthEnbl(true);
				}
				else {
					EthEnbl(false);
				}

				if ($("#ethDhcp").prop(chck)) {
					EthDhcpDsbl(true);
				}
				else {
					EthDhcpDsbl(false);
				}

				if ($("#wifiSsid").val().length > 1) {
					setTimeout(() => {
						$("#collapseWifiPass").collapse("show");
					}, 500);
				}
				/*$.get(apiLink + api.actions.API_GET_PARAM + "&param=coordMode", function (data) {
					if (parseInt(data) != 1) {//not in wifi mode
						$(".card").addClass("card-disabled");
						toastConstructor("wifiDisabled");
					}
				});*/

				if ($("#wifiEnbl").prop(chck)) {
					WifiEnbl(true);
				}
				else {
					WifiEnbl(false);
				}
				if ($("#wifiDhcp").prop(chck)) {
					WifiDhcpDsbl(true);
				} else {
					WifiDhcpDsbl(false);
				}
			});
			break;
		case api.pages.API_PAGE_ZIGBEE.str:
			apiGetPage(api.pages.API_PAGE_ZIGBEE, () => {
				generateConfig("z2m");
			});
			break;
		case api.pages.API_PAGE_SECURITY.str:
			apiGetPage(api.pages.API_PAGE_SECURITY, () => {
				if ($("#webAuth").prop(chck)) {
					SeqInputDsbl(false);
				}
				if ($("#fwEnabled").prop(chck)) {
					SeqInputDsblFw(false);
				}
			});
			break;
		case api.pages.API_PAGE_TOOLS.str:
			apiGetPage(api.pages.API_PAGE_TOOLS, () => {
				$.get(apiLink + api.actions.API_GET_FILELIST, function (data) {
					fillFileTable(data.files);
				});
				$.get(apiLink + api.actions.API_GET_PARAM + "&param=refreshLogs", function (data) {
					if (parseInt(data) >= 1) {
						logRefresh(parseInt(data) * 1000);
					} else {
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
	if (url != api.pages.API_PAGE_NETWORK.str && $('.toast').hasClass("show")) {
		if ($('#toastBody').text().indexOf("Wi-Fi mode") > 0) {
			$('.toast').toast('hide');
		}
	}
}

function espReboot() {
	$.get(apiLink + api.actions.API_CMD + "&cmd=" + api.commands.CMD_ESP_RES);
}

function localizeTitle(url) {
	let page_title = "";
	//console.log(url);
	switch (url) {
		case api.pages.API_PAGE_ROOT.str:
			page_title = i18next.t('l.st');
			break;
		case api.pages.API_PAGE_GENERAL.str:
			page_title = i18next.t('l.ge');
			break;
		case api.pages.API_PAGE_NETWORK.str:
			page_title = i18next.t('l.ne');
			break;
		case api.pages.API_PAGE_ZIGBEE.str:
			page_title = i18next.t('l.zi');
			break;
		case api.pages.API_PAGE_MQTT.str:
			page_title = i18next.t('l.mq');
			break;
		case api.pages.API_PAGE_VPN.str:
			page_title = i18next.t('l.vp');
			break;
		case api.pages.API_PAGE_SECURITY.str:
			page_title = i18next.t('l.se');
			break;
		case api.pages.API_PAGE_TOOLS.str:
			page_title = i18next.t('l.to');
			break;
		case api.pages.API_PAGE_ABOUT.str:
			page_title = i18next.t('l.ab');
			break;
	}
	$("[data-r2v='pageName']").text(page_title);//update page name
	$("title[data-r2v='pageName']").text(page_title + " - XZG");//update page title
}

function apiGetPage(page, doneCall, loader = true) {
	let animDuration = 0;
	const locCall = doneCall;
	if (loader) {
		animDuration = 200;
		showPreloader(true);
	}
	$("#pageContent").fadeOut(animDuration).load(apiLink + api.actions.API_GET_PAGE + "&page=" + page.num, function (response, status, xhr) {
		if (status == "error") {
			const msg = "Page load error: ";
			console.log(msg + xhr.status + " " + xhr.statusText);
			//alert(msg + xhr.status + " " + xhr.statusText); //popup error
		} else {
			if (loader) {
				showPreloader(false);
			}
			if (xhr.getResponseHeader("Authentication") == "ok") $(".logoutLink").removeClass(classHide);
			$("#pageContent").fadeIn(animDuration);

			$("form.saveParams").on("submit", function (e) {
				e.preventDefault();
				showWifiCreds();
				if (this.id === "netCfg") {
					var ethEnblSw = document.getElementById('ethEnbl').checked;
					var wifiEnblSw = document.getElementById('wifiEnbl').checked;
					if (!ethEnblSw && !wifiEnblSw) {
						toastConstructor("anyNetEnbl");
						setTimeout(function () {
							$('.toast').toast('hide');
						}, 10000);
						return;
					}
				}

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
						alert(i18next.t('c.erss'));
					},
					complete: function () {
						spiner.remove();
						spiner2.remove();
						btn.prop("disabled", false);
					}
				});
			});
			$("button").click(function () {
				const btnFail = "btn-cmd-fail";
				const btnSuccess = "btn-cmd-success";

				const jbtn = $(this);
				const cmd = jbtn.attr("data-cmd");
				if (cmd) {
					spiner.appendTo(jbtn);
					//spiner2.appendTo(jbtn);
					jbtn.prop("disabled", true);
					$.get(apiLink + api.actions.API_CMD + "&cmd=" + cmd, function (data) {
						spiner.remove();
						jbtn.prop("disabled", false);
						statusOk.appendTo(jbtn);
						setTimeout(function (jbtn) {
							statusOk.remove();
							if (cmd == 3) {
								modalConstructor("restartWait");
							}
						}, 2000, jbtn);
					}).fail(function () {
						spiner.remove();
						jbtn.prop("disabled", false);
						statusFail.appendTo(jbtn);
						setTimeout(function (jbtn) {
							statusFail.remove();
						}, 2000, jbtn);
						//alert(i18next.t('c.ercn'));
					});
				}
			});


			let selectedTimeZone = null;
			//if (xhr.getResponseHeader("respValuesArr") === null) return;
			if (xhr.getResponseHeader("respValuesArr") !== null) {
				//console.log("[apiGetPage] starting parse values");
				const values = JSON.parse(xhr.getResponseHeader("respValuesArr"));

				for (const property in values) {
					if (property === "timeZoneName") {
						selectedTimeZone = values[property];
						//console.log(selectedTimeZone);
						//console.log("timeZoneName");
						continue;
					}
					if (property === "hwBtnIs") {
						//hwBtnIs
						continue;
					}
					if (property === "hwLedUsbIs") {
						//hwBtnIs
						if (values[property]) {
							showDivById('ledsCard');
							showDivById('modeLedBtn');
						}
						continue;
					}
					if (property === "hwLedPwrIs") {
						if (values[property]) {
							showDivById('ledsCard');
							showDivById('pwrLedBtn');
						}
						//hwBtnIs
						continue;
					}
					if (property === "hwUartSelIs") {
						if (values[property]) {
							showDivById('modeSelCard');
							showDivById('curModeSelCard');
						}

						//hwBtnIs
						continue;
					}
				}
				dataReplace(values);
			}
			updateLocalizedContent();


			if (xhr.getResponseHeader("respTimeZones") !== null) {
				const zones = JSON.parse(xhr.getResponseHeader("respTimeZones"));
				const $dropdown = $("#timeZoneId");
				$dropdown.empty();

				if (Array.isArray(zones)) {
					zones.forEach(item => {
						let option = new Option(item, item);
						if (item === selectedTimeZone) {
							option.selected = true;
						}
						$dropdown.append(option);
					});
				} else {
					console.error("zones is not an array");
				}
			}

			if (typeof (locCall) == "function") locCall();//callback
		}
	});
}

function showDivById(divId) {
	$('#' + divId).removeAttr('hidden');
}

function getReadableTime(beginTime) {
	//let currentTime = Date.now(); // Текущее время в миллисекундах
	let elapsedTime = beginTime; // Прошедшее время в миллисекундах

	let seconds = Math.floor(elapsedTime / 1000); // Конвертируем в секунды
	let minutes = Math.floor(seconds / 60); // Конвертируем в минуты
	let hours = Math.floor(minutes / 60); // Конвертируем в часы
	let days = Math.floor(hours / 24); // Конвертируем в дни

	seconds %= 60;
	minutes %= 60;
	hours %= 24;

	let readableTime = `${days} d ` +
		`${hours < 10 ? '0' : ''}${hours}:` +
		`${minutes < 10 ? '0' : ''}${minutes}:` +
		`${seconds < 10 ? '0' : ''}${seconds}`;

	return readableTime;
}

function setTitleAndActivateTooltip(element, newTitle) {
	if (element) {
		element.setAttribute('data-bs-original-title', newTitle);

		let tooltipInstance = bootstrap.Tooltip.getInstance(element);

		if (tooltipInstance) {
			tooltipInstance.update();
		} else {

			new bootstrap.Tooltip(element, {
				placement: 'bottom',
				boundary: 'viewport'
			});
		}
	}
}

function showCardDrawIcon(property, values) {

	if (property === "ethConn") {
		showDivById("ttEt");
		let status;

		if (values[property] === 1) {
			status = IconsStatusCodes.OK;
		} else {
			status = IconsStatusCodes.ERROR;
		}
		setIconGlow('ethIcon', status);
	}

	if (property === "wifiMode") {
		showDivById("ttWi");
	}

	if (property === "wifiConn") {
		let status = IconsStatusCodes.ERROR;

		const wifiMode = values.wifiMode;

		const wifiConn = values[property];

		if (wifiMode == 1) {
			if (wifiConn == 1) {
				status = IconsStatusCodes.OK;
			}
			else if (wifiConn == 2) {
				status = IconsStatusCodes.WARN;
			}
			else {
				status = IconsStatusCodes.ERROR;
			}
		}
		else if (wifiMode == 2) {
			if (wifiConn == 1) {
				status = IconsStatusCodes.OK;
			}
			else if (wifiConn == 2) {
				status = IconsStatusCodes.WARN;
			}
			else {
				status = IconsStatusCodes.ERROR;
			}
		}
		setIconGlow('wifiIcon', status);
	}

	if (property === "connectedSocketStatus") {
		let status;

		if (values[property] > 0) {
			status = IconsStatusCodes.OK;
		} else {
			status = IconsStatusCodes.ERROR;
		}

		setIconGlow('socketIcon', status);
	}

	if (property === "mqConnect") {
		showDivById("ttMq");
		let status;

		if (values[property] === 1) {
			status = IconsStatusCodes.OK;
		} else {
			status = IconsStatusCodes.WARN;
		}
		setIconGlow('mqttIcon', status);
	}

	if (property === "wgInit") {
		showDivById("ttWg");
		let status;

		const wgInit = values[property];
		const wgConnect = values.wgConnect;

		if (wgInit === 1 && wgConnect === 1) {
			status = IconsStatusCodes.OK;
		} else if (wgInit === wgInit) {
			status = IconsStatusCodes.WARN;
		} else {
			status = IconsStatusCodes.ERROR;
		}
		setIconGlow('vpnIcon', status);
	}
	if (property === "hnInit") {
		showDivById("ttHn");
		let status;

		if (values[property] === 1) {
			status = IconsStatusCodes.OK;
		} else {
			status = IconsStatusCodes.ERROR;
		}
		setIconGlow('vpnIcon', status);
	}
}

function updateTooltips() {
	//console.log("updateTooltips");
	//title = i18next.t("p.st.zbc.scc");
	let valueToSet = "";
	if (updateValues.connectedSocketStatus > 0) {
		valueToSet = i18next.t('p.st.zbc.sccy', { count: updateValues.connectedSocketStatus });
	}
	else {
		valueToSet = i18next.t('p.st.zbc.sccn');
	}
	valueToSet = valueToSet + "<br><i>" + getReadableTime(updateValues.uptime - updateValues.connectedSocket) + "</i>"
	let element = document.getElementById('socketIcon');
	setTitleAndActivateTooltip(element, '<b>' + valueToSet + '</b>');

	if (updateValues.ethConn) {
		valueToSet = i18next.t('c.conn');
		valueToSet = valueToSet + "<br><i>" + updateValues.ethIp + "</i>";
	}
	else {
		valueToSet = i18next.t('c.disconn');
	}
	element = document.getElementById('ethIcon');
	setTitleAndActivateTooltip(element, '<b>' + valueToSet + '</b>');

	if (updateValues.wifiConn) {
		valueToSet = i18next.t('c.conn');
		valueToSet = valueToSet + "<br><i>" + updateValues.wifiIp + "</i>";
		valueToSet = valueToSet + "<br>" + updateValues.wifiSsid;
	}
	else {
		valueToSet = i18next.t('c.disconn');
	}
	element = document.getElementById('wifiIcon');
	setTitleAndActivateTooltip(element, '<b>' + valueToSet + '</b>');

	if (updateValues.mqConnect) {
		valueToSet = i18next.t('c.conn');
		valueToSet = valueToSet + "<br><i>" + updateValues.mqBroker + "</i>";
	}
	else {
		valueToSet = i18next.t('c.disconn');
	}
	element = document.getElementById('mqttIcon');
	setTitleAndActivateTooltip(element, '<b>' + valueToSet + '</b>');

	if (updateValues.wgConnect) {
		valueToSet = i18next.t('c.conn');
		//valueToSet = valueToSet + "<br><i>" + updateValues.wifiIp + "</i>";
	}
	else {
		valueToSet = i18next.t('c.disconn');
	}
	element = document.getElementById('vpnIcon');
	setTitleAndActivateTooltip(element, '<b>' + valueToSet + '</b>');

	valueToSet = i18next.t('p.st.dic.du');
	valueToSet = valueToSet + "<br><i>" + getReadableTime(updateValues.uptime) + "</i>";
	element = document.getElementById('clock');
	setTitleAndActivateTooltip(element, '<b>' + valueToSet + '</b>');


}

function extractTime(dateStr) {
	const date = new Date(dateStr);
	let hours = date.getHours().toString();
	let minutes = date.getMinutes().toString().padStart(2, '0');
	let seconds = date.getSeconds().toString().padStart(2, '0');

	let pm = "AM";
	if (localStorage.getItem('clock_format_12h') == 'true') {
		if (hours > 12) {
			hours = hours - 12;
			pm = "PM";
		}
		return `${hours}:${minutes}:${seconds} ${pm}`;
	}
	else {
		hours = hours.padStart(2, '0');
		return `${hours}:${minutes}:${seconds}`;
	}

}

function dataReplace(values, navOnly = false) {

	//Object.assign(values, new_values);
	var clockButton = document.getElementById('clock');
	if (clockButton) {
		clockButton.textContent = extractTime(values.localTime);
	}

	var baseSelector;

	if (navOnly) {
		baseSelector = "nav.navbar [data-r2v='";
	} else {
		baseSelector = "[data-r2v='";
	}

	for (const property in values) {

		showCardDrawIcon(property, values);
		let $elements = $(baseSelector + property + "']");
		//console.log($elements);
		$elements.map(function () {
			const elemType = $(this).prop('nodeName').toLowerCase();
			let valueToSet = values[property];

			const isIpValue = /^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$/.test(valueToSet);
			const isMaskInPropertyName = property.toLowerCase().includes('mask');

			if (isIpValue && !isMaskInPropertyName) {
				valueToSet = '<a href="http://' + valueToSet + '">' + valueToSet + '</a>';
			}

			switch (property) {
				case "connectedSocketStatus": //clients
					if (valueToSet) {
						valueToSet = i18next.t('p.st.zbc.sccy', { count: valueToSet });
					}
					else {
						valueToSet = i18next.t('p.st.zbc.sccn');
					}
					break;
				case "espHeapSize":
					updateProgressBar("prgHeap", values.espHeapUsed, 1, valueToSet)
					break;
				case "espNvsSize":
					updateProgressBar("prgNvs", values.espNvsUsed, 0, valueToSet)
					break;
				case "espFsSize":
					updateProgressBar("prgFs", values.espFsUsed, 0, valueToSet)
					break;
				case "deviceTemp":
					updateProgressBar("prgTemp", valueToSet, 15, 85)
					break;
				case "wifiRssi":
					updateProgressBar("prgRssi", valueToSet, -105, 0)
					valueToSet = valueToSet + " " + "dBm";
					break;
				case "connectedSocket": // socket time
					valueToSet = getReadableTime(values.uptime - valueToSet);
					break;
				case "uptime": 			// device uptime
					valueToSet = getReadableTime(valueToSet);
					break;
				case "ethSpd":
					if (valueToSet != "noConn") {
						valueToSet = valueToSet + " " + "Mbps";
					}
					break;
				case "espFlashType":
					updateValues[property] = values[property];
					switch (valueToSet) {
						case 1:
							valueToSet = i18next.t('p.st.dic.efti');
							break;
						case 2:
							valueToSet = i18next.t('p.st.dic.efte');
							break;
					}
					break;

				case "operationalMode":
					updateValues[property] = values[property];
					switch (valueToSet) {
						case 0:
							valueToSet = i18next.t('p.st.zbc.opn');
							break;
						case 1:
							valueToSet = i18next.t('p.st.zbc.opu');
							break;
					}
					break;
				case "ethDhcp":
				case "wifiDhcp":
					if (valueToSet) {
						valueToSet = i18next.t('c.en');
					}
					else {
						valueToSet = i18next.t('c.dis');
					}
					break;

				case "ethConn":
				case "wifiConn":
				case "mqConnect":
				case "wgConnect":
					if (valueToSet) {
						valueToSet = i18next.t('c.conn');
					}
					else {
						valueToSet = i18next.t('c.disconn');
					}

					break;

				case "wifiMode":
					switch (valueToSet) {
						case 0: // Error
							valueToSet = i18next.t('c.err');
							break;
						case 1:
							valueToSet = i18next.t('p.st.wc.mcl');
							break;
						case 2:
							valueToSet = i18next.t('p.st.wc.map');
							break;
					}
					break;

				case "wifiConn": //rename to wifiStatus
					switch (valueToSet) {
						case 0: // Error
							valueToSet = i18next.t('c.err');
							break;
						case 1: // Connecting / Not Started
							valueToSet = i18next.t('c.connecting');
							break;
						case 2: // Connected / Started
							valueToSet = i18next.t('c.conn');
							break;
					}
					break;

				case "wgInit":
				case "hnInit":
					if (valueToSet) {
						valueToSet = i18next.t('c.init');
					}
					else {
						valueToSet = i18next.t('c.err');
					}
					break;
			}

			if (valueToSet == "noConn") {
				valueToSet = i18next.t('c.nc');
			}

			switch (elemType) {
				case "input":
				case "select":
				case "textarea":
					const type = $(this).prop('type').toLowerCase();
					if (elemType == "input" && (type == "checkbox" || type == "radio")) {
						$(this).prop(chck, values[property]);
					} else {
						$(this).val(values[property]);
					}
					break;
				case "option":
					$(this).prop("selected", true);
					break;
				default:
					if (isIpValue && !isMaskInPropertyName) {
						$(this).html(valueToSet);
					} else {
						$(this).text(valueToSet);
					}
					break;
			}
		});
	}
}

function tglPassView(button) {
	var passwordInput = button.previousElementSibling;
	var svgUseElement = button.querySelector('svg use');
	if (passwordInput.type === "password") {
		passwordInput.type = "text";
		svgUseElement.setAttribute('xlink:href', 'icons.svg#eye-slash-fill');
	} else {
		passwordInput.type = "password";
		svgUseElement.setAttribute('xlink:href', 'icons.svg#eye-fill');
	}
}

function showPreloader(state) {
	if (state) {
		$("#xzgPreloader").removeClass(classHide);
	} else {
		$("#xzgPreloader").addClass(classHide);
	}
}

function toastConstructor(params, text) {
	$("#toastButtons").html("");
	$("#toastHeaderText").text("");
	$("#toastBody").text("");
	switch (params) {
		case "espUpdAvail":
			$("#toastHeaderText").text(i18next.t("ts.esp.upd.tt"));
			$("#toastBody").text(text);
			$('<button>', {
				type: "button",
				"class": "btn btn-outline-danger",
				text: i18next.t("c.drm"),
				click: function () {
					$('.toast').toast('hide');
					localStorage.setItem('update_notify', 1);
				}
			}).appendTo("#toastButtons");
			$('<button>', {
				type: "button",
				"class": "btn btn-warning",
				text: i18next.t("c.now"),
				click: function () {
					$('.toast').toast('hide');
					modalConstructor("flashESP");
				}
			}).appendTo("#toastButtons");
			$('<button>', {
				type: "button",
				"class": "btn btn-outline-success",
				text: i18next.t("c.ltr"),
				click: function () {
					$('.toast').toast('hide');

				}
			}).appendTo("#toastButtons");
			break;
		case "espBetaFb":
			$("#toastHeaderText").text(i18next.t("ts.esp.beta.tt"));
			$("#toastBody").text(i18next.t("ts.esp.beta.msg"));
			$('<button>', {
				type: "button",
				"class": "btn btn-warning",
				text: i18next.t("ts.esp.beta.cnt"),
				click: function () {
					var url = "https://t.me/xzg_fw";
					window.open(url, '_blank');
				}
			}).appendTo("#toastButtons");
			$('<button>', {
				type: "button",
				"class": "btn btn-outline-primary",
				text: i18next.t("c.cl"),
				click: function () {
					$('.toast').toast('hide');
					localStorage.setItem('beta_feedback', 1);
				}
			}).appendTo("#toastButtons");
			break;
		case "anyNetEnbl":
			$("#toastHeaderText").text(i18next.t("ts.esp.ane.tt"));
			$("#toastBody").text(i18next.t("ts.esp.ane.msg"));
			$('<button>', {
				type: "button",
				"class": "btn btn-outline-primary",
				text: i18next.t("c.cl"),
				click: function () {
					$('.toast').toast('hide');
				}
			}).appendTo("#toastButtons");
			break;
		case "noZbFw":
			$("#toastHeaderText").text(i18next.t("ts.zb.nzfa.tt"));
			$("#toastBody").text(text); //		$("#toastBody").text(i18next.t("ts.zb.nzfa.msg"));
			$('<button>', {
				type: "button",
				"class": "btn btn-outline-primary",
				text: i18next.t("c.cl"),
				click: function () {
					$('.toast').toast('hide');
				}
			}).appendTo("#toastButtons");
			setTimeout(function () {
				$('.toast').toast('hide');
			}, 10000);
			break;
		default:
			break;
	}
	$('.toast').toast('show');
}


function closeModal() {
	$("#modal").modal("hide");
}

function restartWait() {
	setTimeout(function () {
		modalConstructor("restartWait");
		//console.log("[restartWait] start");
	}, 1000);
}

function extractVersionFromReleaseTag(url) {
	const regex = /\/releases\/download\/([\d.]+)\//;
	const match = url.match(regex);
	if (match) {
		return match[1];
	} else {
		return null;
	}
}

function espFlashGitWait(params) {
	//ESPfwStartEvents();

	setTimeout(function () {
		if (typeof params !== 'undefined' && params !== null && typeof params.link !== 'undefined') {
			let version = extractVersionFromReleaseTag(params.link);
			$.get(apiLink + api.actions.API_CMD + "&cmd=" + api.commands.CMD_ESP_UPD_URL + "&url=" + params.link, function (data) { });
			$('#bar').html(i18next.t('md.esp.fu.vgds', { ver: version }));
		}
		else {
			$.get(apiLink + api.actions.API_CMD + "&cmd=" + api.commands.CMD_ESP_UPD_URL, function (data) { });
			$('#bar').html(i18next.t('md.esp.fu.lgds'));
		}
		console.log("[git_flash] start");

	}, 500);


}

let retryCount = 0;
const maxRetries = 30;

function updateRootEvents(callback) {

	if (window.location.pathname.startsWith('/login')) {
		return;
	}

	if (retryCount >= maxRetries) {
		console.log(i18next.t('c.cerp'));
		alert(i18next.t('c.cerp'));
		return;
	}

	var source = new EventSource('/events', { withCredentials: false, timeout: 1000 });
	console.log("Events try to open");

	source.addEventListener('open', function (e) {
		console.log("Events Connected");
		callback(true);
		retryCount = 0;
	}, false);

	source.addEventListener('error', function (e) {
		if (e.target.readyState != EventSource.OPEN) {
			console.log("Events Err. Reconnecting...");
			retryCount++;
			setTimeout(function () {
				source.close();
				updateRootEvents(callback);
			}, 1000);
		}
	}, false);

	source.addEventListener('root_update', function (e) {
		if (e.data != "finish") {
			Object.assign(updateValues, JSON.parse(e.data));
		} else {
			var navOnly = window.location.pathname != "/";
			dataReplace(updateValues, navOnly);
			updateTooltips();
		}
	});

	source.addEventListener('ZB_FW_prgs', function (e) {
		//console.log(e.data);
		$('#zbFlshPgsTxt').html(i18next.t('md.esp.fu.prgs', { per: e.data }));
		$("#zbFlshPrgs").css("width", e.data + '%');
	}, false);

	source.addEventListener('NV', function (e) {
		//console.log(e.data);
		let currentContent = $("#console").val();
		let newContent = currentContent + "\n" + e.data;
		$("#console").val(newContent);
	}, false);

	source.addEventListener('ZB_FW_info', function (e) {
		let data = e.data.replaceAll("`", "<br>");
		console.log(data);

		if (e.data == "start") {
			$("#zbFlshPrgs").removeClass("progress-bar-animated");
			data = i18next.t('md.zg.fu.st');
		}

		if (e.data == "erase") {
			data = i18next.t('md.zg.fu.er');
		}

		if (e.data == "finish") {
			data = i18next.t('md.zg.fu.fn');
			$(".progress").addClass(classHide);
			$(modalBody).css("color", "green");
			setTimeout(() => {
				$(modalBtns).html("");
				modalAddClose();

			}, 1000);
		}

		$("#zbFlshPgsTxt").html(data);

	}, false);

	source.addEventListener('ZB_FW_file', function (e) {
		let fileName = fileFromUrl(e.data);
		if (fileName) {
			data = i18next.t('md.zg.fu.f', { file: fileName });
		}
		else {
			let ver = "Unknown";
			if (e.data != 0) {
				ver = e.data;
			}
			data = i18next.t('md.zg.fu.nv', { ver: ver });
			setTimeout(function () {
				espReboot();
				restartWait();
			}, 2000);
		}
		$("#zbFlshPgsTxt").html(data);
	}, false);

	source.addEventListener('ZB_FW_err', function (e) {
		const data = e.data.replaceAll("`", "<br>");
		$(modalBtns).html("");
		$("#zbFlshPgsTxt").html(data);
		$(".progress").addClass(classHide);
		$(modalBody).html(e.data).css("color", "red");
		modalAddClose();
	}, false);


	source.addEventListener('ESP_FW_prgs', function (e) {

		$('#prg').css('width', e.data + '%');
		$('#bar').html(i18next.t('md.esp.fu.prgs', { per: e.data }));
		$("#prg").removeClass("progress-bar-animated");

		if (Math.round(e.data) > 99) {
			setTimeout(function () {
				$('#bar').html(i18next.t('md.esp.fu.ucr')).css("color", "green");

				setTimeout(function () {
					localStorage.setItem('update_notify', 0);
					restartWait();

				}, 1000);
			}, 500);
		}

	}, false);
}

function fileFromUrl(url) {
	const urlParts = url.split('/');
	if (urlParts.length > 1) {
		return urlParts[urlParts.length - 1];
	}
	return null;
}

function modalAddSpiner() {
	$('<div>', {
		"role": "status",
		"class": "spinner-border spinner-color",
		append: $("<span>", {
			"class": classHide
		})
	}).appendTo(modalBtns);
}

function startZbFlash(link) {
	$(modalBtns).html("");
	$(modalBody).html("");

	$("<div>", {
		text: i18next.t("md.esp.fu.wm"),
		class: "my-1 text-sm-center text-danger"
	}).appendTo(modalBody);

	let fileName = fileFromUrl(link);
	$("<div>", {
		text: fileName,
		class: "my-1 text-sm-center"
	}).appendTo(modalBody);

	modalAddCancel();
	$('<button>', {
		type: "button",
		"class": "btn btn-warning",
		text: i18next.t('c.sure'),
		title: i18next.t("md.esp.fu.wm"),
		click: function () {

			$.get(apiLink + api.actions.API_CMD + "&cmd=" + api.commands.CMD_ZB_FLASH + "&url=" + link);
			$(modalBtns).html("");
			modalAddSpiner();
			$(modalBody).html("");
			$("<div>", {
				id: "zbFlshPgsTxt",
				text: i18next.t("md.esp.fu.wdm"),
				class: "mb-2 text-sm-center"
			}).appendTo(modalBody);
			$("<div>", {
				"class": "progress",
				append: $("<div>", {
					"class": "progress-bar progress-bar-striped progress-bar-animated",
					id: "zbFlshPrgs",
					style: "width: 100%; background-color: var(--link-color);"
				})
			}).appendTo(modalBody);
		}
	}).appendTo(modalBtns);
}

function modalAddClose() {
	$('<button>', {
		type: "button",
		"class": "btn btn-primary",
		text: i18next.t('c.cl'),
		click: function () {
			closeModal();
		}
	}).appendTo(modalBtns);
}

function modalAddCancel() {
	$('<button>', {
		type: "button",
		"class": "btn btn-primary",
		text: i18next.t('c.cancel'),
		click: function () {
			closeModal();
		}
	}).appendTo(modalBtns);
}

function updateProgressBar(id, current, min, max) {
	var progressBar = document.getElementById(id);
	var width = ((current - min) / (max - min)) * 100;
	progressBar.style.width = width + '%';

	var cssVarColorOk = getComputedStyle(document.documentElement)
		.getPropertyValue('--bs-success').trim();
	var cssVarColorWarn = getComputedStyle(document.documentElement)
		.getPropertyValue('--bs-warning').trim();
	var cssVarColorErr = getComputedStyle(document.documentElement)
		.getPropertyValue('--bs-danger').trim();

	var invert = false;
	if (id == "prgRssi") {
		invert = true;
	}

	if ((invert && width > 65) || (!invert && width < 50)) {
		progressBar.style.backgroundColor = cssVarColorOk;
	} else if ((invert && width > 30) || (!invert && width < 80)) {
		progressBar.style.backgroundColor = cssVarColorWarn;
	} else {
		progressBar.style.backgroundColor = cssVarColorErr;
	}


	//progressBar.setAttribute("aria-valuenow", current);
	//progressBar.textContent = width.toFixed(0) + '%';  // Отображаем проценты внутри прогресс бара
}


function findAllVersionsSorted(data, chip) {
	const categories = ['router', 'coordinator', 'thread'];
	const result = {};

	const chipMap = { "CC2652P2_launchpad": "CC2652P2_launchpad", "CC2652P2_other": "CC2652P2_other", "CC2652P7": "CC2652P7", "CC2652RB": "CC2652RB" };
	let deviceName = chipMap[chip];
	if (!deviceName) {
		//throw new Error("Unsupported chip type or deviceName not set.");
		console.error("error with ZB chip detect");
		deviceName = chip;
	}

	categories.forEach(category => {
		if (data[category]) {
			Object.keys(data[category]).forEach(subCategory => {
				if (subCategory.startsWith(deviceName)) {
					Object.keys(data[category][subCategory]).forEach(file => {
						const fileInfo = data[category][subCategory][file];
						if (!result[category]) {
							result[category] = [];
						}
						result[category].push({
							file: file,
							ver: fileInfo.ver,
							link: fileInfo.link,
							notes: fileInfo.notes
						});
					});
				}
			});
		}
	});

	for (const category in result) {
		result[category].sort((a, b) => b.ver - a.ver);
	}

	return result;
}

// Function definition outside the switch-case
function createReleaseBlock(file, deviceType) {

	let deviceName;
	let deviceIcon;
	let buttonClass;

	if (deviceType == 1) {
		deviceName = i18next.t('md.zb.dtc');
		buttonClass = "btn btn-outline-danger";
		deviceIcon = "📡";
	} else if (deviceType == 2) {
		deviceName = i18next.t('md.zb.dtr');
		buttonClass = "btn btn-outline-success";
		deviceIcon = "🛰️";
	} else if (deviceType == 3) {
		deviceName = i18next.t('md.zb.dtt');
		buttonClass = "btn btn-outline-primary";
		deviceIcon = "🚀";
	}

	const releaseBlock = $("<div>", { "class": "release-block", "style": "margin-bottom: 20px;" });
	const headerAndButtonContainer = $('<div>', { "class": "d-flex justify-content-between align-items-start" }).appendTo(releaseBlock);

	const emojiBlock = $('<span>', { "text": deviceIcon }).css('margin-right', '5px').appendTo(headerAndButtonContainer);
	const header = $("<h5>", { "class": "mb-0", "text": file.ver }).appendTo(headerAndButtonContainer);

	setTitleAndActivateTooltip(emojiBlock[0], deviceName);

	let fileName = fileFromUrl(file.link);

	const buttonContainer = $('<div>', { "class": "d-flex align-items-start" }).appendTo(headerAndButtonContainer);
	const button = $('<a>', {
		"class": buttonClass,
		"click": function () {
			startZbFlash(file.link);
			let tooltipInstance = bootstrap.Tooltip.getInstance(this);
			if (tooltipInstance) {
				tooltipInstance.hide();
			}
		},
		"data-bs-toggle": "tooltip",
		"title": fileName,
		"text": i18next.t('c.inst'),
		"role": "button"
	}).css("white-space", "nowrap").appendTo(buttonContainer);

	setTitleAndActivateTooltip(button[0], file.link);

	$("<div>", { "class": "mt-2 release-description", "html": file.notes }).appendTo(releaseBlock);
	$("<hr>").appendTo(releaseBlock);
	return releaseBlock;
}


function modalConstructor(type, params) {
	const headerText = ".modal-title";
	const headerBtnClose = ".modal-btn-close";
	const modalBody = ".modal-body";
	const modalBtns = ".modal-footer";
	//$(".modal").css("display", "");
	$(headerText).text("").css("color", "");
	//$(modalBody).text("").css("color", "");
	$(modalBody).empty().css({ color: "", maxHeight: "400px", overflowY: "auto" });

	$(modalBtns).html("");
	switch (type) {
		case "flashESP":
			$(headerText).text(i18next.t('md.esp.fu.tt')).css("color", "red");;
			let action = 0;
			if (params instanceof FormData) {
				console.log("FormData received:", params);
				action = 1
				$(modalBody).html(i18next.t("md.esp.fu.lfm"));
			}
			else if (params && 'link' in params && typeof params.link === 'string' && /^https?:\/\/.*/.test(params.link)) {
				console.log("URL received:", params.link);
				action = 2;
				$(modalBody).html(i18next.t("md.esp.fu.gvm", { ver: params.ver }));
			} else {
				action = 3
				console.log("Else ? received:", params);
				$(modalBody).html(i18next.t("md.esp.fu.glm"));
			}
			$("<div>", {
				text: i18next.t("md.esp.fu.wm"),
				class: "my-1 text-sm-center text-danger"
			}).appendTo(modalBody);

			modalAddCancel();
			$('<button>', {
				type: "button",
				"class": "btn btn-warning",
				text: i18next.t('c.sure'),
				title: i18next.t("md.esp.fu.wm"),
				click: function () {
					$(modalBtns).html("");
					modalAddSpiner();
					$(modalBody).html("");
					$("<div>", {
						id: "bar",
						text: i18next.t("md.esp.fu.wdm"),
						class: "mb-2 text-sm-center"
					}).appendTo(modalBody);
					$("<div>", {
						class: "progress",
						append: $("<div>", {
							"class": "progress-bar progress-bar-striped progress-bar-animated",
							id: "prg",
							style: "width: 100%; background-color: var(--link-color);"
						})
					}).appendTo(modalBody);
					if (action == 1) {
						$.ajax({
							url: "/update",
							type: "POST",
							data: params,
							contentType: false,
							processData: false,
							xhr: function () {
								return new window.XMLHttpRequest();
							},
							success: function (data, textStatus, jqXHR) {
								console.log("Success!");
							},
							error: function (jqXHR, textStatus, errorThrown) {
								console.log("Error:", errorThrown);
							}
						});
					}
					else if (action == 2) {
						espFlashGitWait(params);
					} else if (action == 3) {
						espFlashGitWait();
					}
				}
			}).appendTo(modalBtns);
			break;
		case "fetchGitReleases":
			$(headerText).text(i18next.t('md.esp.fu.tt'));
			$(modalBody).html(i18next.t('md.esp.fu.fri'));
			modalAddSpiner();

			fetchReleaseData().then(t => {
				modalConstructor("espGitVersions", {
					releases: t
				})
			}).catch(t => {
				console.error("Failed to fetch release data:", t)
			})
			break;
		case "flashZBM":
			$(headerText).text(i18next.t('md.zb.ot')).css("color", "red");
			break;

		case "flashZB":
			$(headerText).text(i18next.t('md.zb.ot')).css("color", "red");

			$(modalBody).html(i18next.t('md.zb.rfm'));
			modalAddSpiner();

			$.get(zbFwInfoUrl).then(data => {
				const json = JSON.parse(data);
				return $.get(apiLink + api.actions.API_GET_PARAM + "&param=zbHwVer").then(chip => {
					return findAllVersionsSorted(json, chip);
				});
			}).then(fw => {
				$(modalBody).html("");
				$(modalBtns).html("");
				modalAddClose();

				if (fw.coordinator && fw.coordinator.length > 0) {
					fw.coordinator.forEach(file => createReleaseBlock(file, 1).appendTo(".modal-body"));
				} else {
					$("<div>", { "text": i18next.t('md.zb.ncf'), "class": "alert alert-warning" }).appendTo(".modal-body");
				}

				if (fw.router && fw.router.length > 0) {
					fw.router.forEach(file => createReleaseBlock(file, 2).appendTo(".modal-body"));
				} else {
					$("<div>", { "text": i18next.t('md.zb.nrf'), "class": "alert alert-warning" }).appendTo(".modal-body");
				}

				if (fw.thread && fw.thread.length > 0) {
					fw.thread.forEach(file => createReleaseBlock(file, 3).appendTo(".modal-body"));
				} else {
					$("<div>", { "text": i18next.t('md.zb.ntf'), "class": "alert alert-warning" }).appendTo(".modal-body");
				}

			}).fail(error => {
				$(modalBody).html(i18next.t('md.zb.efr')).css("color", "red");
				$(modalBtns).html("");
				modalAddClose();
				console.error(error);
			});

			break;
		case "factoryResetWarning":
			$(headerText).text(i18next.t('md.esp.fr.tt')).css("color", "red");
			$(modalBody).text(i18next.t('md.esp.fr.msg')).css("color", "red");
			modalAddCancel();
			$('<button>', {
				type: "button",
				"class": "btn btn-danger",
				text: i18next.t('c.sure'),
				click: function () {
					$.get(apiLink + api.actions.API_CMD + "&cmd=" + api.commands.CMD_ESP_FAC_RES + "&conf=1", function () {
					});
					modalConstructor("restartWait");
				}
			}).appendTo(modalBtns);
			break;
		case "espGitVersions":
			$(headerText).text(i18next.t('md.esp.fu.gvt'));
			params.releases.forEach(release => {
				const releaseBlock = $("<div>", { "class": "release-block", "style": "margin-bottom: 20px;" });
				const headerAndButtonContainer = $('<div>', {
					"class": "d-flex justify-content-between align-items-start"
				}).appendTo(releaseBlock);
				$("<h5>", {
					"class": "mb-0",
					"text": release.tag_name
				}).appendTo(headerAndButtonContainer);

				const buttonContainer = $('<div>', {
					"class": "d-flex align-items-start"
				}).appendTo(headerAndButtonContainer);

				if (release.assets.length > 0) {
					const downloadLink = release.assets[1].browser_download_url;
					$('<a>', {
						"class": "btn btn-outline-warning",
						click: function () {
							var params = {};
							params['link'] = downloadLink;
							params['ver'] = release.tag_name;
							modalConstructor("flashESP", params);
						},
						"data-bs-toggle": "tooltip",
						"title": i18next.t('c.inst') + " " + release.tag_name,
						"text": i18next.t('c.inst'),
						"role": "button"
					}).css("white-space", "nowrap")
						.appendTo(buttonContainer);
				}
				const releaseDescriptionHtml = marked.parse(release.body);
				$("<div>", {
					"class": "mt-2 release-description",
					"html": releaseDescriptionHtml
				}).appendTo(releaseBlock);
				$("<hr>").appendTo(releaseBlock);
				releaseBlock.appendTo(".modal-body");
			});
			/*$('<button>', {
				type: "button",
				"class": "btn btn-primary",
				text: i18next.t('c.cl'),
				click: function () {
					closeModal();
				}
			}).appendTo(modalBtns);*/
			modalAddClose();
			$('<button>', {
				type: "button",
				"class": "btn btn-warning",
				text: i18next.t('p.to.ilfg'),
				click: function () {
					//closeModal();
					//localStorage.setItem('update_notify', 0);
					//espFlashGitWait();
					modalConstructor("flashESP");
				}
			}).appendTo(modalBtns);
			break;
		case "restartWait":
			$(headerText).text(i18next.t('md.esp.rst.tt'));
			$(modalBody).html(i18next.t('md.esp.rst.msg'));
			$('<div>', {
				"role": "status",
				"class": "spinner-border spinner-color",
				append: $("<span>", {
					"class": classHide
				})
			}).appendTo(modalBtns);
			var waitTmr = setInterval(function () {
				$.ajax({
					url: "/",
					method: "GET",
					cache: false,
					timeout: 2000,
					success: function () {
						clearInterval(waitTmr);
						clearTimeout(timeoutTmr);
						closeModal();
						window.location = "/";
					},
					error: function (jqXHR, textStatus) {
						if (textStatus === "timeout") {
							console.log("Request timed out.");
						} else {
							console.log("Error:", textStatus);
						}
					}
				});
			}, 3000);
			var timeoutTmr = setTimeout(function () {
				clearInterval(waitTmr);
				$(modalBtns).html("");
				$(modalBody).text(i18next.t('md.esp.rst.nrps')).css("color", "red");
				$('<button>', {
					type: "button",
					"class": "btn btn-warning",
					text: i18next.t('c.cl'),
					click: function () {
						closeModal();
					}
				}).appendTo(modalBtns);
			}, 60000);
			break;
		case "saveOk":
			$.get(apiLink + api.actions.API_GET_PARAM + "&param=wifiEnable", function (wifiEnable) {
				if (window.location.pathname == "/network" & wifiEnable) {
					$(headerText).text(i18next.t('md.esp.ws.tt'));
					$(modalBody).text(i18next.t('md.esp.ws.msg'));
					$('<div>', {
						"role": "status",
						"class": "spinner-border spinner-color",
						append: $("<span>", {
							"class": classHide
						})
					}).appendTo(modalBtns);
					let counter = 0;
					var getWifiIp = setInterval(function (params) {
						if (counter <= 15) {
							$.get(apiLink + api.actions.API_WIFICONNECTSTAT, function (data) {
								if (data.connected) {
									espReboot();
									clearInterval(getWifiIp);
									setTimeout(() => {//5sec for reboot
										$(".modal-body").html(`<span style="color: green">${i18next.t('c.conn')}!</span><br>${i18next.t('md.esp.ws.nip', { ip: data.ip })}`);
										$(modalBtns).html("");
										$('<button>', {
											type: "button",
											"class": "btn btn-success",
											text: i18next.t('md.esp.ws.btn') + " " + data.ip,
											click: function () {
												window.location = "http://" + data.ip + "/";
											}
										}).appendTo(modalBtns);
									}, 5000);
								} else {
									counter++;
								}
							});
						} else {
							clearInterval(getWifiIp);
							$(modalBody).text(i18next.t('md.esp.ws.err')).css("color", "red");
							$(modalBtns).html("");
							$('<button>', {
								type: "button",
								"class": "btn btn-success",
								text: i18next.t('c.cl'),
								click: function () {
									closeModal();
								}
							}).appendTo(modalBtns);
						}
					}, 1000);
				} else {
					let body = i18next.t('md.ss.msg');
					$(headerText).text(i18next.t('md.ss.tt'));
					/*if ($("#wifiMode").prop(chck)) {
						body += "You will be redirected to the Wi-Fi network selection page.";
						$('<button>', {
							type: "button",
							"class": "btn btn-success",
							text: "Select Wi-Fi network",
							click: function () {
								closeModal();
								loadPage("/network");
							}
						}).appendTo(modalBtns);
					} else {*/
					body += i18next.t('md.ss.rr');
					$('<button>', {
						type: "button",
						"class": "btn btn-warning",
						text: i18next.t('md.ss.rl'),
						click: function () {
							closeModal();
						}
					}).appendTo(modalBtns);
					$('<button>', {
						type: "button",
						"class": "btn btn-primary",
						text: i18next.t('md.ss.rn'),
						click: function () {
							closeModal();
							espReboot();
							restartWait();
						}
					}).appendTo(modalBtns);
					//}
					$(modalBody).text(body);
				}
			});
			break;
		case "keepWeb":
			$(headerText).text(i18next.t('p.ge.kw'));
			$(modalBody).text(i18next.t('md.kw.msg'));
			$('<button>', {
				type: "button",
				"class": "btn btn-primary",
				text: i18next.t('c.ok'),
				click: function () {
					closeModal();
				}
			}).appendTo(modalBtns);
			break;

		default:
			break;
	}
	$("#modal").modal("show");
}

function showWifiCreds() {
	$("#collapseWifiPass").collapse("show");
}
function getWifiList() {
	$("#collapseWifiPass").collapse("hide");
	$("#wifiScanPreloader").removeClass(classHide);
	$("#wifiScanButton").addClass(classHide);
	WifiEnbl(true);
	$.get(apiLink + api.actions.API_STARTWIFISCAN, function (data) { //visually-hidden wifiLoadSpinner
		const tmrUpdWifi = setInterval(function () {
			$.get(apiLink + api.actions.API_WIFISCANSTATUS, function (data) {
				if (!data.scanDone) return;
				if (!data.wifi) {
					alert(i18next.t('p.ne.wifi.nnf'));
				} else {
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
						$(".ssidSelector").click(function (elem) {
							$("#wifiSsid").val(elem.currentTarget.id);
							$("#wifiPass").val("");
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
	return (((window.innerWidth <= 767)) && ('ontouchstart' in document.documentElement));
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
	let touchedElement;

	function handleSwipe(event) {
		if (event.type == "touchstart") {
			startPoint = event.touches[0].clientX;
			touchedElement = $(event.target);
		} else if (event.type == "touchend") {
			let endPoint = event.changedTouches[0].clientX;

			if ((endPoint - startPoint) > 80) {
				$("#sidenav").addClass("sidenav-active");
			} else if ((endPoint === startPoint) && !touchedElement.closest('.ui_set').length) {
				$("#sidenav").removeClass("sidenav-active");
			}
		}
	}
}

function KeepWebDsbl(state) {
	$("#keepWeb").prop(disbl, state);
}

function EthEnbl(state) {
	state = !state;
	var dhcpEnabled = $("#ethDhcp").is(":checked");
	if (dhcpEnabled) {
		$("#ethDhcp").prop(disbl, state);
		$("#ethIp").prop(disbl, true);
		$("#ethMask").prop(disbl, true);
		$("#ethGate").prop(disbl, true);
		$("#ethDns1").prop(disbl, true);
		$("#ethDns2").prop(disbl, true);
	} else {
		$("#ethDhcp").prop(disbl, state);
		$("#ethIp").prop(disbl, state);
		$("#ethMask").prop(disbl, state);
		$("#ethGate").prop(disbl, state);
		$("#ethDns1").prop(disbl, state);
		$("#ethDns2").prop(disbl, state);
	}
}

function WifiEnbl(state) {
	$("#wifiEnbl").prop(chck, state);
	state = !state;
	var dhcpEnabled = $("#wifiDhcp").is(":checked");
	$("#wifiSsid").prop(disbl, state);
	$("#wifiPass").prop(disbl, state);
	if (dhcpEnabled) {
		$("#wifiDhcp").prop(disbl, state);
		$("#wifiIp").prop(disbl, true);
		$("#wifiMask").prop(disbl, true);
		$("#wifiGate").prop(disbl, true);
		$("#wifiDns1").prop(disbl, true);
		$("#wifiDns2").prop(disbl, true);
	} else {
		$("#wifiDhcp").prop(disbl, state);
		$("#wifiIp").prop(disbl, state);
		$("#wifiMask").prop(disbl, state);
		$("#wifiGate").prop(disbl, state);
		$("#wifiDns1").prop(disbl, state);
		$("#wifiDns2").prop(disbl, state);
	}
}

function WifiDhcpDsbl(state) {
	$("#wifiIp").prop(disbl, state);
	$("#wifiMask").prop(disbl, state);
	$("#wifiGate").prop(disbl, state);
	$("#wifiDns1").prop(disbl, state);
	$("#wifiDns2").prop(disbl, state);
}

function EthDhcpDsbl(state) {
	$("#ethIp").prop(disbl, state);
	$("#ethMask").prop(disbl, state);
	$("#ethGate").prop(disbl, state);
	$("#ethDns1").prop(disbl, state);
	$("#ethDns2").prop(disbl, state);
}

function MqttInputDsbl(state) {
	$("#MqttServer").prop(disbl, state);
	$("#MqttPort").prop(disbl, state);
	$("#MqttUser").prop(disbl, state);
	$("#MqttPass").prop(disbl, state);
	$("#MqttTopic").prop(disbl, state);
	$("#MqttInterval").prop(disbl, state);
	$("#MqttDiscovery").prop(disbl, state);
	$("#mqttReconnect").prop(disbl, state);
}

function WgInputDsbl(state) {
	$("#wgLocalIP").prop(disbl, state);
	$("#wgLocalSubnet").prop(disbl, state);
	$("#wgLocalPort").prop(disbl, state);
	$("#wgLocalGateway").prop(disbl, state);
	$("#wgLocalPrivKey").prop(disbl, state);
	$("#wgEndAddr").prop(disbl, state);
	$("#wgEndPubKey").prop(disbl, state);
	$("#wgEndPort").prop(disbl, state);
	$("#wgAllowedIP").prop(disbl, state);
	$("#wgAllowedMask").prop(disbl, state);
	$("#wgMakeDefault").prop(disbl, state);
	$("#wgPreSharedKey").prop(disbl, state);
}

function HnInputDsbl(state) {
	$("#hnJoinCode").prop(disbl, state);
	$("#hnHostName").prop(disbl, state);
	$("#hnDashUrl").prop(disbl, state);
}

function SeqInputDsbl(state) {
	$("#webUser").prop(disbl, state);
	$("#webPass").prop(disbl, state);
	$('#div_show1').toggle(this.checked);
}

function SeqInputDsblFw(state) {
	$("#fwIp").prop(disbl, state);
	$('#div_show2').toggle(this.checked);
}

function readFile(event, file) {
	event.preventDefault();
	$("#config_file").val("Loading file: " + file);
	$.get(apiLink + api.actions.API_GET_FILE + "&filename=" + file, function (data) {
		$("#title").text(file);
		$("#filename").val(file);
		$("#config_file").val(data);
	});
}

function delFile(event, file) {
	event.preventDefault();
	$("#config_file").val("Deleted file: " + file);
	$.get(apiLink + api.actions.API_DEL_FILE + "&filename=" + file, function (data) { });
}

function logRefresh(ms) {
	var logUpd = setInterval(() => {
		$.get(apiLink + api.actions.API_GET_LOG, function (data) {
			if ($("#console").length) {
				$("#console").val(data);
			} else {
				clearInterval(logUpd);
			}
		});
	}, ms);
}

async function fetchData(url, isJson = true) {
	if (isJson) {
		return await $.getJSON(url);
	} else {
		return await $.get(url);
	}
}

async function processResponses() {
	try {
		let jsonUrl = 'https://api.github.com/repos/xyzroe/XZG/releases/latest';
		let textUrl = '/api?action=1&param=espVer';

		let [jsonData, textData] = await Promise.all([
			fetchData(jsonUrl, true),
			fetchData(textUrl, false)
		]);

		return { jsonData, textData };
	} catch (error) {
		console.error('Error while getting versions:', error);
	}
}

function compareDates(dateStr1, dateStr2) {

	dateStr1 = String(dateStr1);
	dateStr2 = String(dateStr2);
	// Преобразуем строки в объекты Date
	const date1 = new Date(dateStr1.substr(0, 4), // Год
		parseInt(dateStr1.substr(4, 2)) - 1, // Месяц (начиная с 0)
		dateStr1.substr(6, 2)); // День
	const date2 = new Date(dateStr2.substr(0, 4),
		parseInt(dateStr2.substr(4, 2)) - 1,
		dateStr2.substr(6, 2));

	// Выполняем сравнение дат
	if (date1 < date2) {
		return -1; // date1 меньше date2
	} else if (date1 > date2) {
		return 1; // date1 больше date2
	} else {
		return 0; // даты равны
	}
}

function checkLatestESPrelease() {


	processResponses().then(combinedData => {

		var gitVer = Number(combinedData.jsonData.tag_name);
		var localVer = Number(combinedData.textData);

		//console.log(gitVer);
		//console.log(localVer);

		var asset = combinedData.jsonData.assets[0];
		var downloadCount = 0;
		for (var i = 0; i < combinedData.jsonData.assets.length; i++) {
			downloadCount += combinedData.jsonData.assets[i].download_count;
		}
		//var localVer = Number(0);

		var releaseInfo = i18next.t('ts.esp.upd.msg', { ver: combinedData.jsonData.tag_name, count: downloadCount.toLocaleString() });

		if (compareDates(gitVer, localVer) == 1) {

			setTimeout(function () {

				if (!(localStorage.getItem('update_notify') == 1)) {
					//modalConstructor("espGitUpdate", releaseInfo);
					toastConstructor("espUpdAvail", releaseInfo);

				}
				console.log(releaseInfo)
			}, 500);
		}
		else if (compareDates(gitVer, localVer) == -1) {
			if (!(localStorage.getItem('beta_feedback') == 1)) {
				//modalConstructor("espBetaFeedback");
				toastConstructor("espBetaFb");
			}
			console.log("betaInfo")
		}
	});

}

// CSS class name for dark theme
const darkTheme = "dark-theme";

// Add dark theme change here
const darkThemeSetUp = () => {
	if (getCurrentTheme() === "dark") {
		document.getElementById("toggleBtn").checked = true;
	} else {
		document.getElementById("toggleBtn").checked = false;
	}
};

const getCurrentTheme = () =>
	document.body.classList.contains(darkTheme) ? "dark" : "light";

//   Get user's theme preference from local storage
const selectedTheme = localStorage.getItem("selected-theme");
if (selectedTheme === "dark") {
	document.body.classList[selectedTheme === "dark" ? "add" : "remove"](
		darkTheme
	);
	darkThemeSetUp();
}

const themeButton = document.getElementById("toggleBtn");
themeButton.addEventListener("change", () => {
	document.body.classList.toggle(darkTheme);
	localStorage.setItem("selected-theme", getCurrentTheme());
	darkThemeSetUp();
});


let languages = [
	{ value: "en", text: "🇬🇧 English" },
	{ value: "uk", text: "🇺🇦 Українська" },
	{ value: "zh", text: "🇨🇳 中文" },
	{ value: "es", text: "🇪🇸 Español" },
	{ value: "pt", text: "🇵🇹 Português" },
	{ value: "ru", text: "🇷🇺 Русский" },
	{ value: "fr", text: "🇫🇷 Français" },
	{ value: "de", text: "🇩🇪 Deutsch" },
	{ value: "ja", text: "🇯🇵 日本語" },
	{ value: "tr", text: "🇹🇷 Türkçe" },
	{ value: "it", text: "🇮🇹 Italiano" },
	{ value: "pl", text: "🇵🇱 Polski" }
];

$(document).ready(() => {
	const $dropdown = $("#langSel");
	$dropdown.empty();

	languages.forEach(({ text, value }) => $dropdown.append(new Option(text, value)));

	const savedLang = localStorage.getItem("selected-lang");
	const browserLang = navigator.language ? navigator.language.substring(0, 2) : navigator.userLanguage;
	let preferredLang = savedLang || (languages.some(lang => lang.value === browserLang) ? browserLang : 'en'); // 'en' как fallback

	$dropdown.val(preferredLang);
	//changeLanguage(preferredLang);
	//console.log("Selected language set to:", preferredLang);


	$dropdown.on("change", function () {
		let selectedLang = $(this).val();
		localStorage.setItem("selected-lang", selectedLang);
		changeLanguage(selectedLang);
		//console.log("Language changed to:", selectedLang);
	});
});


function localize() {
	//console.log("localize");
	const elements = document.querySelectorAll('[data-i18n]');
	elements.forEach(element => {
		const keys = element.getAttribute('data-i18n').split(';');
		keys.forEach(key => {
			if (key.trim()) {
				if (key.includes('[')) {
					const parts = key.split('[');
					const attr = parts[1].slice(0, -1);
					element.setAttribute(attr, i18next.t(parts[0]));
				} else {
					const inputChild = element.querySelector('input');
					if (inputChild) {
						while (element.firstChild !== inputChild) {
							element.removeChild(element.firstChild);
						}
						const textNode = document.createTextNode(i18next.t(key));
						element.insertBefore(textNode, inputChild);
					} else {
						element.textContent = i18next.t(key);
					}
				}
			}
		});
	});
}


i18next
	.use(i18nextHttpBackend)
	.init({
		lng: 'en',
		backend: {
			loadPath: '/lg/{{lng}}.json',
		},
	}, function (err, t) {

	});

function updateLocalizedContent() {
	//console.log("update content");
	localizeTitle(window.location.pathname);
	localize();
}


function changeLanguage(lng) {
	i18next.changeLanguage(lng, () => {
		updateLocalizedContent();
	});
}

function getURLParameter(sParam) {
	var sPageURL = window.location.search.substring(1);
	var sURLVariables = sPageURL.split('&');
	for (var i = 0; i < sURLVariables.length; i++) {
		var sParameterName = sURLVariables[i].split('=');
		if (sParameterName[0] == sParam) {
			return sParameterName[1];
		}
	}
}

function sub_esp(t) {
	t = t.value.split("\\\\");
	"" != t ? ($("#updButton").removeAttr("disabled"), localStorage.setItem("beta_feedback", 0)) : $("#updButton").prop(disbl, 1), document.getElementById("file-input").innerHTML = "   " + t[t.length - 1]
}

function sub_zb(t) {
	t = t.value.split("\\\\");
	"" != t ? $("#updButton_zb").removeAttr("disabled") : $("#updButton_zb").prop(disbl, 1), document.getElementById("file-input_zb").innerHTML = "   " + t[t.length - 1]
}

async function fetchReleaseData() {
	var t = await fetch("https://api.github.com/repos/xyzroe/xzg/releases");
	if (t.ok) return await t.json();
	throw new Error("GitHub API request failed: " + t.statusText)
}

function handleClicks() {

	$("a.nav-link").click(function (e) { //handle navigation
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

	$('#logo').click(function () {
		$('#sidenav').toggleClass('sidenav-active');
	});

	$('#pageContent').click(function () {
		$('#sidenav').removeClass('sidenav-active');
	});

	$('#sidenav').click(function (e) {
		if (!$(e.target).closest('.ui_set').length) {
			$('#sidenav').removeClass('sidenav-active');
		}
	});

	$(document).on('submit', '#esp_upload_form', function (e) {
		e.preventDefault();
		var formData = new FormData(this);
		modalConstructor("flashESP", formData);
	});

	$(document).on('click', '#upd_esp_git', function () {
		console.log("Update from Git started... Just be patient!");
		localStorage.setItem("update_notify", 0);
		modalConstructor("flashESP");
	});

	$(document).on('click', '#info_esp_git', function () {
		modalConstructor("fetchGitReleases");
	});

	$(document).on('click', '#upd_zb_git', function () {
		modalConstructor("flashZB");
	});

	$(document).on('submit', '#upload_form_zb', function (e) {
		e.preventDefault();
		var formData = new FormData(this);
		/*ZBfwStartEvents(), $.ajax({
			url: "/updateZB",
			type: "POST",
			data: formData,
			contentType: false,
			processData: false,
			xhr: function () {
				var xhr = new window.XMLHttpRequest();
				xhr.upload.addEventListener("progress", function (event) {
					if (event.lengthComputable) {
						var percentComplete = event.loaded / event.total;
						$("#prg_zb").html("upload: " + Math.round(100 * percentComplete) + "%");
						$("#bar_zb").css("width", Math.round(100 * percentComplete) + "%");
					}
				}, false);
				return xhr;
			},
			success: function (data, textStatus) {
				console.log("success!"), $("#prg_zb").html("Upload completed! <br>Start validating...");
				$("#bar_zb").css("width", "0%");
			},
			error: function (xhr, textStatus, errorThrown) {
				console.log("Error:", errorThrown);
			}
		});*/
	});

	var lastEscTime = 0;
	var doublePressInterval = 300;

	$(document).on('keydown', function (e) {
		if (e.keyCode === 27) { // 27 - Esc
			var currentTime = new Date().getTime();
			if (currentTime - lastEscTime < doublePressInterval) {
				console.log("Double ESC press detected.");
				closeModal();
				lastEscTime = 0;
			} else {
				lastEscTime = currentTime;
			}
		}
	});

	const clockButton = document.getElementById('clock');

	clockButton.addEventListener('click', function () {
		const currentFormat = localStorage.getItem('clock_format_12h');
		const is12HourFormat = currentFormat === 'true';
		localStorage.setItem('clock_format_12h', !is12HourFormat);
		console.log('Clock format set to:', !is12HourFormat ? '12-hour' : '24-hour');
	});
}

function handleMsg() {
	if (getURLParameter("msg")) {
		let msg_txt = "";
		msg_id = parseInt(decodeURI(getURLParameter("msg")));
		switch (msg_id) {
			case 1:
				msg_txt = "p.lo.mnl";
				break;
			case 2:
				msg_txt = "p.lo.mwc";

				break;
			case 3:
				msg_txt = "p.lo.mlo";
				break;
		}
		document.getElementById("messageTxt").setAttribute("data-i18n", msg_txt);
	}
}