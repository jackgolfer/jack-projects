#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define KGRN "\x1B[32m"
#define KCYN "\x1B[34m"
#define KWHT "\x1B[37m"
#define RESET "\x1B[0m"

typedef struct {
	int size;
	char **items;
} tokenlist;

typedef struct {
	unsigned int bytesPerSector;
	unsigned int sectorsPerCluster;
	unsigned int reservedSectorCount;
	unsigned int numberOfFATs;
	unsigned int totalSectors;
	unsigned int FATsize;
	unsigned int rootCluster;
} FATimageInfo;

// ----------------------------------------------------------v

// linked list structure 
struct LinkedList{
	unsigned char filename[12];
	unsigned int mode; // 1 for r, 2 for w, 3 for rw, and 4 for wr
	unsigned int first_cluster;
	unsigned int offset;
	struct LinkedList *next;
};

// initializing and defining a linked list called node
typedef struct LinkedList *node;
node open_files;

// ----------------------------------------------------------^


FATimageInfo FATinfo;

char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

//--------------------------------------------------v
node createNode();
node addNode(node head, unsigned char *name, unsigned int mode, unsigned int cluster);
node deleteNode(node head, unsigned int first_cluster);
//--------------------------------------------------^

void setFATinfo();
void printInfo();
unsigned int hexToDec(unsigned char hexVal[]);
void decToHexa(int n, unsigned char* hexaDeciNum);
void findSize(char *fileName);
void ls_command(char *dirName); 
void cd_command(char *dirName);
void creat_command(char *fileName);
void mkdir_command(char *dirName);
void mv_command(char *from, char *to);
void rm_command(char *fileName);
void cp_command(char *fileName, char *to);
void open_command(char *fileName, char *mode);
void close_command(char *fileName);
void lseek_command(char *fileName, char *offset);
void read_command(char *fileName, char *size);
void write_command(char *fileName, char *size, char *inputString);
void rmdir_command(char *dirName);
unsigned int find_new_FAT(unsigned int lastFAT);
int new_file(unsigned char* file_entry);

unsigned int FAT_region_start, data_region_start, current_dir;
FILE *FATimage;

int main(int argc, char *argv[]) {

	char command[100];
	FATimage = fopen(argv[1], "r+");
	int i;

	if(FATimage == NULL) {
		printf("File image could not be opened\n");
		return 1;
	}

	setFATinfo(FATimage);

	//--------------------------------------------------------v
	// initialing the open_files linked list
	node open_files = NULL; 
	// -------------------------------------------------------^


	while (strcmp(command, "exit") != 0) {
		printf("$ ");

		char *input = get_input();

		tokenlist *tokens = get_tokens(input);
		strcpy(command, tokens->items[0]);



		if (strcmp(command, "info") == 0 && tokens->size == 1)
			printInfo();
		else if (strcmp(command, "size") == 0 && tokens->size == 2)
			findSize(tokens->items[1]);
		else if (strcmp(command, "ls") == 0 && tokens->size == 2)
			ls_command(tokens->items[1]);
		else if (strcmp(command, "ls") == 0 && tokens->size == 1)
			ls_command(".");
		else if (strcmp(command, "cd") == 0 && tokens->size == 2)
			cd_command(tokens->items[1]);
		else if (strcmp(command, "creat") == 0 && tokens->size == 2)
			creat_command(tokens->items[1]);
		else if (strcmp(command, "mkdir") == 0 && tokens->size == 2)
			mkdir_command(tokens->items[1]);
		else if (strcmp(command, "mv") == 0 && tokens->size == 3)
			mv_command(tokens->items[1],tokens->items[2]);
		else if (strcmp(command, "rm") == 0 && tokens->size == 2)
			rm_command(tokens->items[1]);
		else if (strcmp(command, "cp") == 0 && tokens->size == 3)
			cp_command(tokens->items[1],tokens->items[2]);
		else if (strcmp(command, "write") == 0 && tokens->size == 4)
			write_command(tokens->items[1],tokens->items[2],tokens->items[3]);
		else if (strcmp(command, "exit") == 0) {}
//----------------------------------------------------------v

		// open_command token call
		else if (strcmp(command, "open") == 0) {
			if (tokens->size != 3) {
				printf("incorrect number of arguments for open command \n");
			}
			else 
				open_command(tokens->items[1], tokens->items[2]);
		}

		// close_command token call
		else if (strcmp(command, "close") == 0)
			close_command(tokens->items[1]);
		// lseek_command token call
		else if (strcmp(command, "lseek") == 0) {
			if (tokens->size != 3)
				printf("incorrect number of arguments for lseek command\n");
			else 
				lseek_command(tokens->items[1], tokens->items[2]);
		}
		else if (strcmp(command, "read") == 0) {
			if (tokens->size != 3)
				printf("incorrect number of arguments for read command\n");
			else
				read_command(tokens->items[1], tokens->items[2]);
		}
		else if (strcmp(command, "rmdir") == 0 && tokens->size == 2) {
			rmdir_command(tokens->items[1]);
		}

		//----------------------------------------------------------^
		else
			printf("Invalid command\n");

		free(input);
		free_tokens(tokens);
	}
	fclose(FATimage);

	return 0;
}

tokenlist *new_tokenlist(void)
{
	tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **) malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
	int i = tokens->size;

	tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *) malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

char *get_input(void)
{
	char *buffer = NULL;
	int bufsize = 0;

	char line[5];
	while (fgets(line, 5, stdin) != NULL) {
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL) 
			addby = newln - line;
		else
			addby = 5 - 1;

		buffer = (char *) realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;

		if (newln != NULL)
			break;
	}

	buffer = (char *) realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;

	return buffer;
}

tokenlist *get_tokens(char *input)
{
	char *buf = (char *) malloc(strlen(input) + 1);
	strcpy(buf, input);
	int count = 0;
	char *last;

	tokenlist *tokens = new_tokenlist();

	char *tok = strtok(buf, " ");
	while (tok != NULL && count < 2) {
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
		count++;
	}
	if(tok != NULL) {
		add_token(tokens, tok);
		count++;
	}

	if(count == 3) {
		tok = strtok(NULL, "\n");
		if(tok != NULL) {
			last = (char *) malloc(strlen(tok) + 1);
			strcpy(last,tok);
			add_token(tokens, last);
		}
	}

	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens)
{
	int i;
	for (i = 0; i < tokens->size; i++)
		free(tokens->items[i]);

	free(tokens);
}

void setFATinfo() {
	unsigned char BPS[3],SPC[2],TotSec[5];
	long yeet;
	fseek(FATimage,11,SEEK_SET);
	BPS[1] = fgetc(FATimage);
	BPS[0] = fgetc(FATimage);
	sprintf(BPS,"%02x%02x",BPS[0], BPS[1]);
	FATinfo.bytesPerSector = hexToDec(BPS);

	SPC[0] = fgetc(FATimage);
	sprintf(SPC,"%02x",SPC[0]);
	FATinfo.sectorsPerCluster = hexToDec(SPC);

	BPS[1] = fgetc(FATimage);
	BPS[0] = fgetc(FATimage);
	sprintf(BPS,"%02x%02x",BPS[0], BPS[1]);
	FATinfo.reservedSectorCount = hexToDec(BPS);

	SPC[0] = fgetc(FATimage);
	sprintf(SPC,"%02x",SPC[0]);
	FATinfo.numberOfFATs = hexToDec(SPC);

	fseek(FATimage,32,SEEK_SET);
	TotSec[3] = fgetc(FATimage);
	TotSec[2] = fgetc(FATimage);
	TotSec[1] = fgetc(FATimage);
	TotSec[0] = fgetc(FATimage);
	sprintf(TotSec,"%02x%02x%02x%02x",TotSec[0], TotSec[1], TotSec[2], TotSec[3]);
	FATinfo.totalSectors = hexToDec(TotSec);

	TotSec[3] = fgetc(FATimage);
	TotSec[2] = fgetc(FATimage);
	TotSec[1] = fgetc(FATimage);
	TotSec[0] = fgetc(FATimage);
	sprintf(TotSec,"%02x%02x%02x%02x",TotSec[0], TotSec[1], TotSec[2], TotSec[3]);
	FATinfo.FATsize = hexToDec(TotSec);

	fseek(FATimage,44,SEEK_SET);
	TotSec[3] = fgetc(FATimage);
	TotSec[2] = fgetc(FATimage);
	TotSec[1] = fgetc(FATimage);
	TotSec[0] = fgetc(FATimage);
	sprintf(TotSec,"%02x%02x%02x%02x",TotSec[0], TotSec[1], TotSec[2], TotSec[3]);
	FATinfo.rootCluster = hexToDec(TotSec);

	FAT_region_start = FATinfo.reservedSectorCount * FATinfo.bytesPerSector;
	data_region_start = FAT_region_start + (FATinfo.numberOfFATs * FATinfo.FATsize * FATinfo.bytesPerSector);
	current_dir = FATinfo.rootCluster;

}
void printInfo() {
	printf("Bytes per sector: %i\n",FATinfo.bytesPerSector);
	printf("Sectors per cluster: %i\n",FATinfo.sectorsPerCluster);
	printf("Reserved sector count: %i\n",FATinfo.reservedSectorCount);
	printf("Number of FATs: %i\n",FATinfo.numberOfFATs);
	printf("Total sectors: %i\n",FATinfo.totalSectors);
	printf("FAT size: %i\n",FATinfo.FATsize);
	printf("Root Cluster: %i\n",FATinfo.rootCluster);
}
unsigned int hexToDec(unsigned char hexVal[]) 
{    
    int len = strlen(hexVal), i; 
      
    // Initializing base value to 1, i.e 16^0 
    int base = 1; 
      
    int dec_val = 0; 
      
    // Extracting characters as digits from last character 
    for (i=len-1; i>=0; i--) 
    {    
        // if character lies in '0'-'9', converting  
        // it to integral 0-9 by subtracting 48 from 
        // ASCII value. 
        if (hexVal[i]>='0' && hexVal[i]<='9') 
        { 
            dec_val += (hexVal[i] - 48)*base; 
                  
            // incrementing base by power 
            base = base * 16; 
        } 
  
        // if character lies in 'A'-'F' , converting  
        // it to integral 10 - 15 by subtracting 55  
        // from ASCII value 
        else if (hexVal[i]>='a' && hexVal[i]<='f') 
        { 
            dec_val += (hexVal[i] - 87)*base; 
          
            // incrementing base by power 
            base = base*16; 
        } 
    } 
    return dec_val; 
	
}
void decToHexa(int n, unsigned char* hexaDeciNum) 
{    
      
    // counter for hexadecimal number array 
    int i = 0; 
    while(n!=0) 
    {    
        // temporary variable to store remainder 
        int temp  = 0; 
          
        // storing remainder in temp variable. 
        temp = n % 16; 
          
        // check if temp < 10 
        if(temp < 10) 
        { 
            hexaDeciNum[i] = temp + 48; 
            i++; 
        } 
        else
        { 
            hexaDeciNum[i] = temp + 55; 
            i++; 
        } 
          
        n = n/16; 
    } 
} 

void findSize(char *fileName) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12];
	unsigned int current_FAT, current_address = 0, attribute, size;
	int i,x;

	for(i = 0; i < 8; i++) {
		if(i < strlen(fileName))
			newFileName[i] = toupper(fileName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';
	current_FAT = current_dir;
	
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if(attribute != 15 && strcmp(newFileName,tempFileName) == 0) {

				fseek(FATimage, 16,SEEK_CUR); 
				tempStr[3] = fgetc(FATimage);
				tempStr[2] = fgetc(FATimage);
				tempStr[1] = fgetc(FATimage);
				tempStr[0] = fgetc(FATimage);
				tempStr[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
				size = hexToDec(tempStr);
				printf("\'%s\' size is: %i bytes\n",fileName,size);
				return;
			}
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);

		


	}
	printf("\'%s\' could not be found\n",fileName);
}
void ls_command(char *dirName) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12];
	unsigned int current_FAT, current_address = 0, attribute;
	int i,x, dir_location = -1;

	for(i = 0; i < 8; i++) {
		if(i < strlen(dirName))
			newFileName[i] = toupper(dirName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';

	current_FAT = current_dir;
	
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		if(strcmp(newFileName,".          ") == 0) {
			dir_location = current_dir;
			break;
		}
		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if(attribute != 15 && strcmp(newFileName,tempFileName) == 0) {
				if((attribute & 16) != 16) {
					printf("\'%s\' is not a directory\n",dirName);
					return;
				}



				fseek(FATimage, 8,SEEK_CUR); 
				tempStr[1] = fgetc(FATimage);
				tempStr[0] = fgetc(FATimage);
				fseek(FATimage, 4,SEEK_CUR);
				tempStr[3] = fgetc(FATimage);
				tempStr[2] = fgetc(FATimage);
				tempStr[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
				dir_location = hexToDec(tempStr);
				break;
			}
		}
		if(dir_location != 0)
			break;
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);

		


	}
	if(dir_location == -1) {
		printf("\'%s\' could not be found\n",dirName);
		return;
	}
	else if(dir_location == 0)
		dir_location = 2;
	current_FAT = dir_location;
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if(strcmp(tempFileName,".          ") == 0 || strcmp(tempFileName,"..         ") == 0)          
				continue;
			if(((attribute & 2) == 0) && ((attribute & 4) == 0) && attribute != 0 && tempFileName[0] != 0 && tempFileName[0] != 229) {
				if((attribute&16) == 16)				
					printf("%s\n",tempFileName);
				else
					printf("%s\n",tempFileName);

			}
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);

	}
}
void cd_command(char *dirName) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12];
	unsigned int current_FAT, current_address = 0, attribute;
	int i,x, dir_location = -1;

	for(i = 0; i < 8; i++) {
		if(i < strlen(dirName))
			newFileName[i] = toupper(dirName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';

	current_FAT = current_dir;
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
		if(strcmp(newFileName,".          ") == 0) {
			return;
		}
		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if(attribute != 15 && strcmp(newFileName,tempFileName) == 0) {
				if((attribute & 16) != 16) {
					printf("\'%s\' is not a directory\n",dirName);
					return;
				}



				fseek(FATimage, 8,SEEK_CUR); 
				tempStr[1] = fgetc(FATimage);
				tempStr[0] = fgetc(FATimage);
				fseek(FATimage, 4,SEEK_CUR);
				tempStr[3] = fgetc(FATimage);
				tempStr[2] = fgetc(FATimage);
				tempStr[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
				dir_location = hexToDec(tempStr);
				break;
			}
		}
		if(dir_location != -1)
			break;
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);

		


	}
	if(dir_location == -1) {
		printf("\'%s\' could not be found\n",dirName);
		return;
	}
	else if(dir_location == 0)
		dir_location = FATinfo.rootCluster;
	current_dir = dir_location;

}
void creat_command(char *fileName) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12], dir_entry[32];
	unsigned int current_FAT, current_address = 0, attribute, new_location = 0, previous_FAT = 0;
	int i,x, temp=0;

	for(i = 0; i < 8; i++) {
		if(i < strlen(fileName))
			newFileName[i] = toupper(fileName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';

	if(strcmp(newFileName,".          ") == 0 || strcmp(newFileName,"..         ") == 0) {
		printf("Invalid file name\n");
		return;
	}

	current_FAT = current_dir;
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			temp = 0;
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);

			fseek(FATimage, current_address + (i*32),SEEK_SET);
				temp = fgetc(FATimage);
			if(strcmp(newFileName,tempFileName) == 0) {
				printf("\'%s\' already exists\n",fileName);
				return; 
			}	
		
			if((temp == 0 || temp == 229) && new_location == 0) {
				new_location = current_address + (i*32);
			}
				
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		previous_FAT = current_FAT;
		current_FAT = hexToDec(tempStr);
	}
	if(new_location == 0) {
		new_location = find_new_FAT(previous_FAT);
		new_location = data_region_start + ((new_location-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
	}

	for(i = 0; i < 32; i++)
		dir_entry[i] = 0;
	if(new_file(dir_entry) == -1) {
		printf("FAT table is full\n");
		return;
	}
	for(i = 0; i < 11; i++)
		dir_entry[i] = newFileName[i];
	dir_entry[11] = 32;

	fseek(FATimage, new_location, SEEK_SET); 
	fwrite(dir_entry, sizeof(char), 32, FATimage);
}
void mkdir_command(char *dirName) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12], dir_entry[32], tempStr2[3], tempStr3[9];
	unsigned int current_FAT, current_address = 0, attribute, new_location = 0, previous_FAT = 0, start_FAT = current_dir;
	int i,x, temp=0;

	for(i = 0; i < 8; i++) {
		if(i < strlen(dirName))
			newFileName[i] = toupper(dirName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';

	if(strcmp(newFileName,".          ") == 0 || strcmp(newFileName,"..         ") == 0) {
		printf("Invalid directory name\n");
		return;
	}

	current_FAT = current_dir;
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			temp = 0;
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);

			fseek(FATimage, current_address + (i*32),SEEK_SET);
				temp = fgetc(FATimage);

			if(strcmp(newFileName,tempFileName) == 0) {
				printf("\'%s\' already exists\n",dirName);
				return; 
			}
			if((temp == 0 || temp == 229) && new_location == 0) {
				new_location = current_address + (i*32);
			}				
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		previous_FAT = current_FAT;
		current_FAT = hexToDec(tempStr);
	}

	if(new_location == 0) {
		new_location = find_new_FAT(previous_FAT);
		new_location = data_region_start + ((new_location-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
	}
	for(i = 0; i < 32; i++)
		dir_entry[i] = 0;
	temp = new_file(dir_entry);
	if(temp == -1) {
		printf("FAT table is full\n");
		return;
	}
	for(i = 0; i < 11; i++)
		dir_entry[i] = newFileName[i];
	dir_entry[11] = 16;

	fseek(FATimage, new_location, SEEK_SET); 
	fwrite(dir_entry, sizeof(char), 32, FATimage);
	strcpy(dir_entry,".          ");
	dir_entry[11] = 18;
	for(i=0;i<8;i++)	
		tempStr3[i] = 48;
	tempStr3[8] = 0;

	decToHexa(temp,tempStr3);

	for(i=0; i<2;i++) {
		tempStr2[0] = tolower(tempStr3[(i*2)+1]);
		tempStr2[1] = tolower(tempStr3[i*2]);
		tempStr2[2] = '\0';
		dir_entry[i+26] = hexToDec(tempStr2);
	}
	for(i=2; i<4;i++) {
		tempStr2[0] = tolower(tempStr3[(i*2)+1]);
		tempStr2[1] = tolower(tempStr3[i*2]);
		tempStr2[2] = '\0';
		dir_entry[i+20] = hexToDec(tempStr2);
	}




	fseek(FATimage, data_region_start + ((temp-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster),SEEK_SET);
	fwrite(dir_entry, sizeof(char), 32, FATimage);
	strcpy(dir_entry,"..         ");
	dir_entry[11] = 18;
	for(i=0;i<8;i++)	
		tempStr3[i] = 48;
	tempStr3[8] = 0;

	if(start_FAT == FATinfo.rootCluster)
		start_FAT = 0;
	decToHexa(start_FAT,tempStr3);

	for(i=0; i<2;i++) {
		tempStr2[0] = tolower(tempStr3[(i*2)+1]);
		tempStr2[1] = tolower(tempStr3[i*2]);
		tempStr2[2] = '\0';
		dir_entry[i+26] = hexToDec(tempStr2);
	}
	for(i=2; i<4;i++) {
		tempStr2[0] = tolower(tempStr3[(i*2)+1]);
		tempStr2[1] = tolower(tempStr3[i*2]);
		tempStr2[2] = '\0';
		dir_entry[i+20] = hexToDec(tempStr2);
	}
	fseek(FATimage, data_region_start + ((temp-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster + 32),SEEK_SET);
	fwrite(dir_entry, sizeof(char), 32, FATimage);
}
void mv_command(char *from, char *to) {
	unsigned char newFromFileName[12], newToFileName[12], tempStr[5], tempFileName[12], dir_entry[32], tempStr2[3], tempc[1], tempDest[5];
	unsigned int current_FAT, current_address = 0, attribute, new_location = 0, previous_FAT = 0, start_FAT = current_dir;
	int i,x, temp=0, fromDirOffset = -1,fromFileOffset = -1, toLocation = -1, lastEntry = 0, entryOffset;

	for(i = 0; i < 8; i++) {
		if(i < strlen(from))
			newFromFileName[i] = toupper(from[i]);
		else	
			newFromFileName[i] = ' ';
	}
	newFromFileName[8] = ' ';
	newFromFileName[9] = ' ';
	newFromFileName[10] = ' ';
	newFromFileName[11] = '\0';

	for(i = 0; i < 8; i++) {
		if(i < strlen(to))
			newToFileName[i] = toupper(to[i]);
		else	
			newToFileName[i] = ' ';
	}
	newToFileName[8] = ' ';
	newToFileName[9] = ' ';
	newToFileName[10] = ' ';
	newToFileName[11] = '\0';

	if(strcmp(newFromFileName,newToFileName) == 0) {
		printf("Cannot rename file to same name\n");
		return;
	}
	else if(strcmp(newFromFileName,".          ") == 0 || strcmp(newFromFileName,"..         ") == 0) {
		printf("Cannot move or rename this directory\n");
		return;
	}
	else if(strcmp(newToFileName,".          ") == 0)
		return;
	current_FAT = current_dir;
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if(attribute != 0)	
				lastEntry = 0;

			if(strcmp(newFromFileName,tempFileName) == 0 && (attribute&16) == 16) {
				fromDirOffset = current_address + (i*32);
				lastEntry = 1;
			}
			else if(strcmp(newFromFileName,tempFileName) == 0 && (attribute&16) != 16) {
				fromFileOffset = current_address + (i*32);
				lastEntry = 1;
			}
			else if(strcmp(newToFileName,tempFileName) == 0 && (attribute&16) == 16) {
				fseek(FATimage, 8,SEEK_CUR); 
				tempDest[1] = fgetc(FATimage);
				tempDest[0] = fgetc(FATimage);
				fseek(FATimage, 4,SEEK_CUR);
				tempDest[3] = fgetc(FATimage);
				tempDest[2] = fgetc(FATimage);
				tempDest[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempDest[0], tempDest[1], tempDest[2], tempDest[3]);
				toLocation = hexToDec(tempStr);
				if(toLocation == 0)
					toLocation = FATinfo.rootCluster;
			}			
			else if(strcmp(newToFileName,tempFileName) == 0 && (attribute&16) != 16) {
				printf("\'%s\' is not a directory!\n",to);
				return;
			}
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
	}
	if(fromDirOffset != -1)
		entryOffset = fromDirOffset;
	else if(fromFileOffset != -1)
		entryOffset = fromFileOffset;

	if(fromDirOffset == -1 && fromFileOffset == -1) {
		printf("%s does not exist\n",from);
		return;
	}
	else if(toLocation == -1) { //rename
		if(strcmp(newToFileName,".          ") == 0 || strcmp(newToFileName,"..         ") == 0) {
			printf("\'%s\' cannot be renamed to this\n", from);
			return;
		}
		fseek(FATimage, entryOffset, SEEK_SET);
		fwrite(newToFileName, sizeof(char), 11, FATimage);
	}
	else {
		fseek(FATimage, entryOffset, SEEK_SET);
		for(i = 0; i<32; i++)
			dir_entry[i] = fgetc(FATimage);
		if(lastEntry == 0)	
			tempc[0] = 229;
		if(lastEntry == 1)
			tempc[0] = 0;

		current_FAT = toLocation;
		while(current_FAT < 268435448) {
			current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

			for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
				temp = 0;
				fseek(FATimage, current_address + (i*32),SEEK_SET);
				for(x=0; x<11;x++)
					tempFileName[x] = fgetc(FATimage);
				tempFileName[11] = '\0';
				attribute = fgetc(FATimage);

				fseek(FATimage, current_address + (i*32),SEEK_SET);
				temp = fgetc(FATimage);
				if(strcmp(newFromFileName,tempFileName) == 0) {
					printf("\'%s\' already exists inside \'%s\'\n",from,to);
					return; 
				}	
		
				if((temp == 0 || temp == 229) && new_location == 0) {
					new_location = current_address + (i*32);

				}
				
			}
			fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
			tempStr[3] = fgetc(FATimage);
			tempStr[2] = fgetc(FATimage);
			tempStr[1] = fgetc(FATimage);
			tempStr[0] = fgetc(FATimage);
			tempStr[4] = '\0';
			sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
			previous_FAT = current_FAT;
			current_FAT = hexToDec(tempStr);
		}
		if(new_location == 0) {
			new_location = find_new_FAT(previous_FAT);
			new_location = data_region_start + ((new_location-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
		}

		fseek(FATimage, new_location, SEEK_SET); 
		fwrite(dir_entry, sizeof(char), 32, FATimage);
		fseek(FATimage, entryOffset, SEEK_SET);
		fwrite(tempc, sizeof(char), 1, FATimage);
	}
	if(fromDirOffset != -1) {
		tempStr[1] = dir_entry[20];
		tempStr[0] = dir_entry[21];
		tempStr[3] = dir_entry[26];
		tempStr[2] = dir_entry[27];
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		new_location = hexToDec(tempStr);


		fseek(FATimage, data_region_start + ((new_location-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster), SEEK_SET); 
		fseek(FATimage, 52, SEEK_CUR);
		tempc[0] = tempDest[1];
		fwrite(tempc, sizeof(char), 1, FATimage);
		tempc[0] = tempDest[0];
		fwrite(tempc, sizeof(char), 1, FATimage);
		fseek(FATimage, 4, SEEK_CUR);
		tempc[0] = tempDest[3];
		fwrite(tempc, sizeof(char), 1, FATimage);
		tempc[0] = tempDest[2];
		fwrite(tempc, sizeof(char), 1, FATimage);
	}
}
void cp_command(char *fileName, char *to) {
	unsigned char newFileName[12], newToFileName[12], tempStr[5], tempFileName[12], dir_entry[32], tempStr2[3], tempc[1], tempDest[5], cpArr[512];
	unsigned int current_FAT, current_address = 0, attribute, new_location = 0, previous_FAT = 0, start_FAT = current_dir, cpFAT;
	int i,x, temp=0, fromDirOffset = -1, fileOffset = -1, toLocation = -1, entryOffset;

	for(i = 0; i < 8; i++) {
		if(i < strlen(fileName))
			newFileName[i] = toupper(fileName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';

	for(i = 0; i < 8; i++) {
		if(i < strlen(to))
			newToFileName[i] = toupper(to[i]);
		else	
			newToFileName[i] = ' ';
	}
	newToFileName[8] = ' ';
	newToFileName[9] = ' ';
	newToFileName[10] = ' ';
	newToFileName[11] = '\0';

	if(strcmp(newFileName,newToFileName) == 0) {
		printf("Cannot create file of same name\n");
		return;
	}
	else if(strcmp(newToFileName,".          ") == 0) {
		printf("Cannot be name a file %s\n",newToFileName);
		return;
	}
	current_FAT = current_dir;
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);

			if(strcmp(newFileName,tempFileName) == 0 && (attribute&16) == 16) {
				printf("Cannot copy directory\n");
				return;
			}
			else if(strcmp(newFileName,tempFileName) == 0 && (attribute&16) != 16) {
				fileOffset = current_address + (i*32);
			}
			else if(strcmp(newToFileName,tempFileName) == 0 && (attribute&16) == 16) {
				fseek(FATimage, 8,SEEK_CUR); 
				tempDest[1] = fgetc(FATimage);
				tempDest[0] = fgetc(FATimage);
				fseek(FATimage, 4,SEEK_CUR);
				tempDest[3] = fgetc(FATimage);
				tempDest[2] = fgetc(FATimage);
				tempDest[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempDest[0], tempDest[1], tempDest[2], tempDest[3]);
				toLocation = hexToDec(tempStr);
				if(toLocation == 0)
					toLocation = FATinfo.rootCluster;
			}			
			else if(strcmp(newToFileName,tempFileName) == 0 && (attribute&16) != 16) {
				printf("\'%s\' is not a directory!\n",to);
				return;
			}
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
	}
	if(fileOffset == -1) {
		printf("\'%s\' could not be found\n",newFileName);
		return;	
	}
	fseek(FATimage, fileOffset, SEEK_SET);
	for(i=0;i<32;i++)
		dir_entry[i] = fgetc(FATimage);
	if(toLocation == -1) {
		for(i = 0; i < 11; i++)
			dir_entry[i] = newToFileName[i];	
	}
	tempStr[0] = dir_entry[21];
	tempStr[1] = dir_entry[20];
	tempStr[2] = dir_entry[27];
	tempStr[3] = dir_entry[26];
	sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
	previous_FAT = hexToDec(tempStr);
	current_FAT = previous_FAT;

	while(current_FAT < 268435448) {
		if(current_FAT == previous_FAT)
			cpFAT = new_file(dir_entry);

		for(i=0;i<(FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/512;i++) {
			fseek(FATimage,data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster+(i*512)),SEEK_SET);
			for(x=0;x<512;x++)
				cpArr[x] = fgetc(FATimage);
			fseek(FATimage,data_region_start + ((cpFAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster+(i*512)),SEEK_SET);
			fwrite(cpArr, sizeof(char), 512, FATimage);
		}
		cpFAT = find_new_FAT(cpFAT);

		
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
	}
	if(toLocation == -1)
		current_FAT = current_dir;
	else
		current_FAT = toLocation;

	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			temp = 0;
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);

			fseek(FATimage, current_address + (i*32),SEEK_SET);
				temp = fgetc(FATimage);
			if(strcmp(newToFileName,tempFileName) == 0) {
				printf("\'%s\' already exists\n",fileName);
				return; 
			}	
		
			if((temp == 0 || temp == 229) && new_location  == 0) {
				new_location = current_address + (i*32);
			}
				
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		previous_FAT = current_FAT;
		current_FAT = hexToDec(tempStr);
	}
	if(new_location == 0) {
		new_location = find_new_FAT(previous_FAT);
		new_location = data_region_start + ((new_location-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
	}
	fseek(FATimage, new_location, SEEK_SET); 
	fwrite(dir_entry, sizeof(char), 32, FATimage);
}
void rm_command(char *fileName) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12];
	unsigned int current_FAT, current_address = 0, attribute;
	int i,x, dir_location = -1, lastEntry = 0, old_entry = 0;

	for(i = 0; i < 8; i++) {
		if(i < strlen(fileName))
			newFileName[i] = toupper(fileName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';

	current_FAT = current_dir;
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			lastEntry = 0;
			if(attribute != 15 && strcmp(newFileName,tempFileName) == 0) {
				if((attribute & 16) == 16) {
					printf("\'%s\' is not a file\n",fileName);
					return;
				}
				fseek(FATimage, 8,SEEK_CUR); 
				tempStr[1] = fgetc(FATimage);
				tempStr[0] = fgetc(FATimage);
				fseek(FATimage, 4,SEEK_CUR);
				tempStr[3] = fgetc(FATimage);
				tempStr[2] = fgetc(FATimage);
				tempStr[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
				dir_location = hexToDec(tempStr);
				fseek(FATimage, current_address + (i*32),SEEK_SET);
				lastEntry = 1;
				old_entry = current_address + (i*32);
				break;
			}
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
	}
	if(dir_location == -1) {
		printf("\'%s\' could not be found\n",fileName);
		return;
	}
	
	current_FAT = dir_location;
	while(current_FAT < 268435448) {
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
		fseek(FATimage,-4,SEEK_CUR);
		for(i = 0; i<4; i++)
			tempStr[i] = 0;
		fwrite(tempStr, sizeof(char), 4, FATimage);
		
		if(lastEntry == 1)
			tempStr[0] = 0;
		else
			tempStr[0] = 229;
		fseek(FATimage, old_entry, SEEK_SET);
		fwrite(tempStr, sizeof(char), 1, FATimage);
	}	
}
//----------------------------------------------------------------------------------------v
void open_command(char *fileName, char *mode) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12], tempC[1];
	unsigned int current_FAT, current_address = 0, attribute, size;
	int i,x;
	unsigned int modeInt, first_cluster, entryOffset = 0, temp;
	unsigned char tempMode[2];

	// check if mode is valid, and set modeInt to cooresponding integer if it is
	if (strcmp(mode,"r") == 0)
		modeInt = 1;
	else if (strcmp(mode, "w") == 0)
		modeInt = 2;
	else if (strcmp(mode, "rw") == 0)
		modeInt = 3;
	else if (strcmp(mode, "wr") == 0)
		modeInt = 4;
	else {
		printf("invalid mode\n");
		return;
	}


	for(i = 0; i < 8; i++) {
		if(i < strlen(fileName))
			newFileName[i] = toupper(fileName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';
	current_FAT = current_dir;

	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if((attribute != 15) && (strcmp(newFileName,tempFileName) == 0)) {

				entryOffset = current_address + (i*32);

				// filename found!

				// check if it is a directory
				if ((attribute&16) == 16) {
					printf("argument is a directory name, not a file name\n");
					return;
				}

				// check for read only
				if ((strcmp(mode,"r")!=0) && ((attribute&1) == 1)) {
					printf("file is read only\n");
					return;
				}
				if (strcmp(mode,"r") == 0) {
					fseek(FATimage, current_address + (i*32) + 11,SEEK_SET);
					tempC[0] = attribute|1;
					fwrite(tempC, sizeof(char), 1, FATimage);
				}

				// determine first cluster
				fseek(FATimage, 8,SEEK_CUR); 
             			tempStr[1] = fgetc(FATimage);
             	   		tempStr[0] = fgetc(FATimage);
              	  		fseek(FATimage, 4,SEEK_CUR);
                		tempStr[3] = fgetc(FATimage);
                		tempStr[2] = fgetc(FATimage);
                		tempStr[4] = '\0';
                		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
                		first_cluster = hexToDec(tempStr);

				// check if file is already opened
				node p;
				p = open_files;
				while(p != NULL){
					if (p->first_cluster == first_cluster) {
						printf("file is already opened\n");
						return;
					}
    				p = p->next;
				}

				// if there are no errors create new node with file data and add to linked list
				open_files = addNode(open_files, newFileName, modeInt, first_cluster); 
			}
		}
		if(entryOffset == 0) {
					printf("File cannot be found!\n");
					return;
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
	}

	if(strcmp(mode, "w") == 0 || strcmp(mode, "wr") == 0) {
		current_FAT = first_cluster;
		while(current_FAT < 268435448) {
			fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
			tempStr[3] = fgetc(FATimage);
			tempStr[2] = fgetc(FATimage);
			tempStr[1] = fgetc(FATimage);
			tempStr[0] = fgetc(FATimage);
			tempStr[4] = '\0';
			sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
			fseek(FATimage,-4,SEEK_CUR);
			temp = current_FAT;
			current_FAT = hexToDec(tempStr);
			if(temp != first_cluster) {
				for(i = 0; i<4; i++)
					tempStr[i] = 0;
				fwrite(tempStr, sizeof(char), 4, FATimage);
			}
			else {
				for(i = 0; i<4; i++)
					tempStr[i] = 255;
				fwrite(tempStr, sizeof(char), 4, FATimage);
			}
		}
		fseek(FATimage, data_region_start + ((first_cluster-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster),SEEK_SET);
		for(i=0; i < FATinfo.bytesPerSector * FATinfo.sectorsPerCluster; i++)
			fputc(0, FATimage);
		fseek(FATimage, entryOffset + 28, SEEK_SET);
		for(i=0;i<4;i++)
			fputc(0, FATimage);
	}
}

void close_command(char *fileName) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12];
	unsigned int current_FAT, current_address = 0, attribute, size;
	int i,x;
	unsigned int first_cluster;
	unsigned char tempMode[2];

	for(i = 0; i < 8; i++) {
		if(i < strlen(fileName))
			newFileName[i] = toupper(fileName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';
	current_FAT = current_dir;


	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if((attribute != 15) && (strcmp(newFileName,tempFileName) == 0) && (attribute != 16)) {

				// filename found!

				// determine first cluster
				fseek(FATimage, 8,SEEK_CUR); 
                tempStr[1] = fgetc(FATimage);
                tempStr[0] = fgetc(FATimage);
                fseek(FATimage, 4,SEEK_CUR);
                tempStr[3] = fgetc(FATimage);
                tempStr[2] = fgetc(FATimage);
                tempStr[4] = '\0';
                sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
                first_cluster = hexToDec(tempStr);

				// remove file from open_files list
				open_files = deleteNode(open_files, first_cluster);
				return;
			}
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
	}

	printf("\'%s\' could not be found\n",fileName);
}

void lseek_command(char *fileName, char *offset) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12];
	unsigned int current_FAT, current_address = 0, attribute, size;
	int i,x;
	unsigned int first_cluster, offsetInt = 0;
	unsigned char tempMode[2];

	for(i = 0; i < 8; i++) {
		if(i < strlen(fileName))
			newFileName[i] = toupper(fileName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';
	current_FAT = current_dir;


	// extract offset integer value from char array
	int len = strlen(offset); 
      
    // Initializing base value to 1
    int base = 1; 
      
    // Extracting characters as digits from last character 
    for (i=len-1; i>=0; i--) 
    {    
        // if character lies in '0'-'9', converting  
        // it to integral 0-9 by subtracting 48 from 
        // ASCII value. 
        if (offset[i]>='0' && offset[i]<='9') 
        { 
            offsetInt += (offset[i] - 48)*base; 
                  
            // incrementing base by power 
            base = base * 10; 
        } 
  
    }


	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if((attribute != 15) && (strcmp(newFileName,tempFileName) == 0) && (attribute != 16)) {

				// filename found!

				// determine first cluster
				fseek(FATimage, 8,SEEK_CUR); 
                tempStr[1] = fgetc(FATimage);
                tempStr[0] = fgetc(FATimage);
                fseek(FATimage, 4,SEEK_CUR);
                tempStr[3] = fgetc(FATimage);
                tempStr[2] = fgetc(FATimage);
                tempStr[4] = '\0';
                sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
                first_cluster = hexToDec(tempStr);


                // determine the size of the file
				tempStr[3] = fgetc(FATimage);
				tempStr[2] = fgetc(FATimage);
				tempStr[1] = fgetc(FATimage);
				tempStr[0] = fgetc(FATimage);
				tempStr[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
				size = hexToDec(tempStr);

				if (offsetInt > size) {
					printf("offset is larger than file size\n");
					return;
				}

				// find file in opened files list
				node temp = open_files;
				while (temp != NULL && temp->first_cluster != first_cluster) 
			    {  
			        temp = temp->next; 
			    } 

			    // If file was not present in linked list 
			    if (temp == NULL) {
			    	printf("file is not open\n");
			    	return;
			    }

			    // change offseYEET9t of temp
			    temp->offset = offsetInt;
				return;
			}
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
	}

	printf("\'%s\' could not be found\n",fileName);
}
void read_command(char *fileName, char *size) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12];
	unsigned int current_FAT, current_address = 0, attribute;
	int i,x;
	unsigned int first_cluster, sizeInt, check, start_cluster, fileSize, randomCounter;
	unsigned char tempMode[2];
	unsigned int firstClust, tempOffset, temp, moveClusters, temp2, stage, count;
	node tempNode;


	for(i = 0; i < 8; i++) {
		if(i < strlen(fileName))
			newFileName[i] = toupper(fileName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';
	current_FAT = current_dir;


	// extract size integer value from char array
	int len = strlen(size); 
      
    // Initializing base value to 1
    int base = 1; 

	sizeInt = 0;
      
    // Extracting characters as digits from last character 
    for (i=len-1; i>=0; i--) 
    {    
        // if character lies in '0'-'9', converting  
        // it to integral 0-9 by subtracting 48 from 
        // ASCII value. 
        if (size[i]>='0' && size[i]<='9') 
        { 
            sizeInt += (size[i] - 48)*base; 
                  
            // incrementing base by power 
            base = base * 10; 
        } 
  
    }


	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if((attribute != 15) && (strcmp(newFileName,tempFileName) == 0)) {

				// filename found!

				// check if it is a directory
				if (attribute == 16) {
					printf("argument is a directory name, not a file name\n");
					return;
				}

				// determine first cluster
				fseek(FATimage, 8,SEEK_CUR); 
                tempStr[1] = fgetc(FATimage);
                tempStr[0] = fgetc(FATimage);
                fseek(FATimage, 4,SEEK_CUR);
                tempStr[3] = fgetc(FATimage);
                tempStr[2] = fgetc(FATimage);
                tempStr[4] = '\0';
                sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
                first_cluster = hexToDec(tempStr);

                // save fileSize
				tempStr[3] = fgetc(FATimage);
				tempStr[2] = fgetc(FATimage);
				tempStr[1] = fgetc(FATimage);
				tempStr[0] = fgetc(FATimage);
				tempStr[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
				fileSize = hexToDec(tempStr);




				// find file in opened files list
				tempNode = open_files;
				while (tempNode != NULL && tempNode->first_cluster != first_cluster) 
			    {  
			        tempNode = tempNode->next; 
			    } 

			    // If file was not present in linked list 
			    if (tempNode == NULL) {
			    	printf("file is not open\n");
			    	return;
			    }

			    // save values of node
				firstClust = tempNode->first_cluster;
				tempOffset = tempNode->offset;
				temp = tempNode->offset;


				// check if mode is write only
				if (tempNode->mode == 2) {
					printf("File is open in write-only");
					return;
				}
				
				


				// find number of clusters to move through
				moveClusters = 0;
				while (temp >= (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)) {
					moveClusters = moveClusters + 1;
					temp = temp - (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
				}

				// set current FAT to first cluster 
				current_FAT = firstClust;

				// move through file clusters "moveClusters" times
				for (i=0; i<moveClusters; i++) {
					
					fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
					tempStr[3] = fgetc(FATimage);
					tempStr[2] = fgetc(FATimage);
					tempStr[1] = fgetc(FATimage);
					tempStr[0] = fgetc(FATimage);
					tempStr[4] = '\0';
					sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
					current_FAT = hexToDec(tempStr);
				}
				

				// seek to starting address
				current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
				fseek(FATimage,current_address + temp, SEEK_SET);
				
				// start buffer for sizeInt characters
				stage = 1;
				temp2 = 0;
				count = 0;
				for (i = 0; i < sizeInt; i++) {

					if (tempOffset + i >= fileSize) {
						printf("// end of file reached\n");
						return;
					}



					// check if offset plus i is larger than bytes per cluster and if so go to next cluster
					if (((i + temp) >= (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)) && (stage == 1)) {
						fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
						tempStr[3] = fgetc(FATimage);
						tempStr[2] = fgetc(FATimage);
						tempStr[1] = fgetc(FATimage);
						tempStr[0] = fgetc(FATimage);
						tempStr[4] = '\0';
						sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
						current_FAT = hexToDec(tempStr);
						current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
						fseek(FATimage, current_address, SEEK_SET);
						temp2 = i;
						stage = 2;
					}



					
					// if already went to a new cluster, check if new i is larger than bytes per cluster and continue going to next clusters if it is
					if ((i - (count * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster) >= (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster) + temp2) && (stage == 2)) {
						fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
						tempStr[3] = fgetc(FATimage);
						tempStr[2] = fgetc(FATimage);
						tempStr[1] = fgetc(FATimage);
						tempStr[0] = fgetc(FATimage);
						tempStr[4] = '\0';
						sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
						current_FAT = hexToDec(tempStr);
						current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
						fseek(FATimage, current_address, SEEK_SET);
						count = count + 1;
					}


					printf("%c", fgetc(FATimage));
				}
				printf("\n");

				// update offset
				if (tempOffset + sizeInt >= fileSize)
					tempNode->offset = fileSize;
				else 
					tempNode->offset = tempOffset + sizeInt;

				return;

			}
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
	}

}


//----------------------------------------------------------------------------------------^
void write_command(char *fileName, char *stringInputSize, char *inputString) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12], tempC[1], tempStr2[3], tempStr3[9];
	unsigned int current_FAT, current_address = 0, attribute, size, inputSize, buffCount;
	int i,x;
	unsigned int modeInt, first_cluster, entryOffset = 0, loopCount, buffCountTotal;
	unsigned char tempMode[2];
	node temp = open_files;

	inputSize = atoi(stringInputSize);

	for(i = 0; i < 8; i++) {
		if(i < strlen(fileName))
			newFileName[i] = toupper(fileName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';
	current_FAT = current_dir;


	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if(strcmp(newFileName,tempFileName) == 0 && (attribute != 16)) {
				// filename found!

				// determine first cluster

				entryOffset = current_address + (i*32);
				fseek(FATimage, 8,SEEK_CUR); 
               			tempStr[1] = fgetc(FATimage);
                		tempStr[0] = fgetc(FATimage);
                		fseek(FATimage, 4,SEEK_CUR);
                		tempStr[3] = fgetc(FATimage);
                		tempStr[2] = fgetc(FATimage);
                		tempStr[4] = '\0';
                		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
                		first_cluster = hexToDec(tempStr);


                // determine the size of the file
				tempStr[3] = fgetc(FATimage);
				tempStr[2] = fgetc(FATimage);
				tempStr[1] = fgetc(FATimage);
				tempStr[0] = fgetc(FATimage);
				tempStr[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2],tempStr[3]);
				size = hexToDec(tempStr);

				// find file in opened files list

				while (temp != NULL && temp->first_cluster != first_cluster) 
			    	{  
			        temp = temp->next; 
				printf("fileName: %s\n",temp->filename);
			    	} 

			    	// If file was not present in linked list 
			   	 if (temp == NULL) {
			    		printf("file is not open\n");
			    		return;
			   	 }

				// print opened files
				if((attribute&1) == 1) {
					printf("\'%s\' is read only!\n",fileName);
					return;
				}
			}
		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
	}


	buffCount = temp->offset;
	current_FAT = temp->first_cluster;	
	while(buffCount > FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)  {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);
		buffCount = buffCount - (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

	}
	buffCountTotal = temp->offset;







	fseek(FATimage,buffCount + data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster), SEEK_SET);

	for(i=0; i<inputSize;i++) {
		if(buffCountTotal >= size && buffCount%(FATinfo.bytesPerSector * FATinfo.sectorsPerCluster) == 0 && size > 0)  {
			current_FAT = find_new_FAT(current_FAT);
			buffCount = 0;
			fseek(FATimage,buffCount + data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster), SEEK_SET); 
		}
		else if(buffCount%(FATinfo.bytesPerSector * FATinfo.sectorsPerCluster) == 0 && buffCountTotal != 0) {
			current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);
			fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET); // actual byte address
			tempStr[3] = fgetc(FATimage);
			tempStr[2] = fgetc(FATimage);
			tempStr[1] = fgetc(FATimage);
			tempStr[0] = fgetc(FATimage);
			tempStr[4] = '\0';
			sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
			current_FAT = hexToDec(tempStr);
			buffCount = buffCount - (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

			buffCount = 0;
			fseek(FATimage,buffCount + data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster), SEEK_SET);
		}
		if(i < strlen(inputString)-2) {
			fputc(inputString[i+1], FATimage);
		}
		else {
			fputc(0, FATimage);
		}
		if(buffCountTotal == size)
			size++;
		buffCount++;
		buffCountTotal++;
	} 






	temp->offset = temp->offset + inputSize;
	fseek(FATimage, entryOffset + 28, SEEK_SET);
	for(i=0;i<8;i++)
		tempStr3[i] = 48;
	tempStr3[8] = 0;

	decToHexa(size, tempStr3);


	for(i=0; i<4;i++) {
		tempStr2[0] = tolower(tempStr3[(i*2)+1]);
		tempStr2[1] = tolower(tempStr3[i*2]);
		tempStr2[2] = '\0';
		fputc(hexToDec(tempStr2),FATimage);
	}

}
void rmdir_command(char *dirName) {
	unsigned char newFileName[12], tempStr[5], tempFileName[12];
	unsigned int current_FAT, current_address = 0, attribute;
	int i,x, dir_location = -1, filesFound = 0, lastEntry = 0, old_entry = 0;

	for(i = 0; i < 8; i++) {
		if(i < strlen(dirName))
			newFileName[i] = toupper(dirName[i]);
		else	
			newFileName[i] = ' ';
	}
	newFileName[8] = ' ';
	newFileName[9] = ' ';
	newFileName[10] = ' ';
	newFileName[11] = '\0';

	current_FAT = current_dir;
	
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		if(strcmp(newFileName,".          ") == 0) {
			dir_location = current_dir;
			break;
		}
		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if(attribute != 15 && strcmp(newFileName,tempFileName) == 0) {
				if((attribute & 16) != 16) {
					printf("\'%s\' is not a directory\n",dirName);
					return;
				}



				fseek(FATimage, 8,SEEK_CUR); 
				tempStr[1] = fgetc(FATimage);
				tempStr[0] = fgetc(FATimage);
				fseek(FATimage, 4,SEEK_CUR);
				tempStr[3] = fgetc(FATimage);
				tempStr[2] = fgetc(FATimage);
				tempStr[4] = '\0';
				sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
				dir_location = hexToDec(tempStr);
				fseek(FATimage, current_address + (i*32),SEEK_SET);
				lastEntry = 1;
				old_entry = current_address + (i*32);
				break;
			}
		}
		if(dir_location != 0)
			break;
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);

		


	}
	if(dir_location == -1) {
		printf("\'%s\' could not be found\n",dirName);
		return;
	}
	else if(dir_location == 0)
		dir_location = 2;
	current_FAT = dir_location;
	while(current_FAT < 268435448) {
		current_address = data_region_start + ((current_FAT-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster);

		for(i=0;i < (FATinfo.bytesPerSector * FATinfo.sectorsPerCluster)/32;i++) {
			fseek(FATimage, current_address + (i*32),SEEK_SET);
			for(x=0; x<11;x++)
				tempFileName[x] = fgetc(FATimage);
			tempFileName[11] = '\0';
			attribute = fgetc(FATimage);
			if(strcmp(tempFileName,".          ") == 0 || strcmp(tempFileName,"..         ") == 0)          
				continue;
			if(((attribute & 2) == 0) && ((attribute & 4) == 0) && attribute != 0 && tempFileName[0] != 0 && tempFileName[0] != 229)
				filesFound = 1;

		}
		fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		current_FAT = hexToDec(tempStr);

	}
	if (filesFound == 1) {
		printf("files exist within \'%s\'\n",dirName);
	}
	else {
		current_FAT = dir_location;
		while(current_FAT < 268435448) {
			fseek(FATimage,FAT_region_start + ((current_FAT)*4),SEEK_SET);
			tempStr[3] = fgetc(FATimage);
			tempStr[2] = fgetc(FATimage);
			tempStr[1] = fgetc(FATimage);
			tempStr[0] = fgetc(FATimage);
			tempStr[4] = '\0';
			sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
			current_FAT = hexToDec(tempStr);
			fseek(FATimage,-4,SEEK_CUR);
			for(i = 0; i<4; i++)
				tempStr[i] = 0;
			fwrite(tempStr, sizeof(char), 4, FATimage);
		
			if(lastEntry == 1)
				tempStr[0] = 0;
			else
				tempStr[0] = 229;
			fseek(FATimage, old_entry, SEEK_SET);
			fwrite(tempStr, sizeof(char), 1, FATimage);
		}
	}
}
unsigned int find_new_FAT(unsigned int lastFAT) {
	unsigned int newFAT, end = 0, counter = 2;
	unsigned char tempStr[5], tempStr2[3], inputStr[4], tempStr3[9];
	int i;

	fseek(FATimage,FAT_region_start + 8,SEEK_SET);

	while(counter < FATinfo.numberOfFATs * FATinfo.FATsize * FATinfo.bytesPerSector/4) {
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		newFAT = hexToDec(tempStr);
		if(newFAT == 0)
			break;
		counter++;
	}
	for(i=0;i<8;i++)
		tempStr3[i] = 48;
	tempStr3[8] = 0;

	
	decToHexa(counter,tempStr3);
	for(i=0; i<4;i++) {
		tempStr2[0] = tolower(tempStr3[(i*2)+1]);
		tempStr2[1] = tolower(tempStr3[i*2]);
		tempStr2[2] = '\0';
		inputStr[i] = hexToDec(tempStr2);
	}

	fseek(FATimage,FAT_region_start + lastFAT*4,SEEK_SET);
	fwrite(inputStr, sizeof(char), 4, FATimage);

	for(i = 0;i < 4; i++)
		inputStr[i] = 255;
	fseek(FATimage,FAT_region_start + counter*4,SEEK_SET);
	fwrite(inputStr, sizeof(char), 4, FATimage);
	fseek(FATimage, data_region_start + ((counter-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster),SEEK_SET);
	for(i=0; i < FATinfo.bytesPerSector * FATinfo.sectorsPerCluster; i++)
		fputc(0, FATimage);
	return counter;
}
int new_file(unsigned char* file_entry) {
	unsigned int newFAT, end = 0, counter = FATinfo.rootCluster;
	unsigned char tempStr[5], tempStr2[3], inputStr[4], tempStr3[9];
	int i;

	fseek(FATimage,FAT_region_start + 8,SEEK_SET);

	while(counter < FATinfo.numberOfFATs * FATinfo.FATsize * FATinfo.bytesPerSector/4) {
		tempStr[3] = fgetc(FATimage);
		tempStr[2] = fgetc(FATimage);
		tempStr[1] = fgetc(FATimage);
		tempStr[0] = fgetc(FATimage);
		tempStr[4] = '\0';
		sprintf(tempStr,"%02x%02x%02x%02x",tempStr[0], tempStr[1], tempStr[2], tempStr[3]);
		newFAT = hexToDec(tempStr);
		if(newFAT == 0)
			break;
		counter++;
	}
	if(counter == FATinfo.numberOfFATs * FATinfo.FATsize * FATinfo.bytesPerSector/4-1)
		return -1;
	for(i=0;i<8;i++)
		tempStr3[i] = 48;
	tempStr3[8] = 0;

	decToHexa(counter,tempStr3);
	for(i=0; i<2;i++) {
		tempStr2[0] = tolower(tempStr3[(i*2)+1]);
		tempStr2[1] = tolower(tempStr3[i*2]);
		tempStr2[2] = '\0';
		file_entry[i+26] = hexToDec(tempStr2);
	}
	for(i=2; i<4;i++) {
		tempStr2[0] = tolower(tempStr3[(i*2)+1]);
		tempStr2[1] = tolower(tempStr3[i*2]);
		tempStr2[2] = '\0';
		file_entry[i+20] = hexToDec(tempStr2);
	}
	for(i = 0;i < 4; i++)
		inputStr[i] = 255;
	fseek(FATimage,FAT_region_start + counter*4,SEEK_SET);
	fwrite(inputStr, sizeof(char), 4, FATimage);
	fseek(FATimage, data_region_start + ((counter-2) * FATinfo.bytesPerSector * FATinfo.sectorsPerCluster),SEEK_SET);
	for(i=0; i < FATinfo.bytesPerSector * FATinfo.sectorsPerCluster; i++)
		fputc(0, FATimage);
	return counter;
}

// helper function for creating a new node in linked list
node createNode(){
    	node temp; // declare a node
    	temp = (node) malloc(sizeof(struct LinkedList)); // allocate memory using malloc()
    	temp->next = NULL; // make next point to NULL
    	return temp; //return the new node
}


// helper function for adding the new node to the end of the linked list
node addNode(node head, unsigned char *name, unsigned int mode, unsigned int cluster){
    	node temp,p; // declare two nodes temp and p
    	temp = createNode(); //createNode will return a new node with empty data and next pointing to NULL.
    	strcpy(temp->filename, name); // set filename
    	temp->mode = mode; // set mode
    	temp->first_cluster = cluster; // set first cluster
    	temp->offset = 0; // set offset00 00 00 00
    	if(head == NULL){
        	head = temp; //when linked list is empty
   		}
    	else{
        	p = head; //assign head to p 
        	while(p->next != NULL){
            		p = p->next; // traverse the list until p is the last node. The last node always points to NULL.
        	}
        	p->next = temp; // Point the previous last node to the new node created.
    	} 
	return head;
}

node deleteNode(node head, unsigned int first_cluster) {
	node temp = head;
	node prev = NULL;
	if (temp != NULL && temp->first_cluster == first_cluster) {
		head = temp -> next;
		free(temp);
		return head;
	}

	while (temp != NULL && temp->first_cluster != first_cluster) 
    { 
        prev = temp; 
        temp = temp->next; 
    } 


    // If key was not present in linked list 
    if (temp == NULL) {
    	printf("file is already closed\n");
    	return head;
    }

    // Unlink the node from linked list 
    prev->next = temp->next;

    free(temp);  // Free memory 
    return head;
}
