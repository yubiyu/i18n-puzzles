#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

//const int COLS = 38;


int main()
{
	std::ifstream inputFile;
	inputFile.open("input.txt");

	std::string line;

	size_t xPosition = 0;
	size_t yPosition = 0;
	int poosEncountered = 0;

	//poopEmoji = "\xF0\x9F\x92\xA9";
	std::bitset<8>xF0("11110000");
	std::bitset<8>x9F("10011111");
	std::bitset<8>x92("10010010");
	std::bitset<8>xA9("10101001");
	
	std::bitset<8>E2("11100010");

	
	while (getline(inputFile, line))
	{
		std::string pooDetector;

		for (size_t i = 0; i < line.size();)
		{
			unsigned char byte1 = static_cast<unsigned char>(line[i]);
			std::bitset<8>bits1 = byte1;

			if (bits1 == std::bitset<8>("00100000")) // " "
			{
				i++;
				pooDetector += "_";
			}
			else if (bits1 == xF0) // " First byte of poo, rabbit, dog and evergreen emojis"
			{
				i++;
				unsigned char byte2 = static_cast<unsigned char>(line[i]);
				std::bitset<8>bits2 = byte2;
				if (bits2 == x9F) // " Second byte of poo, rabbit dog, and evergreen emojis"
				{
					i++;
					unsigned char byte3 = static_cast<unsigned char>(line[i]);
					std::bitset<8>bits3 = byte3;
					if (bits3 == x92) // " Third byte of poo emoji "
					{
						i++;
						unsigned char byte4 = static_cast<unsigned char>(line[i]);
						std::bitset<8>bits4 = byte4;
						if (bits4 == xA9) // "Fourth byte of poo emoji"
						{
							i++;
							pooDetector += "P";
						}
						else // NOT fourth byte of poo emoji
						{
							i++;
							pooDetector += "X";
						}
					}
					else // NOT third byte of poo emoji
					{
						i += 2;
						pooDetector += "X";
					}
				}
				else // NOT second byte of poo emoji
				{
					i += 3;
					pooDetector += "X";
				}
			}
			else if(bits1 == E2)
			{
				i += 3;
				pooDetector += "X";

			}
		}
		std::cout << pooDetector;
		std::cout << " xPosition = " << xPosition << std::endl;

		if (pooDetector[xPosition] == 'P')
			poosEncountered++;

		xPosition += 2;
		if (xPosition >= pooDetector.size())
			xPosition -= pooDetector.size();

		yPosition++;
	}

	std::cout << "Poos encountered: " << poosEncountered << std::endl;
	std::cout << "lines walked: " << yPosition << std::endl;
	inputFile.close();
	return 0;
}