class HttpServer{
private:
    short port;
    const char*IP;
    int ser_fd;
public:
    HttpServer(short port,const char*IP));
    void run();
    ~HttpServer();

}