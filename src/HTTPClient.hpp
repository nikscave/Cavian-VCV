#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <curl/curl.h>  // Not used yet, but safe to keep
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

class SimpleHTTPClient {
private:
    std::string hostname;
    int port;

    void setTimeouts(SOCKET sock, int seconds) {
#ifdef _WIN32
        DWORD timeout = seconds * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec = seconds;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const struct timeval*)&tv, sizeof(struct timeval));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const struct timeval*)&tv, sizeof(struct timeval));
#endif
    }

public:
    SimpleHTTPClient(const std::string& host, int p = 80)
        : hostname(host), port(p) {}

    ~SimpleHTTPClient() = default;

    // Existing GET - unchanged (good as is)
    bool get(const std::string& path, std::vector<uint8_t>& response) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) return false;

        setTimeouts(sock, 3);

        struct sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        struct hostent* he = gethostbyname(hostname.c_str());
        if (!he) {
            closesocket(sock);
            return false;
        }
        memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) != 0) {
            closesocket(sock);
            return false;
        }

        std::string request =
            "GET " + path + " HTTP/1.1\r\n"
            "Host: " + hostname + "\r\n"
            "Connection: close\r\n"
            "\r\n";

        if (send(sock, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
            closesocket(sock);
            return false;
        }

        char buffer[4096];
        response.clear();
        int bytesReceived;
        while ((bytesReceived = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            response.insert(response.end(), buffer, buffer + bytesReceived);
        }

        closesocket(sock);
        return !response.empty();
    }

bool put(const std::string& path,
         const uint8_t* data,
         size_t dataLen,
         const std::string& contentType = "application/octet-stream") {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return false;

    setTimeouts(sock, 3);

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    hostent* he = gethostbyname(hostname.c_str());
    if (!he) {
        closesocket(sock);
        return false;
    }
    memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sock, (sockaddr*)&server, sizeof(server)) != 0) {
        closesocket(sock);
        return false;
    }

    std::string header =
        "PUT " + path + " HTTP/1.1\r\n"
        "Host: " + hostname + "\r\n"
        "Content-Type: " + contentType + "\r\n"
        "Content-Length: " + std::to_string(dataLen) + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    if (send(sock, header.c_str(), header.length(), 0) == SOCKET_ERROR) {
        closesocket(sock);
        return false;
    }

    if (data && dataLen > 0) {
        if (send(sock, (const char*)data, dataLen, 0) == SOCKET_ERROR) {
            closesocket(sock);
            return false;
        }
    }

    // Drain response (CRITICAL)
    char buffer[4096];
    while (recv(sock, buffer, sizeof(buffer), 0) > 0) {}

    closesocket(sock);
    return true;
}


    // === NEW: POST method ===
    /**
     * Send an HTTP POST request
     * @param path        URL path (e.g. "/api/patterns")
     * @param data        Pointer to body data
     * @param dataLen     Length of body data
     * @param contentType Content-Type header (default: application/octet-stream)
     * @return true if request was sent successfully (does not check HTTP status)
     */
    bool post(const std::string& path,
              const uint8_t* data,
              size_t dataLen,
              const std::string& contentType = "application/octet-stream") {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) return false;

        setTimeouts(sock, 3);

        struct sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        struct hostent* he = gethostbyname(hostname.c_str());
        if (!he) {
            closesocket(sock);
            return false;
        }
        memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) != 0) {
            closesocket(sock);
            return false;
        }

        // Build headers
        std::string headers =
            "POST " + path + " HTTP/1.1\r\n"
            "Host: " + hostname + "\r\n"
            "Content-Type: " + contentType + "\r\n"
            "Content-Length: " + std::to_string(dataLen) + "\r\n"
            "Connection: close\r\n"
            "\r\n";

        // Send headers
        if (send(sock, headers.c_str(), headers.length(), 0) == SOCKET_ERROR) {
            closesocket(sock);
            return false;
        }

        // Send body (if any)
        if (dataLen > 0 && data) {
            if (send(sock, (const char*)data, dataLen, 0) == SOCKET_ERROR) {
                closesocket(sock);
                return false;
            }
        }

// In both post() versions, after sending body:
char buffer[4096];
while (recv(sock, buffer, sizeof(buffer), 0) > 0) {
    // Drain response
}

        // We don't read the response here - just confirm send succeeded
        // If you need the response, add an overload or separate method
        closesocket(sock);
        return true;
    }

    // Optional convenience overload for string data (e.g. JSON)
    bool post(const std::string& path, const std::string& body) {
        return post(path,
                    reinterpret_cast<const uint8_t*>(body.data()),
                    body.size(),
                    "application/json");
    }
};