#include <iostream>

#include "input.h"
#include "functions.h"

#include <vector>
#include <map>
#include <algorithm>
#include <string>

class Subfragment
{
    static size_t IDToAssign;

public:
    size_t assignedID;

    enum enumDirections
    {
        DIR_UP = 0,
        DIR_DOWN = 1,
        DIR_LEFT = 2,
        DIR_RIGHT = 3
    };
    static const int NUM_ADJ_DIRS = 4;
    Subfragment* adjacencies[NUM_ADJ_DIRS];
    bool acceptsLHSConnection;
    bool acceptsRHSConnection;

    std::vector<std::string>content;

    Subfragment()
    {
        assignedID = IDToAssign;
        IDToAssign ++;

        for(size_t i = 0; i < NUM_ADJ_DIRS; i++)
            adjacencies[i] = nullptr;

        acceptsLHSConnection = true;
        acceptsRHSConnection = true;
    }
};
size_t Subfragment::IDToAssign = 0;

class MapFragment
{
    static size_t IDToAssign;

public:
    size_t assignedID;

    std::vector<Subfragment*>hexSubfragments;
    std::vector<Subfragment*>UTF8Subfragments;

    MapFragment()
    {
        assignedID = IDToAssign;
        IDToAssign ++;
    };
    ~MapFragment()
    {
        for(std::vector<Subfragment*>::iterator it = hexSubfragments.begin(); it != hexSubfragments.end();)
        {
            delete *it;
            it = hexSubfragments.erase(it);
        }

        for(std::vector<Subfragment*>::iterator it = UTF8Subfragments.begin(); it != UTF8Subfragments.end();)
        {
            delete *it;
            it = UTF8Subfragments.erase(it);
        }
    }
};
size_t MapFragment::IDToAssign = 0;

int main()
{
/// CONTROL PANEL BEGIN
    bool usingRealInput = true;
    std::stringstream ssInput;
    if(usingRealInput)
        ssInput = std::stringstream(puzzleInputReal);
    else
        ssInput = std::stringstream(puzzleInputTest);
    std::string line;

    size_t subfragmentHeight; // Should be equal to the shortest subfragment height.
    size_t GRID_COLS = 20;
    size_t GRID_ROWS = 15;
    if(usingRealInput)
    {
        subfragmentHeight = 8;
        GRID_COLS = 32;
        GRID_ROWS = 32;
    }
    else
    {
        subfragmentHeight = 4;
        GRID_COLS = 3;
        GRID_ROWS = 6;
    }
    Subfragment* subfragmentHexGrid[GRID_ROWS][GRID_COLS];
    Subfragment* subfragmentUTF8Grid[GRID_ROWS][GRID_COLS];
    for(size_t y = 0; y < GRID_ROWS; y++)
    {
        for(size_t x = 0; x < GRID_COLS; x++)
        {
            subfragmentHexGrid[y][x] = nullptr;
            subfragmentUTF8Grid[y][x] = nullptr;
        }
    }

    size_t assemblyCol = 0;
    size_t assemblyRow = 0;

    std::string checkerboardWhite("        "); // Forms a cosmetic checkerboard of black and white squares to help visualize space.
    std::string checkerboardBlack("........");

    //const size_t MAP_ASSEMBLY_MAX_LINES = 100;
    //std::array<std::string,MAP_ASSEMBLY_MAX_LINES>mapAssemblyHex;
    //std::array<std::string,MAP_ASSEMBLY_MAX_LINES>mapAssemblyUTF8;

    std::vector<MapFragment*>fragmentsPool;
    std::vector<Subfragment*>hexSubfragments;
    std::vector<Subfragment*>UTF8Subfragments;

    MapFragment*currentMapFragment = nullptr;
    Subfragment*currentHexSubfragment = nullptr;
    Subfragment*previousHexSubfragment = nullptr; // Necessary to track previous, so that the current and previous subfragments can be linked.
    Subfragment*currentUTF8Subfragment = nullptr;
    Subfragment*previousUTF8Subfragment = nullptr; // Necessary to track previous, so that the current and previous subfragments can be linked.

/// CONTROL PANEL END

/// POPULATE FRAGMENTS POOL BEGIN
    std::cout << "Puzzle Input:" << std::endl << std::endl;

    bool needsNewMapFragment = true;
    size_t inputblockIndex = 0;
    while(std::getline(ssInput, line))
    {
        std::string UTF8Line = ConvertHexLineToUTF8Line(line);

        if(needsNewMapFragment)
        {
            currentMapFragment = new MapFragment();
            fragmentsPool.push_back(currentMapFragment);

            std::cout << "     MapFragment ID " << currentMapFragment->assignedID << ":" << std::endl;

            previousHexSubfragment = nullptr; // No subfragment above what will become the current.
            currentHexSubfragment = nullptr;
            previousUTF8Subfragment = nullptr; // Not strictly necessary, but whatever.
            currentUTF8Subfragment = nullptr;

            needsNewMapFragment = false;
        }

        if(line.length() > 0)
        {
            if(inputblockIndex%subfragmentHeight == 0)
            {
                currentHexSubfragment = new Subfragment();
                if(previousHexSubfragment != nullptr)
                {
                    currentHexSubfragment->adjacencies[Subfragment::DIR_UP] = previousHexSubfragment; // Register each other as adjacenies.
                    previousHexSubfragment->adjacencies[Subfragment::DIR_DOWN] = currentHexSubfragment;
                }

                currentUTF8Subfragment = new Subfragment();
                if(previousUTF8Subfragment != nullptr)
                {
                    currentUTF8Subfragment->adjacencies[Subfragment::DIR_UP] = previousUTF8Subfragment; // Register each other as adjacenies.
                    previousUTF8Subfragment->adjacencies[Subfragment::DIR_DOWN] = currentUTF8Subfragment;
                }

                currentMapFragment->hexSubfragments.push_back(currentHexSubfragment);
                hexSubfragments.push_back(currentHexSubfragment);

                currentMapFragment->UTF8Subfragments.push_back(currentUTF8Subfragment);
                UTF8Subfragments.push_back(currentUTF8Subfragment);
            }

            currentHexSubfragment->content.push_back(line);
            currentUTF8Subfragment->content.push_back(UTF8Line);

            std::cout << line << " :: " << UTF8Line;
            if(inputblockIndex%subfragmentHeight == 0)
                std::cout << " :: Hex subfragment " << currentHexSubfragment->assignedID << " :: UTF8 subfragment " << currentUTF8Subfragment->assignedID;
            std::cout << std::endl;
            UTF8Line.clear();
            inputblockIndex ++;
        }
        else // Line is blank, indicating end of a fragment.
        {
            inputblockIndex = 0;
            needsNewMapFragment = true;
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
    std::cout << "Size of fragments pool: " << fragmentsPool.size() << std::endl;
    std::cout << "Size of hex subfragments pool: " << hexSubfragments.size() << std::endl;
    std::cout << "Size of UTF8 subfragments pool: " << UTF8Subfragments.size() << std::endl;

/// POPULATE FRAGMENTS POOL END

/// LOCATE TOP LEFT CORNER FRAGMENT BEGIN
    MapFragment* originFragment = nullptr;

    std::cout << "Locating fragment containing top left corner... " << std::endl;

    for(std::vector<MapFragment*>::iterator it = fragmentsPool.begin(); it != fragmentsPool.end(); ++it)
    {
        if((*it)->hexSubfragments[0]->content[0].substr(0,2) == "e2"
                && (*it)->hexSubfragments[0]->content[0].substr(2,2) == "95"
                && (*it)->hexSubfragments[0]->content[0].substr(4,2) == "94")
        {
            std::cout << "Top left corner located in fragment ID " << (*it)->assignedID << std::endl << std::endl;;
            originFragment = (*it);
            break;
        }
    }

    if(originFragment == nullptr)
    {
        std::cout << "ERROR ERROR ERROR: NO CORNER FRAGMENT FOUND!" << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Establishing map assembly origin using corner fragment..." << std::endl;
        assemblyCol = 0;
        assemblyRow = 0;
        for(size_t i = 0; i < originFragment->hexSubfragments.size(); i++)
        {
            currentHexSubfragment = originFragment->hexSubfragments[i];
            currentUTF8Subfragment = originFragment->UTF8Subfragments[i];

            subfragmentHexGrid[assemblyRow][assemblyCol] = currentHexSubfragment;
            subfragmentUTF8Grid[assemblyRow][assemblyCol] = currentUTF8Subfragment;

            /// Don't delete these! They're still needed.
            //hexSubfragments.erase(std::remove(hexSubfragments.begin(), hexSubfragments.end(), currentHexSubfragment), hexSubfragments.end());
            //UTF8Subfragments.erase(std::remove(UTF8Subfragments.begin(), UTF8Subfragments.end(), currentUTF8Subfragment), UTF8Subfragments.end());

            assemblyRow++;
        }

        //std::cout << "Removing origin fragment from fragments pool. ";
        //fragmentsPool.erase(std::remove(fragmentsPool.begin(), fragmentsPool.end(), originFragment), fragmentsPool.end());
        //delete originFragment;
        std::cout << fragmentsPool.size() << " fragments remain." << std::endl << std::endl;

        std::cout << "cin.get() to continue." << std::endl;
        std::cin.get();
    }
/// LOCATE TOP LEFT CORNER FRAGMENT END

/// FRAGMENT MERGING BEGIN
    bool assemblyIsComplete = false;
    assemblyCol = 0;
    assemblyRow = 0;

    while(!assemblyIsComplete)
    {
/// MAP READOUT BEGIN
        std::cout << "Current map assembly:" << std::endl;
        std::map<size_t, std::string>assemblyReadout = std::map<size_t, std::string>();
        for(size_t y = 0; y < GRID_ROWS; y++)
        {
            for(size_t x = 0; x < GRID_COLS; x++)
            {
                size_t assemblyReadoutY = y*subfragmentHeight; // Actual line (key of the assemblyReadout map).


                bool isCheckerboardWhite = false; // Purely cosmetic grid to help visualize space.
                if(y%2 == 0)
                {
                    if(x%2 == 0)
                        isCheckerboardWhite = true;
                }
                else
                {
                    if(x%2 == 1)
                        isCheckerboardWhite = true;
                }

                if(subfragmentUTF8Grid[y][x] != nullptr)
                {
                    for(size_t i = 0; i < subfragmentHeight; i++)
                    {
                        assemblyReadout[assemblyReadoutY + i].append(subfragmentUTF8Grid[y][x]->content[i]);
                    };
                }
                else
                {
                    for(size_t i = 0; i < subfragmentHeight; i++)
                    {
                        if(isCheckerboardWhite)
                            assemblyReadout[assemblyReadoutY + i].append(checkerboardWhite);
                        else
                            assemblyReadout[assemblyReadoutY + i].append(checkerboardBlack);
                    }
                }
            }
        }
        for(std::map<size_t, std::string>::iterator it = assemblyReadout.begin(); it != assemblyReadout.end(); ++it)
        {
            std::cout << (*it).second << std::endl;
        }
        std::cout << std::endl;

        std::cout << "cin.get() to continue." << std::endl;
        std::cin.get();
/// MAP READOUT END
/// MAIN MERGE STEP BEGIN
        std::cout << "Fragment merge progress: " << fragmentsPool.size() << " fragments remain." << std::endl;
        MapFragment*fragmentToMerge = nullptr;
        assemblyCol = 0;
        assemblyRow = 0;
        bool fragmentMatchFound = false;

        bool emptyCellFound = false;
        for(size_t y = 0; y < GRID_ROWS; y++) // Find empty cell to attempt match in.
        {
            if(emptyCellFound)
                break;

            for(size_t x = 0; x < GRID_COLS; x++)
            {
                if(subfragmentHexGrid[y][x] == nullptr)
                {
                    bool hasAdjacent = false;
                    // It only makes sense to attempt merge on the LHS/RHS of an already established fragment.
                    if(x > 0)
                        if(subfragmentHexGrid[y][x-1] != nullptr)
                            hasAdjacent = true;
                    if(x < GRID_COLS-1)
                        if(subfragmentHexGrid[y][x+1] != nullptr)
                            hasAdjacent = true;

                    if(hasAdjacent)
                    {
                        assemblyRow = y;
                        assemblyCol = x;
                        emptyCellFound = true;
                        break;
                    }
                }
            }
        }
        emptyCellFound = false;

        std::cout << "Attempting match at grid ROW " << assemblyRow << " | COL " << assemblyCol << "." << std::endl;
        for(std::vector<MapFragment*>::iterator it = fragmentsPool.begin(); it != fragmentsPool.end(); ++it)
        {
            std::cout << "Testing fragment " << (*it)->assignedID << " for non-edge misencodings:" << std::endl;
            bool fragmentRejected = false;
            std::string testHex;
            std::string testUTF8;

            for(size_t y = assemblyRow; y < assemblyRow+(*it)->hexSubfragments.size(); y++)
            {
                if(subfragmentHexGrid[y][assemblyCol] != nullptr ) // Insufficient vertical space to place fragment. Obviously, rejected.
                {
                    fragmentRejected = true;
                    std::cout << "Fragment " << (*it)->assignedID << " rejected for insufficient space." << std::endl;
                    break;
                }
            }

            if(!fragmentRejected)
            {
                for(size_t i = 0; i < (*it)->hexSubfragments.size(); i++)
                {
                    for(size_t j = 0; j < subfragmentHeight; j++)
                    {
                        // Append content from one cell to the left, if it is occupied.
                        if(assemblyCol > 0 && subfragmentHexGrid[assemblyRow+i][assemblyCol-1] != nullptr)
                            testHex.append(subfragmentHexGrid[assemblyRow+i][assemblyCol-1]->content[j]);

                        // Append content of current cell.
                        testHex.append((*it)->hexSubfragments[i]->content[j]);

                        // Append content from one cell to the right, if it is occupied.
                        if(assemblyCol < GRID_COLS-1 && subfragmentHexGrid[assemblyRow+i][assemblyCol+1] != nullptr)
                            testHex.append(subfragmentHexGrid[assemblyRow+i][assemblyCol+1]->content[j]);

                        testUTF8 = ConvertHexLineToUTF8Line(testHex);
                        std::cout << testHex << " :: " << testUTF8;

                        size_t LHSMisencodedBytes = CountReplacementCharsForFirst(testUTF8);
                        if(IsValidWithinRange(testUTF8, LHSMisencodedBytes, testUTF8.length() - GetLastUTF8CharLength(testUTF8)))
                            std::cout << " :: PASS";
                        else
                        {
                            std::cout << " :: FAIL";
                            fragmentRejected = true;
                        }

                        const std::vector<std::string> invalidPairs =
                        {
                            "║║", "║|", "|║",
                            "-╔", "=╔", "|╔", "║╔",
                            "╗-", "╗=", "╗|", "╗║", "╗╔"
                        };

                        for (const std::string& whatPair : invalidPairs)
                        {
                            if(testUTF8.find(whatPair) != std::string::npos)
                            {
                                std::cout << " :: ERROR! - edges incompatible.";
                                fragmentRejected = true;
                                break;
                            }
                        }
                        std::cout << std::endl;
                        testHex.clear();
                        testUTF8.clear();
                    }
                } // for loop
            } // ! fragmentRejected
            if(!fragmentRejected)
            {
                fragmentMatchFound = true;
                fragmentToMerge = (*it);
                std::cout << "Fragment accepted." << std::endl;
                break;
            }
            else
            {
                std::cout << "Fragment rejected." << std::endl;
            }
        }
        std::cout << std::endl;
        if(fragmentMatchFound)
        {
            std::cout << "Merging map fragment " << fragmentToMerge->assignedID << " to grid:" << std::endl;
            for(size_t i = 0; i < fragmentToMerge->hexSubfragments.size(); i++)
            {
                Subfragment* hexSubfragmentToMerge = fragmentToMerge->hexSubfragments[i];
                subfragmentHexGrid[assemblyRow+i][assemblyCol] = hexSubfragmentToMerge;

                Subfragment* UTF8SubfragmentToMerge = fragmentToMerge->UTF8Subfragments[i];
                subfragmentUTF8Grid[assemblyRow+i][assemblyCol] = UTF8SubfragmentToMerge;

                std::cout << "     Hex subfragment " << hexSubfragmentToMerge->assignedID << " / " << "UTF8 subfragment " << UTF8SubfragmentToMerge->assignedID << " merged. Removing from pool." << std::endl;
                /// Planning to do a erase/remove here, but I want to find out whether subfragment pool is ever accessed.
                /// If not, can go ahead and just delete all subfragment pool code.
            }

            std::cout << "Removing merged subfragments from pool..." << std::endl;
            std::cout << "Removing merged map fragment from pool..." << std::endl;

        }
        else // !fragmentMatchFound
        {
            std::cout << "Could not find fragment match. Map assembly is at an impasse. Terminating assembly..." << std::endl;
            assemblyIsComplete = true;
        }

        if(fragmentsPool.size() == 0)
        {
            std::cout << "No fragments remain in pool. Assembly complete." << std::endl;
            assemblyIsComplete = true;
        }

        ///assemblyIsComplete = true;
/// MAIN MERGE STEP END
    }

/// FRAGMENT MERGING END.


/// Cleanup
    for(std::vector<MapFragment*>::iterator it = fragmentsPool.begin(); it != fragmentsPool.end();)
    {
        delete *it;
        it = fragmentsPool.erase(it);
    }

    return 0;
}
