#include "ApiServer.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string frontendDir;

    if (argc >= 2) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Invalid port argument '" << argv[1] << "', defaulting to 8080.\n";
            port = 8080;
        }
    }
    if (argc >= 3) {
        frontendDir = argv[2];
    }

    try {
        qlite::ApiServer server(port, frontendDir);
        server.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal server error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
