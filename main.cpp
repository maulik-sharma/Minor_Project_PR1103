#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <thread>
#include <sstream>
#include <fstream>
#include <ctime>
using namespace std;

#define PORT "3490"
#define BACKLOG 10

string mimeType(const string& path) {
    size_t last_dot_pos = path.find_last_of('.');

    if(last_dot_pos == string::npos) return "text/plain";

    string ext = path.substr(last_dot_pos);

    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".gif") return "image/gif";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".json") return "application/json";

    return "application/octet-stream";
}

string get_http_date() {
    time_t now = time(NULL);
    struct tm tm = *gmtime(&now);
    char buffer[128];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return string(buffer);
}

void handle_client(int* client_fd_ptr, string root_dir) {

    // got the fd and freed the heap memory
    int client_fd = *client_fd_ptr;
    delete client_fd_ptr;

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while(true) {

        // Now we have to recieve the things that we get from the browser
        
        // 1. buffer bana lete h
        // 2. read krte h req

        char buffer[16384] = {0};
        
        int bytes_rec = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if(bytes_rec == 0) {
            cout << "Client closed the connections." << endl;
            break;
        }
        else if(bytes_rec < 0) {
            cout << "Connection timeout (5 seconds)" << endl;
            break;
        }

        // cout << buffer << endl;

        string req_str(buffer);
        stringstream ss(req_str);

        string method, path, version;

        ss >> method >> path >> version;

        cout << "Method: " << method << endl;
        cout << "Path: " << path << endl;
        cout << "Version: " << version << endl;

        string line;
        getline(ss, line);
        // cout << "Line: " << line << endl;
        map<string, string> dictionaryy;

        while(getline(ss, line) && line != "\r" && !line.empty()) {
            // cout << line << endl;

            if( !line.empty() && line.back() == '\r') line.pop_back();

            size_t colon_position = line.find(":");
            if(colon_position != string::npos) {            
                string key = line.substr(0, colon_position);
                string value = line.substr(colon_position + 1);
                
                size_t start = value.find_first_not_of(" ");
                if (start != string::npos) value = value.substr(start);
                
                dictionaryy[key] = value;
            }
        }

        cout << "\n--- PARSED HEADERS ---" << endl;
        for(auto& it:dictionaryy) cout << it.first << " = " << it.second << endl;

        // RFC 9112 HOST VALIDATION
        if (dictionaryy.find("Host") == dictionaryy.end()) {
            string error_res = "HTTP/1.1 400 Bad Request\r\nContent-Length: 26\r\n\r\n400: Missing Host Header!";
            if(send(client_fd, error_res.c_str(), error_res.length(), 0) == -1) {
                perror("send");
            }
            close(client_fd);
            return; 
        }


        if (path.find("../") != string::npos) {
            string error_res = "HTTP/1.1 403 Forbidden\r\nContent-Length: 17\r\n\r\n403: Hacker blocked!";
            send(client_fd, error_res.c_str(), error_res.length(), 0);
            close(client_fd);
            return; 
        }

        if(path == "/") path = "/index.html";

        string http_res;

        if(method == "GET") {
            ifstream file(root_dir + path);
            if(file.good()) {
                stringstream file_buffer;
                file_buffer << file.rdbuf();
                string body = file_buffer.str();

                string mime = mimeType(path);

                http_res = "HTTP/1.1 200 OK\r\n"
                        "Server: Maulik-Bobby-Server/1.0\r\n"
                        "Date: " + get_http_date() + "\r\n"
                        "Content-Type: " + mime + "\r\n"
                        "Content-Length: " + to_string(body.length()) + "\r\n"
                        "\r\n" + body;
            }
            else http_res = "HTTP/1.1 500 Internal Server Error\r\n\r\nFile missing!";
        }
        else if(method == "POST") {
            if(dictionaryy.find("Content-Length") == dictionaryy.end()) {
                http_res = "HTTP/1.1 400 Bad Request\r\nContent-Length: 26\r\n\r\n400: Missing Content-Length Header!";
            }
            else {
                int content_length = stoi(dictionaryy["Content-Length"]);      
                    
                size_t body_start = req_str.find("\r\n\r\n");
                
                //blank line takes 4 characters
                string body_content = req_str.substr(body_start + 4);

                while(body_content.length() < content_length) {
                    char chunk_buffer[2048] = {0};

                    int bytes_recvd = recv(client_fd, chunk_buffer, sizeof(chunk_buffer) - 1, 0);

                    if(bytes_recvd <= 0) {
                        http_res = "HTTP/1.1 400 Bad Request\r\nContent-Length: 26\r\n\r\n400: Mismatch in Content-Length!";
                        send(client_fd, http_res.c_str(), http_res.length(), 0);
                        close(client_fd);
                        return;
                    }

                    body_content.append(chunk_buffer, bytes_recvd);
                }
                
                cout << "\n--- BODY CONTENT ---" << endl;
                cout << body_content << endl;
                cout << "--------------------" << endl;

                http_res = "HTTP/1.1 200 OK\r\n"
                        "Server: Maulik-Bobby-Server/1.0\r\n"
                        "Date: " + get_http_date() + "\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: " + to_string(body_content.length()) + "\r\n"
                        "\r\n" + body_content;
            }
        }
        else { 
            http_res = "HTTP/1.1 404 Not Found\r\n"
                    "Server: Maulik-Bobby-Server/1.0\r\n"
                    "Date: " + get_http_date() + "\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 13\r\n"
                    "\r\n"
                    "404 Not Found";
        }

        // send the reponse
        if(send(client_fd, http_res.c_str(), http_res.length(), 0) == -1) {
            perror("send");
        }

        if(dictionaryy.find("Connection") != dictionaryy.end() && dictionaryy["Connection"] == "close") {
            break;
        }
    }
    close(client_fd);
}

void* get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {

    string root_dir = ".";
    if(argc > 1) root_dir = argv[1];

    struct addrinfo hints, *serverinfo, *p;
    int new_fd, sockfd;
    int rev, yes = 1;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rev = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0 ) {
        cout << gai_strerror(rev) << endl;
        return 1;
    };

    for(p=serverinfo; p != NULL; p=p->ai_next) {
        if((sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol)) == -1 ) {
            perror("sever : socket");
            continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("server : setsockopt");
            continue;
        }

        if(bind(sockfd, p -> ai_addr, p -> ai_addrlen) == -1) {
            close(sockfd);
            perror("server : bind");
            continue;
        }

        break;
    }

    freeaddrinfo(serverinfo);

    if(p == NULL) {
        cout << "server: failed to bind" << endl;
        exit(1);
    }

    if(listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    cout << "server: waiting for browser connections..." << endl;
    
    while(1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (sockaddr *)&their_addr, &sin_size);

        if(new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        cout << "server: got connection from " << s << endl;

        // if(!fork()) {
        //     close(sockfd);
        //     if(send(new_fd, "HELLO, WORLD!", 13, 0) == -1) {
        //         perror("send");
        //     }
        //     close(new_fd);
        //     exit(0);
        // }

        int *client_fd = new int(new_fd);
        thread client_thread(handle_client, client_fd, root_dir);
        client_thread.detach();
        
        // close(new_fd);
    }   
    return 0;
}