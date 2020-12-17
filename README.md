# webserver
1. Download
```
git clone https://github.com/DaurKas/webserver
```
2. Compile
```
make all
```
3. Start server 
```
./bin/server <port>
```
4. Start client or use browser
```
./bin/client
```
5. Request
```
[host]:[port]/[path]
```
6. Example
```
127.0.0.1:2001/resources/multimedia/test.jpg
127.0.0.1:2001/resources/cgi-bin/grades?name=dauren&subject=math
127.0.0.1:2001/resources/html/index.html
```
7. Post text
```
[host]:[post]/[file]%[args]
127.0.0.1:2001/resources/database/grades.csv%name=dauren&subject=math&grade=5
```
