# webserv
42's webserv w/ Molasz

### TODOs

* CGI post
* CGI check for while true, add a timeout
* binary data ?

### current bugs

* ctrl+C in telnet/nc leads to a weird timeout situation? I guess. *Idk if that's an improper termination problem or the server's problem.*

### CONFIGURATION FILE

#### SERVER
* port: mandatory
* host: default 127.0.0.1
* server\_name:
* error\_page:
* client\_max\_body\_size: default 10MB
* index: default index.html
* root: mandatory

#### ROUTE
* allow\_methods: default GET
* return:
* name: mandatory
* root: mandatory
* autoindex: default off
* index: default index.html
