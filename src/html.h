// const char HTTP_HEADER[] PROGMEM = R"=====(
// <head>
//   <meta name='viewport' content='width=device-width, initial-scale=1'>
//   <script type='text/javascript' src='/js/jquery-min.js' async></script>
//   <script type='text/javascript' src='/js/bootstrap.min.js' async></script>
//   <script type='text/javascript' src='/js/functions.js' async></script>
//   <script>function logoutButton() {
//       var xhr = new XMLHttpRequest();
//       xhr.open('GET', '/logout', true);
//       xhr.send();
//       setTimeout(function () { window.open('/logged-out', '_self'); }, 500);
//     }</script>
//   <link href='css/required.css' rel='stylesheet' type='text/css' />
//   <title>{{pageName}} - SLZB-06 Zigbee Ethernet POE USB Adapter</title>
// </head>

// <body>
//   <div id="smPreloader">
//     <div class="smPreloaderWrapper">
//         <div id="circle-1" class="smCircle">
//         </div>
//         <div id="circle-2" class="smCircle">
//         </div>
//         <div id="circle-3" class="smCircle">
//         </div>
//     </div>
// </div>
// <div class="toast-container position-fixed bottom-0 end-0 p-3">
//     <div class="toast fade toast_show hide" data-bs-autohide="false" role="alert">
//         <div class="toast-header">
//             <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" class="bi bi-info-circle"
//                 viewBox="0 0 16 16" reserveAspectRatio="xMidYMid slice" focusable="false" role="img">
//                 <path d="M8 15A7 7 0 1 1 8 1a7 7 0 0 1 0 14zm0 1A8 8 0 1 0 8 0a8 8 0 0 0 0 16z" />
//                 <path
//                     d="m8.93 6.588-2.29.287-.082.38.45.083c.294.07.352.176.288.469l-.738 3.468c-.194.897.105 1.319.808 1.319.545 0 1.178-.252 1.465-.598l.088-.416c-.2.176-.492.246-.686.246-.275 0-.375-.193-.304-.533L8.93 6.588zM9 4.5a1 1 0 1 1-2 0 1 1 0 0 1 2 0z" />
//             </svg>
//             <strong class="mr-auto" style="padding-left: 10px;">Tip</strong>
//         </div>
//         <div class="toast-body">
//             Statuses and other information in this window are updated when the page refreshes.
//             <div class="mt-2 pt-2 border-top">
//                 <button type="button" class="btn btn-outline-primary" data-bs-dismiss="toast"
//                     onclick="localStorage.setItem('refresh_tip_got', 1)">Got it!</button>
//             </div>
//         </div>
//     </div>
// </div>
// <div class="modal fade" id="modal" data-bs-backdrop="static" data-bs-keyboard="false" tabindex="100" aria-modal="true" role="dialog">
//   <div class="modal-dialog">
//     <div class="modal-content">
//       <div class="modal-header">
//         <h5 class="modal-title" id="modalTitle">Modal title</h5>
//         <!-- <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button> -->
//       </div>
//       <div class="modal-body" id="modalBody">
//         Flashing wrong 
//       </div>
//       <div class="modal-footer" id="modalButtons">
        
//       </div>
//     </div>
//   </div>
// </div>
// <nav class="navbar shadow sticky-top bg-light" style="display: none;">
// <div class="container">
//   <div class="navbar-brand">
//     <span class="navbar-toggler-icon" onclick="$('#sidenav').addClass('sidenav-active'); sidenavAutoclose();"></span>
//   </div>
//   <h2 class="position-absolute top-50 start-50 translate-middle" style="color: lightcoral;">{{pageName}}</h2>
// </div>
// </nav>
//   <div class='container-fluid'>
//     <div class='row flex-nowrap'>
//       <div id="sidenav" class='col-auto nav-container px-0 col-md-3 col-xl-2 bg-dark nav-shadow'>
//         <div class='d-flex flex-column align-items-center align-items-sm-start pt-2 text-white min-vh-100'>
//           <div class='logo-wrapper'>
//             <img src='./img/logo.png'>
//             <span class='fs-5 d-none d-sm-inline'>SLZB-06</span>
//             </a>
//           </div>
//           <ul class='nav nav-pills flex-column mb-sm-auto mb-0 align-items-center align-items-sm-start' id='menu'>
//             <li class='nav-item'>
//               <a href='/' class='nav-link align-middle' onclick=''>
//                 <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor' class='bi bi-house'
//                   viewBox='0 0 16 16'>
//                   <path fill-rule='evenodd'
//                     d='M2 13.5V7h1v6.5a.5.5 0 0 0 .5.5h9a.5.5 0 0 0 .5-.5V7h1v6.5a1.5 1.5 0 0 1-1.5 1.5h-9A1.5 1.5 0 0 1 2 13.5zm11-11V6l-2-2V2.5a.5.5 0 0 1 .5-.5h1a.5.5 0 0 1 .5.5z' />
//                   <path fill-rule='evenodd'
//                     d='M7.293 1.5a1 1 0 0 1 1.414 0l6.647 6.646a.5.5 0 0 1-.708.708L8 2.207 1.354 8.854a.5.5 0 1 1-.708-.708L7.293 1.5z' />
//                 </svg>
//                 <span class='ms-1 d-none d-sm-inline'> Status</span>
//               </a>
//             </li>
//             <li class='nav-item'>
//               <a href='/general' class='nav-link align-middle'>
//                 <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor'
//                   class='bi bi-hdd-stack' viewBox='0 0 16 16'>
//                   <path
//                     d='M14 10a1 1 0 0 1 1 1v1a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1v-1a1 1 0 0 1 1-1h12zM2 9a2 2 0 0 0-2 2v1a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2v-1a2 2 0 0 0-2-2H2z' />
//                   <path
//                     d='M5 11.5a.5.5 0 1 1-1 0 .5.5 0 0 1 1 0zm-2 0a.5.5 0 1 1-1 0 .5.5 0 0 1 1 0zM14 3a1 1 0 0 1 1 1v1a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1V4a1 1 0 0 1 1-1h12zM2 2a2 2 0 0 0-2 2v1a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V4a2 2 0 0 0-2-2H2z' />
//                   <path d='M5 4.5a.5.5 0 1 1-1 0 .5.5 0 0 1 1 0zm-2 0a.5.5 0 1 1-1 0 .5.5 0 0 1 1 0z' />
//                 </svg>
//                 <span class='ms-1 d-none d-sm-inline'>General</span>
//               </a>
//             </li>
//             <li class='nav-item'>
//               <a href='/ethernet' class='nav-link align-middle'>
//                 <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor'
//                   class='bi bi-ethernet' viewBox='0 0 16 16'>
//                   <path
//                     d='M14 13.5v-7a.5.5 0 0 0-.5-.5H12V4.5a.5.5 0 0 0-.5-.5h-1v-.5A.5.5 0 0 0 10 3H6a.5.5 0 0 0-.5.5V4h-1a.5.5 0 0 0-.5.5V6H2.5a.5.5 0 0 0-.5.5v7a.5.5 0 0 0 .5.5h11a.5.5 0 0 0 .5-.5ZM3.75 11h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5a.25.25 0 0 1 .25-.25Zm2 0h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5a.25.25 0 0 1 .25-.25Zm1.75.25a.25.25 0 0 1 .25-.25h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5ZM9.75 11h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5a.25.25 0 0 1 .25-.25Zm1.75.25a.25.25 0 0 1 .25-.25h.5a.25.25 0 0 1 .25.25v1.5a.25.25 0 0 1-.25.25h-.5a.25.25 0 0 1-.25-.25v-1.5Z' />
//                   <path
//                     d='M2 0a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V2a2 2 0 0 0-2-2H2ZM1 2a1 1 0 0 1 1-1h12a1 1 0 0 1 1 1v12a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1V2Z' />
//                 </svg>
//                 <span class='ms-1 d-none d-sm-inline'>Ethernet</span>
//               </a>
//             </li>
//             <li class='nav-item'>
//               <a href='/wifi' class='nav-link align-middle'>
//               <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor' class='bi bi-wifi' viewBox='0 0 16 16'>
//               <path d='M15.384 6.115a.485.485 0 0 0-.047-.736A12.444 12.444 0 0 0 8 3C5.259 3 2.723 3.882.663 5.379a.485.485 0 0 0-.048.736.518.518 0 0 0 .668.05A11.448 11.448 0 0 1 8 4c2.507 0 4.827.802 6.716 2.164.205.148.49.13.668-.049z'/>
//               <path d='M13.229 8.271a.482.482 0 0 0-.063-.745A9.455 9.455 0 0 0 8 6c-1.905 0-3.68.56-5.166 1.526a.48.48 0 0 0-.063.745.525.525 0 0 0 .652.065A8.46 8.46 0 0 1 8 7a8.46 8.46 0 0 1 4.576 1.336c.206.132.48.108.653-.065zm-2.183 2.183c.226-.226.185-.605-.1-.75A6.473 6.473 0 0 0 8 9c-1.06 0-2.062.254-2.946.704-.285.145-.326.524-.1.75l.015.015c.16.16.407.19.611.09A5.478 5.478 0 0 1 8 10c.868 0 1.69.201 2.42.56.203.1.45.07.61-.091l.016-.015zM9.06 12.44c.196-.196.198-.52-.04-.66A1.99 1.99 0 0 0 8 11.5a1.99 1.99 0 0 0-1.02.28c-.238.14-.236.464-.04.66l.706.706a.5.5 0 0 0 .707 0l.707-.707z'/>
//               </svg>
//               <span class='ms-1 d-none d-sm-inline'>WiFi</span>
//               </a>
//               </li>
//             <li class='nav-item'>
//               <a href='/serial' class='nav-link align-middle'>
//                 <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor'
//                   class='bi bi-activity' viewBox='0 0 16 16'>
//                   <path fill-rule='evenodd'
//                     d='M6 2a.5.5 0 0 1 .47.33L10 12.036l1.53-4.208A.5.5 0 0 1 12 7.5h3.5a.5.5 0 0 1 0 1h-3.15l-1.88 5.17a.5.5 0 0 1-.94 0L6 3.964 4.47 8.171A.5.5 0 0 1 4 8.5H.5a.5.5 0 0 1 0-1h3.15l1.88-5.17A.5.5 0 0 1 6 2Z' />
//                 </svg>
//                 <span class='ms-1 d-none d-sm-inline'>Serial</span>
//               </a>
//             </li>
//             <li class='nav-item'>
//               <a href='/security' class='nav-link align-middle'>
//                 <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor' class='bi bi-lock'
//                   viewBox='0 0 16 16'>
//                   <path
//                     d='M8 1a2 2 0 0 1 2 2v4H6V3a2 2 0 0 1 2-2zm3 6V3a3 3 0 0 0-6 0v4a2 2 0 0 0-2 2v5a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2zM5 8h6a1 1 0 0 1 1 1v5a1 1 0 0 1-1 1H5a1 1 0 0 1-1-1V9a1 1 0 0 1 1-1z' />
//                 </svg>
//                 <span class='ms-1 d-none d-sm-inline'>Security</span>
//               </a>
//             </li>
//             <li class='nav-item'>
//               <a href='/sys-tools' class='nav-link align-middle'>
//                 <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor' class='bi bi-cpu'
//                   viewBox='0 0 16 16'>
//                   <path
//                     d='M5 0a.5.5 0 0 1 .5.5V2h1V.5a.5.5 0 0 1 1 0V2h1V.5a.5.5 0 0 1 1 0V2h1V.5a.5.5 0 0 1 1 0V2A2.5 2.5 0 0 1 14 4.5h1.5a.5.5 0 0 1 0 1H14v1h1.5a.5.5 0 0 1 0 1H14v1h1.5a.5.5 0 0 1 0 1H14v1h1.5a.5.5 0 0 1 0 1H14a2.5 2.5 0 0 1-2.5 2.5v1.5a.5.5 0 0 1-1 0V14h-1v1.5a.5.5 0 0 1-1 0V14h-1v1.5a.5.5 0 0 1-1 0V14h-1v1.5a.5.5 0 0 1-1 0V14A2.5 2.5 0 0 1 2 11.5H.5a.5.5 0 0 1 0-1H2v-1H.5a.5.5 0 0 1 0-1H2v-1H.5a.5.5 0 0 1 0-1H2v-1H.5a.5.5 0 0 1 0-1H2A2.5 2.5 0 0 1 4.5 2V.5A.5.5 0 0 1 5 0zm-.5 3A1.5 1.5 0 0 0 3 4.5v7A1.5 1.5 0 0 0 4.5 13h7a1.5 1.5 0 0 0 1.5-1.5v-7A1.5 1.5 0 0 0 11.5 3h-7zM5 6.5A1.5 1.5 0 0 1 6.5 5h3A1.5 1.5 0 0 1 11 6.5v3A1.5 1.5 0 0 1 9.5 11h-3A1.5 1.5 0 0 1 5 9.5v-3zM6.5 6a.5.5 0 0 0-.5.5v3a.5.5 0 0 0 .5.5h3a.5.5 0 0 0 .5-.5v-3a.5.5 0 0 0-.5-.5h-3z' />
//                 </svg>
//                 <span class='ms-1 d-none d-sm-inline'>System and Tools</span>
//               </a>
//             </li>
//             <li class='nav-item'>
//               <a href='/about' class='nav-link align-middle'>
//                 <svg xmlns='http://www.w3.org/2000/svg' width='25' height='25' fill='currentColor'
//                   class='bi bi-info-square' viewBox='0 0 16 16'>
//                   <path
//                     d='M14 1a1 1 0 0 1 1 1v12a1 1 0 0 1-1 1H2a1 1 0 0 1-1-1V2a1 1 0 0 1 1-1h12zM2 0a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V2a2 2 0 0 0-2-2H2z' />
//                   <path
//                     d='m8.93 6.588-2.29.287-.082.38.45.083c.294.07.352.176.288.469l-.738 3.468c-.194.897.105 1.319.808 1.319.545 0 1.178-.252 1.465-.598l.088-.416c-.2.176-.492.246-.686.246-.275 0-.375-.193-.304-.533L8.93 6.588zM9 4.5a1 1 0 1 1-2 0 1 1 0 0 1 2 0z' />
//                 </svg>
//                 <span class='ms-1 d-none d-sm-inline'>About</span>
//               </a>
//             </li>
//           </ul>
//           <hr>
//           {{logoutLink}}
//         </div>
//       </div>
//       <div class='col py-3'>
//         <h2 class="mb-3" id="pagenamePC">{{pageName}}</h2>
//       )=====";

// const char LOGOUT_LINK[] PROGMEM =
//     "<li class='nav-item'>"
//     "<a class='nav-link' href='javascript:logoutButton();'><i class='glyphicon glyphicon glyphicon-log-out'></i>Logout</a>"
//     "</li>";

// const char HTTP_ROOT[] PROGMEM = R"=====(
// <div id='main2' class='row masonry' data-masonry='{"percentPosition": true }'>
//     <!-- <span>Statuses and other information in this window are updated when the page refreshes</span> -->
//       <div class='col-sm-12 col-md-6 mb-4'>
//         <div class='card'>
//           <div class='card-header'>Device status</div>
//           <div class='card-body'>
//             <table class="table">
//               <tbody>
//                 <tr>
//                   <td>Operational mode:</td>
//                   <td>{{operationalMode}}</td>
//                 </tr>
//                 <tr>
//                   <td>Ethernet connected:</td>
//                   <td>{{connectedEther}}</td>
//                 </tr>
//                 <tr>
//                   <td>Socket client connected:</td>
//                   <td>{{connectedSocketStatus}}</td>
//                 </tr>
//                 <tr>
//                   <td>WiFi Client enabled:</td>
//                   <td>{{wifiEnabled}}</td>
//                 </tr>
//                 <tr>
//                   <td>WiFi Client status:</td>
//                   <td>{{wifiConnected}}</td>
//                 </tr>
//                 <tr>
//                   <td>WiFi Access Point enabled:</td>
//                   <td>{{wifiModeAP}}</td>
//                 </tr>
//                 <tr>
//                   <td>WiFi Access Point status:</td>
//                   <td>{{wifiModeAPStatus}}</td>
//                 </tr>
//                 <tr>
//                   <td>Device Uptime:</td>
//                   <td>{{uptime}}</td>
//                 </tr>
//                 <tr>
//                   <td>Socket uptime:</td>
//                   <td>{{connectedSocket}}</td>
//                 </tr>
//               </tbody>
//             </table>
//           </div>
//         </div>
//       </div>
  
//       <div class='col-sm-12 col-md-6 mb-4'>
//         <div class='card'>
//           <div class='card-header'>Device information</div>
//           <div class='card-body'>
//             <table class="table">
//               <tbody>
//                 <tr>
//                   <td>Model:</td>
//                   <td>{{hwRev}}</td>
//                 </tr>
//                 <tr>
//                   <td>ESP32 Firmware ver:</td>
//                   <td>{{VERSION}}</td>
//                 </tr>
//                 <tr>
//                   <td>ESP32 version:</td>
//                   <td>{{espModel}}</td>
//                 </tr>
//                 <tr>
//                   <td>ESP32 temperature:</td>
//                   <td>{{deviceTemp}} &deg;C</td>
//                 </tr>
//                 <tr>
//                   <td>ESP32 frequency:</td>
//                   <td>{{espCores}} cores @ {{espFreq}} MHz</td>
//                 </tr>
//                 <tr>
//                   <td>ESP32 flash size:</td>
//                   <td>{{espFlashSize}} Mb, {{espFlashType}}</td>
//                 </tr>
//                 <tr>
//                   <td>ESP32 Free heap:</td>
//                   <td>{{espHeapFree}} / {{espHeapSize}} KiB</td>
//                 </tr>
//                 <tr>
//                   <td>CC2652P version:</td>
//                   <td>CC2652PP1FRGZR</td>
//                 </tr>
//                 <tr>
//                   <td>CC2652P frequency:</td>
//                   <td>48 MHz</td>
//                 </tr>
//                 <tr>
//                   <td>CC2652P flash size:</td>
//                   <td>352 Kb</td>
//                 </tr>
//               </tbody>
//             </table>
//           </div>
//         </div>
//       </div>
  
//       <div class='col-sm-12 col-md-6 mb-4'>
//         <div class='card'>
//           <div class='card-header'>Ethernet</div>
//           <div class='card-body'>
//             <table class="table">
//               <tbody>
//                 <tr>
//                   <td>Connection status:</td>
//                   <td>{{ethConnection}}</td>
//                 </tr>
//                 <tr>
//                   <td>DHCP:</td>
//                   <td>{{ethDhcp}}</td>
//                 </tr>
//                 <tr>
//                   <td>IP Address:</td>
//                   <td>{{ethIp}}</td>
//                 </tr>
//                 <tr>
//                   <td>Subnet Mask:</td>
//                   <td>{{etchMask}}</td>
//                 </tr>
//                 <tr>
//                   <td>Default Gateway:</td>
//                   <td>{{ethGate}}</td>
//                 </tr>
//                 <tr>
//                   <td>Connection speed:</td>
//                   <td>{{ethSpd}}</td>
//                 </tr>
//                 <tr>
//                   <td>MAC address:</td>
//                   <td>{{ethMac}}</td>
//                 </tr>
//               </tbody>
//             </table>
//           </div>
//         </div>
//       </div>
  
//       <div class='col-sm-12 col-md-6 mb-4'>
//         <div class='card'>
//           <div class='card-header'>Wifi</div>
//           <div class='card-body'>
//             <table class="table">
//               <tbody>
//                 <tr>
//                   <td>Mode:</td>
//                   <td>{{wifiMode}}</td>
//                 </tr>
//                 <tr>
//                   <td>SSID:</td>
//                   <td>{{wifiSsid}}</td>
//                 </tr>
//                 <tr>
//                   <td>MAC Address:</td>
//                   <td>{{wifiMac}}</td>
//                 </tr>
//                 <tr>
//                   <td>IP Address:</td>
//                   <td>{{wifiIp}}</td>
//                 </tr>
//                 <tr>
//                   <td>Subnet Mask:</td>
//                   <td>{{wifiSubnet}}</td>
//                 </tr>
//                 <tr>
//                   <td>Default Gateway:</td>
//                   <td>{{wifiGate}}</td>
//                 </tr>
//                 <tr>
//                   <td>RSSI:</td>
//                   <td>{{wifiRssi}}</td>
//                 </tr>
//                 <tr>
//                   <td>DHCP:</td>
//                   <td>{{wifiDhcp}}</td>
//                 </tr>
//               </tbody>
//             </table>
//           </div>
//         </div>
//       </div>
//     </div>
//   </div>
  
//   </div>
//   </div>
//   <script src="/js/masonry.min.js"></script>
//      )=====";

// const char HTTP_GENERAL[] PROGMEM = R"=====(
//     <div class='row'>
//     <div class="col-sm-12">
//         <form method="POST" action="saveGeneral">
//             <div class="card">
//                 <div class="card-header">General settings</div>
//                 <div class="card-body">
//                     <div class="row">
//                         <div class="col-sm-12 col-md-6 mb-3">
//                             <div class="row">
//                                 <div class="col-sm-12 mb-3">
//                                     <span>Select the device operating mode:</span>
//                                     <div class="form-check">
//                                         <input class="form-check-input" type="radio" name="coordMode" id="usbMode"
//                                         {{checkedUsbMode}} value="0">
//                                         <label class="form-check-label" for="lanMode">
//                                             USB mode
//                                         </label>
//                                     </div>
//                                     <div class="form-check">
//                                         <input class="form-check-input" type="radio" name="coordMode" id="wifiMode"
//                                         {{checkedWifiMode}} value="1">
//                                         <label class="form-check-label" for="lanMode">
//                                             WIFI mode
//                                         </label>
//                                     </div>
//                                     <div class="form-check">
//                                         <input class="form-check-input" type="radio" name="coordMode" id="lanMode"
//                                             {{checkedLanMode}} value="2">
//                                         <label class="form-check-label" for="usbMode">
//                                             LAN mode
//                                         </label>
//                                     </div>
//                                     <hr class="border border-dark border-top">
//                                     <br>
//                                     <span>Specify the hostname. The device will be displayed in the LAN network with
//                                         this name:</span>
//                                     <div class="col-sm-12 mb-3">
//                                         <label for="hostname">Hostname</label>
//                                         <input class="form-control" id="hostname" type="text" name="hostname"
//                                             value="{{hostname}}">
//                                         <label for='refreshLogs'>Refresh console log</label>
//                                         <input class='form-control' id='refreshLogs' type="number" min="100" max="10000"
//                                             step="100" name='refreshLogs' value='{{refreshLogs}}'
//                                             placeholder="in milliseconds">
//                                     </div>
//                                 </div>
//                             </div>
//                         </div>
//                         <div class="col-sm-12 col-md-6 mb-3">
//                             <div class="row">
//                                 <div class="col-sm-12">
//                                     <span>Using the Toggle buttons, you can control the state of the LEDs in the current
//                                         session (not saved on reboot):</span>
//                                 </div>
//                                 <div class="col-sm-12">
//                                     <button type="button" onclick="cmd('LedYellowToggle');"
//                                         class="btn btn-outline-primary col col-md-auto mb-1">Power LED (Yellow)
//                                         Toggle</button>
//                                     <button type="button" onclick="cmd('LedBlueToggle');"
//                                         class="btn btn-outline-primary col col-md-auto mb-1">Mode LED (Blue)
//                                         Toggle</button>
//                                     <br>
//                                 </div>
//                                 <br>

//                                 <div class="col-sm-12">
//                                     <br>
//                                     <span>Control the behavior of the LEDs (with saving on reboot) using the switches
//                                         below:</span>
//                                     <div class='form-check form-switch'>
//                                         <input class='form-check-input' id='disableLedYellow' type='checkbox'
//                                             role="switch" name='disableLedYellow' {{checkedDisableLedYellow}}>
//                                         <label class='form-check-label' for='disableLedYellow'>Disable Power LED
//                                             (yellow)</label>
//                                     </div>
//                                     <div class='form-check form-switch'>
//                                         <input class='form-check-input' id='disableLedBlue' type='checkbox'
//                                             role="switch" name='disableLedBlue' {{checkedDisableLedBlue}}>
//                                         <label class='form-check-label' for='disableLedBlue'>Disable USB mode LED
//                                             (blue)</label>
//                                     </div>
//                                 </div>
//                             </div>
//                         </div>
//                     </div>
//                     <div class="col-sm-12">
//                         <div class="row justify-content-md-center">
//                             <button type="submit" class="btn btn-outline-primary col-sm-12 col-md-6"
//                                 name="save">Save</button>
//                         </div>
//                     </div>
//                 </div>
//             </div>
//     </div>
//     </form>
// </div>
// </div>

// </div>
// </div>
// )=====";

// const char HTTP_ETHERNET[] PROGMEM = R"=====(
//     <div class='row'>
//   <form method='POST' action='saveEther'>
//       <div class='col-sm-12 cardPadding'>
//           <div class='card'>
//               <div class="card-header">Ethernet options</div>
//               <div class="card-body">
//                   <div class='mb-2 form-check form-switch'>
//                       <input id="EthDhcpTog" class='form-check-input' type='checkbox' role="switch" name='dhcp'
//                           onclick="EthInputDsbl(this.checked)" {{modeEther}}>
//                       <label class='form-check-label' for='dhcp'>DHCP</label>
//                   </div>
//                   <div class='mb-2'>
//                       <label for='ip'>IP</label>
//                       <input class='form-control' id='EthIp' type='text' name='ipAddress' value='{{ipEther}}'
//                           placeholder="192.168.x.x">
//                   </div>
//                   <div class='mb-2'>
//                       <label for='mask'>Mask</label>
//                       <input class='form-control' id='EthMask' type='text' name='ipMask' value='{{maskEther}}'
//                           placeholder="255.255.255.0">
//                   </div>
//                   <div class='mb-2'>
//                       <label for='gateway'>Gateway</label>
//                       <input type='text' class='form-control' id='EthGateway' name='ipGW' value='{{GWEther}}'
//                           placeholder="192.168.x.x">
//                   </div>
//                   <div class='mb-2 form-check form-switch'>
//                       <input class='form-check-input' id='disablePingCtrl' type='checkbox' role="switch"
//                           name='disablePingCtrl' {{disablePingCtrl}}>
//                       <label class='form-check-label' for='disablePingCtrl'><b>Disable</b> gateway ping
//                           check</label>
//                   </div>
//                   <br>
//                   <div class="row justify-content-md-center">
//                   <button type='submit' class='btn btn-outline-primary col-sm-12 col-md-6' name='save'>Save</button>
//                   </div>
//               </div>
//           </div>
//       </div>
//   </form>
// </div>

// </div>
// </div>
//      )=====";

// const char HTTP_WIFI[] PROGMEM = R"=====(
// <div class="col-sm-12 cardPadding">
//   <div class="card">
//     <div class="card-header">Wifi Setup</div>
//     <div class="card-body">
//       <form method="POST" action="saveWifi">
//         <div class="row">
//           <div class="mb-2 col-sm-12 col-md-6">
//             <table class="table table-hover">
//               <thead>
//                 <tr>
//                   <th scope="col">SSID</th>
//                   <th scope="col">Security</th>
//                   <th scope="col">Channel</th>
//                   <th scope="col">RSSI</th>
//                 </tr>
//               </thead>
//               <tbody id="wifiTable">
//                 <tr id="wifiScanPreloader" class="visually-hidden">
//                   <td colspan="4">
//                     <div class="progress">
//                       <div class="progress-bar progress-bar-striped progress-bar-animated" role="progressbar" aria-label="Animated striped example" aria-valuenow="100" aria-valuemin="0" aria-valuemax="100" style="width: 100%"></div>
//                     </div>
//                   </td>
//                 </tr>
//                 <tr id="wifiScanButton">
//                   <td colspan="4">
//                     <div class="row justify-content-md-center">
//                       <button type="button" class="btn btn-outline-primary col-sm-12 col-md-6" onclick="getWifiList()">Scan for WIFI networks</button>
//                     </div>
//                   </td>
//                 </tr>
//               </tbody>
//             </table>
//             <div class="collapse" id="collapseWifiPass">
//               <div class="card card-body">
//                 <div class="mb-2">
//                   <label for="WIFISSID">SSID</label>
//                   <input class="form-control" id="WIFIssid" type="text" name="WIFISSID" id="WIFISSID"
//                     value="{{ssid}}">
//                 </div>
//                 <div class="mb-2">
//                   <label for="pass">Password</label>
//                   <input class="form-control" id="WIFIpassword" type="password" name="WIFIpassword" id="WIFIpassword"
//                     value="{{passWifi}}">
//                 </div>
//               </div>
//             </div>
//           </div>
//           <br>
//           <div class="mb-2 col-sm-12 col-md-6">
//             <div class="mb-2">
//               <div class="form-check form-switch">
//                 <input class="form-check-input" id="dhcpWiFi" type="checkbox" role="switch" name="dhcpWiFi"
//                   onclick="WifiDhcpDsbl(this.checked)" {{dchp}}>
//                 <label class="form-check-label" for="dhcpWiFi">DHCP</label>
//               </div>
//             </div>
//             <div class="mb-2">
//               <label for="ip">IP</label>
//               <input class="form-control" id="WifiIp" type="text" name="ipAddress" value="{{ip}}"
//                 placeholder="192.168.x.x">
//             </div>
//             <div class="mb-2">
//               <label for="mask">Mask</label>
//               <input class="form-control" id="WifiMask" type="text" name="ipMask" value="{{mask}}"
//                 placeholder="255.255.255.0">
//             </div>
//             <div class="mb-2">
//               <label for="gateway">Gateway</label>
//               <input type="text" class="form-control" id="WifiGateway" name="ipGW" value="{{gw}}"
//                 placeholder="192.168.x.x">
//             </div>
//           </div>
//           <br>
//           <div class="col-sm-12">
//             <div class="row justify-content-md-center">
//               <button type="submit" class="btn btn-outline-primary col-sm-12 col-md-6" name="save">Save</button>
//             </div>
//           </div>
//         </div>
//       </form>
//     </div>
//   </div>
// </div>

// </div>
// </div>
//     )=====";

// const char HTTP_SERIAL[] PROGMEM = R"=====(
//   <div id='main'>
//     <form method='POST' action='saveSerial'>
//       <div class='card'>
//         <div class='card-header'>Serial options</div>
//         <div class='card-body'>
//           <div class="row">
//             <div class='col-sm-12 col-md-6 mb-4'>
//               <div class='form-group'>
//                 <label for='baud'>Serial Speed</label>
//                 <select class='form-select' id='baud' name='baud'>
//                   <option value='9600' {{selected9600}}>9600 bauds</option>
//                   <option value='19200' {{selected19200}}>19200 bauds</option>
//                   <option value='38400' {{selected38400}}>38400 bauds</option>
//                   <option value='57600' {{selected57600}}>57600 bauds</option>
//                   <option value='115200' {{selected115200}}>115200 bauds</option>
//                 </select>
//               </div>
//             </div>
//             <div class='col-sm-12 col-md-6 mb-4'>
//               <div class='form-group'>
//                 <label for='port'>Socket Port</label>
//                 <input class='form-control' id='port' type='number' name='port' min='100' max='65000'
//                   value='{{socketPort}}'>
//               </div>
//             </div>
//           </div>
//           <div class='col-sm-12'>
//             <div class="row justify-content-md-center">
//               <button type='submit' class='btn btn-outline-primary col-sm-12 col-md-6' name='save'>Save</button>
//             </div>
//           </div>
//         </div>
//       </div>
//     </form>
//   </div>
// </div>

// </div>
// </div>
//     )=====";

// const char HTTP_SECURITY[] PROGMEM = R"=====(
//   <div id='main'>
//     <form method='POST' action='saveSecurity'>
//       <div class='card'>
//         <div class='card-header'>Security options</div>
//         <div class='card-body'>
//           <div class="row">
//             <div class="col-sm-12 col-md-6 mb-4">
//               <div class='form-check'>
//                 <input class='form-check-input' id='disableWeb' type='checkbox' name='disableWeb' {{disableWeb}}>
//                 <label class='form-check-label' for='disableWeb'>Disable web server when socket is connected</label>
//                 <br>
//               </div>
//               <div class='form-check'>
//                 <input class='form-check-input' id='webAuth' type='checkbox' name='webAuth' {{webAuth}} onclick="SeqInputDsbl(!this.checked)">
//                 <label class='form-check-label' for='webAuth'>Enable web server authentication</label>
//               </div>
//             </div>
//             <div class="col-sm-12 col-md-6 mb-2">
//               <label for='webUser'>Username</label>
//               <input class='form-control' id='webUser' type='text' name='webUser' value='{{webUser}}' disabled>
//               <label for='webPass'>Password</label>
//               <input class='form-control' id='webPass' type='password' name='webPass' value='{{webPass}}' disabled>
//             </div>
//             <div class="col-sm-12">
//               <div class="row justify-content-md-center">
//                 <button type='submit' class='btn btn-outline-primary col-sm-12 col-md-6' name='save'>Save</button>
//               </div>
//             </div>
//           </div>
//         </div>
//       </div>
//     </form>
//   </div>
// </div>

// </div>
// </div>
// )=====";

// const char HTTP_SYSTOOLS[] PROGMEM = R"=====(
//   <ul class="nav nav-tabs nav-fill mb-3" role="tablist" style="flex-wrap: inherit;">
//     <li class="nav-item" role="presentation">
//       <button class="nav-link active" id="home-tab" data-bs-toggle="tab" data-bs-target="#systemControl" type="button"
//         role="tab" aria-controls="home" aria-selected="true"><svg class="me-1" xmlns="http://www.w3.org/2000/svg" width="16"
//           height="16" fill="currentColor" class="bi bi-sliders" viewBox="0 0 16 16">
//           <path fill-rule="evenodd"
//             d="M11.5 2a1.5 1.5 0 1 0 0 3 1.5 1.5 0 0 0 0-3zM9.05 3a2.5 2.5 0 0 1 4.9 0H16v1h-2.05a2.5 2.5 0 0 1-4.9 0H0V3h9.05zM4.5 7a1.5 1.5 0 1 0 0 3 1.5 1.5 0 0 0 0-3zM2.05 8a2.5 2.5 0 0 1 4.9 0H16v1H6.95a2.5 2.5 0 0 1-4.9 0H0V8h2.05zm9.45 4a1.5 1.5 0 1 0 0 3 1.5 1.5 0 0 0 0-3zm-2.45 1a2.5 2.5 0 0 1 4.9 0H16v1h-2.05a2.5 2.5 0 0 1-4.9 0H0v-1h9.05z" />
//         </svg>System Control</button>
//     </li>
//     <li class="nav-item" role="presentation">
//       <button class="nav-link" id="profile-tab" data-bs-toggle="tab" data-bs-target="#systemTools" type="button"
//         role="tab" aria-controls="profile" aria-selected="false" onclick="$('.masonry').masonry()"><svg class="me-1" xmlns="http://www.w3.org/2000/svg" width="16"
//           height="16" fill="currentColor" class="bi bi-tools" viewBox="0 0 16 16">
//           <path
//             d="M1 0 0 1l2.2 3.081a1 1 0 0 0 .815.419h.07a1 1 0 0 1 .708.293l2.675 2.675-2.617 2.654A3.003 3.003 0 0 0 0 13a3 3 0 1 0 5.878-.851l2.654-2.617.968.968-.305.914a1 1 0 0 0 .242 1.023l3.27 3.27a.997.997 0 0 0 1.414 0l1.586-1.586a.997.997 0 0 0 0-1.414l-3.27-3.27a1 1 0 0 0-1.023-.242L10.5 9.5l-.96-.96 2.68-2.643A3.005 3.005 0 0 0 16 3c0-.269-.035-.53-.102-.777l-2.14 2.141L12 4l-.364-1.757L13.777.102a3 3 0 0 0-3.675 3.68L7.462 6.46 4.793 3.793a1 1 0 0 1-.293-.707v-.071a1 1 0 0 0-.419-.814L1 0Zm9.646 10.646a.5.5 0 0 1 .708 0l2.914 2.915a.5.5 0 0 1-.707.707l-2.915-2.914a.5.5 0 0 1 0-.708ZM3 11l.471.242.529.026.287.445.445.287.026.529L5 13l-.242.471-.026.529-.445.287-.287.445-.529.026L3 15l-.471-.242L2 14.732l-.287-.445L1.268 14l-.026-.529L1 13l.242-.471.026-.529.445-.287.287-.445.529-.026L3 11Z" />
//         </svg>System Tools</button>
//     </li>
//   </ul>

//   <div id='main'>
//     <div class="tab-content">
//       <div class="tab-pane fade show active" id="systemControl" role="tabpanel" aria-labelledby="home-tab">
//         <div class="row masonry" data-masonry='{"percentPosition": true }'>
//           <div class="col-sm-12 col-md-6 mb-4">
//             <div class='card'>
//               <div class='card-header'>Modules control</div>
//               <div class='card-body'>
//                 <div class="row">
//                   <button type='button' onclick="cmd('ZigRST');"
//                     class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>Zigbee Restart</button>
//                   <button type='button' onclick="cmd('ZigBSL');"
//                     class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>Zigbee Flash Mode</button>
//                   <button type='button' onclick="cmd('EspReboot');"
//                     class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>ESP32 Restart</button>
//                 </div>
//               </div>
//             </div>
//           </div>
//           <div class="col-sm-12 col-md-6 mb-4">
//             <div class='card'>
//               <div class='card-header'>Current session control</div>
//               <div class='card-body'>
//                 <div class="row">
//                   <button type='button' onclick="cmd('AdapterModeLAN');"
//                     class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>LAN Mode ON</button>
//                   <button type='button' onclick="cmd('AdapterModeUSB');"
//                     class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>USB Mode ON</button>
//                   <button type='button' onclick="cmd('LedYellowToggle');"
//                     class='btn btn-outline-primary col-sm-12 col-md-auto mb-1'>Yellow LED
//                     Toggle</button>
//                   <button type="button" onclick="cmd('LedBlueToggle');"
//                     class="btn btn-outline-primary col-sm-12 col-md-auto mb-1">Blue LED Toggle</button>
//                 </div>
//               </div>
//             </div>
//           </div>
//         </div>

//       </div>
//       <div class="tab-pane fade" id="systemTools" role="tabpanel" aria-labelledby="profile-tab">
//         <div class="row masonry" data-masonry='{"percentPosition": true }'>
//           <div class="col-sm-12 col-md-6 mb-4">
//             <div class='card'>
//               <div class='card-header'>ESP32 Update</div>
//               <div class='card-body'>
//                 <form class="container" method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
//                   <input type='file' name='update' id='file' onchange='sub(this)' style=display:none accept='.bin'>
//                   <label id='file-input' for='file'> Choose file...</label>
//                   <input id="updButton" type='submit' class='btn btn-warning mb-2' value='ESP32 OTA Update' disabled>
//                   <br>
//                   <div id='prg'></div>
//                   <div id='prgbar'>
//                     <div id='bar'></div>
//                   </div>
//                 </form>
//               </div>
//             </div>
//           </div>
//           <div class="col-sm-12 col-md-6 mb-4">
//             <div class='card'>
//               <div class='card-header'>File Browser</div>
//               <div class='card-body'>
//                 <table class="table">
//                   {{fileList}}
//                 </table>
//                 <form method="POST" action="saveFile">
//                   <div class="form-group">
//                     <div><label for="file">File : <span id="title"></span></label><input type="hidden" name="filename"
//                         id="filename" value=""></div><textarea class="form-control mb-2" id="config_file" name="file"
//                       rows="10"></textarea>
//                   </div><button type="submit" class="btn btn-outline-primary col-sm-12 col-md-6">Save</button>
//                 </form>
//               </div>
//             </div>
//           </div>
//           <div class="col-sm-12 col-md-6 mb-4">
//             <div class='card'>
//               <div class='card-header'>Debug console</div>
//               <div class='card-body'>
//                 <div class="row">
//                   <div class="col">
//                     <div class="col-sm-12">Raw data :</div>
//                     <textarea class="form-control col-sm-12 mb-2" id="console" rows="8"></textarea>
//                     <button type="button" onclick="cmd('ClearConsole');document.getElementById('console').value=''"
//                       class="btn btn-outline-primary col-sm-12 col-md-6">Clear Console</button>
//                   </div>
//                 </div>
//               </div>
//             </div>
//           </div>
//         </div>

//       </div>
//     </div>
//   </div>
// </div>

// </div>
// </div>
// <script src="/js/masonry.min.js"></script>
// <script language="javascript">logRefresh({{refreshLogs}});</script>
// <script>
//   function sub(obj) {
//     let fileName = obj.value.split('\\\\');
//     if (fileName != "") {
//       if(!(localStorage.getItem('flash_warning') == 1)){
//       $(".modal").modal("show");
//       }else{
//         $("#updButton").removeAttr("disabled");
//       }
//     }
//     document.getElementById('file-input').innerHTML = '   ' + fileName[fileName.length - 1];
//   };
//   $('form#upload_form').submit(function (e) {
//     e.preventDefault();
//     var form = $('#upload_form')[0];
//     var data = new FormData(form);
//     $.ajax({
//       url: '/update',
//       type: 'POST',
//       data: data,
//       contentType: false,
//       processData: false,
//       xhr: function () {
//         var xhr = new window.XMLHttpRequest();
//         xhr.upload.addEventListener('progress', function (evt) {
//           if (evt.lengthComputable) {
//             var per = evt.loaded / evt.total;
//             $('#prg').html('progress: ' + Math.round(per * 100) + '%');
//             $('#bar').css('width', Math.round(per * 100) + '%');
//           }
//         }, false);
//         return xhr;
//       },
//       success: function (d, s) {
//         console.log('success!');
//         $('#prg').html('Update completed!<br>Rebooting!');
//         window.location.href = '/';
//       },
//       error: function (a, b, c) {
//       }
//     });
//   });
//   $(document).ready(function () { 
//     if(!(localStorage.getItem('flash_warning') == 1)){//toast localStorage.setItem('refresh_tip_got', 1)
//     $("#modalTitle").text("WARNING").css("color", "red");
//     $("#modalBody").text("Flashing unofficial, incorrect or corrupted firmware can damage or brick your device!!!").css("color", "red");
//     $("#modalButtons").append('<button type="button" class="btn btn-success" data-bs-dismiss="modal">Close</button>');
//     $("#modalButtons").append('<button type="button" class="btn btn-danger" data-bs-dismiss="modal" onclick="localStorage.setItem(\'flash_warning\', 1);$(\'#updButton\').removeAttr(\'disabled\');">I agree and I am taking the risk');
// 	}
// });
// </script>
//     )=====";

// const char LOGS_BROWSER[] PROGMEM = R"=====(
// <div class="col py-3">
//   <h2>{{pageName}}</h2>
//   <div id='main' class="row">
//     <div class="col-sm-12 col-md-6 mb-4">
//       <div class='card'>
//         <div class='card-header'>File Browser</div>
//         <div class='card-body'>
//           <table class="table">
//             {{fileList}}
//           </table>
//           <form method="POST" action="saveFile">
//             <div class="form-group">
//               <div><label for="file">File : <span id="title"></span></label><input type="hidden" name="filename"
//                   id="filename" value=""></div><textarea class="form-control mb-2" id="file" name="file" rows="10"></textarea>
//             </div><button type="submit" class="btn btn-outline-primary col-sm-12 col-md-6">Save</button>
//           </form>
//         </div>
//       </div>
//     </div>
//     <div class="col-sm-12 col-md-6 mb-4">
//       <div class='card'>
//         <div class='card-header'>Debug console</div>
//         <div class='card-body'>
//           <div class="row">
//             <div class="col">
//               <div class="col-sm-12">Raw data :</div>
//               <textarea class="form-control col-sm-12 mb-2" id="console" rows="8"></textarea>
//               <button type="button" onclick="cmd('ClearConsole');document.getElementById('console').value=''" class="btn btn-outline-primary col-sm-12 col-md-6">Clear Console</button>
//             </div>
//           </div>
//         </div>
//       </div>
//     </div>
//   </div>

// </div>
// </div>
// <script language="javascript">logRefresh({{refreshLogs}});</script>
// )=====";

// const char HTTP_ABOUT[] PROGMEM = R"=====(
//     <div id='main' class='row justify-content-center mt-5'>
//         <!-- <div id='help_btns'>
//             <a href='#' class='btn btn-primary'><i class='glyphicon glyphicon-cog'></i>Primary</a>
//             <a href='#' class='btn btn-secondary'><i class='glyphicon glyphicon-file'></i>Secondary</a>
//             <a href='#' class='btn btn-success'><i class='glyphicon glyphicon-flag'></i>Success</a>
//             <a href='#' class='btn btn-danger'><i class='glyphicon glyphicon-lock'></i>Danger</a>
//             <a href='#' class='btn btn-warning'><i class='glyphicon glyphicon-tags'></i>Warning</a>
//         </div>
//         //
//         <script
//             src='https://cdn.jsdelivr.net/npm/@webcomponents/webcomponentsjs@2/webcomponents-loader.min.js'></script>
//         //
//         <script type='module' src='https://cdn.jsdelivr.net/gh/zerodevx/zero-md@1/src/zero-md.min.js'></script>
//         // <zero-md src='https://raw.githubusercontent.com/smlight-dev/SLZB-06/main/README.md'></zero-md> -->
//         <div class="col-sm-12 text-center">
//             <a href='http://smlight.tech/product/slzb-06/' target="_blank" style="text-decoration: none; color: link;"><h6><svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" class="bi bi-link-45deg" viewBox="0 0 16 16">
//                 <path d="M4.715 6.542 3.343 7.914a3 3 0 1 0 4.243 4.243l1.828-1.829A3 3 0 0 0 8.586 5.5L8 6.086a1.002 1.002 0 0 0-.154.199 2 2 0 0 1 .861 3.337L6.88 11.45a2 2 0 1 1-2.83-2.83l.793-.792a4.018 4.018 0 0 1-.128-1.287z"/>
//                 <path d="M6.586 4.672A3 3 0 0 0 7.414 9.5l.775-.776a2 2 0 0 1-.896-3.346L9.12 3.55a2 2 0 1 1 2.83 2.83l-.793.792c.112.42.155.855.128 1.287l1.372-1.372a3 3 0 1 0-4.243-4.243L6.586 4.672z"/>
//               </svg>Official web site with support page</h6></a>
//         </div>
//     </div>
// </div>

// </div>
// </div>
//     )=====";

// const char HTTP_ERROR[] PROGMEM =
//     "<h2>{{pageName}}</h2>"
//     "<div id='main' class='col-sm-9'>"
//     "</div>"
//     "</div>"
//     "</div>";
