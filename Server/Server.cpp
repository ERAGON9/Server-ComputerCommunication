#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <string>
#include <windows.h>
#include <map>

#define TIME_PORT	27015
#define SUMMER_CLOCK "1"
#define WINTER_CLOCK "0"
#define DOHA "12-Doha"
#define PRAGUE "12-Prague"
#define NEW_YORK "12-New-York"
#define BERLIN "12-Berlin"

void main()
{
	// Initialize Winsock (Windows Sockets).
	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	// Server side:
	// Create and bind a socket to an internet address.
	// After initialization, a SOCKET object is ready to be instantiated.
	// Create a SOCKET object called m_socket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							datagram sockets (SOCK_DGRAM), 
	//							and the UDP/IP protocol (IPPROTO_UDP).
	SOCKET m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The "if" statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == m_socket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a server to communicate on a network, it must first bind the socket to 
	// a network address.
	// Need to assemble the required data for connection in sockaddr structure.
	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigdned long (4 bytes) data type).
	// INADDR_ANY means to listen on all interfaces.
	// inet_addr (Internet address) is used to convert a string (char *) into unsigned int.
	// inet_ntoa (Internet address) is the reverse function (converts unsigned int to char *)
	// The IP address 127.0.0.1 is the host itself, it's actually a loop-back.
	serverService.sin_addr.s_addr = INADDR_ANY;	//inet_addr("127.0.0.1");
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order (which is big-endian).
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.
	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(m_socket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(m_socket);
		WSACleanup();
		return;
	}

	// Waits for incoming requests from clients.

	// Send and receive data.
	sockaddr client_addr;
	int client_addr_len = sizeof(client_addr);
	int bytesSent = 0;
	int bytesRecv = 0;
	char sendBuff[255];
	char recvBuff[255];

	// Get client's requests and answer them.
	// The recvfrom function receives a datagram and stores the source address.
	// The buffer for data to be received and its available size are returned by recvfrom. 
	// The fourth argument is an idicator specifying the way in which the call is made (0 for default).
	// The two last arguments are optional and will hold the details of the client for further communication. 
	// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
	cout << "Time Server: Wait for clients' requests.\n\n";

	// My variables.
	map<pair<string, unsigned short>, int> clientMap;

	while (true)
	{
		bytesRecv = recvfrom(m_socket, recvBuff, 255, 0, &client_addr, &client_addr_len);
		if (SOCKET_ERROR == bytesRecv)
		{
			cout << "Time Server: Error at recvfrom(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return;
		}

		recvBuff[bytesRecv] = '\0'; // Add the null-terminating to make it a string.
		cout << "Time Server: Received: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";

		time_t timer;
		time(&timer);
		struct tm* timeinfo = localtime(&timer); // UTC+2 (Israel time.)

		int year = 1900 + timeinfo->tm_year;
		int month = timeinfo->tm_mon + 1;
		int dayOfMonth = timeinfo->tm_mday;
		int dayOfYear = timeinfo->tm_yday + 1;
		int hours = timeinfo->tm_hour;
		int minutes = timeinfo->tm_min;
		int seconds = timeinfo->tm_sec;

		// Answer client's request by the Client number request.
		if (strcmp(recvBuff, "1") == 0) // GetTime
		{
			strcpy(sendBuff, ctime(&timer)); // ctime => Parse the current time(TimeSinceEpoch) to printable string.
			sendBuff[strlen(sendBuff) - 1] = '\0'; //to remove the new-line from the created string
		}
		else if (strcmp(recvBuff, "2") == 0) // GetTimeWithoutDate
		{
			string timeDisplay = to_string(hours) + ":" + to_string(minutes) + ":" + to_string(seconds);
			strcpy(sendBuff, timeDisplay.c_str());
		}
		else if (strcmp(recvBuff, "3") == 0) // GetTimeSinceEpoch
		{
			string timeDisplay = to_string((int)timer) + " seconds since Epoch";
			strcpy(sendBuff, timeDisplay.c_str());
		}
		else if (strcmp(recvBuff, "4") == 0) // GetClientToServerDelayEstimation
		{
			DWORD currentTime = GetTickCount();
			string timeDisplay = to_string(currentTime);
			strcpy(sendBuff, timeDisplay.c_str());
		}
		else if (strcmp(recvBuff, "5") == 0) // MeasureRTT
		{
			string timeDisplay = "Request goten at the server";
			strcpy(sendBuff, timeDisplay.c_str());
		}
		else if (strcmp(recvBuff, "6") == 0) // GetTimeWithoutDateOrSeconds
		{
			string timeDisplay = to_string(hours) + ":" + to_string(minutes);
			strcpy(sendBuff, timeDisplay.c_str());
		}
		else if (strcmp(recvBuff, "7") == 0) // GetYear
		{
			string timeDisplay = to_string(year);
			strcpy(sendBuff, timeDisplay.c_str());
		}
		else if (strcmp(recvBuff, "8") == 0) // GetMonthAndDay
		{
			string timeDisplay = to_string(month) + "/" + to_string(dayOfMonth);
			strcpy(sendBuff, timeDisplay.c_str());
		}
		else if (strcmp(recvBuff, "9") == 0) // GetSecondsSinceBeginingOfMonth
		{
			int secondsPassed = ((dayOfMonth - 1) * 24 * 60 * 60 ) + (hours * 60 * 60) + (minutes * 60) + seconds;
			string timeDisplay = to_string(secondsPassed);
			strcpy(sendBuff, timeDisplay.c_str());
		}
		else if (strcmp(recvBuff, "10") == 0) // GetWeekOfYear
		{
			string timeDisplay = to_string(1 + (dayOfYear / 7)); // Adding one because starting from week 1.
			strcpy(sendBuff, timeDisplay.c_str());
		}
		else if (strcmp(recvBuff, "11") == 0) // GetDaylightSavings
		{
			if (month >= 3 && dayOfMonth >= 29 && hours >= 2 && minutes >= 0 && seconds >= 0) // 29/03 2:00-> 27/10 2:00 => Summer Clock
			{
				if (month <= 10 && dayOfMonth <= 27 && hours <= 2)
					strcpy(sendBuff, SUMMER_CLOCK); // Summer Clock
			}
			else
				strcpy(sendBuff, WINTER_CLOCK); // Winter Clock
		}
		else if ((recvBuff[0] == '1') && (recvBuff[1] == '2')) // GetTimeWithoutDateInCity
		{
			if (strcmp(recvBuff, DOHA) == 0)
			{
				timeinfo->tm_hour += 1;   // Doha -> UTC+3 // My local time (ISR) is UTC+2
				mktime(timeinfo);

				string timeDisplay = to_string(timeinfo->tm_hour) + ":" + to_string(timeinfo->tm_min) + ":" + to_string(timeinfo->tm_sec);
				strcpy(sendBuff, timeDisplay.c_str());
			}
			else if (strcmp(recvBuff, PRAGUE) == 0) 
			{
				timeinfo->tm_hour -= 1;   // Prague -> UTC+1 // My local time (ISR) is UTC+2
				mktime(timeinfo);

				string timeDisplay = to_string(timeinfo->tm_hour) + ":" + to_string(timeinfo->tm_min) + ":" + to_string(timeinfo->tm_sec);
				strcpy(sendBuff, timeDisplay.c_str());
			}
			else if (strcmp(recvBuff, NEW_YORK) == 0)
			{
				timeinfo->tm_hour -= 7;   // New York -> UTC-5 // My local time (ISR) is UTC+2
				mktime(timeinfo);

				string timeDisplay = to_string(timeinfo->tm_hour) + ":" + to_string(timeinfo->tm_min) + ":" + to_string(timeinfo->tm_sec);
				strcpy(sendBuff, timeDisplay.c_str());
			}
			else if (strcmp(recvBuff, BERLIN) == 0)
			{
				timeinfo->tm_hour -= 1;   // Berlin -> UTC+1 // My local time (ISR) is UTC+2
				mktime(timeinfo);

				string timeDisplay = to_string(timeinfo->tm_hour) + ":" + to_string(timeinfo->tm_min) + ":" + to_string(timeinfo->tm_sec);
				strcpy(sendBuff, timeDisplay.c_str());
			}
			else // Other cities
			{
				timeinfo->tm_hour -= 2;   // Other -> UTC // My local time (ISR) is UTC+2
				mktime(timeinfo);

				string timeDisplay = "This city not at the list, global UTC is: " + to_string(timeinfo->tm_hour) + ":" + to_string(timeinfo->tm_min) + ":" + to_string(timeinfo->tm_sec);
				strcpy(sendBuff, timeDisplay.c_str());
			}
		}
		else if (strcmp(recvBuff, "13") == 0) // MeasureTimeLap
		{
			string clientIP = inet_ntoa(((struct sockaddr_in*)&client_addr)->sin_addr);
			unsigned short clientPort = ntohs(((struct sockaddr_in*)&client_addr)->sin_port);

			auto iterator = clientMap.find(make_pair(clientIP, clientPort));
			if (iterator == clientMap.end()) // If client information doesn't exist, add it to the map.
			{
				clientMap[make_pair(clientIP, clientPort)] = (int)timer;
				strcpy(sendBuff, "Measurement started!");
			}
			else // If client information exists.
			{
				if ((int)timer - iterator->second > 180) // 3 minutes = 180 seconds.
				{
					clientMap[make_pair(clientIP, clientPort)] = (int)timer;
					strcpy(sendBuff, "Measurement started!");
				}
				else // Time past less than 3 minutes.
				{
					int timeElapsed = (int)timer - iterator->second;
					sprintf(sendBuff, "Time elapsed since last request: %d seconds", timeElapsed);
					clientMap[make_pair(clientIP, clientPort)] = 0;
					// Update the stored time at Epoch for this client to be more than 180 secounds, therefore next request will restart the measurement.
				}
			}
		}
		else
		{
			string timeDisplay = "Eror try again.";
			strcpy(sendBuff, timeDisplay.c_str());
		}

		
		// Sends the answer to the client, using the client address gathered by recvfrom. 
		bytesSent = sendto(m_socket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr*)&client_addr, client_addr_len);
		if (SOCKET_ERROR == bytesSent)
		{
			cout << "Time Server: Error at sendto(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return;
		}

		cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n\n";
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(m_socket);
	WSACleanup();
}