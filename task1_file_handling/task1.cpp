#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class FileManager {
    fstream& fileStream;

public:
    FileManager(fstream& fs) : fileStream(fs) {}

    FileManager& operator<<(const string & text) {
        fileStream << text << "\n";
        return *this;
    }

    FileManager& operator<<(ifstream & sourceFile) {
        fileStream << sourceFile.rdbuf();
        return *this;
    }

    FileManager& operator>>(ostream & outConsole) {
        fileStream.clear();
        fileStream.seekg(0, ios::beg); 
        outConsole << fileStream.rdbuf();
        fileStream.clear(); 
        return *this;
    }
};

void handleInput(FileManager& fm) {
    int choice;
    cout << "  1. Text from keyboard\n";
    cout << "  2. Text from another file\n";
    cout << "  Choice: ";
    cin >> choice;

    if (choice == 1) {
        string input;
        cout << "  Enter your text: ";
        cin.ignore();
        getline(cin, input);
        fm << input;
        cout << "  Data written successfully.\n";
    } else if (choice == 2) {
        string sourceName;
        cout << "  Enter source filename: ";
        cin >> sourceName;
        ifstream sourceFile(sourceName);
        if (sourceFile.is_open()) {
            fm << sourceFile;
            cout << "  Data copied successfully.\n";
            sourceFile.close();
        } else {
            cout << "  Error: Could not open " << sourceName << "\n";
        }
    } else {
        cout << "  Invalid choice.\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    string mainFileName = argv[1];
    
    ofstream createTemp(mainFileName, ios::app);
    createTemp.close();

    fstream mainFile(mainFileName, ios::in | ios::out);

    if (!mainFile.is_open()) {
        cout << "Critical error: Could not open file.\n";
        return 1;
    }

    FileManager fm(mainFile);
    int option;

    do {
        cout << "\nFile Menu : \n";
        cout << "1 : Write (Overwrite from start)\n";
        cout << "2 : Append\n";
        cout << "3 : Read\n";
        cout << "4 : Exit\n";
        cout << "Choice: ";
        cin >> option;

        if (option == 1) {
            mainFile.clear(); 
            mainFile.seekp(0, ios::beg); 
            handleInput(fm);
        } else if (option == 2) {
            mainFile.clear(); 
            mainFile.seekp(0, ios::end); 
            handleInput(fm);
        } else if (option == 3) {
            cout << "\n--- Contents ---\n\n";
            fm >> cout;
            cout << "----------------\n\n";
        }
    } while (option != 4);

    mainFile.close();
    return 0;
}