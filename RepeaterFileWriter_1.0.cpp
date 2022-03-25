/*
Intention Repeater File Writer v1.0
Created 3/25/2022 by Anthro Teacher aka Thomas Sweet
Repeats an intention to a file a specified number of times.
*/

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char **argv)
{

    std::string intention, str_repetitions, filename;
    unsigned long long int repetitions;

    cout << "Intention Repeater File Writer v1.0" << endl;
    cout << "By Anthro Teacher (AnthroHeart) aka Thomas Sweet" << endl
         << endl;

    cout << "Intention: ";
    std::getline(std::cin, intention);

    cout << "Repetitions: ";
    std::getline(std::cin, str_repetitions);

    repetitions = std::stoll(str_repetitions);

    cout << "Filename: ";
    std::getline(std::cin, filename);

    ofstream o;
    o.open(filename);

    for (unsigned long long int i = 0; i < repetitions; i++)
    {
        o << intention << "\n";
    }

    o.close();

    cout << "INTENTION REPEATED TO " << filename << endl;

    return 0;
}