#pragma once

#include <iostream>
#include <string>
#include <iomanip>
#include <bitset>
#include <codecvt>

std::string HexToUTF8(const std::string& hexStr)
{
    std::string utf8Char;
    for (size_t i = 0; i < hexStr.length(); i += 2)
    {
        std::string byteStr = hexStr.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
        utf8Char += byte;
    }
    return utf8Char;
}

std::string ConvertHexLineToUTF8Line(const std::string& hexStr)
{
    if (hexStr.length() % 2 != 0)
        std::cout << "?!?!?!?! " << std::endl;

    std::string utf8Str;
    utf8Str.reserve(hexStr.length() / 2);

    for (size_t i = 0; i < hexStr.length(); i += 2)
    {
        std::string byteStr = hexStr.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
        utf8Str += byte;
    }

    return utf8Str;
}

size_t GetFirstUTF8CharLength(const std::string& str)
{
    if (str.empty())
        return 0;

    unsigned char firstByte = static_cast<unsigned char>(str[0]);

    if ((firstByte & 0x80) == 0)
        return 1;

    else if ((firstByte & 0xE0) == 0xC0)
    {
        if (str.size() > 1 && (str[1] & 0xC0) == 0x80)
            return 2;
        else
            return 1;
    }
    else if ((firstByte & 0xF0) == 0xE0)
    {
        if (str.size() > 2 && (str[1] & 0xC0) == 0x80 && (str[2] & 0xC0) == 0x80)
            return 3;
        else
            return 1;
    }
    else if ((firstByte & 0xF8) == 0xF0)
    {
        if (str.size() > 3 && (str[1] & 0xC0) == 0x80 && (str[2] & 0xC0) == 0x80 && (str[3] & 0xC0) == 0x80)
            return 4;
        else
            return 1;
    }

    return 1;
}


size_t GetLastUTF8CharLength(const std::string& str)
{
    if (str.empty()) return 0;

    size_t position = str.length() - 1;
    while (position > 0 && (str[position] & 0xC0) == 0x80)
        position--;

    return str.length() - position;
}

bool IsValid(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    try
    {
        std::wstring wideStr = converter.from_bytes(str);
    }
    catch (const std::exception&)
    {
        return false;
    }
    return true;
}

bool IsValidWithinRange(const std::string& str, size_t start, size_t end)
{
    if (start >= str.size() || end > str.size() || start > end)
        std::cout << "?!?!?!?!?!";

    size_t i = start;
    while (i < end)
    {
        unsigned char byte = str[i];

        size_t charLen = 0;
        if ((byte & 0x80) == 0)
            charLen = 1;
        else if ((byte & 0xE0) == 0xC0)
            charLen = 2;
        else if ((byte & 0xF0) == 0xE0)
            charLen = 3;
        else if ((byte & 0xF8) == 0xF0)
            charLen = 4;
        else
            return false;

        if (i + charLen > end)
            return false;

        for (size_t j = 1; j < charLen; j++)
        {
            if (i + j >= end || (str[i + j] & 0xC0) != 0x80)
                return false;
        }

        i += charLen;
    }

    return true;
}

size_t CountReplacementCharsForFirst(const std::string& str)
{
    if (str.empty())
        return 0;

    size_t i = 0, replacementCount = 0;

    while (i < str.size())
    {
        unsigned char byte = static_cast<unsigned char>(str[i]);

        if ((byte & 0x80) == 0) return replacementCount;

        if ((byte & 0xC0) == 0x80 || byte >= 0xF8)
        {
            replacementCount++;
            i++;
            continue;
        }

        size_t expectedLength = 1;
        if ((byte & 0xE0) == 0xC0)
            expectedLength = 2;
        else if ((byte & 0xF0) == 0xE0)
            expectedLength = 3;
        else if ((byte & 0xF8) == 0xF0)
            expectedLength = 4;

        if (i + expectedLength > str.size())
        {
            replacementCount += str.size() - i;
            return replacementCount;
        }

        bool valid = true;
        for (size_t j = 1; j < expectedLength; j++)
        {
            if ((static_cast<unsigned char>(str[i + j]) & 0xC0) != 0x80)
            {
                valid = false;
                break;
            }
        }

        if (valid) return replacementCount;

        replacementCount++;
        i++;
    }

    return replacementCount;
}

std::string HexToBytes(const std::string& hex)
{
    std::string bytes;
    if (hex.size() % 2 != 0)
        return "";
    for (size_t i = 0; i < hex.size(); i += 2)
    {
        unsigned int byte;
        if (sscanf(hex.substr(i, 2).c_str(), "%02x", &byte) != 1)
            return "";
        bytes.push_back(static_cast<char>(byte));
    }
    return bytes;
}

size_t CountMissingContinuationBytes(const std::string& hexString)
{
    std::string str = HexToBytes(hexString);
    if (str.empty())
        return 0;

    size_t stringLength = str.size();
    if (stringLength == 0)
        return 0;

    size_t i = stringLength - 1;
    size_t missingCount = 0;

    while (i < stringLength)
    {
        unsigned char byte = static_cast<unsigned char>(str[i]);

        if ((byte & 0x80) == 0)
            return missingCount;

        size_t expectedLength = 1;
        if ((byte & 0xE0) == 0xC0)
            expectedLength = 2;
        else if ((byte & 0xF0) == 0xE0)
            expectedLength = 3;
        else if ((byte & 0xF8) == 0xF0)
            expectedLength = 4;
        else if ((byte & 0xC0) == 0x80)
        {
            if (i == 0)
                return 0;

            i--;
            continue;
        }
        else
        {
            return missingCount;
        }

        if (i + expectedLength > stringLength)
        {
            missingCount = (i + expectedLength) - stringLength;
            return missingCount;
        }

        return missingCount;
    }

    return missingCount;
}
