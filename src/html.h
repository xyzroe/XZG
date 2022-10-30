const char HTTP_HEADER[] PROGMEM = R"=====(

<head>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <script type='text/javascript' src='/js/jquery-min.js'></script>
  <script type='text/javascript' src='/js/bootstrap.min.js'></script>
  <script type='text/javascript' src='/js/functions.js'></script>
  <script>function logoutButton() {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/logout', true);
      xhr.send();
      setTimeout(function () { window.open('/logged-out', '_self'); }, 500);
    }</script>
  <link href='css/required.css' rel='stylesheet' type='text/css' />
  <title>{{pageName}} - SLZB-06 Zigbee Ethernet POE USB Adapter</title>
</head>

<body>
<div class="toast position-fixed bottom-0 end-0 fade toast_show hide"
" data-bs-autohide="false">
    <div class="toast-header">
      <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" class="bi bi-info-circle" viewBox="0 0 16 16" reserveAspectRatio="xMidYMid slice" focusable="false" role="img">
  <path d="M8 15A7 7 0 1 1 8 1a7 7 0 0 1 0 14zm0 1A8 8 0 1 0 8 0a8 8 0 0 0 0 16z"/>
  <path d="m8.93 6.588-2.29.287-.082.38.45.083c.294.07.352.176.288.469l-.738 3.468c-.194.897.105 1.319.808 1.319.545 0 1.178-.252 1.465-.598l.088-.416c-.2.176-.492.246-.686.246-.275 0-.375-.193-.304-.533L8.93 6.588zM9 4.5a1 1 0 1 1-2 0 1 1 0 0 1 2 0z"/>
</svg>
        <strong class="mr-auto" style="padding-left: 10px;">Tip</strong>
    </div>
    <div class="toast-body">
    Statuses and other information in this window are updated when the page refreshes.
    <div class="mt-2 pt-2 border-top">
      <button type="button" class="btn btn-outline-primary" data-bs-dismiss="toast" onclick="localStorage.setItem('refresh_tip_got', 1)">Got it!</button>
    </div>
    </div>
</div>
<div class="modal fade" id="modal" data-bs-backdrop="static" data-bs-keyboard="false" tabindex="100" aria-modal="true" role="dialog">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <h5 class="modal-title" id="modalTitle">Modal title</h5>
        <!-- <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button> -->
      </div>
      <div class="modal-body" id="modalBody">
        Flashing wrong 
      </div>
      <div class="modal-footer" id="modalButtons">
        
      </div>
    </div>
  </div>
</div>
  <div class='container-fluid'>
    <div class='row flex-nowrap'>
      <div class='col-auto nav-container px-0 col-md-3 col-xl-2 bg-dark nav-shadow'>
        <div class='d-flex flex-column align-items-center align-items-sm-start pt-2 text-white min-vh-100'>
          <div class='logo-wrapper'>
            <img src='./img/logo.png'>
            <span class='fs-5 d-none d-sm-inline'>SLZB-06</span>
            </a>
          </div>
          <ul class='nav nav-pills flex-column mb-sm-auto mb-0 align-items-center align-items-sm-start' id='menu'>
            <li class='nav-item'>
              <a href='/' class='nav-link align-middle' onclick=''>
                <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor' class='bi bi-house'
                  viewBox='0 0 16 16'>
                  <path fill-rule='evenodd'
                    d='M2 13.5V7h1v6.5a.5.5 0 0 0 .5.5h9a.5.5 0 0 0 .5-.5V7h1v6.5a1.5 1.5 0 0 1-1.5 1.5h-9A1.5 1.5 0 0 1 2 13.5zm11-11V6l-2-2V2.5a.5.5 0 0 1 .5-.5h1a.5.5 0 0 1 .5.5z' />
                  <path fill-rule='evenodd'
                    d='M7.293 1.5a1 1 0 0 1 1.414 0l6.647 6.646a.5.5 0 0 1-.708.708L8 2.207 1.354 8.854a.5.5 0 1 1-.708-.708L7.293 1.5z' />
                </svg>
                <span class='ms-1 d-none d-sm-inline'> Status</span>
              </a>
            </li>
            <li class='nav-item'>
              <a href='/general' class='nav-link align-middle'>
                <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor'
                  class='bi bi-hdd-stack' viewBox='0 0 16 16'>
                  <path
                    d='M14 10a1 1 0 0 1 1 1v1a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1v-1a1 1 0 0 1 1-1h12zM2 9a2 2 0 0 0-2 2v1a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2v-1a2 2 0 0 0-2-2H2z' />
                  <path
                    d='M5 11.5a.5.5 0 1 1-1 0 .5.5 0 0 1 1 0zm-2 0a.5.5 0 1 1-1 0 .5.5 0 0 1 1 0zM14 3a1 1 0 0 1 1 1v1a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1V4a1 1 0 0 1 1-1h12zM2 2a2 2 0 0 0-2 2v1a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V4a2 2 0 0 0-2-2H2z' />
                  <path d='M5 4.5a.5.5 0 1 1-1 0 .5.5 0 0 1 1 0zm-2 0a.5.5 0 1 1-1 0 .5.5 0 0 1 1 0z' />
                </svg>
                <span class='ms-1 d-none d-sm-inline'>General</span>
              </a>
            </li>
            <li class='nav-item'>
              <a href='/ethernet' class='nav-link align-middle'>
                <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor'
                  class='bi bi-ethernet' viewBox='0 0 16 16'>
                  <path
                    d='M14 13.5v-7a.5.5 0 0 0-.5-.5H12V4.5a.5.5 0 0 0-.5-.5h-1v-.5A.5.5 0 0 0 10 3H6a.5.5 0 0 0-.5.5V4h-1a.5.5 0 0 0-.5.5V6H2.5a.5.5 0 0 0-.5.5v7a.5.5 0 0 0 .5.5h11a.5.5 0 0 0 .5-.5ZM3.75 11h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5a.25.25 0 0 1 .25-.25Zm2 0h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5a.25.25 0 0 1 .25-.25Zm1.75.25a.25.25 0 0 1 .25-.25h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5ZM9.75 11h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5a.25.25 0 0 1 .25-.25Zm1.75.25a.25.25 0 0 1 .25-.25h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5Z' />
                  <path
                    d='M2 0a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V2a2 2 0 0 0-2-2H2ZM1 2a1 1 0 0 1 1-1h12a1 1 0 0 1 1 1v12a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1V2Z' />
                </svg>
                <span class='ms-1 d-none d-sm-inline'>Ethernet</span>
              </a>
            </li>
            <li class='nav-item'>
              <a href='/serial' class='nav-link align-middle'>
                <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor'
                  class='bi bi-activity' viewBox='0 0 16 16'>
                  <path fill-rule='evenodd'
                    d='M6 2a.5.5 0 0 1 .47.33L10 12.036l1.53-4.208A.5.5 0 0 1 12 7.5h3.5a.5.5 0 0 1 0 1h-3.15l-1.88 5.17a.5.5 0 0 1-.94 0L6 3.964 4.47 8.171A.5.5 0 0 1 4 8.5H.5a.5.5 0 0 1 0-1h3.15l1.88-5.17A.5.5 0 0 1 6 2Z' />
                </svg>
                <span class='ms-1 d-none d-sm-inline'>Serial</span>
              </a>
            </li>
            <li class='nav-item'>
              <a href='/security' class='nav-link align-middle'>
                <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor' class='bi bi-lock'
                  viewBox='0 0 16 16'>
                  <path
                    d='M8 1a2 2 0 0 1 2 2v4H6V3a2 2 0 0 1 2-2zm3 6V3a3 3 0 0 0-6 0v4a2 2 0 0 0-2 2v5a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2zM5 8h6a1 1 0 0 1 1 1v5a1 1 0 0 1-1 1H5a1 1 0 0 1-1-1V9a1 1 0 0 1 1-1z' />
                </svg>
                <span class='ms-1 d-none d-sm-inline'>Security</span>
              </a>
            </li>
            <li class='nav-item'>
              <a href='/sys-tools' class='nav-link align-middle'>
                <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor' class='bi bi-cpu'
                  viewBox='0 0 16 16'>
                  <path
                    d='M5 0a.5.5 0 0 1 .5.5V2h1V.5a.5.5 0 0 1 1 0V2h1V.5a.5.5 0 0 1 1 0V2h1V.5a.5.5 0 0 1 1 0V2A2.5 2.5 0 0 1 14 4.5h1.5a.5.5 0 0 1 0 1H14v1h1.5a.5.5 0 0 1 0 1H14v1h1.5a.5.5 0 0 1 0 1H14v1h1.5a.5.5 0 0 1 0 1H14a2.5 2.5 0 0 1-2.5 2.5v1.5a.5.5 0 0 1-1 0V14h-1v1.5a.5.5 0 0 1-1 0V14h-1v1.5a.5.5 0 0 1-1 0V14h-1v1.5a.5.5 0 0 1-1 0V14A2.5 2.5 0 0 1 2 11.5H.5a.5.5 0 0 1 0-1H2v-1H.5a.5.5 0 0 1 0-1H2v-1H.5a.5.5 0 0 1 0-1H2v-1H.5a.5.5 0 0 1 0-1H2A2.5 2.5 0 0 1 4.5 2V.5A.5.5 0 0 1 5 0zm-.5 3A1.5 1.5 0 0 0 3 4.5v7A1.5 1.5 0 0 0 4.5 13h7a1.5 1.5 0 0 0 1.5-1.5v-7A1.5 1.5 0 0 0 11.5 3h-7zM5 6.5A1.5 1.5 0 0 1 6.5 5h3A1.5 1.5 0 0 1 11 6.5v3A1.5 1.5 0 0 1 9.5 11h-3A1.5 1.5 0 0 1 5 9.5v-3zM6.5 6a.5.5 0 0 0-.5.5v3a.5.5 0 0 0 .5.5h3a.5.5 0 0 0 .5-.5v-3a.5.5 0 0 0-.5-.5h-3z' />
                </svg>
                <span class='ms-1 d-none d-sm-inline'>System and Tools</span>
              </a>
            </li>
            <li class='nav-item'>
              <a href='/logs-browser' class='nav-link align-middle'>
                <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor' class='bi bi-folder'
                  viewBox='0 0 16 16'>
                  <path
                    d='M.54 3.87.5 3a2 2 0 0 1 2-2h3.672a2 2 0 0 1 1.414.586l.828.828A2 2 0 0 0 9.828 3h3.982a2 2 0 0 1 1.992 2.181l-.637 7A2 2 0 0 1 13.174 14H2.826a2 2 0 0 1-1.991-1.819l-.637-7a1.99 1.99 0 0 1 .342-1.31zM2.19 4a1 1 0 0 0-.996 1.09l.637 7a1 1 0 0 0 .995.91h10.348a1 1 0 0 0 .995-.91l.637-7A1 1 0 0 0 13.81 4H2.19zm4.69-1.707A1 1 0 0 0 6.172 2H2.5a1 1 0 0 0-1 .981l.006.139C1.72 3.042 1.95 3 2.19 3h5.396l-.707-.707z' />
                </svg>
                <span class='ms-1 d-none d-sm-inline'>Console and Browser</span>
              </a>
            </li>
            <li class='nav-item'>
              <a href='/about' class='nav-link align-middle'>
                <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor'
                  class='bi bi-info-square' viewBox='0 0 16 16'>
                  <path
                    d='M14 1a1 1 0 0 1 1 1v12a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1V2a1 1 0 0 1 1-1h12zM2 0a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V2a2 2 0 0 0-2-2H2z' />
                  <path
                    d='m8.93 6.588-2.29.287-.082.38.45.083c.294.07.352.176.288.469l-.738 3.468c-.194.897.105 1.319.808 1.319.545 0 1.178-.252 1.465-.598l.088-.416c-.2.176-.492.246-.686.246-.275 0-.375-.193-.304-.533L8.93 6.588zM9 4.5a1 1 0 1 1-2 0 1 1 0 0 1 2 0z' />
                </svg>
                <span class='ms-1 d-none d-sm-inline'>About</span>
              </a>
            </li>
          </ul>
          <hr>
          {{logoutLink}}
        </div>
      </div>
      )=====";

const char LOGOUT_LINK[] PROGMEM =
    "<li class='nav-item'>"
    "<a class='nav-link' href='javascript:logoutButton();'><i class='glyphicon glyphicon glyphicon-log-out'></i>Logout</a>"
    "</li>";

const char HTTP_ROOT[] PROGMEM = R"=====(
<div class='col py-3'>
  <h2>{{pageName}}</h2>
  <div id='main2' class='row'>
  <!-- <span>Statuses and other information in this window are updated when the page refreshes</span> -->
    <div class='col-sm-12 col-md-6 mb-4'>
      <div class='card'>
        <div class='card-header'>Device status</div>
        <div class='card-body'>
          <table class="table">
            <tbody>
              <tr>
                <td>Operational mode:</td>
                <td>{{operationalMode}}</td>
              </tr>
              <tr>
                <td>Ethernet connected:</td>
                <td>{{connectedEther}}</td>
              </tr>
              <tr>
                <td>Socket client connected:</td>
                <td>{{connectedSocketStatus}}</td>
              </tr>
              <!-- <tr>
                <td>WiFi Client enabled:</td>
                <td>{{wifiEnabled}}</td>
              </tr>
              <tr>
                <td>WiFi Client status:</td>
                <td>{{wifiConnected}}</td>
              </tr> -->
              <tr>
                <td>WiFi Access Point enabled:</td>
                <td>{{wifiModeAP}}</td>
              </tr>
              <tr>
                <td>WiFi Access Point status:</td>
                <td>{{wifiModeAPStatus}}</td>
              </tr>
              <tr>
                <td>Device Uptime:</td>
                <td>{{uptime}}</td>
              </tr>
              <tr>
                <td>Socket uptime:</td>
                <td>{{connectedSocket}}</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>

    <div class='col-sm-12 col-md-6 mb-4'>
      <div class='card'>
        <div class='card-header'>Device information</div>
        <div class='card-body'>
          <table class="table">
            <tbody>
              <tr>
                <td>Model:</td>
                <td>{{hwRev}}</td>
              </tr>
              <tr>
                <td>ESP32 Firmware ver:</td>
                <td>{{VERSION}}</td>
              </tr>
              <tr>
                <td>ESP32 version:</td>
                <td>{{espModel}}</td>
              </tr>
              <tr>
                <td>ESP32 temperature:</td>
                <td>{{deviceTemp}} &deg;C</td>
              </tr>
              <tr>
                <td>ESP32 frequency:</td>
                <td>{{espCores}} cores @ {{espFreq}} MHz</td>
              </tr>
              <tr>
                <td>ESP32 flash size:</td>
                <td>{{espFlashSize}} Mb, {{espFlashType}}</td>
              </tr>
              <tr>
                <td>ESP32 Free heap:</td>
                <td>{{espHeapFree}} / {{espHeapSize}} KiB</td>
              </tr>
              <tr>
                <td>CC2652P version:</td>
                <td>CC2652PP1FRGZR</td>
              </tr>
              <tr>
                <td>CC2652P frequency:</td>
                <td>48 MHz</td>
              </tr>
              <tr>
                <td>CC2652P flash size:</td>
                <td>352 Kb</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>

    <div class='col-sm-12 col-md-6 mb-4'>
      <div class='card'>
        <div class='card-header'>Ethernet</div>
        <div class='card-body'>
          <table class="table">
            <tbody>
              <tr>
                <td>Connection status:</td>
                <td>{{ethConnection}}</td>
              </tr>
              <tr>
                <td>DHCP:</td>
                <td>{{ethDhcp}}</td>
              </tr>
              <tr>
                <td>IP Address:</td>
                <td>{{ethIp}}</td>
              </tr>
              <tr>
                <td>Subnet Mask:</td>
                <td>{{etchMask}}</td>
              </tr>
              <tr>
                <td>Default Gateway:</td>
                <td>{{ethGate}}</td>
              </tr>
              <tr>
                <td>Connection speed:</td>
                <td>{{ethSpd}}</td>
              </tr>
              <tr>
                <td>MAC address:</td>
                <td>{{ethMac}}</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>

    <!-- <div class='col-sm-12 col-md-6 mb-4'>
      <div class='card'>
        <div class='card-header'>Wifi</div>
        <div class='card-body'>
          <table class="table">
            <tbody>
              <tr>
                <td>Mode:</td>
                <td>{{wifiMode}}</td>
              </tr>
              <tr>
                <td>SSID:</td>
                <td>{{wifiSsid}}</td>
              </tr>
              <tr>
                <td>MAC Address:</td>
                <td>{{wifiMac}}</td>
              </tr>
              <tr>
                <td>IP Address:</td>
                <td>{{wifiIp}}</td>
              </tr>
              <tr>
                <td>Subnet Mask:</td>
                <td>{{wifiSubnet}}</td>
              </tr>
              <tr>
                <td>Default Gateway:</td>
                <td>{{wifiGate}}</td>
              </tr>
              <tr>
                <td>RSSI:</td>
                <td>{{wifiRssi}}</td>
              </tr>
              <tr>
                <td>DHCP:</td>
                <td>{{wifiDhcp}}</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div> -->
  </div>
</div>

</div>
</div>
     )=====";

const char HTTP_GENERAL[] PROGMEM = R"=====(
<div class='col py-3'>
    <h2>{{pageName}}</h2>
    <div class='row'>
        <div class="col-sm-12">
            <form method="POST" action="saveGeneral">
                <div class="card">
                    <div class="card-header">General settings</div>
                    <div class="card-body">
                        <div class="row">
                            <div class="col-sm-12 col-md-6 mb-3">
                                <div class="row">
                                    <div class="col-sm-12 mb-3">
                                        <span>Select the device operating mode - LAN or USB connection:</span>
                                        <div class="form-check">
                                            <input class="form-check-input" type="radio" name="usbMode" id="lanMode"
                                                value="off">
                                            <label class="form-check-label" for="lanMode">
                                                LAN mode
                                            </label>
                                        </div>
                                        <div class="form-check">
                                            <input class="form-check-input" type="radio" name="usbMode" id="usbMode"
                                                {{checkedUsbMode}} value="on">
                                            <label class="form-check-label" for="usbMode">
                                                USB mode
                                            </label>
                                        </div>
                                        <hr class="border border-dark border-top">
                                        <br>
                                        <span>Specify the hostname. The device will be displayed in the LAN network with
                                            this name:</span>
                                        <div class="col-sm-12 mb-3">
                                            <label for="hostname">Hostname</label>
                                            <input class="form-control" id="hostname" type="text" name="hostname"
                                                value="{{hostname}}">
                                            <label for='refreshLogs'>Refresh console log</label>
                                            <input class='form-control' id='refreshLogs' type="number" min="100"
                                                max="10000" step="100" name='refreshLogs' value='{{refreshLogs}}'
                                                placeholder="in milliseconds">
                                        </div>
                                    </div>
                                </div>
                            </div>
                            <div class="col-sm-12 col-md-6 mb-3">
                                <div class="row">
                                    <div class="col-sm-12">
                                        <span>Using the Toggle buttons, you can control the state of the LEDs in the current session (not saved on reboot):</span>
                                    </div>
                                    <div class="col-sm-12">
                                        <button type="button" onclick="cmd('LedYellowToggle');"
                                            class="btn btn-outline-primary col col-md-auto mb-1">Power LED (Yellow)
                                            Toggle</button>
                                        <button type="button" onclick="cmd('LedBlueToggle');"
                                            class="btn btn-outline-primary col col-md-auto mb-1">Mode LED (Blue)
                                            Toggle</button>
                                            <br>
                                        </div>
                                        <br>

                                    <div class="col-sm-12">
                                    <br>
                                        <span>Control the behavior of the LEDs (with saving on reboot) using the switches below:</span>
                                        <div class='form-check form-switch'>
                                            <input class='form-check-input' id='disableLedYellow' type='checkbox'
                                                role="switch" name='disableLedYellow' {{checkedDisableLedYellow}}>
                                            <label class='form-check-label' for='disableLedYellow'>Disable Power LED
                                                (yellow)</label>
                                        </div>
                                        <div class='form-check form-switch'>
                                            <input class='form-check-input' id='disableLedBlue' type='checkbox'
                                                role="switch" name='disableLedBlue' {{checkedDisableLedBlue}}>
                                            <label class='form-check-label' for='disableLedBlue'>Disable USB mode LED
                                                (blue)</label>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div class="col-sm-12">
                            <div class="row justify-content-md-center">
                                <button type="submit" class="btn btn-outline-primary col-sm-12 col-md-6"
                                    name="save">Save</button>
                            </div>
                        </div>
                    </div>
                </div>
        </div>
        </form>
    </div>
</div>

</div>
</div>
)=====";

const char HTTP_ETHERNET[] PROGMEM = R"=====(
    <div class='col py-3'>
    <h2>{{pageName}}</h2>
    <div class='row'>
        <form method='POST' action='saveEther'>
            <div class='col-sm-12 cardPadding'>
                <div class='card'>
                    <div class="card-header">Ethernet options</div>
                    <div class="card-body">
                        <div class='mb-2 form-check form-switch'>
                            <input id="EthDhcpTog" class='form-check-input' type='checkbox' role="switch" name='dhcp'
                                onclick="EthInputDsbl(this.checked)" {{modeEther}}>
                            <label class='form-check-label' for='dhcp'>DHCP</label>
                        </div>
                        <div class='mb-2'>
                            <label for='ip'>IP</label>
                            <input class='form-control' id='EthIp' type='text' name='ipAddress' value='{{ipEther}}'
                                placeholder="192.168.x.x">
                        </div>
                        <div class='mb-2'>
                            <label for='mask'>Mask</label>
                            <input class='form-control' id='EthMask' type='text' name='ipMask' value='{{maskEther}}'
                                placeholder="255.255.255.0">
                        </div>
                        <div class='mb-2'>
                            <label for='gateway'>Gateway</label>
                            <input type='text' class='form-control' id='EthGateway' name='ipGW' value='{{GWEther}}'
                                placeholder="192.168.x.x">
                        </div>
                        <div class='mb-2 form-check form-switch'>
                            <input class='form-check-input' id='disablePingCtrl' type='checkbox' role="switch"
                                name='disablePingCtrl' {{disablePingCtrl}}>
                            <label class='form-check-label' for='disablePingCtrl'><b>Disable</b> gateway ping
                                check</label>
                        </div>
                        <br>
                        <div class="row justify-content-md-center">
                        <button type='submit' class='btn btn-outline-primary col-sm-12 col-md-6' name='save'>Save</button>
                        </div>
                    </div>
                </div>
            </div>
        </form>
    </div>
</div>
     )=====";

const char HTTP_WIFI[] PROGMEM =
    "<h2>{{pageName}}</h2>"
    "<div id='main' class='col-sm-12'>"
    "<div id='main' class='col-sm-6'><form method='POST' action='saveWifi'>"
    "<div class='form-group'>"
    "<div class='form-check'>"
    "<input class='form-check-input' id='wifiEnable' type='checkbox' name='wifiEnable' {{checkedWiFi}}>"
    "<label class='form-check-label' for='wifiEnable'>Enable Wi-Fi client mode</label>"
    "</div>"
    "</div>"
    "<div class='form-group'>"
    "<label for='ssid'>SSID</label>"
    "<input class='form-control' id='ssid' type='text' name='WIFISSID' value='{{ssid}}'>"
    "</div>"
    "<div class='form-group'>"
    "<a onclick='scanNetwork();' class='btn btn-warning mb-2'>Scan</a>"
    "</div>"
    "<div class='form-group'>"
    "<div id='networks'></div>"
    "</div>"
    "<div class='form-group'>"
    "<label for='pass'>Password</label>"
    "<input class='form-control' id='pass' type='password' name='WIFIpassword' value='{{passWifi}}'>"
    "</div>"
    "<div class='form-group'>"
    "<div class='form-check'>"
    "<input class='form-check-input' id='dhcpWiFi' type='checkbox' name='dhcpWiFi' {{dchp}}>"
    "<label class='form-check-label' for='dhcpWiFi'>DHCP</label>"
    "</div>"
    "</div>"
    "<div class='form-group'>"
    "<label for='ip'>IP</label>"
    "<input class='form-control' id='ip' type='text' name='ipAddress' value='{{ip}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='mask'>Mask</label>"
    "<input class='form-control' id='mask' type='text' name='ipMask' value='{{mask}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='gateway'>Gateway</label>"
    "<input type='text' class='form-control' id='gateway' name='ipGW' value='{{gw}}'>"
    "</div>"
    "<div class='form-group'>"
    "<div class='form-check'>"
    "<input class='form-check-input' id='disableEmerg' type='checkbox' name='disableEmerg' {{checkedDisEmerg}}>"
    "<label class='form-check-label' for='disableEmerg'><b>Disable</b> emergency mode (AP)</label>"
    "</div>"
    "</div>"
    "<button type='submit' class='btn btn-primary mb-2' name='save'>Save</button>"
    "</form>"
    "</div>"
    "</div>";

const char HTTP_SERIAL[] PROGMEM = R"=====(
    <div class='col py-3'>
  <h2>{{pageName}}</h2>
  <div id='main'>
    <form method='POST' action='saveSerial'>
      <div class='card'>
        <div class='card-header'>Serial options</div>
        <div class='card-body'>
          <div class="row">
            <div class='col-sm-12 col-md-6 mb-4'>
              <div class='form-group'>
                <label for='baud'>Serial Speed</label>
                <select class='form-select' id='baud' name='baud'>
                  <option value='9600' {{selected9600}}>9600 bauds</option>
                  <option value='19200' {{selected19200}}>19200 bauds</option>
                  <option value='38400' {{selected38400}}>38400 bauds</option>
                  <option value='57600' {{selected57600}}>57600 bauds</option>
                  <option value='115200' {{selected115200}}>115200 bauds</option>
                </select>
              </div>
            </div>
            <div class='col-sm-12 col-md-6 mb-4'>
              <div class='form-group'>
                <label for='port'>Socket Port</label>
                <input class='form-control' id='port' type='number' name='port' min='100' max='65000'
                  value='{{socketPort}}'>
              </div>
            </div>
          </div>
          <div class='col-sm-12'>
            <div class="row justify-content-md-center">
              <button type='submit' class='btn btn-outline-primary col-sm-12 col-md-6' name='save'>Save</button>
            </div>
          </div>
        </div>
      </div>
    </form>
  </div>
</div>

</div>
</div>
    )=====";

const char HTTP_SECURITY[] PROGMEM = R"=====(
<div class='col py-3'>
  <h2>{{pageName}}</h2>
  <div id='main'>
    <form method='POST' action='saveSecurity'>
      <div class='card'>
        <div class='card-header'>Security options</div>
        <div class='card-body'>
          <div class="row">
            <div class="col-sm-12 col-md-6 mb-4">
              <div class='form-check'>
                <input class='form-check-input' id='disableWeb' type='checkbox' name='disableWeb' {{disableWeb}}>
                <label class='form-check-label' for='disableWeb'>Disable web server when socket is connected</label>
                <br>
              </div>
              <div class='form-check'>
                <input class='form-check-input' id='webAuth' type='checkbox' name='webAuth' {{webAuth}} onclick="SeqInputDsbl(!this.checked)">
                <label class='form-check-label' for='webAuth'>Enable web server authentication</label>
              </div>
            </div>
            <div class="col-sm-12 col-md-6 mb-2">
              <label for='webUser'>Username</label>
              <input class='form-control' id='webUser' type='text' name='webUser' value='{{webUser}}' disabled>
              <label for='webPass'>Password</label>
              <input class='form-control' id='webPass' type='password' name='webPass' value='{{webPass}}' disabled>
            </div>
            <div class="col-sm-12">
              <div class="row justify-content-md-center">
                <button type='submit' class='btn btn-outline-primary col-sm-12 col-md-6' name='save'>Save</button>
              </div>
            </div>
          </div>
        </div>
      </div>
    </form>
  </div>
</div>

</div>
</div>
)=====";

const char HTTP_SYSTOOLS[] PROGMEM = R"=====(
<div class='col py-3'>
  <h2>{{pageName}}</h2>
  <div id='main' class="row">
    <div class="col-sm-12 col-md-6 mb-4">
      <div class='card'>
        <div class='card-header'>ESP32 Update</div>
        <div class='card-body'>
          <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
            <input type='file' name='update' id='file' onchange='sub(this)' style=display:none accept='.bin'>
            <label id='file-input' for='file'> Choose file...</label>
            <input id="updButton" type='submit' class='btn btn-warning mb-2' value='ESP32 OTA Update' disabled>
            <br>
            <div id='prg'></div>
            <div id='prgbar'>
              <div id='bar'></div>
            </div>
          </form>
        </div>
      </div>
    </div>
    <div class="col-sm-12 col-md-6 mb-4">
      <div class='card'>
        <div class='card-header'>Modules control</div>
        <div class='card-body'>
          <div class="row">
            <button type='button' onclick="cmd('ZigRST');" class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>Zigbee Restart</button>
            <button type='button' onclick="cmd('ZigBSL');" class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>Zigbee Flash Mode</button>
            <button type='button' onclick="cmd('EspReboot');" class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>ESP32 Restart</button>
          </div>
        </div>
      </div>
    </div>
    <div class="col-sm-12 col-md-6 mb-4">
      <div class='card'>
        <div class='card-header'>Current session control</div>
        <div class='card-body'>
          <div class="row">
            <button type='button' onclick="cmd('AdapterModeLAN');" class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>LAN Mode ON</button>
            <button type='button' onclick="cmd('AdapterModeUSB');" class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>USB Mode ON</button>
            <button type='button' onclick="cmd('LedYellowToggle');" class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>Yellow LED
              Toggle</button>
            <button type="button" onclick="cmd('LedBlueToggle');" class="btn btn-outline-primary col-sm-12 col-md-auto mb-1">Blue LED Toggle</button>
          </div>
        </div>
      </div>
    </div>
  </div>
</div>

</div>
</div>
<script>
  function sub(obj) {
    let fileName = obj.value.split('\\\\');
    if (fileName != "") {
      if(!(localStorage.getItem('flash_warning') == 1)){
      $(".modal").modal("show");
      }else{
        $("#updButton").removeAttr("disabled");
      }
    }
    document.getElementById('file-input').innerHTML = '   ' + fileName[fileName.length - 1];
  };
  $('form').submit(function (e) {
    e.preventDefault();
    var form = $('#upload_form')[0];
    var data = new FormData(form);
    $.ajax({
      url: '/update',
      type: 'POST',
      data: data,
      contentType: false,
      processData: false,
      xhr: function () {
        var xhr = new window.XMLHttpRequest();
        xhr.upload.addEventListener('progress', function (evt) {
          if (evt.lengthComputable) {
            var per = evt.loaded / evt.total;
            $('#prg').html('progress: ' + Math.round(per * 100) + '%');
            $('#bar').css('width', Math.round(per * 100) + '%');
          }
        }, false);
        return xhr;
      },
      success: function (d, s) {
        console.log('success!');
        $('#prg').html('Update completed!<br>Rebooting!');
        window.location.href = '/';
      },
      error: function (a, b, c) {
      }
    });
  });
  $(document).ready(function () { 
    if(!(localStorage.getItem('flash_warning') == 1)){//toast localStorage.setItem('refresh_tip_got', 1)
    $("#modalTitle").text("WARNING").css("color", "red");
    $("#modalBody").text("Flashing unofficial, incorrect or corrupted firmware can damage or brick your device!!!").css("color", "red");
    $("#modalButtons").append('<button type="button" class="btn btn-success" data-bs-dismiss="modal">Close</button>');
    $("#modalButtons").append('<button type="button" class="btn btn-danger" data-bs-dismiss="modal" onclick="localStorage.setItem(\'flash_warning\', 1);$(\'#updButton\').removeAttr(\'disabled\');">I agree and I am taking the risk');
	}
});
</script>
    )=====";

const char LOGS_BROWSER[] PROGMEM = R"=====(
<div class="col py-3">
  <h2>{{pageName}}</h2>
  <div id='main' class="row">
    <div class="col-sm-12 col-md-6 mb-4">
      <div class='card'>
        <div class='card-header'>File Browser</div>
        <div class='card-body'>
          <table class="table">
            {{fileList}}
          </table>
          <form method="POST" action="saveFile">
            <div class="form-group">
              <div><label for="file">File : <span id="title"></span></label><input type="hidden" name="filename"
                  id="filename" value=""></div><textarea class="form-control mb-2" id="file" name="file" rows="10"></textarea>
            </div><button type="submit" class="btn btn-outline-primary col-sm-12 col-md-6">Save</button>
          </form>
        </div>
      </div>
    </div>
    <div class="col-sm-12 col-md-6 mb-4">
      <div class='card'>
        <div class='card-header'>Debug console</div>
        <div class='card-body'>
          <div class="row">
            <div class="col">
              <div class="col-sm-12">Raw data :</div>
              <textarea class="form-control col-sm-12 mb-2" id="console" rows="8"></textarea>
              <button type="button" onclick="cmd('ClearConsole');document.getElementById('console').value=''" class="btn btn-outline-primary col-sm-12 col-md-6">Clear Console</button>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>

</div>
</div>
<script language="javascript">logRefresh({{refreshLogs}});</script>
)=====";

const char HTTP_ABOUT[] PROGMEM =
    "<div class='col py-3'>"
    "<h2>About</h2>"
    "<div id='main' class='col-sm-9'>"
    /*
    "<div id='help_btns'>"
    "<a href='#' class='btn btn-primary'><i class='glyphicon glyphicon-cog'></i>Primary</a>"
    "<a href='#' class='btn btn-secondary'><i class='glyphicon glyphicon-file'></i>Secondary</a>"
    "<a href='#' class='btn btn-success'><i class='glyphicon glyphicon-flag'></i>Success</a>"
    "<a href='#' class='btn btn-danger'><i class='glyphicon glyphicon-lock'></i>Danger</a>"
    "<a href='#' class='btn btn-warning'><i class='glyphicon glyphicon-tags'></i>Warning</a>"
    "</div>"
    */
    // "<script src='https://cdn.jsdelivr.net/npm/@webcomponents/webcomponentsjs@2/webcomponents-loader.min.js'></script>"
    // "<script type='module' src='https://cdn.jsdelivr.net/gh/zerodevx/zero-md@1/src/zero-md.min.js'></script>"
    // "<zero-md src='https://raw.githubusercontent.com/smlight-dev/SLZB-06/main/README.md'></zero-md>"
    "<a class='nav-link' href='https://smlight.tech/products/slzb06'><i class='glyphicon glyphicon-info-sign'></i>Official web site with support page</a>"
    "</div>"
    "</div>"
    "</div>"
    "</div>";

const char HTTP_ERROR[] PROGMEM =
    "<h2>{{pageName}}</h2>"
    "<div id='main' class='col-sm-9'>"
    "</div>"
    "</div>"
    "</div>";
