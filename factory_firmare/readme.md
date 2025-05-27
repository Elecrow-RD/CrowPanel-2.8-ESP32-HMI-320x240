# How to install the factory demo firmware with flash download tool?

#### 1.Preparation:

- USB C cable
- Laptop/PC
- Crowpanel 2.8 HMI ESP32 Display
- Flash download tool

Check whether the materials on the PCBA board are defective. Such as less material, false welding, tin and other bad phenomena.

#### 2.Flash the firmware 

##### Step1 Connect the ESP32 display to the laptop/PC with USB-C cable. The power indicator will light up.

![flash01](./flash01.jpg)

##### Step2 Open the flash download tool. 

![flash02](./flash02.jpg)



For the  Crowpanel 2.8 HMI ESP32 Display, the chip select ESP32:

![firmware1](https://github.com/user-attachments/assets/55887bd4-7fe5-423c-89c1-9548dbfdcad2)



##### Step3

①Choose the firmware files;

![flash0](./flash0.jpg)

②Enter the address: 

boot_app0.bin------0xe000

xxxx.ino.bin--------0x10000

xxxx.ino.bootloader.bin------0x1000

xxxx.ino.partitions.bin-------0x8000



③Check the firmware files;

④Select the correct port number;

⑤Click “Start” to download

![firmware2](https://github.com/user-attachments/assets/83ce3c19-dd1d-4759-b0d0-c0099193a902)

### Note: If it is v1.0 version, before downloading firmware or program, you need to press the boot button, then short press the reset button, and then release the boot button to enter the burn mode. If it is v2.0 or above, this step is not needed, it will automatically enter into burn mode.


##### Step5 After the download is complete, blue FINISH will be displayed, and the progress bar at the bottom shows that it has come to an end.

![firmware3](https://github.com/user-attachments/assets/80c630c0-1905-4640-a14b-00ea490a871d)



##### Step6 Press the reset button to show the demo.

![flash8](./flash8.jpg)
