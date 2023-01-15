/*
	TCP Port Scanner using SFML - Simple and Fast Multimedia Library
*/

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include "SFML/Network.hpp"

using namespace std;

// Check whether port is open or not
static bool is_open(const string& address, int port) {
	sf::Time t = sf::microseconds(1);
	return (sf::TcpSocket().connect(address, port, t) == sf::TcpSocket::Done);
}

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

// Get max value of a vector
template <typename T>
static T max_value(const vector<T>& values)
{
	T max = values[0];
	for (T value : values) {
		if (value > max)
			max = value;
	}
	return max;
}

// Count number's digit
template <typename T>
static size_t digits(T value)
{
	size_t count = (value < 0) ? 1 : 0;
	if (value == 0)
		return 0;
	while (value) {
		value /= 10;
		++count;
	};
	return count;
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

int main(int argc, char* argv[])
{
	string ip_addr, port_list;
	vector<int> ports;

	// Get ip address and range of port from user
	cout << "Simple Port Scanner\n";
	cout << "IP Address: ";
	getline(cin, ip_addr);
	cout << "Port (range): ";
	getline(cin, port_list);
	cout << "Scanning open ports on " << ip_addr << " ...\n";

	// Check all ports in port list
	ports = parse_ports_list(port_list);
	size_t width = digits(max_value(ports));

	for (int port : ports) {
		if (is_open(ip_addr, port)) {
			cout << "Port " << setw(width) << port << ": OPEN\n";
		}
	}

	return 0;
}
