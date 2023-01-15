#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <ws2ipdef.h>
#include <stdlib.h>
#include <thread>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;

// Split string to tokens and append them to a vector
static vector<string> split(const string& str, char delimiter = ' ', bool allow_empty = false) {
	vector<string> tokens;
	stringstream sstream(str);
	string token;

	while (getline(sstream, token, delimiter)) {
		if (allow_empty || token.size() > 0) {
			tokens.push_back(token);
		}
	}
	return tokens;
}

// Converts a string to an integer using stringstream
static int string_to_int(const string& string)
{
	stringstream sstream(string);
	int i;
	sstream >> i;
	return i;
}

// Swaps two values
template <typename T>
static void Swap(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}

// Generates a vector containing a range of values
template <typename T>
static vector<T> range(T min, T max)
{
	vector<T> values;
	if (min > max)
		Swap(min, max);
	if (min == max)
		return vector<T>(1, min);
	for (; min <= max; ++min)
		values.push_back(min);
	return values;
}

// Parses a list of ports containing numbers and ranges. For example: 1-1000,9999,8080
static vector<int> parse_ports_list(const string& list)
{
	vector<int> ports;
	// Split list items using Ranged Based for Loop: for (variable : collection) -> Add every value in collection to variable one by one
	for (const string& token : split(list, ',')) {
		// Split ranges.
		vector<string> strrange = split(token, '-');
		switch (strrange.size()) {

			// Case with one port
		case 0:
			ports.push_back(string_to_int(token));
			break;
		case 1:
			ports.push_back(string_to_int(strrange[0]));
			break;

			// Case with 2 or more ports
		case 2:
		{
			int min = string_to_int(strrange[0]),
				max = string_to_int(strrange[1]);
			for (int port : range(min, max))
				ports.push_back(port);
			break;
		}

		default:
			break;
		}
	}
	return ports;
}

// Print error from WSAGetLastError()
void get_error() 
{
	wchar_t* s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	wcout << s;
	LocalFree(s);
}

// Check for open ports
bool port_is_open(char* ip, int port) 
{
	SOCKADDR_IN serv;
	SOCKET sock = SOCKET_ERROR;
	fd_set fdset;
	timeval tv;
	u_long mode = 1;

	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serv.sin_addr.s_addr);

	// Enable socket's non-blocking mode and make a connection to server
	sock = socket(AF_INET, SOCK_STREAM, 0);
	ioctlsocket(sock, FIONBIO, &mode);
	connect(sock, (SOCKADDR*)&serv, sizeof(serv));

	// Check connect() result
	FD_ZERO(&fdset);
	FD_SET(sock, &fdset);
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1) {
		int so_error;
		socklen_t size = sizeof(so_error);

		getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&so_error, &size);
		if (so_error == 0) {
			cout << "Port " << port << " is open\n";
			closesocket(sock);
			return true;
		}
	}

	closesocket(sock);
	return false;
}

int main(int argc, char* argv[]) 
{
	SOCKADDR_IN test;
	char* target_ip;
	string port_range;
	WSADATA wsa;

	cout << "SIMPLE PORT SCANNER\n\n";

	// Check user input
	if (argc != 3) {
		cerr << "Usage: " << argv[0] << " address port(s)\n"
			<< "Examples:\n"
			<< "\t" << argv[0] << " localhost 80,8080\n"
			<< "\t" << argv[0] << " scanme.nmap.org 80,8080\n"
			<< "\t" << argv[0] << " 192.168.1.10 0-65535\n"
			<< "\t" << argv[0] << " example.com 0-100,80,8080\n";
		exit(0);
	}

	// Validate IP address
	cout << "Initializing\n";
	if (inet_pton(AF_INET, argv[1], &test.sin_addr.s_addr) == 1) {
		target_ip = argv[1];
		port_range = argv[2];
		cout << "Target IP: " << target_ip << endl;
	}
	else {
		cout << "Invalid IP address. Please try again...\n";
		exit(0);
	}

	vector<int> port_list = parse_ports_list(port_range);

	// Load Winsock DLL
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cout << "Failed to load Winsock DLL: ";
		get_error();
		exit(0);
	}
	else {
		cout << "Loaded Winsock DLL\n";
	}

	// Start looping to check for open ports
	// Scan time - start
	char str1[26];
	auto start = chrono::system_clock::now();
	time_t start_time = chrono::system_clock::to_time_t(start);
	ctime_s(str1, sizeof(str1), &start_time);
	cout << "Scanner started at: " << str1;

	vector<thread*> tasks;	// Thread vector
	for (int port : port_list) {
		tasks.push_back(new thread(port_is_open, target_ip, port));
	}

	for (int i = 0; i < tasks.size(); i++) {
		tasks[i]->join();
		delete tasks[i];
	}

	// Scan time - end
	char str2[26];
	auto end = chrono::system_clock::now();
	time_t end_time = chrono::system_clock::to_time_t(end);
	chrono::duration<double> elapsed_time = end - start;
	ctime_s(str2, sizeof(str2), &end_time);
	cout << "Scanner finished at: " << str2;
	cout << "Scanning time: " << elapsed_time.count() << "s" << endl;

	WSACleanup();
	return 0;
}
