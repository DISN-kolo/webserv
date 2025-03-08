# webserv
42's webserv w/ Molasz

### TODOs

* CGI post
* CGI check for while true, add a timeout response
* graceful stop after ctrl c
* binary data ?

### current bugs

* weird dangling stuff in i.php for example, at the end of the "view source" after the \</html\> already happened
* CGI buffering size and send's crash in strange cases that we can't reproduce on 42's macs

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
