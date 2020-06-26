# usbloader
>a simple tool to burn Hisilicon bootloader (4G, 5G series modems)

## introduce
代码根据usb抓包逆向而获取。海思私有协议的实现.仅用于测试交流，不用于商业目的！
(The code is an implementation of Hisilicon private protocol which comes from the analyzation of USBMON. Anyone can use this but not for commercial purposes)

## what to do next?
很重要！ 在烧写完bootloader后，modem将转为fastboot模式，然后用fastboot继续烧写！
(Much important! The modem will switch into fastboot mode after burning bootloader, so we can continue with fastboot tool!)

## operation log
```bash
sh-5.0# ./usbloader usbloader.bin /dev/ttyUSB0
[2020-06-22 11:45:41] info: bootloader: usbloader.bin, device /dev/ttyUSB0
[2020-06-22 11:45:41] info: try to burn bootloader(usbloader.bin)
[2020-06-22 11:45:41] info: try to parser file header
[2020-06-22 11:45:41] info: raminit.bin
[2020-06-22 11:45:41] info: 	pad0 0	index 1	length 100684(1894c)	magic 0(0)
[2020-06-22 11:45:41] info: onchip.bin
[2020-06-22 11:45:41] info: 	pad0 84	index 2	length 706688(ac880)	magic 49278976(2eff000)
[2020-06-22 11:45:41] info: try to burn raminit.bin
[2020-06-22 11:45:41] info: ..................................................................................................
[2020-06-22 11:45:41] info: status: 100684/100684
[2020-06-22 11:45:41] info: burn_raminit finished
[2020-06-22 11:45:41] info: try to burn onchip.bin
[2020-06-22 11:45:42] info: ..................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................
[2020-06-22 11:45:42] info: status: 706689/706689
[2020-06-22 11:45:42] info: burn_onchip finished
```
