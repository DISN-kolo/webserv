# webserv
42's webserv w/ Molasz

### TODOs

* cleanup TODOs, XXXs, FIXMEs...
* evaluate

### current bugs

* cgi POST stopped working, up to the point of not even beginning the cgi process. maybe, add a body checker right in the on head located function.. goes for both post variants btw 

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
