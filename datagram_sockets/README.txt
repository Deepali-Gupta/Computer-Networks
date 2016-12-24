DATAGRAM PACKET COMMUNICATION
------------------------------

This submission is part of HW1 of the Computer Networks Course.

Attached in folder:
-client.c : client program
-echo.c : server program
-Makefile
-bin : contains files for custom commands of -h option
-exp : file to run export command through make to enable -h
-Report : PDF report of the entire Homework1
-README
-T2000 : Folder containing RTT values for part (a), T=2000
-T8000 : Folder containing RTT values for part (a), T=8000
-T2000b : Folder containing RTT values for part (b), T=2000
-T2000b : Folder containing RTT values for part (b), T=8000

make client : To compile client.c with executable as client
		Run ./client to run
		Pass arguments as follows:
		./client [Value of P] [Value of T] [File Name] [Server IP]
make echo : To compile echo.c with executable as server
		Run ./server to run
make clean : Deletes the executables created in the folder
client -h : Diplays usage of client.c
echo -h : Displays usage of echo.c
If -h options show error, run: export PATH=$PATH":$(pwd)/bin"
