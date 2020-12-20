#include<stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define MAXLINE 80
void init_reset(char* args[])
{
	int dem = 0;
	for (dem; dem < MAXLINE / 2 + 1; dem++)
	{
		args[dem] = NULL;
	}
}
void report_and_exit(const char* msg)
{
	perror(msg);
	exit(-1);
}
int checkCharacterAmpersand(char* args[])
{
	int i = 0;
	while (args[i] != 0)
	{
		if (strcmp(args[i], "&") == 0)
		{
			args[i] = NULL;
			return 0;
		}
		i++;
	}
	return 1;
}
int checkUsingPipe(char* args[])
{
	int i = 0;
	while (args[i] != NULL)
	{
		if (strcmp(args[i], "|") == 0)
			return 0;
		i++;
	}
	return 1;
}
void Execution(char* args[], int check)
{
	int temp = fork();
	if (temp == 0)
	{
		execvp(args[0], args);
		init_reset(args);
	}
	else
	{
		if (check != 0)
		{
			wait(NULL);
		}
		else
		{
			_exit(0);
		}
	}
}
int checkOperator(char* tempt, char* args[])
{
	int op = 3;
	int i = 0;
	while (args[i] != NULL)
	{
		if (strcmp(args[i], "<") == 0)
		{
			op = 1;
			break;
		}
		else if (strcmp(args[i], ">") == 0)
		{
			op = 2;
			break;
		}
		else
		{
			op = 0;
		}
		i++;
	}
	return op;
}
void RedirectedExecution(char* args[], int op, int check)
{
	pid_t pid;
	pid = fork();
	if (pid == 0)
	{
		int i = 0;
		char file_name[70];
		while (args[i] != NULL)
		{
			if (strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0)
			{
				args[i] = NULL;
				strcpy(file_name, args[i + 1]);
				break;
			}
			i++;
		}
		int fd;
		if (op == 1)
		{
			fd = open(file_name, O_RDONLY, 0);
			dup2(fd, STDIN_FILENO);
		}
		else
		{
			fd = creat(file_name, 0644);
			dup2(fd, STDOUT_FILENO);
		}
		close(fd);
		execvp(args[0], args);
		init_reset(args);
	}
	else
	{
		if (check != 0)
		{
			wait(NULL);
		}
		else
		{
			_exit(0);
		}
	}
}
void ExecuteUsingPipe(char* args[])
{
	char* args1[40];
	char* args2[40];
	int pipeFDs[2];
	int k = 0;
	for (k; k < 40; k++)
	{
		args1[k] = NULL;
		args2[k] = NULL;
	}
	int i = 0;
	int j = 0;
	while (strcmp(args[i], "|") != 0 && args[i] != NULL)
	{
		args1[i] = (char*)malloc(30);
		strcpy(args1[i], args[i]);
		i++;
	}

	i++;
	while (args[i] != NULL)
	{
		args2[j] = (char*)malloc(30);
		strcpy(args2[j], args[i]);
		i++;
		j++;
	}
	if (pipe(pipeFDs) < 0) report_and_exit("pipeFD");
	int cpid = fork();
	if (cpid < 0) report_and_exit("fork");

	if (0 == cpid)
	{    /*** child ***/
		close(pipeFDs[0]);
		dup2(pipeFDs[1], STDOUT_FILENO);
		execvp(args1[0], args1);
		//perror("execvp");
		close(pipeFDs[1]);
		_exit(0);
	}
	else
	{

		close(pipeFDs[1]);
		dup2(pipeFDs[0], STDIN_FILENO);
		execvp(args2[0], args2);
		// perror("execvp");
		close(pipeFDs[0]);
		_exit(0);
	}

}
void enterHandle(char* args[], char* History)
{
	char* Tempt = (char*)malloc(MAXLINE * sizeof(char));
	gets(Tempt);
	if (strcmp(Tempt, "!!") != 0)
	{
		strcpy(History, Tempt);
		int i = 0;
		args[0] = strtok(Tempt, " ");
		do
		{
			i++;
			args[i] = strtok(NULL, " ");
		} while (args[i] != NULL);
		int check = checkCharacterAmpersand(args);
		if (checkOperator(Tempt, args) == 1 || checkOperator(Tempt, args) == 2)
		{
			RedirectedExecution(args, checkOperator(Tempt, args), check);
		}
		else
		{
			if (checkUsingPipe(args) == 0)
			{
				int pid2 = fork();
				if (pid2 == 0) {
					if (check == 0) printf("[%d]\n", getpid());
					ExecuteUsingPipe(args);
					if (check == 0) {

						_exit(0);
					}
				}
				else if (pid2 != 0 && check != 0)
				{
					wait(NULL);
				}
			}
			else
			{
				Execution(args, check);
			}
		}
	}
	else if (strcmp(Tempt, "!!") == 0 && strcmp(History, "") == 0)
	{
		printf("No commands in history\n");
	}
	else
	{
		printf("previous command: %s\n", History);
	}

}

int main()
{
	char* args[MAXLINE / 2 + 1];
	init_reset(args);
	char* History = (char*)malloc(MAXLINE * sizeof(char));
	strcpy(History, "");
	int shouldrun = 1;
	while (shouldrun)
	{
		printf("osh>");
		fflush(stdout);
		enterHandle(args, History);

	}
	return 0;
}
