#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <ctime>
#include <sys/stat.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/times.h>
#include <math.h>
#include <fcntl.h> 

using namespace std;

void tokenString(string input, vector<string> &tokens, char breakChar);

void myPwd();

void myCD(vector<string> tokens);

void myTree(vector<string> tokens);

void printDir(string location, int depth, int &totalDir, int &totalFiles);

void myMTimes(vector<string> tokens);

void timesDir(string location, int timesCount[24], time_t curTime);

string path_search(string filePath);

int execute_command(string file_path,vector<string> tokens);

int chooseCommand(vector<string> &tokens);

void myTimeOut(vector<string> &tokens);

void myTime(vector<string> &tokens);

void IORedirect(vector<string> &tokens);

void piping(vector<string> &tokens);


int main()
{
		string userInput = "";
		vector<string> tokens;
		int temp, cmdStatus;
		
		
		
	while(1){
		cout << "$ ";
		getline(cin,userInput,'\n');		
		tokenString(userInput, tokens, ' ');
				
		cmdStatus = chooseCommand(tokens);
		
		if(cmdStatus == -1)
			break;
		if(cmdStatus == 1)
			continue;
	}
	
	
	return 0;	
}

void tokenString(string input, vector<string> &tokens, char breakChar){
	tokens.clear();
	string tempStr = input;
	int counter = 0;
	
	while(1){
		string tempTok = "";
		
		if(input[counter] == '\0'){
			counter++;
			break;
		}
		else if(input[counter] == breakChar){
			counter++;
			continue;
		}
		else {
			while(input[counter] != breakChar && input[counter] != '\0'){
				tempTok.push_back(input[counter]);
				counter++;
			}
			tokens.push_back(tempTok);
		}
	}
}
void myCD(vector<string> tokens){
	string target;
	
	if (tokens.size() > 2) {
		cout << "Error: more than one argument present\n";
		return;
	}
	if (tokens.size() == 1) {
		cout << "Error: no target specified\n";
		return;
	}
	
	int error = chdir(tokens[1].c_str()); 
	
	if(error == -1) 
		cout << "Error: no directory or target found \n";
	
	char *cwd = getcwd(NULL, 0);
	setenv("PWD", cwd, 1);
}


void myPwd(){
	if(getenv("PWD") == NULL)
		cout << "ERROR\n";
	else
		cout << getenv("PWD") << endl;
}

void myTree(vector<string> tokens){
	DIR *dirp;
	dirent *dp;
	int totalDirs=0, totalFiles=0;
	
	if(tokens.size() == 1)
		tokens.push_back(".");
	else if(tokens.size() > 2){
		cout << "Error: more than one argument present\n";
		return;
	}

	dirp = opendir(tokens[1].c_str());
	errno = 0;
	if(dirp == NULL && errno == ENOENT){
		cout << "Error: directory does not exist\n";
		closedir(dirp);
		return;
	}
	else if(dirp == NULL && errno == ENOTDIR){
		cout << "Error: target is not a directory\n";
		return;
		closedir(dirp);
	}
	closedir(dirp);
	cout << tokens[1] << endl;
	printDir(tokens[1],1,totalDirs,totalFiles);
	cout << endl << totalDirs << " directories, " << totalFiles << " files\n";
}

void printDir(string location, int depth, int &totalDir, int &totalFiles){
	DIR *dirp = opendir(location.c_str());
	dirent *dp;
	string nextLocation;
	int temp;
		
	while (dirp) {
		if ((dp = readdir(dirp)) != NULL) {
			if(dp->d_name[0] != '.'){
				for(int i=0; i < depth-1; i++)
					cout << "│   ";
				cout << "│--" << dp->d_name << endl;
				if(dp->d_type == DT_DIR){
					totalDir++;
					nextLocation = location;
					nextLocation.push_back('/');
					nextLocation.append(dp->d_name);
					printDir(nextLocation, depth+1,totalDir,totalFiles);
				}
				else
					totalFiles++;
			}
		} 
		else
			break;
	}
	closedir(dirp);
	return;
}

void myMTimes(vector<string> tokens){
	DIR *dirp;
	dirent *dp;
	string location;
	
	if(tokens.size() == 1)
		tokens.push_back(".");
	else if(tokens.size() > 2){
		cout << "Error: more than one argument present\n";
		return;
	}
	
	dirp = opendir(tokens[1].c_str());
	errno = 0;
	if(dirp == NULL && errno == ENOENT){
		cout << "Error: directory does not exist\n";
		closedir(dirp);
		return;
	}
	else if(dirp == NULL && errno == ENOTDIR){
		cout << "Error: target is not a directory\n";
		return;
		closedir(dirp);
	}
	closedir(dirp);
	
	int timesCount[24];
	
	for(int i=0;i<24;i++)
		timesCount[i] = 0;
	
	time_t curTime;
	time(&curTime);
	
	timesDir(tokens[1], timesCount, curTime);
	
	curTime = curTime - 86400 - 3600;
	tm *printTime;
	char *printChar;
	
	for(int i=0; i<24; i++){
		curTime = curTime + 3600;
		printTime = localtime(&curTime);
		printChar = asctime(printTime);
		printChar[24] = 0;
		cout << printChar << ": " << timesCount[23-i] << endl;
	}
}

void timesDir(string location, int timesCount[24], time_t curTime){
	DIR *dirp = opendir(location.c_str());
	dirent *dp;
	string nextLocation;
	int temp;
	timespec fileTime;
	struct stat fileStat;
	int modHours;
	
	while (dirp) {
		if ((dp = readdir(dirp)) != NULL) {
			if(dp->d_name[0] != '.'){
				nextLocation = location;
				nextLocation.push_back('/');
				nextLocation.append(dp->d_name);
				if(dp->d_type == DT_DIR){
					timesDir(nextLocation, timesCount, curTime);
				}
				else{
					if(stat(nextLocation.c_str(), &fileStat) != -1){
						fileTime = fileStat.st_mtimespec;
						modHours = difftime(curTime, fileTime.tv_sec);
						modHours = modHours / 3600;
						if(modHours < 24){
							timesCount[modHours]++;
						}
					}
				}
			}
		} 
		else
			break;
	}
	closedir(dirp);
	return;
}

string path_search(string file_path){
// finds the path for a given command that is not a built in

	string paths = getenv("PATH");
	vector<string> pathTokens;
	tokenString(paths, pathTokens, ':');
	
	for (int i = 0; i < pathTokens.size(); i++){
			string tempStr;
			tempStr = pathTokens[i];
			tempStr.push_back('/');
			tempStr.append(file_path);
			// used to see if that path exist
			if(access(tempStr.c_str(), F_OK) == 0)
					return tempStr;			
	}
	paths = "";
	return paths;		
}
int execute_command(string file_path,vector<string> tokens)
// basic execution of an external command
{
	char **argv;
	argv = new char*[tokens.size()+1];
	argv[tokens.size()] = NULL;
	
	for(int i=0;i<tokens.size();i++){
		argv[i] = (char *) malloc(tokens[i].size() + 1);
		strcpy(argv[i],tokens[i].c_str());
	}
	int status = 0;
	int child_pid = fork();
	if(child_pid == 0){
			// This is the child; it turns itself into /bin/ls via execv(2).
			// There is no return from successful execv(2)...
			status = execv(file_path.c_str(),argv); 
			cout << "Error: could not execute\n";
	}
	else{
			// This is the parent; it will wait for the child.
			
			waitpid(child_pid, NULL, 0);
	}
	for(int i=0;i<tokens.size()+1;i++)
		free(argv[i]);
	delete argv;
	
	return status;	
}

int chooseCommand(vector<string> &tokens){
	int IOfound = 0;
	int pipeFound = 0;
	
	for(int i=0;i<tokens.size();i++){
		if(tokens[i] == "<" || tokens[i] == ">")
			IOfound = 1;
		if(tokens[i] == "|")
			pipeFound = 1;
	}
	if(pipeFound == 1)
		piping(tokens);
	else if(IOfound == 1)
		IORedirect(tokens);
	else if(tokens[0] == "myexit" || cin.eof() == 1)
		return -1;
	else if(tokens[0] == "mypwd")
		myPwd();
	else if(tokens[0] == "mycd")
		myCD(tokens);
	else if(tokens[0] == "mytree")
		myTree(tokens);
	else if(tokens[0] == "mymtimes")
		myMTimes(tokens);
	else if(tokens[0] == "mytimeout")
		myTimeOut(tokens);
	else if(tokens[0] == "mytime")
		myTime(tokens);
	else{
		if(tokens[0].find('/') == -1)
			tokens[0] = path_search(tokens[0]);
		if(tokens[0] == ""){
			cout << "Error: Command could not be found\n";
			return 1;
		}
		execute_command(tokens[0], tokens);
	}
	return 0;
}
void myTimeOut(vector<string> &tokens){
	int seconds;
	
	if(tokens.size() < 3){
		cout << "Error: Invalid number of commands\n";
		return;
	}
	for(int i=0; i<tokens[1].length();i++){
		if(tokens[1][i] < 48 || tokens[1][i] > 57){
			cout << "Error: Argument 2 must be an integer\n";
			return;
		}
	}
	seconds = stoi(tokens[1]);
	
	tokens.erase(tokens.begin());
	tokens.erase(tokens.begin());
	
	pid_t status = 0;
	int child_pid = fork();
	if(child_pid == 0){
		chooseCommand(tokens);
		exit(0);
	}
	else{
		// This is the parent; it will wait for the child.
		for(int i=0; i<seconds;i++){
			sleep(1);
			status = waitpid(child_pid, NULL, WNOHANG);
			if(status != 0){
				return;
			}
		}
		kill(child_pid, SIGTERM);
		return;
		
	}
}

void myTime(vector<string> &tokens){
	tms startTime, endTime;
	clock_t realStart, realEnd;
	float sysTime, userTime, seconds, realTime;
	int minutes;
	double clockTicks = sysconf(_SC_CLK_TCK);
	
	if(tokens.size() < 2){
		cout << "Error: Invalid number of commands\n";
		return;
	}
	tokens.erase(tokens.begin());
	
	realStart = times(&startTime);
	
	pid_t status = 0;
	int child_pid = fork();
	if(child_pid == 0){
		chooseCommand(tokens);
		exit(0);
	}
	else{
		// This is the parent; it will wait for the child.
		waitpid(child_pid, NULL, 0);
		realEnd = times(&endTime);
		sysTime = (endTime.tms_cstime - endTime.tms_stime) - (startTime.tms_cstime - startTime.tms_stime);
		userTime = (endTime.tms_cutime - endTime.tms_utime) - (startTime.tms_cutime - startTime.tms_utime);
		realTime = realEnd - realStart;
		
		sysTime = sysTime/clockTicks;
		userTime = userTime/clockTicks;
		realTime = realTime/clockTicks;
		
		minutes = userTime/60;
		seconds = remainderf(userTime, 60);
		cout << "\nuser\t" << minutes << "m " << seconds << "s\n";
		
		minutes = sysTime/60;
		seconds = remainderf(sysTime, 60);
		cout << "sys \t" << minutes << "m " << seconds << "s\n";
		
		minutes = realTime/60;
		seconds = remainderf(realTime, 60);
		cout << "real\t" << minutes << "m " << seconds << "s\n";
	}
	return;
}
void IORedirect(vector<string> &tokens)
{
	// finds location of < or >
	int fdO = -2;
	int fdI = -2;	
	int foundInput = -1, foundOutput = -1;
	
	for(int i = 0; i < tokens.size(); i++){
			if(tokens[i] == "<")
				foundInput = i;
			else if(tokens[i] == ">")
				foundOutput = i;
	}
	
	// if a > is found try opening the next token
	if(foundOutput != -1)
		fdO = open(tokens[foundOutput+1].c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	
	
	// if a < is found try opening the next token
	if(foundInput != -1)
		fdI = open(tokens[foundInput+1].c_str(), O_RDONLY);	
	
	// file did not open
	if(fdO == -1 || fdI == -1){
			printf("ERROR: No file found\n");
			return;
	}
	
	if(foundInput != -1){
		for(vector<string>::iterator it = tokens.begin(); it < tokens.end(); it++){
			if(*it == "<"){
				tokens.erase(it);
				tokens.erase(it);
			}
		}
	}
	if(foundOutput != -1){
		for(vector<string>::iterator it = tokens.begin(); it < tokens.end(); it++){
			if(*it == ">"){
				tokens.erase(it);
				tokens.erase(it);
			}
		}
	}

	//fork and change output or input to be the new IO
	pid_t pid = fork();
	if(pid == 0){
			if(foundInput != -1){
					close(STDIN_FILENO);
					dup(fdI);
					close(fdI);
			}
			if(foundOutput != -1){
					close(STDOUT_FILENO);
					dup(fdO);
					close(fdO);
			}
			chooseCommand(tokens);
			
			exit(0);
		}
	else
		{
			if(foundOutput != -1)
				close(fdO);
			if(foundInput != -1)
				close(fdI);
			waitpid(pid, NULL, 0);
		}
}

void piping(vector<string> &tokens){
	int firstPipePos = -1;
	int secondPipePos = -1;
	char firstCommandIO = 0;
	char secondCommandIO = 0;
	char thirdCommandIO = 0;
	
	vector<string> firstCommand, secondCommand, thirdCommand;
	
	int pipePos = 0;
	
	while(tokens[pipePos] != "|"){
		cout << "tok: " << tokens[pipePos] << endl;
		firstCommand.push_back(tokens[pipePos]);
		pipePos++;
	}
	cout << "pipe: " << pipePos << endl;
	pipePos++;
	while(tokens[pipePos] != "|" && pipePos < tokens.size()){
		cout << "tok2: " << tokens[pipePos] << endl;
		secondCommand.push_back(tokens[pipePos]);
		pipePos++;
	}
	cout << "pipe: " << pipePos << endl;
	pipePos++;
	while(pipePos < tokens.size()){
		cout << "tok3: " << tokens[pipePos] << endl;
		thirdCommand.push_back(tokens[pipePos]);
		pipePos++;
	}
	
	for(int i=0; i < firstCommand.size();i++)
		cout << firstCommand[i] << endl;
	cout << "-------\n";
	for(int i=0; i < secondCommand.size();i++)
		cout << secondCommand[i] << endl;
	cout << "-------\n";
	for(int i=0; i < thirdCommand.size();i++)
		cout << thirdCommand[i] << endl;
	cout << "-------\n";
	
		
	int error;
	int p_fds[2];
	int p_fds2[2];
	// p_fds[0] = output
	// p_fds[1] = input
	pipe(p_fds);
	if (thirdCommand.size() > 0) {
		pipe(p_fds2);
	}
	
	int pid1 = fork();
	if(pid1 == 0) {

		if (thirdCommand.size() > 0) {
			close(p_fds2[0]);
			close(p_fds2[1]);
		}
		
		close(p_fds[0]);
		close(STDOUT_FILENO);
		dup(p_fds[1]);
		close(p_fds[1]);
		
		chooseCommand(firstCommand);
		
		exit(0);
	}
	
	int pid2 = fork();
	if(pid2 == 0) {
		if (thirdCommand.size() > 0) {
			close(p_fds2[0]);
			close(STDOUT_FILENO);
			dup(p_fds2[1]);
			close(p_fds2[1]);
		}
		
		
		close(p_fds[1]);
		close(STDIN_FILENO);
		dup(p_fds[0]);
		close(p_fds[0]);
		
		chooseCommand(secondCommand);
		
		exit(0);
	}
	
	int pid3;
	if (thirdCommand.size() > 0) {
		pid3 = fork();
		if(pid3 == 0) {
			close(p_fds[0]);
			close(p_fds[1]);
			
			close(p_fds2[1]);
			close(STDIN_FILENO);
			dup(p_fds2[0]);
			close(p_fds2[0]);
			
			chooseCommand(thirdCommand);
			
			exit(0);
		}
	}
	
	close(p_fds[0]);
	close(p_fds[1]);
	if(thirdCommand.size() > 0) {
		close(p_fds2[0]);
		close(p_fds2[1]);
	}
	
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
	if (thirdCommand.size() > 0) {
		waitpid(pid3, NULL, 0);
	}	
	return;
}
	
