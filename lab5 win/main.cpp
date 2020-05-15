#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <sstream>
#include <Windows.h>

using namespace std;
typedef BOOL(__cdecl Reader)(char[BUFSIZ], char[BUFSIZ]);
typedef BOOL(__cdecl Writer)(char[BUFSIZ], char[BUFSIZ]);

mutex locker;
condition_variable readingFinished;
condition_variable writingFinished;

int currentFile = 0;
int filesNum = 3;
char buffer[BUFSIZ]{ 0 };

BOOL readFileAsync(HINSTANCE asyncRW, string file, char dest[BUFSIZ]) {
	Reader* reader = (Reader*)GetProcAddress(asyncRW, "asyncFileRead");
	if (reader == nullptr) {
		return FALSE;
	}
	stringstream stream;
	stream << "ref" << "\\" << file << ".txt";
	if (reader((char*)stream.str().data(), dest) == FALSE) {
		return FALSE;
	}

	return TRUE;
}

BOOL writeFileAsync(HINSTANCE asyncRW, string file, char dest[BUFSIZ]) {
	Writer* writer = (Writer*)GetProcAddress(asyncRW, "asyncFileWrite");
	if (writer == nullptr) {
		return FALSE;
	}
	stringstream stream;
	stream << "ref" << "\\" << file << ".txt";
	if (writer((char*)stream.str().data(), dest) == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL appendFileAsync(HINSTANCE AsyncRW, string file, char dest[BUFSIZ]) {
	char source[BUFSIZ]{ 0 };
	if (readFileAsync(AsyncRW, file, source) == FALSE) {
		return FALSE;
	}
	string result = source;
	result += dest;
	char resultBuffer[BUFSIZ]{ 0 };
	memcpy_s(resultBuffer, BUFSIZ, result.data(), result.size());

	if (writeFileAsync(AsyncRW, file, resultBuffer) == FALSE) {
		return FALSE;
	}
	return TRUE;
}

int main() {

	HINSTANCE AsyncRW = LoadLibrary("aioLib.dll");
	if (AsyncRW == NULL) {
		cout << "Error: cannot load dynamic library" << endl;
		return 0;
	}

	thread readerThread([&]() {
		while (true) {
			unique_lock<mutex> uniqueLocker(locker);
			writingFinished.wait(uniqueLocker);

			if (readFileAsync(AsyncRW, to_string(++currentFile), buffer) == FALSE) {
				continue;
			}

			thread tr([]() {
				Sleep(100);
				readingFinished.notify_one();
				});
			tr.detach();

			if (currentFile == filesNum) {
				break;
			}
		}
		});

	thread writerThread([&]() {
		while (true) {
			unique_lock<mutex> uniqueLocker(locker);
			readingFinished.wait(uniqueLocker);
			if (appendFileAsync(AsyncRW, "concatenated", (char*)buffer) == FALSE) {
				break;
			}
			memset(buffer, 0, BUFSIZ);

			thread tr([]() {
				Sleep(100);
				writingFinished.notify_one();
				});
			tr.detach();

			if (currentFile == filesNum) {
				break;
			}
		}
		});

	Sleep(100);
	writingFinished.notify_one();

	readerThread.join();
	writerThread.join();

	FreeLibrary(AsyncRW);
	return 0;
}