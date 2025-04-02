#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>

const int ENTRY_LINES = 2;
const int LINES_BETWEEN_ENTRIES = 1;
const int DEPARTURE_LABEL_LENGTH = 11; // "Departure:_" = 11 chars
const int ARRIVAL_LABEL_LENGTH = 11;   // "Arrival:___" = 11 chars

const int TIME_CHARACTER_LENGTH = 19;
const int DEPARTURE_TIME_INDEX = 42;
const int ARRIVAL_TIME_INDEX = 61 + 42 + 1;

int main()
{
	std::ifstream inputFile;
	inputFile.open("input.txt");

	int answer = 0;

	while (!inputFile.eof())
	{
		std::string entry;
		std::string line;
		for (int i = 0; i < ENTRY_LINES; i++)
		{
			getline(inputFile, line);
			entry = entry + line + '\n';
		}
		for (int i = 0; i < LINES_BETWEEN_ENTRIES; i++)
			getline(inputFile, line);

		
		std::cout << entry;

		size_t departureLocationBegin = entry.find("Departure:") + DEPARTURE_LABEL_LENGTH;
		size_t departureLocationEnd = entry.find(' ', departureLocationBegin);
		std::string departureLocationString = entry.substr(departureLocationBegin, departureLocationEnd - departureLocationBegin);
		std::string departureTimeString = entry.substr(DEPARTURE_TIME_INDEX, TIME_CHARACTER_LENGTH);
		std::istringstream departureSS(departureTimeString);
		std::tm departureTimeStruct = {};
		departureSS >> std::get_time(&departureTimeStruct, "%b %d, %Y, %H:%M");
		std::time_t departureTime_t = std::mktime(&departureTimeStruct);

		size_t arrivalLocationBegin = entry.find("Arrival:") + ARRIVAL_LABEL_LENGTH;
		size_t arrivalLocationEnd = entry.find(' ', arrivalLocationBegin);
		std::string arrivalLocationString = entry.substr(arrivalLocationBegin, arrivalLocationEnd - arrivalLocationBegin);
		std::string arrivalTimeString = entry.substr(ARRIVAL_TIME_INDEX, TIME_CHARACTER_LENGTH);
		std::istringstream arrivalSS(arrivalTimeString);
		std::tm arrivalTimeStruct = {};
		arrivalSS >> std::get_time(&arrivalTimeStruct, "%b %d, %Y, %H:%M");
		std::time_t arrivalTime_t = std::mktime(&arrivalTimeStruct);

		std::cout << departureLocationString << " (" << departureTimeString << ")" << " --> " << arrivalLocationString << " (" << arrivalTimeString << ")" << std::endl;

		const std::chrono::time_zone* departureZone = std::chrono::get_tzdb().locate_zone(departureLocationString);
		const std::chrono::zoned_time departureTime{ departureZone, std::chrono::system_clock::from_time_t(departureTime_t) };
		auto departureSysTime = departureTime.get_sys_time();
		std::chrono::sys_time<std::chrono::minutes> departureSysTimeInMinutes = std::chrono::time_point_cast<std::chrono::minutes>(departureSysTime);
		std::cout << "Departure system time (minutes since epoch): "
			<< departureSysTimeInMinutes.time_since_epoch().count() << " minutes" << std::endl;


		const std::chrono::time_zone* arrivalZone = std::chrono::get_tzdb().locate_zone(arrivalLocationString);
		const std::chrono::zoned_time arrivalTime{ arrivalZone, std::chrono::system_clock::from_time_t(arrivalTime_t)};
		auto arrivalSysTime = arrivalTime.get_sys_time();
		std::chrono::sys_time<std::chrono::minutes> arrivalSysTimeInMinutes = std::chrono::time_point_cast<std::chrono::minutes>(arrivalSysTime);
		std::cout << "Arrival system time (minutes since epoch): "
			<< arrivalSysTimeInMinutes.time_since_epoch().count() << " minutes" << std::endl;

		auto sysTimeDifference = arrivalSysTimeInMinutes - departureSysTimeInMinutes;

		std::cout << "Departure/Arrival minutes difference: " << sysTimeDifference.count() << std::endl;
		answer += sysTimeDifference.count();

		std::cout << std::endl;
	}

	std::cout << std::endl << "Sum of travel time: " << answer + 60 << std::endl;

	inputFile.close();
	return 0;
}