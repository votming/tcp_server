=����� sql-�������� ��� ���������� ����� �� ��� ������ �������=
���� calc_base
�������:
"Create table users (login varchar(100) not null, password varchar(40) not null, balance integer);"
"Create table activities (login varchar(100) not null, activity_time timestamp default NULL, user_request varchar(100) not null, result varchar(100) not null);"


client.cpp - tcp-������ ��� ����� ������ �������
client_overload-test.cpp - ������, ������� �������� 3 ������ ������� �� ������ ��� ���������, ���� ������ ������������ �����������
server.cpp - ��������� ��� tcp-������� 
makefile - �������� ���� ��� ������ �������