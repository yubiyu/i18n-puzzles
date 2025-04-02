#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

const int COMBINED_FEE = 13;
const int TWEET_FEE = 7;
const int SMS_FEE = 11;

int main()
{
    std::ifstream inputFile;
    inputFile.open("input.txt");

    int fee = 0;
    int lineBytes = 0;
    int lineCharacters = 0;
    std::string line;

    while (getline(inputFile, line))
    {
        std::cout << line << std::endl;
        lineBytes = line.size();

        for (size_t i = 0; i < lineBytes; i++)
        {
            unsigned char byte = static_cast<unsigned char>(line[i]);
            std::bitset<8> bits = byte;

            std::cout << "Byte " << i << ": " << bits << " (" << byte << ")";


            if (bits[7] == 1 && bits[6] == 0)
            {
                std::cout << " -- IGNORE -- ";
            }
            else
            {
                lineCharacters++;
            }
            std::cout << std::endl;
        }

        std::cout << lineBytes << " bytes, " << lineCharacters << " characters." << std::endl << std::endl;

        if (lineBytes <= 160 && lineCharacters <= 140)
            fee += COMBINED_FEE;
        else if (lineCharacters <= 140)
            fee += TWEET_FEE;
        else if (lineBytes <= 160)
            fee += SMS_FEE;

        lineBytes = 0;
        lineCharacters = 0;
    }
    std::cout << std::endl << "Total fee: " << fee << std::endl;

    inputFile.close();
    return 0;
}
