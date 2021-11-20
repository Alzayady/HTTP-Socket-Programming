# HTTP-Socket-Programming
# Server 
### 1) assign every new client to a new thread 
### 2) can handle GET and POST requests 
### 3) in case of GET request, it response with OK 200 and the file or Not Found 404 in case of the file doesn't exist
### 4) in case of POST it save the file into the server and response with OK 202
### 5) add a dynamic timeout ( depending on the current number of clients on the server ) for each client so avoid useless waiting on an unused socket



# Client 

### 1) for each request assign it to a new thread to allow pipelining
### 2) add mutex for each request to avoid race condition


