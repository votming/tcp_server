=Набор sql-скриптов для воссоздаия схемы БД для работы сервера=
База calc_base
Таблицы:
"Create table users (login varchar(100) not null, password varchar(40) not null, balance integer);"
"Create table activities (login varchar(100) not null, activity_time timestamp default NULL, user_request varchar(100) not null, result varchar(100) not null);"


client.cpp - tcp-клиент для ввода команд вручную
client_overload-test.cpp - клиент, который отсылает 3 разных команды на сервер без остановки, пока баланс пользователя положителен
server.cpp - исходники для tcp-сервера 
makefile - исходный файл для сборки сервера