=SQL scripts for server database=
Base name: calc_base
Tables: 
"Create table users (login varchar(100) not null, password varchar(40) not null, balance integer);"
"Create table activities (login varchar(100) not null, activity_time timestamp default NULL, user_request varchar(100) not null, result varchar(100) not null);"


client.cpp - tcp-client with manual input(mode 1) or machine input(mode 2)
server.cpp - tcp-server source code
makefile - file for server build