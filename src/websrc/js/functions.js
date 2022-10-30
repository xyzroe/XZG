const disbl = "disabled";

$(document).ready(function () { //handle active nav
	$("a[href='" + document.location.pathname + "']").parent().addClass('nav-active'); //handle sidenav page selection
	if (!$("#usbMode").prop("checked")) { //
		$("#lanMode").prop("checked", true);
	}

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

	if(!(localStorage.getItem('refresh_tip_got') == 1)){//toast localStorage.setItem('refresh_tip_got', 1)
		$('.toast').toast('show');
	}
});

function EthInputDsbl(state) {
	$("#EthIp").attr(disbl, state);
	$("#EthMask").attr(disbl, state);
	$("#EthGateway").attr(disbl, state);
}

function SeqInputDsbl(state) {
	$("#webUser").attr(disbl, state);
	$("#webPass").attr(disbl, state);
}


function getXhr(){
	var xhr = null;
	if(window.XMLHttpRequest) // Firefox et autres
	   xhr = new XMLHttpRequest();
	else if(window.ActiveXObject){ // Internet Explorer
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

function power(mac,cmd)
{
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 ){
			leselect = xhr.responseText;
			document.getElementById(mac).innerHTML=leselect;
		}
	}
	xhr.open("GET","SetPower?mac="+escape(mac)+"&cmd="+escape(cmd),true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();

}
function GetGSMStatus()
{
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 ){
			leselect = xhr.responseText;
			document.getElementById("gsmstatus").innerHTML=leselect;
		}
	}
	xhr.open("GET","GetGSMStatus",true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();

}
function GetThermostatStatus()
{
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 ){
			leselect = xhr.responseText;
			document.getElementById("thermostat").innerHTML=leselect;
		}
	}
	xhr.open("GET","GetThermostatStatus",true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();
}

function GetAction(mac)
{
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 ){
			leselect = xhr.responseText;
			document.getElementById(mac).innerHTML=leselect;
			setTimeout(function(){ GetAction(mac); }, 3000);
		}
	}
	xhr.open("GET","GetAction?mac="+escape(mac),true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();
}

function readfile(file)
{
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 ){
			leselect = xhr.responseText;
			document.getElementById("title").innerHTML=file;
			document.getElementById("filename").value=file;
			document.getElementById("file").innerHTML=leselect;
		}
	}
	xhr.open("GET","readFile?file="+escape(file),true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();
}

function logRefresh(ms)
{
	var xhr = getXhr();
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 ){
			leselect = xhr.responseText;
			document.getElementById("console").value=leselect;
			setTimeout(function(){ logRefresh(); }, ms);
		}
	}
	xhr.open("GET","getLogBuffer",true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();
}

function scanNetwork()
{
	var xhr = getXhr();
	document.getElementById("networks").innerHTML="<img src='/img/wait.gif'>";
	xhr.onreadystatechange = function(){
		if(xhr.readyState == 4 ){
			leselect = xhr.responseText;
			document.getElementById("networks").innerHTML=leselect;
		}
	}
	xhr.open("GET","scanNetwork",true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();
}

function updateSSID(val)
{
	document.getElementById("ssid").value=val;
}

function cmd(val)
{

	var xhr = getXhr();
	xhr.open("GET","cmd"+val,true);
	xhr.setRequestHeader('Content-Type','application/html');
	xhr.send();
}

