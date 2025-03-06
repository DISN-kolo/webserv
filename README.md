# webserv
42's webserv w/ Molasz

### TODOs

* CGI response parsing for http response code
* CGI post
* CGI check for while true, add a timeout response
* graceful stop after ctrl c
* binary data ?

### current bugs

* none :)

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
