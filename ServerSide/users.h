
typedef struct Users
{
	char username[250];
	char password[250];
	struct Users *next;
} User;


void addUser(User **r, char *username, char *password);
void fillUsers(User **r);
int checkUser(char *username, char **LoggedUsername);
int checkPass(char *username, char *password);
