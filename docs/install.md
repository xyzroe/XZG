---
title: XZG - Web Flasher
description: Web Installer for XZG Firmware
---

<style>
  .md-content__button {
    display: none;
  }
  .md-main__inner {
    width: 65%;
  }
  .pick-variant select {
    background: transparent;
    width: 300px;
    padding: 1px;
    font-size: 16pt;
    border: 1px solid #ddd;
    height: 51px;
    border-radius: 15px;
  }
  .md-sidebar--primary {
      display: none;
  }
  .md-content {
      margin-left: 0 !important; 
  }
</style>

# XZG - Web Flasher

Flash or Find your device using next options:

<ol>
  <li>Plug in your gateway to a USB port.</li>
  <li id="coms">Hit "Install" and select the correct COM port. <a onclick="showSerialHelp()">No device found?</a></li>
  <li>Get XZG firmware installed and connected in less than 3 minutes!</li>
</ol>

<div class="pick-variant">
  <select id="firmwareVersion" onchange="updateManifestUrl()">
    
  </select>
</div>

<script type="module" src="https://unpkg.com/esp-web-tools@9/dist/web/install-button.js?module"></script>

<esp-web-install-button manifest="" class="button-connect">
  <button slot="activate" class="md-button md-button--primary">INSTALL</button>
  <span slot="unsupported">Use Chrome Desktop</span>
  <span slot="not-allowed">Not allowed to use this on HTTP!</span>
</esp-web-install-button>
<a href="http://xzg.local/"><button class="md-button">OPEN DEVICE</button></a>

<br>Powered by <a href="https://esphome.github.io/esp-web-tools/" target="_blank">ESP Web Tools</a><br>

<script>	

function loadFirmwareVersions() {
  fetch('https://api.github.com/repos/xyzroe/XZG/releases')
    .then(response => response.json())
    .then(data => {
      const select = document.getElementById('firmwareVersion');
      select.innerHTML = '';
      data.forEach(release => {
        const option = document.createElement('option');
        option.value = `https://raw.githubusercontent.com/xyzroe/XZG/releases/${release.name}/manifest.json`;
        option.textContent = release.name;
        select.appendChild(option);
      });
      updateManifestUrl();
    })
    .catch(error => console.error('Error fetching releases:', error));
}

function updateManifestUrl() {
  var selectedManifest = document.getElementById('firmwareVersion').value;
  var installButton = document.querySelector('esp-web-install-button');
  installButton.setAttribute('manifest', selectedManifest);
}

function showSerialHelp() {
  document.getElementById('coms').innerHTML = `Hit "Install" and select the correct COM port.<br><br>
  You might be missing the drivers for your board.<br>
  Here are drivers for one of the most popular chip:
  <a href="https://sparks.gogo.co.nz/ch340.html" target="_blank">CH340C</a><br><br>
  Make sure your USB cable supports data transfer.<br><br>
  `;
}

loadFirmwareVersions();
</script>