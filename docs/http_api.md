# HTTP API

!!! warning "TO DO - WRITE DESCRIPTION"

!!! tip "http://xzg.local/api?action=X"
    | action ID | Description         |
    | --------- | ------------------- |
    | 0         | API_GET_PAGE        |
    | 1         | API_GET_PARAM       |
    | 2         | API_STARTWIFISCAN   |
    | 3         | API_WIFISCANSTATUS  |
    | 4         | API_GET_FILELIST    |
    | 5         | API_GET_FILE        |
    | 6         | API_SEND_HEX        |
    | 7         | API_WIFICONNECTSTAT |
    | 8         | API_CMD             |
    | 9         | API_GET_LOG         |
    | 10        | API_DEL_FILE        |
    | 11        | API_FLASH_ZB        |

!!! question  "http://xzg.local/api?action=1&param=X"
    | param       | Description                                     |
    | ----------- | ----------------------------------------------- |
    | refreshLogs | Refresh interval                                |
    | zbFwVer     | Zigbee firmware version                         |
    | zbHwVer     | Zigbee hardware revision                        |
    | espVer      | ESP32 firmware version                          |
    | wifiEnable  | WiFi enabled status                             |
    | coordMode   | Working mode (Network (0) or USB (1))           |
    | all         | All gateway configuration params in JSON object |
    | vars        | All gateway variables in JSON object            |
    | update_root | Realtime updating values                        |
    | root        | Values to update on root page load              |

!!! question  "http://xzg.local/api?action=8&cmd=X"
    | cmd ID | Description         |
    | ------ | ------------------- |
    | 0      | CMD_ZB_ROUTER_RECON |
    | 1      | CMD_ZB_RST          |
    | 2      | CMD_ZB_BSL          |
    | 3      | CMD_ESP_RES         |
    | 4      | CMD_ADAP_LAN        |
    | 5      | CMD_ADAP_USB        |
    | 6      | CMD_LED_ACT         |
    | 7      | CMD_ZB_FLASH        |
    | 8      | CMD_CLEAR_LOG       |
    | 9      | CMD_ESP_UPD_URL     |
    | 10     | CMD_ZB_CHK_FW       |
    | 11     | CMD_ZB_CHK_HW       |
    | 12     | CMD_ZB_LED_TOG      |
    | 13     | CMD_ESP_FAC_RES     |
    | 14     | CMD_ZB_ERASE_NVRAM  |

!!! danger "D!"

http://xzg.local/api?action=5&filename=configHw.json


!!! tip "Pages"

`http://xzg.local/api?action=0&page=X`
    | page ID | Name              |
    | ------- | ----------------- |
    | 0       | root              |
    | 1       | General settings  |
    | 2       | Network settings  |
    | 3       | API_PAGE_NETWORK  |
    | 4       | API_PAGE_ZIGBEE   |
    | 5       | API_PAGE_SECURITY |
    | 6       | API_PAGE_TOOLS    |
    | 7       | API_PAGE_ABOUT    |
    | 8       | API_PAGE_MQTT     |
    | 9       | API_PAGE_VPN      |
