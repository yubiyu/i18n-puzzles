#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <locale>
#include <cwctype>

int numValidPasswords = 0;

int main()
{
	std::locale::global(std::locale("en_US.UTF-8"));

	std::ifstream inputFile;
	inputFile.open("input.txt");

	std::string line;

	while (getline(inputFile, line))
	{
		int lineNumCharacters = 0;
		bool meetsCharacterLengthReq = false;
		bool meetsDigitReq = false;
		bool meetsUppercaseReq = false;
		bool meetsLowercaseReq = false;
		bool meetsOutsideASCIIReq = false;

		for (size_t i = 0; i < line.size(); i++)
		{
			unsigned char byte = static_cast<unsigned char>(line[i]);
			std::bitset<8> bits = byte; // There's 8 bits in a byte.

			if (bits[7] == 1 && bits[6] == 0)
			{
				//do nothing
			}
			else
			{
				lineNumCharacters++;
			}
			std::cout << line[i];
		}

		for(wchar_t ch : line)
		{
			if (std::isdigit(ch))
				meetsDigitReq = true;

			if (ch > 127) // The ascii table has 128 characters
				meetsOutsideASCIIReq = true;

			if (std::iswupper(ch))
			{
				meetsUppercaseReq = true;
			}
			if(std::iswlower(ch))
			{
				meetsLowercaseReq = true;
			}

		}

		std::cout << " -- ";

		if (lineNumCharacters < 4 || lineNumCharacters > 12)
		{
			std::cout << "invalid character length: " << lineNumCharacters;
		}
		else if (!meetsDigitReq)
		{
			std::cout << "does not contain at least one digit";
		}
		else if (!meetsUppercaseReq)
		{
			std::cout << "does not contain at least one uppercase character";
		}
		else if (!meetsLowercaseReq)
		{
			std::cout << "does not contain at least one lowercase character";
		}
		else if (!meetsOutsideASCIIReq)
		{
			std::cout << "does not have a character outside standard ASCII";
		}
		else
		{
			std::cout << " SEEMS LEGIT";
			numValidPasswords++;
			std::cout << " -- #" << numValidPasswords;
		}

		std::cout << std::endl;
	}

	std::cout << std::endl << "Nubmer of valid passwords found: " << numValidPasswords << std::endl;

	inputFile.close();
	return 0;
}