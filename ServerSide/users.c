#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "users.h"


void addUser(User **r, char *username, char *password)
{
    User *user = malloc(sizeof(User));
    if (!user) exit(EXIT_FAILURE);
    strcpy(user->username, username);
    strcpy(user-> password, password);
    user->next = *r;
    *r = user;
}

void fillUsers(User **r) {
    addUser(r, "hussein", "bassam");
    addUser(r, "ali", "tarhini");
    addUser(r, "lewandowski", "RL");
    addUser(r, "reus", "dortmund");
    addUser(r, "khodor", "ps4");
}
int checkUser(char *username, char **LoggedUsername) {
    User *r = NULL, *tmp = NULL;
    fillUsers(&r);
    tmp = r;
    while (tmp && strcmp(tmp->username, username)) tmp = tmp->next;
    if (tmp) {
        *LoggedUsername = tmp->username;
        return 1;
    }
    return 0;

}
int checkPass(char * username, char *password) {
    User *r = NULL, *tmp = NULL;
    fillUsers(&r);
    tmp = r;
    while (tmp && strcmp(tmp->username, username)) tmp = tmp->next;
    if (tmp && !strcmp(tmp->password, password)) return 1;
    return 0;
}


