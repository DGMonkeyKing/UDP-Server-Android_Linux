#UDP Server for Android and Linux
This is an UDP Server that allows to connect Linux and Android via IP direction. In this case, I going to show you how to stablish the Android device as the host and the Linux device as the guest

##First of all...
First thing you must do is to download or to clone this project. There are 2 _**.c**_ files:
* _**modlist.c**_ file is used as an example for the P2P connection. Within the Makefile, this code generate a _**.ko**_ file (a module for the kernel) which will interact with the host application.
* _**servUDPmod.c**_ file is the server itself. With the proper compilation can be used in Android architecture x86 or in Linux.

##Compilation
For develope this app, I used a Virtual Machine with Android x86 and Linux Debian 7. I also used Android NDK to compile 

Once you have the project in your PC, use the _make_ directive to compile the module and the gcc compilator to compile the host aplication for Android.
* **Host application (Android)**

	`gcc -m32 -static -g -Wall servUDPmod.c -o udp-android`

* **Host application (Linux)**

	`gcc -o udp servUDPmod.c`

The result should be something like this:

<img src="https://raw.githubusercontent.com/DGMonkeyKing/UDP-Server-Android_Linux/master/img/COMP.png">

##Connecting with Android
You must know the IP of your Android device. With a shell bash emulator and with the command `ip addr show` you can know your IP address:

<img src="https://raw.githubusercontent.com/DGMonkeyKing/UDP-Server-Android_Linux/master/img/IP.png">

Once you have the IP, connect to the Android device with `adb` command utility of Linux **(If your trying to connect 2 Linux devices this step won't be neccesary, just skip it)**

<img src="https://raw.githubusercontent.com/DGMonkeyKing/UDP-Server-Android_Linux/master/img/CONNECT.png">

We need to connect to the IP address of the Android machine and become the superuser as I do in the image above. Once we have the superuser permissions, a re-connect must be done.

With `adb push <file> <path>` I copy the file to the path specified in the Android file system.

##Running
With `adb shell` we take control of the shell of Android. Go to the path where you copied the files and then load the module _**modlist.ko**_ with the command `insmod <module>`.

Is time to run the host and the guest application.

In the Linux device, run this command on the shell to execute the guest:

`nc -u <IP address> <port>`	-	EJ: nc -u 192.168.21.131 3000

In the Android device, run the **udp-android** on the shell to execute the host:

`./udp-android <IP address> <port>`	-	EJ: ./udp-android 192.168.21.131 3000

And know just write the command in the guest application. The commands allowed (at this moment) are:

* _**add (int)**_ = Adds an element in _/proc/multilist/default_
* _**remove (int)**_ = Remove an element in _/proc/multilist/default_
* _**cleanup**_ = Remove all the elements in _/proc/multilist/default_
* _**create (string)**_ = Create a new entry on _/proc/multilist_ called string
* _**delete (string)**_ = Delete an old entry on _/proc/multilist_ called string

<img src="https://raw.githubusercontent.com/DGMonkeyKing/UDP-Server-Android_Linux/master/img/MESSAGES.png">

Now to know what has been load in the /proc module just execute this command:

`cat /proc/multilist/default`

##License
_**All rights reserved 2016**_
