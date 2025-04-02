#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <chrono>
#include <algorithm>

std::chrono::system_clock::time_point ConvertLocalToUTC(
    const std::chrono::system_clock::time_point& local_time_point,
    const std::chrono::time_zone* timezone)
{
    if (timezone == nullptr) {
        throw std::invalid_argument("Timezone cannot be null.");
    }

    // Cast the local_time_point to a std::chrono::local_time object
    auto local_time = std::chrono::local_time<std::chrono::system_clock::duration>(
        local_time_point.time_since_epoch());

    // Convert to UTC
    try {
        return timezone->to_sys(local_time);
    }
    catch (const std::chrono::nonexistent_local_time& e) {
        std::cerr << "Nonexistent local time encountered: " << e.what() << '\n';
        // Adjust forward by 1 hour (or choose a suitable resolution strategy)
        auto adjusted_local_time = local_time + std::chrono::hours(1);
        return timezone->to_sys(adjusted_local_time);
    }
    catch (const std::chrono::ambiguous_local_time& e) {
        std::cerr << "Ambiguous local time encountered: " << e.what() << '\n';
        // Resolve ambiguity by choosing the earliest valid UTC time
        //return timezone->to_sys(local_time, std::chrono::choose::earliest);
        return timezone->to_sys(local_time, std::chrono::choose::latest);
    }
}

std::chrono::system_clock::time_point ConvertUTCToLocal(
    const std::chrono::system_clock::time_point& utc_time_point,
    const std::chrono::time_zone* timezone)
{
    if (timezone == nullptr) {
        throw std::invalid_argument("Timezone cannot be null.");
    }

    // Convert UTC to local time
    auto local_time = timezone->to_local(utc_time_point);

    return std::chrono::system_clock::time_point(local_time.time_since_epoch());
}

inline const std::map<std::string, int>monthToIntMap =
{
    {"January", 1},
    {"February", 2},
    {"March", 3},
    {"April", 4},
    {"May", 5},
    {"June", 6},
    {"July", 7},
    {"August", 8},
    {"September", 9},
    {"October", 10},
    {"November", 11},
    {"December", 12}
};

inline const std::map<int, std::string>intToMonthMap =
{
    {1, "January"},
    {2, "February"},
    {3, "March"},
    {4, "April"},
    {5, "May"},
    {6, "June"},
    {7, "July"},
    {8, "August"},
    {9, "September"},
    {10, "October"},
    {11, "November"},
    {12, "December"}
};

inline const std::map<int, std::string>supportOfficeTags = // For debug
{
    {0,  "[MEL]"},
    {1,  "[DEL]"},
    {2,  "[MAN]"},
    {3,  "[SAO]"},
    {4,  "[ORL]"}
};

int supportOfficeID = 0;

class Location
{
public:
    bool isSupport{}; // Active 8:30-17:00 if isSupport. Active 24 hours weekdays otherwise.

    std::string nameString;
    std::string tag; // Just two letters that I can output to show the support office's identity. 

    std::string timezoneString;
    const std::chrono::time_zone* timezone{};

    std::string holidaysString;
    std::vector<int> holidayMonths; // Adjusted to UTC 0.
    std::vector<int> holidayDates;  // Adjusted to UTC 0.
    std::vector<std::chrono::system_clock::time_point>holidayTimePointsUTC;
    std::vector<std::chrono::system_clock::time_point>holidayTimePointsLocal;

    std::chrono::system_clock::time_point currentTimePointUTC;
    std::chrono::system_clock::time_point currentTimePointLocal;

    int overtimeNeeded = 0;

    void Setup(bool is_support)
    {
        isSupport = is_support;
        if (isSupport)
        {
            tag = supportOfficeTags.at(supportOfficeID);
            supportOfficeID++;
        }
        else
        {
            tag = nameString[0];
        }

        timezone = std::chrono::locate_zone(timezoneString);

        currentTimePointUTC = std::chrono::system_clock::time_point{ std::chrono::sys_days{std::chrono::year{2022} / std::chrono::January / 1} };
        currentTimePointLocal = ConvertUTCToLocal(currentTimePointUTC, timezone);

        std::cout << "DEBUG: " << currentTimePointUTC << " UTC ---> " << currentTimePointLocal << " Local " << std::endl;

        for (size_t i = 0; i < holidayDates.size(); i++)
        {
            holidayTimePointsLocal.push_back(std::chrono::system_clock::time_point{ std::chrono::sys_days{ std::chrono::year{2022} / holidayMonths[i] / holidayDates[i] } });
            holidayTimePointsUTC.push_back(ConvertLocalToUTC(std::chrono::system_clock::time_point{ std::chrono::sys_days{ std::chrono::year{2022} / holidayMonths[i] / holidayDates[i] } }, timezone));
        }
    }
    

    void TestOutput()
    {
        std::cout << "#### " << nameString << " ####" << std::endl;
        std::cout << "Timezone: " << timezoneString << ": " << timezone->name() << std::endl;
        std::cout << "Holidays: " << std::endl;
        for (size_t i = 0; i < holidayDates.size(); i++)
        {
            std::cout << intToMonthMap.at(holidayMonths[i]) << " " << holidayDates[i] << " = ";
            std::cout << holidayTimePointsLocal[i] << " local --> " << holidayTimePointsUTC[i] << " UTC" << std::endl;
        }
        std::cout << std::endl;
    }

    bool IsTimepointActive()
    {
        // Check if the *UTC* (not local!) time is within the year of 2022. I'm not sure this step is necessary, since time gets synchronized in main().
        /*
        if (currentTimePointUTC >= std::chrono::system_clock::time_point{ std::chrono::sys_days{std::chrono::year{2022} / std::chrono::January / 1} } &&
            currentTimePointUTC <= std::chrono::system_clock::time_point{ std::chrono::sys_days{std::chrono::year{2022} / std::chrono::December / 31} })
        {
        */

            auto days = std::chrono::floor<std::chrono::days>(currentTimePointLocal);
            auto timeOfDay = std::chrono::hh_mm_ss(currentTimePointLocal - days);
            auto date = std::chrono::year_month_day(days);
            auto weekday = std::chrono::weekday(days);

            // Check if it's a LOCAL weekday 
            if (weekday.c_encoding() < std::chrono::Monday.c_encoding() || weekday.c_encoding() > std::chrono::Friday.c_encoding()) // chrono::weekday does not directly support comparison operators. Get its int representation with c_encoding()
            {
                return false;
            }

            // If SUPPORT, check if LOCAL time is within shift
            if (isSupport) 
            {
                auto timeDuration = std::chrono::duration_cast<std::chrono::seconds>( timeOfDay.hours() + timeOfDay.minutes() + timeOfDay.seconds() );

                auto startTime = std::chrono::hours(8) + std::chrono::minutes(30); // 08:30
                auto endTime = std::chrono::hours(17);                             // 17:00
                if (timeDuration < startTime || timeDuration >= endTime) {
                    return false;
                }
            }
            // No need to check the hour for client locations. It's "24 hour" service, after all.
            

            // Check if it's a not a holiday
            //for (std::vector<std::chrono::system_clock::time_point>::iterator it = holidayTimePointsLocal.begin(); it != holidayTimePointsLocal.end(); ++it)
            //{
                // Check against each holiday in the vector
                for (const auto& holiday : holidayTimePointsLocal) {
                    auto holidayDays = std::chrono::floor<std::chrono::days>(holiday);
                    auto holidayDate = std::chrono::year_month_day(holidayDays);

                    // Compare the year, month, and day
                    if (date == holidayDate) 
                    {
                        //std::cout << "H";
                        return false;
                    }
                }
            //}
            

            return true;
        /* }
        else // not within 2022
        {
            return false;
        }
        */

    }
};

std::vector<Location*>supportLocations;
std::vector<Location*>clientLocations;

int main()
{

    const char* custom_tzdb_path = "C:\Development\Libraries\tzdb-2024b";
    std::string env_var = "TZDIR=" + std::string(custom_tzdb_path);
    if (_putenv(env_var.c_str()) != 0) {
        std::cout << "Failed to set TZDIR environment variable.\n";
        return 1;
    }

    const auto& tzdb = std::chrono::get_tzdb();
    std::cout << "Timezone database version: " << tzdb.version << std::endl;
    std::cout << std::endl;

    std::ifstream supportFile;
    supportFile.open("supportInput.txt");
    std::string supportString;

    while (getline(supportFile, supportString))
    {
        /**
        So, my parsing strategy follows:
        The word containing a "/" is the timezone.
        Every word I encounter before finding the timezone word is appended to Location name string.
        The timezone word is stored in timezone string when found.
        Every word I encounter after finding the timezone word is parsed into vacation vectors.
        If "2022" or ";" is found in any part of the vacation words, discard it.
        */

        Location* office = new Location();

        std::istringstream iss(supportString);
        std::string word;
        bool timezoneWordFound = false;
        bool holidayWordIsMonth = false;

        while (iss >> word)
        {
            if (!timezoneWordFound) // Timezone word not yet found.
            {
                if (word.find('/') != std::string::npos) // '/' exists in a word, therefore timezone.
                {
                    office->timezoneString = word;
                    timezoneWordFound = true;
                }
                else
                {
                    office->nameString.append(word + " ");
                }
            }
            else // Timezone word already found.
            {
                std::size_t pos = word.find(';');
                if (pos != std::string::npos)
                    word.erase(pos, 1);

                pos = word.find("2022");
                if (pos != std::string::npos)
                    word.erase(pos, 4);

                if (word.size() == 0)
                    break;

                for (std::map<std::string, int>::const_iterator it = monthToIntMap.begin(); it != monthToIntMap.end(); ++it)
                {
                    if (word == (*it).first)
                    {
                        //std::cout << "(" << word << ") ";
                        office->holidayMonths.push_back((*it).second);
                        holidayWordIsMonth = true;
                        break;
                    }
                }
                if (!holidayWordIsMonth)
                {
                    //std::cout << "[" << word << "] ";
                    office->holidayDates.push_back(std::stoi(word));
                }
                holidayWordIsMonth = false;

                office->holidaysString.append(word);
            }
        }

        office->Setup(true);
        supportLocations.push_back(office);

        office->TestOutput();

        std::cout << std::endl;

    }
    supportFile.close();
    std::cout << "###########################################" << std::endl << std::endl;


    std::ifstream clientFile;
    clientFile.open("clientInput.txt");
    std::string clientString;

    while (getline(clientFile, clientString))
    {
        Location* office = new Location();

        std::istringstream iss(clientString);
        std::string word;
        bool timezoneWordFound = false;
        bool holidayWordIsMonth = false;

        while (iss >> word)
        {
            if (!timezoneWordFound) // Timezone word not yet found.
            {
                if (word.find('/') != std::string::npos) // '/' exists in a word, therefore timezone.
                {
                    office->timezoneString = word;
                    timezoneWordFound = true;
                }
                else
                {
                    office->nameString.append(word + " ");
                }
            }
            else // Timezone word already found.
            {
                std::size_t pos = word.find(';');
                if (pos != std::string::npos)
                    word.erase(pos, 1);

                pos = word.find("2022");
                if (pos != std::string::npos)
                    word.erase(pos, 4);

                if (word.size() == 0)
                    break;

                for (std::map<std::string, int>::const_iterator it = monthToIntMap.begin(); it != monthToIntMap.end(); ++it)
                {
                    if (word == (*it).first)
                    {
                        //std::cout << "(" << word << ") ";
                        office->holidayMonths.push_back((*it).second);
                        holidayWordIsMonth = true;
                        break;
                    }
                }
                if (!holidayWordIsMonth)
                {
                    //std::cout << "[" << word << "] ";
                    office->holidayDates.push_back(std::stoi(word));
                }
                holidayWordIsMonth = false;

                office->holidaysString.append(word);
            }
        }

        office->Setup(false);
        clientLocations.push_back(office);

        office->TestOutput();
    }
    clientFile.close();

    const auto SIM_TIME_POINT_BEGIN = std::chrono::system_clock::time_point{ std::chrono::sys_days{std::chrono::year{2022} / std::chrono::January / 1} };
    const auto SIM_TIME_POINT_END = std::chrono::system_clock::time_point{ std::chrono::sys_days{std::chrono::year{2023} / std::chrono::January / 1} };
    const auto SIM_TIME_POINT_INCREMENT = std::chrono::minutes{ 30 };

    const std::string weekdays[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    for (auto simTimePointCurrent = SIM_TIME_POINT_BEGIN; simTimePointCurrent < SIM_TIME_POINT_END; simTimePointCurrent += SIM_TIME_POINT_INCREMENT)
    {

        auto days_since_epoch = std::chrono::floor<std::chrono::days>(simTimePointCurrent);
        auto weekday = std::chrono::weekday(days_since_epoch);
        //std::cout << simTimePointCurrent << " (" << weekdays[weekday.c_encoding()] << "): ";

        for (std::vector<Location*>::iterator it = supportLocations.begin(); it != supportLocations.end(); ++it)
        {
            (*it)->currentTimePointUTC = simTimePointCurrent;
            (*it)->currentTimePointLocal = ConvertUTCToLocal(simTimePointCurrent, (*it)->timezone);
        }
        for (std::vector<Location*>::iterator it = clientLocations.begin(); it != clientLocations.end(); ++it)
        {
            (*it)->currentTimePointUTC = simTimePointCurrent;
            (*it)->currentTimePointLocal = ConvertUTCToLocal(simTimePointCurrent, (*it)->timezone);

        }

        for (std::vector<Location*>::iterator cit = clientLocations.begin(); cit != clientLocations.end(); ++cit)
        {
            if ((*cit)->IsTimepointActive())
            {
                bool supportAvailable = false;
                //std::cout << (*cit)->tag;

                for (std::vector<Location*>::iterator sit = supportLocations.begin(); sit != supportLocations.end(); ++sit)
                {
                    if ((*sit)->IsTimepointActive())
                    {
                        //std::cout << (*sit)->tag;
                        supportAvailable = true;
                    }

                }
                if (!supportAvailable)
                {
                    //std::cout << "+";
                    (*cit)->overtimeNeeded += 30;
                }
            }
            //std::cout << " ";
        }
        //std::cout << std::endl;
    }

    //Descending order sort
    std::sort(clientLocations.begin(), clientLocations.end(), [](Location* a, Location* b){ return a->overtimeNeeded > b->overtimeNeeded;} );

    std::cout << std::endl << "Summary: " << std::endl;
    for (std::vector<Location*>::iterator it = clientLocations.begin(); it != clientLocations.end(); ++it)
    {
        std::cout << (*it)->nameString << ": " << (*it)->overtimeNeeded << " OT minutes needed" << std::endl;
    }
    
    std::cout << std::endl;

    int highestOT = 0;
    int lowestOT = 10000000;
    for (std::vector<Location*>::iterator it = clientLocations.begin(); it != clientLocations.end(); ++it)
    {
        if ((*it)->overtimeNeeded > highestOT)
            highestOT = (*it)->overtimeNeeded;

        if ((*it)->overtimeNeeded < lowestOT)
            lowestOT = (*it)->overtimeNeeded;
    }

    std::cout << "Highest OT: " << highestOT << std::endl;
    std::cout << "Lowest OT: " << lowestOT << std::endl;
    std::cout << "Answer: " << highestOT - lowestOT << std::endl;


    for (std::vector<Location*>::iterator it = supportLocations.begin(); it != supportLocations.end();)
    {
        delete* it;
        it = supportLocations.erase(it);
    }

    for (std::vector<Location*>::iterator it = clientLocations.begin(); it != clientLocations.end();)
    {
        delete* it;
        it = clientLocations.erase(it);
    }

    return 0;
}
