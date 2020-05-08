#define _CRT_SECURE_NO_WARNINGS

#define STRING_SIZE 33
#define MAX_PROCESS_QTY 33
#define CREATE_NEW_PROCESS '+'
#define DESTROY_LAST_ADDED_PROCESS '-' 
#define QUIT 'q'

#include <windows.h>
#include <conio.h>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

const char* alphabetChars[] = { 
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
	"l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v",
	"w", "x", "y", "z", "_", "#", "$", "@", "&", "*", "^" 
};

//--------------------------FUNCTIONS------------------------------
string getUniqString();
PROCESS_INFORMATION createNewProcess(string appName, string commandLine);
void printUniqString(char* uniqString);
void processManager(string appName);
char* stringToChars(string str);
//-----------------------------------------------------------------


int main(int argc, char* argv[]) {
	setlocale(0, "RUS");
	if (argc == 2) {
		printUniqString(argv[1]);
	} else {
		processManager(argv[0]);
	}
	return 1;
}

string getUniqString() {
	string uniqString = "";

	srand(time(nullptr));
	for (size_t i = 0; i < STRING_SIZE; i++) {
		uniqString += alphabetChars[rand() % (sizeof(alphabetChars) / sizeof(char*))];
	}

	return uniqString;
}

PROCESS_INFORMATION createNewProcess(string appName, string commandLine) {
	STARTUPINFO startupinfo;
	ZeroMemory(&startupinfo, sizeof(startupinfo));
	PROCESS_INFORMATION processInformation;
	ZeroMemory(&processInformation, sizeof(processInformation));
	string str = appName + " " + commandLine;
	CreateProcess(nullptr, stringToChars(str), nullptr, nullptr, TRUE,
		NULL, nullptr, nullptr, &startupinfo, &processInformation);
	return processInformation;
}

void printUniqString(char* uniqString) {
	HANDLE canWriteEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "canWrite");
	HANDLE cannotWriteEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, uniqString);
	while (true) {
		if (WaitForSingleObject(canWriteEvent, INFINITE) == WAIT_OBJECT_0) {
			if (WaitForSingleObject(cannotWriteEvent, 50) == WAIT_OBJECT_0) {
				SetEvent(canWriteEvent);
				return;
			}

			for (size_t i = 0; i < STRING_SIZE; i++) {
				cout << uniqString[i];
				Sleep(100);
			}

			cout << endl;

			SetEvent(canWriteEvent);
		}
	}
	return;
}

void processManager(string appName) {
	int activeProcessQty = 0;
	char userChoice = NULL;

	HANDLE canWriteEvent = CreateEvent(nullptr, FALSE, TRUE, "canWrite");

	PROCESS_INFORMATION processInformationArray[MAX_PROCESS_QTY];
	HANDLE cannotWriteEventArray[MAX_PROCESS_QTY];

	while (userChoice = _getch()) {
		if (tolower(userChoice) == CREATE_NEW_PROCESS && activeProcessQty <= MAX_PROCESS_QTY) {
			char* uniqString = stringToChars(getUniqString());
			cannotWriteEventArray[activeProcessQty] = CreateEvent(nullptr, FALSE, FALSE, uniqString);
			processInformationArray[activeProcessQty] = createNewProcess(appName, uniqString);
			activeProcessQty++;
		} else if (userChoice == DESTROY_LAST_ADDED_PROCESS) {
			if (--activeProcessQty >= 0) {				
				WaitForSingleObject(canWriteEvent, INFINITE);
				SetEvent(cannotWriteEventArray[activeProcessQty]);
				TerminateProcess((processInformationArray[activeProcessQty]).hProcess, 9);
				SetEvent(canWriteEvent);
			}
		} else if (tolower(userChoice) == QUIT) {
			break;
		}
	}
	
	while (activeProcessQty >= 0) {
		WaitForSingleObject(canWriteEvent, INFINITE);
		SetEvent(cannotWriteEventArray[activeProcessQty]);
		TerminateProcess((processInformationArray[activeProcessQty]).hProcess, 9);
		activeProcessQty--;
		SetEvent(canWriteEvent);
	}
	
	system("pause");
}

char* stringToChars(string str) {
	size_t strSize = str.length();
	char* chars = (char*)malloc((strSize) * sizeof(char));
	for (size_t i = 0; i < strSize; i++) {
		chars[i] = str[i];
	}
	return chars;
}
