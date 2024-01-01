#include <cstdio>
#include <filesystem>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;

#define   PATH_COL_ID 0
#define    DIR_COL_ID 1
#define   FILE_COL_ID 2
#define STATUS_COL_ID 3

void PrintWithCol(std::string value, unsigned int colId)
{
    attron(COLOR_PAIR(colId));
    printw("%s", value.c_str());
    attroff(COLOR_PAIR(colId));
    refresh();
}

void ShowEntries(fs::path path, unsigned int entryIdx)
{
    PrintWithCol(path.string() + '\n', PATH_COL_ID);
    try{
        unsigned int curIdx = 0;
        for(const auto& entry : fs::directory_iterator(path)){
            std::string entryVal = (curIdx == entryIdx ? "> " : "  ") + entry.path().filename().string() + " " +
                    (fs::is_directory(entry.path()) ? "(dir)" : "(file)") + '\n';
            PrintWithCol(entryVal, (fs::is_directory(entry.path()) ? DIR_COL_ID : FILE_COL_ID));
            ++curIdx;
        }
    } catch(const fs::filesystem_error er){
        endwin();
        std::cerr << "An error occured: " << er.what() << std::endl;
        exit(1);
    }
}

std::string ReadFromUser()
{
    std::string input;
    int ch;
    while((ch = getch()) != '\n'){
        input.push_back(ch);
        addch(ch);
    }
    return input;
}

int main(int argc, char* argv[])
{
    initscr();
    noecho();
    cbreak();
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    init_pair(PATH_COL_ID, COLOR_GREEN, COLOR_BLACK);
    init_pair(DIR_COL_ID, COLOR_BLUE, COLOR_BLACK);
    init_pair(FILE_COL_ID, COLOR_RED, COLOR_BLACK);
    init_pair(STATUS_COL_ID, COLOR_YELLOW, COLOR_BLACK);

    fs::path curPath = fs::current_path();
    int curEntryIdx = 0;
    while(true){
        unsigned int entryCnt = 0;
        std::vector<fs::directory_entry> entries; 
        for (const auto& entry : fs::directory_iterator(curPath)) {
            ++entryCnt;
            entries.push_back(entry);
        }

        ShowEntries(curPath, curEntryIdx);

        char inp = static_cast<char>(getch());
        if(inp == 's'){
            ++curEntryIdx;
            curEntryIdx %= entryCnt;
        } else if(inp == 'w'){
            if(curEntryIdx > 0) --curEntryIdx;
            curEntryIdx = abs(curEntryIdx) % entryCnt;
        } else if(inp == '\n'){
            if(fs::is_directory(entries[curEntryIdx].path())){
                curPath = entries[curEntryIdx].path().string();
                curEntryIdx = 0;
            }
        } else if(inp == 'b'){
            if(curPath.has_parent_path()) curPath = curPath.parent_path().string();
            curEntryIdx = 0;
        } else if(inp == 'a'){
            PrintWithCol("\n\nAdding entry (dirs end with a /): ", STATUS_COL_ID);
            std::string entryNm = ReadFromUser();

            if(entryNm[entryNm.length() - 1] == '/'){
                if(!fs::create_directory(curPath.string() + "/" + entryNm)){
                    endwin();
                    std::cerr << "Could not create directory" << std::endl;
                    exit(1);
                }
            } else{
                std::ofstream newFile(curPath.string() + "/" + entryNm);
                if(newFile.is_open())
                    newFile.close();
                else{
                    endwin();
                    std::cerr << "Could not create file" << std::endl;
                    exit(1);
                }
            }
        } else if(inp == 'd'){
            PrintWithCol("\n\nYou sure you wanna delete that one? (y/n): ", STATUS_COL_ID);
            std::string choice = ReadFromUser();

            if(choice == "y"){
                if(!fs::remove(entries[curEntryIdx].path())){
                    endwin();
                    std::cerr << "Could not delete entry" << std::endl;
                    exit(1);
                }
            }
        } else if(inp == 'r'){
            PrintWithCol("\n\nRenaming entry: ", STATUS_COL_ID);
            std::string newEntryNm = ReadFromUser();
            fs::rename(entries[curEntryIdx].path(), curPath.string() + "/" + newEntryNm);
        } else if(inp == 'q'){
            endwin();
            std::cout << "Good bye sir!" << std::endl;
            exit(0);
        }

        clear();
    }

    endwin();
    return 0;
}
