#include <iostream>
#include <thread>
#include <dlfcn.h>
#include <sstream>
#include <mutex>
#include <cstring>
#include <condition_variable>

typedef bool(*Reader)(char[BUFSIZ], char[BUFSIZ]);
typedef bool(*Writer)(char[BUFSIZ], char[BUFSIZ]);

std::condition_variable read;
std::condition_variable written;

std::mutex locker;

int currentFile = 0;
int filesNum = 3;

void* handle;

char buffer[BUFSIZ] { 0 };

bool readFile(std::string filename, char destination[BUFSIZ]) {
    Reader reader = (Reader)dlsym(handle, "asyncFileRead");
    const char* error = dlerror();
    if(error) {
        std::cout << error << std::endl;
        return false;
    }

    std::stringstream sstream;
    sstream << "ref" << "//" << filename << ".txt";
    std::cout << sstream.str() << std::endl;
    if(false == reader((char*)sstream.str().data(), destination)) {
        return false;
    }

    return true;
}

bool writeFile(std::string filename, char source[BUFSIZ]) {
    Writer writer = (Writer)dlsym(handle, "asyncFileWrite");
    const char* error = dlerror();
    if(error) {
        std::cout << error << std::endl;
        return false;
    }

    std::stringstream sstream;
    sstream << "ref" << "//" << filename << ".txt";
    if(false == writer((char*)sstream.str().data(), source)) {
        return false;
    }

    return true;
}

bool appendFile(std::string filename, std::string source) {
    char buffer[BUFSIZ] { 0 };

    if(false == readFile(filename, buffer)) {
        return false;
    }

    std::string result = buffer;
    std::cout << "Result: " << result << std::endl; 
    result += source;
    char concatenatedString[BUFSIZ] { 0 };

    memcpy(concatenatedString, result.data(), result.size());

    if(false == writeFile(filename, concatenatedString)) {
        return false;
    }

    return true;
}


int main() {
    handle = dlopen("aioLib/aioLib.so", RTLD_LAZY);

    if(!handle) {
        std::cout << "Error: " << dlerror() << std::endl;
    }

    std::thread readerThread([&](){
        while(true) {
        
            std::unique_lock<std::mutex> uniqueLocker(locker);
            written.wait(uniqueLocker);

            if(currentFile++ == filesNum) {
                break;
            }

            if(readFile(std::to_string(currentFile), buffer) == false) {
                continue;
            }

            std::thread temp([](){
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                read.notify_one();
            });

            temp.detach();
            
        }
    });

    std::thread writerThread([&]() {
        while(true) {

            std::unique_lock<std::mutex> ulocker(locker);
            read.wait(ulocker);

            if(!appendFile("concatenated", buffer)) {
                return -1;
            } 

            memset(buffer, 0, BUFSIZ);

            std::thread temp([](){
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                written.notify_one();
            });

            temp.detach();

            if(currentFile == filesNum) {
                break;
            }

        }
    });


    std::thread begin([]() {
        written.notify_one();
    });

    readerThread.join();
    writerThread.join();

    begin.join();

    
    dlclose(handle);

    return 0;
}