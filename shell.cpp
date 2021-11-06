#include<iostream>
#include<unistd.h>
#include<limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
using namespace std;
#define LINE_LEN     80
#define MAX_ARGS     64
#define MAX_ARG_LEN  16
#define MAX_PATHS    64
#define MAX_PATH_LEN 96
#define WHITESPACE   " .,\t\n"


void removeAfterSpaces(char* ptr, int index, int& inputIndex, int& outputIndex)
{
    int count = 0;
    index++;
    while (ptr[index + count] == ' ')
    {
        count++;
    }

    for (int i = index + count; i <= strlen(ptr); ++i)
    {
        ptr[i - count] = ptr[i];
    }

    for (int i = 0; i < strlen(ptr); ++i)
    {
        if (ptr[i] == '<' || ptr[i] == '>')
        {
            ptr[i] == '<' ? inputIndex = i : outputIndex = i;
        }
    }

    return;
}

void removeBeforeSpaces(char* ptr, int index, int &inputIndex, int &outputIndex)
{
    int count = 0;
    index--;
    while (ptr[index - count] == ' ')
    {
        count++;
    }

    for (int i = index - count + 1; i <= strlen(ptr); ++i)
    {
        ptr[i] = ptr[i + count];
    }

    for (int i = 0; i < strlen(ptr); ++i)
    {
        if (ptr[i] == '<' || ptr[i] == '>')
        {
            ptr[i] == '<' ? inputIndex = i : outputIndex = i;
        }
    }
    return;
}

int inputF(char* commandLine, char* basicCommand, char* inputFile, char* outputFile, char commands[][30], int &totalCommands)
{
    int inputIndex = 0, outputIndex = 0;
    int pipeCount = 0;
    int Return = 0;
    for (int i = 0; i < strlen(commandLine); ++i)
    {
        if (commandLine[i] == '<' || commandLine[i] == '>')
        {
            commandLine[i] == '<' ? inputIndex = i : outputIndex = i;
        }
        else if (commandLine[i] == '|')
        {
            pipeCount++;
        }
    }

    if (inputIndex == 0 && outputIndex == 0)
    {
        strcpy(basicCommand, commandLine);
        Return = 0;
    }
    else if (inputIndex != 0 && outputIndex != 0)
    {
        removeBeforeSpaces(commandLine, inputIndex, inputIndex, outputIndex);
        removeAfterSpaces(commandLine, inputIndex, inputIndex, outputIndex);
        removeBeforeSpaces(commandLine, outputIndex, inputIndex, outputIndex);
        removeAfterSpaces(commandLine, outputIndex, inputIndex, outputIndex);
        int i = 0;
        for (i = 0; i < inputIndex; ++i)
        {
            basicCommand[i] = commandLine[i];
        }
        basicCommand[i++] = '\0';
        int j = 0;
        for ( j = 0; i < outputIndex; i++)
        {
            inputFile[j++] = commandLine[i];
        }
        inputFile[j++] = '\0';
        ++i;
        for (j = 0; i <= strlen(commandLine); ++i)
        {
            outputFile[j++] = commandLine[i];
        }
        Return = 3;
    }
    else if (inputIndex != 0)
    {
        removeBeforeSpaces(commandLine, inputIndex, inputIndex, outputIndex);
        removeAfterSpaces(commandLine, inputIndex, inputIndex, outputIndex);
        int j = 0;
        for (j = 0; j < inputIndex; ++j)
        {
            basicCommand[j] = commandLine[j];
        }
        basicCommand[j++] = '\0';
        for (int i = 0; j <= strlen(commandLine); j++)
        {
            inputFile[i++] = commandLine[j];
        }
        Return = 1;
    }
    else
    {
        removeBeforeSpaces(commandLine, outputIndex, inputIndex, outputIndex);
        removeAfterSpaces(commandLine, outputIndex, inputIndex, outputIndex);
        int k = 0;
        for (k = 0; k < outputIndex; ++k)
        {
            basicCommand[k] = commandLine[k];
        }
        basicCommand[k++] = '\0';
        for (int i = 0; k <= strlen(commandLine); k++)
        {
            outputFile[i++] = commandLine[k];
        }
       // cout << outputFile;
        Return = 2;
    }
    
    totalCommands = pipeCount + 1;
    if (totalCommands > 1)
    {
        int index = 0;
        int element = 0;
        for (int i = 0; i < strlen(basicCommand); ++i)
        {
            if (basicCommand[i] != '|')
            {
                commands[element][index] = basicCommand[i];
                index++;
            }
            else
            {
                commands[element][index] = '\0';
                element++;
                index = 0;
            }
        }
        commands[element][index] = '\0';
    }

    return Return;
}

struct command_t
{
  char *name;
  int argc;
  char *argv[MAX_ARGS]; //64

  command_t()
  {
  	name = new char[50];
  	argc = 0;
  }
  
  void print()
  {
  	for (int i = 0; i < argc; ++i)
  		cout << argv[i] << endl;
  }
  
  void freeMemory ()
  {
  	if (name != nullptr)
  		delete [] name;
  	if (argc != 0)
  	{
  		for (int i = 0; i < argc; ++i)
  			delete [] argv[i];
  	}
  	argc = 0;
  	name = nullptr;
  }
  
  ~command_t ()
  {
  	freeMemory();
  }
};

void printPrompt()
{
    char promptCharacter = '>';
    char hostName[HOST_NAME_MAX];
    char userName[LOGIN_NAME_MAX];
    char currentDir[PATH_MAX + 1];
    gethostname(hostName, HOST_NAME_MAX);
    getlogin_r(userName, LOGIN_NAME_MAX);
    getcwd(currentDir, PATH_MAX + 1);
    cout << userName << "@" << hostName << ":~" << currentDir << promptCharacter << " ";
    return;
}

int readCommand(char *buffer)
{
    int index = 0;
    int ch;
    while (index < LINE_LEN) 
    {
        ch = getchar();
	if (index == 0 && ch == EOF)
		return -1;
        if (ch == EOF || ch == '\n' || index == LINE_LEN - 1) 
        {
            buffer[index] = '\0';
            return 0;
        }
        else 
        {
            buffer[index] = ch;
        }
        index++;
    }
    return 0;
}

int parsePath(char *dirs[])
{
  
  const char *pathEnvVar;
  char *thePath;
  for (int i=0; i < MAX_PATHS; i++)
    dirs[i] = nullptr;
  pathEnvVar = (char *) getenv("PATH");
  thePath = (char *) malloc (strlen(pathEnvVar) + 1);
  strcpy(thePath, pathEnvVar);


/* Loop to parse thePath. Look for a ":"
 * delimiter between each path name.
 */
 
  bool temp = false;
  int  j = 0, k = 0;
  for(int i =0; i < strlen(thePath); i++)
  {
  	if(temp == false)
  	{
  		dirs[j] = new char[MAX_PATH_LEN];
  		temp = true;
  	}
  	if(thePath[i] != ':')
  	{
  		dirs[j][k++] = thePath[i];
  	}
  	else
  	{
  	    dirs[j][k] = '\0';
  		temp = false;
  		k = 0;
  		j++;
  	}
  }
  dirs[j][k] = '\0';
  free(thePath);
  return 0;
}

void parseCommand(char* commandLine, command_t *command)
{
	int i;
	for (i = 0; commandLine[i] != ' ' && i < strlen(commandLine); i++)
	{

		command->name[i] = commandLine[i];
	}
	command->name[i] = '\0';

	bool temp = true;
	int index = 0;
	int element = 0;
	command -> argv[element] = new char[50];
	strcpy (command->argv[element], command->name);
	command->argc += 1;
	int j = 0;
	for (j = i + 1; j < strlen(commandLine); j++)
	{
		element = command->argc;
		if (temp)
		{
			command -> argv[element] = new char[15];
			temp = false;
		}

		if (commandLine[j] != ' ')
		{
			command->argv[element][index] = commandLine[j];
			index++;
		}
		else
		{
			command->argv[element][index] = '\0';
			command->argc += 1;
			index = 0;
			temp = true;
		}

	}
	if (j == strlen(commandLine))
	{
		command->argc += 1;
		command->argv[element][index] = '\0';
	}
	command->argv[element + 1] = nullptr;
	return;
}

char *lookupPath (char *argv[], char *dirs[])
{
	
// Check to see if file name is already an absolute path
  if (*argv[0] == '/') 
  {
  	return argv[0];
  }

// Look in PATH directories.
// Use access() to see if the file is in a dir.

  char *temp = new char[MAX_PATH_LEN + 50];
  for (int i = 0; dirs[i]  != nullptr; i++) 
  {
  	int k = strlen(dirs[i]);
  	strcpy (temp, dirs[i]);
  	temp[k++] = '/';
  	
  	
	for(int j = 0; j <= strlen (argv[0]); j++)
	{	
		temp[k++] = argv[0][j];
	} 
	
	const char* ptr = temp;
	if(access(ptr,F_OK)==0)
	{
		return temp;	
	}
  }
  delete [] temp;
  return nullptr;
}

int main() {
  char* dirs[MAX_PATHS];
  parsePath(dirs); 
  command_t command; //changed
  char commandLine[LINE_LEN];
  char basicCommand[LINE_LEN];
  char inputFile[LINE_LEN];
  char outputFile[LINE_LEN];	
  int totalCommands = 0;
  char Commands[10][30];
  bool flag = true;
  bool ampersand = false;
  int io_redirection = 0;
  command_t allCommands[10];
    
  while(flag) 
  {
    ampersand = false;	
    printPrompt();
    if (readCommand(commandLine) == -1 || strcmp(commandLine, "exit") == 0)
    {
    	flag = false;
    }
    else
    {
    	    if (commandLine[strlen(commandLine) - 1] == '&')
	    {
	    	ampersand = true;
	    	commandLine[strlen(commandLine) - 1] = '\0';
	    }
	    io_redirection = inputF(commandLine, basicCommand, inputFile, outputFile, Commands, totalCommands);
	    if (totalCommands == 1)
	    {
		    pid_t pid = fork();	
		    if(pid==0)
	            {    
			    //command = new command_t;//changed
			    parseCommand(basicCommand, &command);
			    char *ptr = lookupPath(command.argv, dirs); //changed
			    if (ptr == nullptr) 
			    {
			    	cout <<"Error Finding Command" << endl;
			    	return 0;
			    }
			    else
			    {	
			    	if (io_redirection == 1)
			    	{
			    		int inputFD = open (inputFile, O_RDONLY);
			    		if (inputFD == -1)
			    		{
			    			cout << "Input File does not exit" << endl;
			    			return 0;
			    		}
			    		dup2(inputFD, 0);
			    	}
			    	else if (io_redirection == 2)
			    	{
			    		int outputFD = open (outputFile, O_WRONLY | O_CREAT);
			    		dup2 (outputFD, 1);
			    	}
			    	else if(io_redirection == 3)
			    	{
			    		int inputFD = open (inputFile, O_RDONLY);
			    		if (inputFD == -1)
			    		{
			    			cout << "Input File does not exit" << endl;
			    			return 0;
			    		}
			    		dup2(inputFD, 0);
			    		int outputFD = open (outputFile, O_WRONLY | O_CREAT);
			    		dup2 (outputFD, 1);
			    	}
			    	execv(ptr, command.argv);
			    }
		     } 
	    	          
	    }
	    else if (totalCommands > 1)
            {
		pid_t pid2 = fork();
            	if (pid2 == 0)
            	{
		    	int pipes[9][2];
		    	int TotalPipes = totalCommands - 1;
		    	for (int i = 0; i < totalCommands - 1; ++i)
		    	{
		    		pipe(pipes[i]);
		    	}
		    	for (int i = 0 ; i < totalCommands; ++i)
		    	{
		    		parseCommand(Commands[i], &allCommands[i]);
		    		char *ptr1 = lookupPath(allCommands[i].argv, dirs);
		    		if (ptr1 == nullptr) 
			    	{
			 		cout <<"Error Finding Command" << endl;
				   	return 0;
				}
				pid_t pid1 = fork();
				if (pid1 > 0)
				{
					if (i - 1 >= 0)
						close(pipes[i - 1][0]);
					if (i < TotalPipes)
						close(pipes[i][1]);			
				}
				else if (pid1 == 0)
				{
					if (i == 0 && (io_redirection == 1 || io_redirection == 3))
					{
						int inPutFD = open (inputFile, O_RDONLY);
						dup2(inPutFD, 0);
					}
					
					if (i - 1 >= 0)
						dup2(pipes[i - 1][0], 0);
					if (i < TotalPipes)
						dup2(pipes[i][1], 1);
					if (i == TotalPipes && io_redirection == 2)
					{
						int outPutFD = open (outputFile, O_WRONLY | O_CREAT);
						dup2(outPutFD, 1);
					}
					execv(ptr1, allCommands[i].argv);
						
				}
		    	}
	    		wait (NULL);
	    		return 0;
	    	}
	    }
	    if (!ampersand)
	    {
	    	wait(NULL);
	    }
    }
  }
  return 0;
}


