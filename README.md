# PlatformIO HomeKit WS2812b

## Snippets

### Attach USB device

```shell
usbipd list

usbipd bind --busid 1-4

usbipd list

usbipd attach --wsl --busid 1-4
```

## Links

### WSL connect USB device

https://learn.microsoft.com/en-us/windows/wsl/connect-usb

### Fix for HomeKit-ESP8266 and WiFiMamanger

https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266/commit/ae4780e05cc1faa424baf9beb4aa96a0d181f896#diff-8cd0b8a39470a4f4283e552dfc18eb08c74c97cb455c83d7fb7e9d9dedd55535
