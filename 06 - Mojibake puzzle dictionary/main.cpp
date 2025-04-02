#include <iostream>
#include <fstream>
#include <string>
#include <codecvt>
#include <locale>
#include <vector>
#include <unordered_map>

/**
Rules:
Every third and fifth line are stored as UTF8, but is supposed to be ISO Latin 1.

Notes:
Looks like in order to solve this, I need the following functions:
1) Convert UTF to Unicode.
2) Convert Unicode to ISO Latin 1 (ISO-8859-1).
3) Convert characters with diacritics to regular letters.

/// Converting UTF8 to a wide string *is* converting to Unicode.
/// But not really.
/// Windows wchar_t uses 16 bit (2 bytes = 2x8 bits long).
/// While Linux/Mac wchar_t is 32 bit (4 bytes = 4x8 bits long).

/// Note: Iso Latin 1 maps 1:1 with Unicode's first 256 indexes ("code points").
/// Should check if any characters encountered are outside 0-255 and do something about it.

*/

bool IsUnicodeCharacter(char c)
{
    /// If the first bit of the char is 1, then it's part of a multi-byte unicode character.
    return (c & 0x80) != 0;
}

uint32_t DecodeUtf8(const std::string& str, size_t& index)
{
    unsigned char c1 = str[index];
    uint32_t codepoint = 0;

    if (c1 < 0x80)
    {
        // 1-byte character (ASCII)
        codepoint = c1;
        index += 1;
    }
    else if (c1 < 0xE0)
    {
        // 2-byte character
        codepoint = (c1 & 0x1F) << 6;
        codepoint |= (str[index + 1] & 0x3F);
        index += 2;
    }
    else if (c1 < 0xF0)
    {
        // 3-byte character
        codepoint = (c1 & 0x0F) << 12;
        codepoint |= (str[index + 1] & 0x3F) << 6;
        codepoint |= (str[index + 2] & 0x3F);
        index += 3;
    }
    else
    {
        // 4-byte character
        codepoint = (c1 & 0x07) << 18;
        codepoint |= (str[index + 1] & 0x3F) << 12;
        codepoint |= (str[index + 2] & 0x3F) << 6;
        codepoint |= (str[index + 3] & 0x3F);
        index += 4;
    }

    return codepoint;
}

std::string Transliterate(const std::string& input)
{
    std::unordered_map<char32_t, char32_t> accentMap =
    {
        ///https://en.wikipedia.org/wiki/List_of_Unicode_characters
        {0x00E0, 0x0061}, {0x00E1, 0x0061}, {0x00E2, 0x0061}, {0x00E3, 0x0061}, {0x00E4, 0x0061}, {0x00E5, 0x0061}, {0x0103, 0x0061},  // convert à, á, â, ã, å, ă to a (0061)
        {0x00E8, 0x0065}, {0x00E9, 0x0065}, {0x00EA, 0x0065}, {0x00EB, 0x0065},  // convert è, é, ê, ë to e (0065)
        {0x00EC, 0x0069}, {0x00ED, 0x0069}, {0x00EE, 0x0069}, {0x00EF, 0x0069},  // convert to i (0069)
        {0x00F3, 0x006F}, {0x00F2, 0x006F}, {0x00F4, 0x006F}, {0x00F6, 0x006F}, {0x00F5, 0x006F},  // covert to o (006F)
        {0x00FA, 0x0075}, {0x00F9, 0x0075}, {0x00FB, 0x0075}, {0x00FC, 0x0075},  // convert to u (0075)

        {0x00F1, 0x006E}, {0x00C7, 0x0043}, {0x00E7, 0x0063}, {0x00FD, 0x0079}, {0x00FF, 0x0079}, {0x00C0, 0x0041}, // ñ, ç, y

        {0x00F8, 0x006F},  // ø
        //{0x00F0, 0x0064} // ð

        //{0x00E6,       }, // æ
        //{0x0153,       }, // œ
        {0x00DF, 0x0073}, // ß -> s
        //{0x00FE,       }  // þ

        {0x021B, 0x0074}, // ț -> t
        {0x0219, 0x0073} // ș -> s

    };

    std::string result;
    size_t index = 0;
    while (index < input.size())
    {
        uint32_t codepoint = DecodeUtf8(input, index);

        if (accentMap.find(codepoint) != accentMap.end())
        {
            result.push_back(accentMap[codepoint]);
        }
        else
        {
            if (codepoint <= 0x7F) // ASCII
            {
                result.push_back(static_cast<char>(codepoint));
            }
            else // Multi-byte
            {
                std::string utf8Char;
                if (codepoint < 0x800)
                {
                    utf8Char += static_cast<char>(0xC0 | (codepoint >> 6));
                    utf8Char += static_cast<char>(0x80 | (codepoint & 0x3F));
                }
                else if (codepoint < 0x10000)
                {
                    utf8Char += static_cast<char>(0xE0 | (codepoint >> 12));
                    utf8Char += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    utf8Char += static_cast<char>(0x80 | (codepoint & 0x3F));
                }
                else
                {
                    utf8Char += static_cast<char>(0xF0 | (codepoint >> 18));
                    utf8Char += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                    utf8Char += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    utf8Char += static_cast<char>(0x80 | (codepoint & 0x3F));
                }
                result.append(utf8Char);
                std::cout << utf8Char << " ";
            }
        }
    }
    return result;
}

std::vector<char32_t> UTF8ToUnicode(const std::string& utf8_str)
{
    std::vector<char32_t> unicodePoints;
    size_t i = 0;
    while (i < utf8_str.size())
    {
        char32_t codePoint = 0;
        unsigned char byte = utf8_str[i];

        /// Can refer to Tom Scott's video on this one. Remember that the leading bits tells me how many bytes long the char is.
        /// Should also remember that w_char is 16 bits long on Windows and 32 bits long on mac/linux.
        if (byte < 0x80) // 1 byte
        {
            codePoint = byte;
            i += 1;
        }
        else if ((byte & 0xE0) == 0xC0) // 2-bytes
        {
            codePoint = ((byte & 0x1F) << 6) | (utf8_str[i + 1] & 0x3F);
            i += 2;
        }
        else if ((byte & 0xF0) == 0xE0) // 3-bytes
        {
            codePoint = ((byte & 0x0F) << 12) | ((utf8_str[i + 1] & 0x3F) << 6) | (utf8_str[i + 2] & 0x3F);
            i += 3;
        }
        else if ((byte & 0xF8) == 0xF0) // 4 bytes
        {
            codePoint = ((byte & 0x07) << 18) | ((utf8_str[i + 1] & 0x3F) << 12) | ((utf8_str[i + 2] & 0x3F) << 6) | (utf8_str[i + 3] & 0x3F);
            i += 4;
        }
        else // What's even going on here
        {
            std::cout << "?!?!?!?!" << std::endl;
            //assert("Invalid UTF-8 sequence");
        }

        unicodePoints.push_back(codePoint);
    }
    return unicodePoints;
}

int main()
{
    std::ifstream inputFile;
    inputFile.open("input.txt");

    std::string inputLine;
    std::cout << "Input file: " << std::endl << std::endl;

    size_t lineNumber = 1; // Not starting from 0.

    std::vector <std::string> dictionary;

    while (getline(inputFile, inputLine))
    {
        std::string lineBeforeConversion;
        std::string lineAfterAllConversions;
        std::string lineAfterNormalization; // Strip

        lineBeforeConversion = inputLine;
        if(lineNumber%3 == 0 && lineNumber%5 == 0) // Every third and fifth line needs conversion; Every fifteenth line needs to be converted twice.
        {
            std::string lineAfterFirstConversion;
            std::vector<char32_t>unicodePoints = UTF8ToUnicode(lineBeforeConversion);

            for(char32_t cp: unicodePoints)
                lineAfterFirstConversion += cp;

            unicodePoints = UTF8ToUnicode(lineAfterFirstConversion);
            for(char32_t cp: unicodePoints)
                lineAfterAllConversions += cp;

            ///std::cout << lineNumber << ". " << lineBeforeConversion << " --> " << lineAfterFirstConversion << " --> " << lineAfterAllConversions;

        }
        else if(lineNumber%3 == 0 || lineNumber%5 == 0)
        {
            std::vector<char32_t>unicodePoints = UTF8ToUnicode(lineBeforeConversion);
            for(char32_t cp: unicodePoints)
                lineAfterAllConversions += cp;

            ///std::cout << lineNumber << ". " << lineBeforeConversion << " --> " << lineAfterAllConversions;
        }
        else
        {
            lineAfterAllConversions = lineBeforeConversion;
            ///std::cout << lineNumber << ". " << lineAfterAllConversions;
        }

        lineAfterNormalization = Transliterate(lineAfterAllConversions);
        ///std::cout << " --> " << lineAfterNormalization;

        dictionary.push_back(lineAfterNormalization);

        lineNumber ++;
        ///std::cout << std::endl;
    }
    inputFile.close();

    std::cout << std::endl;

    std::ifstream crosswordFile;
    crosswordFile.open("crossword.txt");

    std::string crosswordLine;
    int sumOfLineNumbers = 0;

    while(getline(crosswordFile, crosswordLine))
    {
        size_t lineLength = 0;
        char keyCharacter = '?';
        size_t keyCharacterIndex = 0;

        std::cout << crosswordLine << " -- ";
        for(size_t i = 0; i < crosswordLine.size(); i++)
        {
            if(crosswordLine[i] != ' ')
            {
                lineLength ++;
                if(std::isalpha(crosswordLine[i]))
                {
                    keyCharacter = std::tolower(crosswordLine[i]);
                    keyCharacterIndex = lineLength; // lineLength up to this point.
                }
            }
        }
        std::cout << "Length: " << lineLength << " || Key Char: " << keyCharacter << " || Key Index: " << keyCharacterIndex << std::endl;
        std::cout << "Solution candidates: ";

        for(std::vector<std::string>::iterator it = dictionary.begin(); it != dictionary.end(); ++it)
        {
            if((*it).size() == lineLength)
            {
                if((*it)[keyCharacterIndex-1] == keyCharacter)
                {
                    int lineNumber = std::distance( dictionary.begin(), it) + 1;
                    std::cout << (*it) << "=" << lineNumber << " ";
                    sumOfLineNumbers += lineNumber;
                }

                /*
                if(keyCharacter == 'd')
                {
                    std::cout << (*it) << "=" << std::distance( dictionary.begin(), it ) + 1 << " ";
                }
                */
            }
        }
        std::cout << std::endl << std::endl;

    }
    std::cout << "Answer: " << sumOfLineNumbers;
    crosswordFile.close();


    return 0;
}
