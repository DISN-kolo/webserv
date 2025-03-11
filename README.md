# webserv
42's webserv w/ Molasz

### TODOs

* cleanup TODOs, XXXs, FIXMEs...
* evaluate

### current bugs

* no check of keepalive in case of throw. if we do apache benchmark tests with http 1.0, it'd be cool to respect non-keep-aliving even if we get a throw (404, for example)

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
