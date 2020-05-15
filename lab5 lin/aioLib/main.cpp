#include <sys/types.h>
#include <aio.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>


extern "C" bool asyncFileRead(char path[BUFSIZ], char buffer[BUFSIZ]) {
    int fileDescriptor = open((char*)path, O_RDONLY, S_IRUSR);
    
    if(fileDescriptor == -1) {
        std::cout  << "Unable to open file" << std::endl;
        return false;
    }

    // create the control block structure
    aiocb cb {0};

    cb.aio_nbytes = BUFSIZ;
    cb.aio_fildes = fileDescriptor;
    cb.aio_offset = 0;
    cb.aio_buf = buffer;
    cb.aio_lio_opcode = LIO_READ;

    // read
    if(aio_read(&cb) == -1) {
        std::cout << "Unable to create request" << std::endl;
        close(fileDescriptor);
        return false;
    }

    // wait until the request has finished
    while(aio_error(&cb) == EINPROGRESS);

    int numBytes = aio_return(&cb);

    if(numBytes == -1) {
        std::cout << "Error: "  << errno << std::endl;
        return false;
    }

    close(fileDescriptor);
    return true;
}

extern "C" bool asyncFileWrite(char path[BUFSIZ], char buffer[BUFSIZ]) {
    int fileDescriptor = open((char*)path, O_WRONLY, S_IWUSR);

    if(fileDescriptor == -1) {
        std::cout << "Unable to open file" << std::endl;
        return false;
    }

    // create the control block structure
    aiocb cb {0};

    cb.aio_nbytes = BUFSIZ;
    cb.aio_fildes = fileDescriptor;
    cb.aio_offset = 0;
    cb.aio_buf = buffer;
    cb.aio_lio_opcode = LIO_WRITE;

    if(aio_write(&cb) == -1) {
        std::cout << "Unable to create request" << std::endl;
        close(fileDescriptor);
        return false;
    }

    while(aio_error(&cb) == EINPROGRESS);

    int err = aio_error(&cb);

    if(err != 0) {
        close(fileDescriptor);
        std::cout << "Error: " << err << std::endl;
        return false;
    }

    return true;
}


int main() {
    char buffer[BUFSIZ] { 0 };
    char path[BUFSIZ] = "ref//test.txt";
    asyncFileRead(path, buffer);

    std::cout << buffer << std::endl;
    char path1[BUFSIZ] = "ref//concatenated.txt";
    char writeBuffer[BUFSIZ] = "SPOVM rocks!!!!!!!!!!!!";
    asyncFileWrite(path1, writeBuffer);

    return 0;
}