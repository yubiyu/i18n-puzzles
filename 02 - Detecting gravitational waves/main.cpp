#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

const int YEAR_INDEX = 0;
const int YEAR_SIZE = 4;

const int MONTH_INDEX = 5;
const int MONTH_SIZE = 2;

const int DATE_INDEX = 8;
const int DATE_SIZE = 2;

const int HOUR_INDEX = 11;
const int HOUR_SIZE = 2;

const int MINUTE_INDEX = 14;
const int MINUTE_SIZE = 2;

const int SECOND_INDEX = 17;
const int SECOND_SIZE = 2;

const int TIMEZONE_SIGN_INDEX = 19;
const int TIMEZONE_SIGN_SIZE = 1;

const int TIMEZONE_HOUR_INDEX = 20;
const int TIMEZONE_HOUR_SIZE = 2;

const int TIMEZONE_MINUTE_INDEX = 23;
const int TIMEZONE_MINUTE_SIZE = 2;

std::vector<std::string>timestamps;
std::map<int, int>detectionTally;

int main()
{
    std::ifstream inputFile;
    inputFile.open("input.txt");

    std::string line;

    std::string year;
    std::string month;
    std::string date;

    std::string hour;
    std::string minute;
    std::string second;

    std::string timezoneSign;
    std::string timezoneHour;
    std::string timezoneMinute;

    while (getline(inputFile, line))
    {
        timestamps.push_back(line);

        long t = 0;
        long tZoneAdjust = 0;

        year = line.substr(YEAR_INDEX, YEAR_SIZE);
        t += (stol(year) - 2000) * 31536000;

        month = line.substr(MONTH_INDEX, MONTH_SIZE);
        t += stol(month) * 2628288;

        date = line.substr(DATE_INDEX, DATE_SIZE);
        t += stol(date) * 86400;

        hour = line.substr(HOUR_INDEX, HOUR_SIZE);
        t += stol(hour) * 3600;

        minute = line.substr(MINUTE_INDEX, MINUTE_SIZE);
        t += stol(minute) * 60;

        second = line.substr(SECOND_INDEX, SECOND_SIZE);
        t += stol(second);

        timezoneSign = line.substr(TIMEZONE_SIGN_INDEX, TIMEZONE_SIGN_SIZE);
        timezoneHour = line.substr(TIMEZONE_HOUR_INDEX, TIMEZONE_HOUR_SIZE);
        tZoneAdjust += stol(timezoneHour) * 3600;
        timezoneMinute = line.substr(TIMEZONE_MINUTE_INDEX, TIMEZONE_MINUTE_SIZE);
        tZoneAdjust += stol(timezoneMinute) * 60;
        if (timezoneSign == "+") // Kinda funky, isn't it?
            tZoneAdjust *= (-1);

        t += tZoneAdjust;

        std::cout << line << " -- " << year << " " << month << " " << date << " "
            << hour << " " << minute << " " << second << " "
            << timezoneSign << " " << timezoneHour << " " << timezoneMinute << " -- "
            << "t=" << t;

        if (t == 659168280) // Hacking by plugging in the discovered value afterward
            std::cout << " --- BEEP BEEP BEEP ";
        std::cout << std::endl;

        if (detectionTally.count(t) == 0)
            detectionTally[t] = 1;
        else
            detectionTally[t]++;
    }

    std::cout << std::endl << "Wave detection tally: " << std::endl;
    for (std::map<int, int>::iterator it = detectionTally.begin(); it != detectionTally.end(); ++it)
    {
        std::cout << "t of " << it->first << " detected " << it->second << " times.";

        if (it->second >= 4)
            std::cout << " -- BEEP BEEP BEEP BEEP";

        std::cout << std::endl;
    }

    inputFile.close();
    return 0;
}
