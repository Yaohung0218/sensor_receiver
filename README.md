# sensor_receiver for Pure Data

Decodes serial bytes from a Sensor Receiver into named values that can be used directly in Pure Data. Supports macOS and Linux.

[Chinese version below](#中文版本)

## Requirements

- Pure Data
- The `[comport]` external
- A compatible Sensor and USB Receiver
- Git, make, and a C compiler

## Download and Installation

It is recommended to place `sensor_receiver` inside Pure Data's `externals` directory.

Open Terminal, enter your `externals` directory, then clone and compile the project:

```bash
cd /YOUR/PATH/TO/Pd/externals
git clone --recurse-submodules https://github.com/Yaohung0218/sensor_receiver.git
cd sensor_receiver
make
```

Replace `/YOUR/PATH/TO/Pd/externals` with the path used on your computer.

Compilation produces the external format for the current platform:

- macOS: `.pd_darwin`
- Linux: `.pd_linux`

Alternatively, add the `sensor_receiver` directory to **Pure Data → Preferences → Path** so that Pd can find the objects.

If `[comport]` is not installed, open **Help → Find externals**, search for `comport`, and install it.

## Quick Start

1. **Connect the Sensor and USB Receiver before opening Pure Data.**
2. Open `sensor-example.pd` for usage instructions and examples.
3. Use `[sensor-window YOUR_ID]` to view data from a specific Sensor.
4. In your own patch, use two `[route]` objects: the first selects the Sensor ID, and the second selects a named parameter such as `acc-z` or `gyro`.

## Serial Port

If no data is received, check that `[comport]` inside `sensor.pd` uses the USB Receiver's serial port and a baud rate of `115200`.

Common port names:

```text
macOS   /dev/cu.usbmodem*
Linux   /dev/ttyACM* or /dev/ttyUSB*
```

The `*` is a wildcard for the variable part of the device name. You may keep the wildcard or replace the entire pattern with the exact port name.

---

# 中文版本

[Back to English](#sensor_receiver-for-pure-data)

把 Sensor Receiver 傳來的 serial bytes 解碼成 Pure Data 可直接使用的具名數值。支援 macOS 與 Linux。

## 需要

- Pure Data
- `[comport]` external
- 可用的 Sensor 與 USB Receiver
- Git、make 及 C 編譯工具

## 下載與安裝

建議將 `sensor_receiver` 放在 Pure Data 的 `externals` 資料夾中。

開啟 Terminal，進入你的 `externals` 資料夾，再下載並編譯：

```bash
cd /YOUR/PATH/TO/Pd/externals
git clone --recurse-submodules https://github.com/Yaohung0218/sensor_receiver.git
cd sensor_receiver
make
```

請將 `/YOUR/PATH/TO/Pd/externals` 替換成你實際使用的路徑。

編譯後會產生對應平台的 external：

- macOS：`.pd_darwin`
- Linux：`.pd_linux`

也可以在 Pure Data 的 **Preferences → Path** 中加入 `sensor_receiver` 資料夾，讓 Pd 能找到其中的物件。

如果尚未安裝 `[comport]`，可透過 **Help → Find externals** 搜尋並安裝。

## Quick Start

1. **先連接 Sensor 與 USB Receiver，再開啟 Pure Data。**
2. 開啟 `sensor-example.pd`，查看使用說明與範例。
3. 使用 `[sensor-window 你的ID]` 查看對應 Sensor 的數據。
4. 若要在自己的 patch 中使用數據，可以串接兩個 `[route]`：第一個依 Sensor ID 篩選，第二個依參數名稱篩選，例如 `acc-z` 或 `gyro`。

## Serial Port

如果沒有收到數據，請確認 `sensor.pd` 中的 `[comport]` 使用 USB Receiver 對應的 serial port，baud rate 為 `115200`。

常見的 port 名稱：

```text
macOS   /dev/cu.usbmodem*
Linux   /dev/ttyACM* 或 /dev/ttyUSB*
```

其中 `*` 是裝置名稱可變部分的 wildcard。可以保留 wildcard，也可以將整個 pattern 換成完整的 port 名稱。
