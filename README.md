# sensor_receiver for Pure Data

把 Sensor Receiver 傳來的 serial bytes 解碼成 Pure Data 可直接使用的具名數值。支援 macOS 與 Linux。

## 需要

- Pure Data
- `[comport]` external
- 可用的 Sensor 與 USB Receiver

## 下載與編譯

開啟 Terminal，進入你想安裝 external 的位置：

```bash
cd /YOUR/PATH
git clone --recurse-submodules https://github.com/Yaohung0218/sensor_receiver.git
cd sensor_receiver
make
```

編譯結果依作業系統不同：

- macOS：`.pd_darwin`
- Linux：`.pd_linux`

## Serial port

在 `sensor.pd` 裡設定 Receiver 的 port：

```pd
[comport /dev/cu.usbmodem* 115200]
```

常見名稱：

```text
macOS   /dev/cu.usbmodem*
Linux   /dev/ttyACM* 或 /dev/ttyUSB*
```
