//Alex Stalter
//Jack Nichols
//Joe Silveira
//#include <stdio.h>

void printString(char*);
void printChar(char);
void readString(char*);
void readSector(char*,int);
void handleInterrupt21(int,int,int,int,int);
void readFile(char*,char*,int*);
void writeSector(char*, int);
void executeProgram(char*);
void terminate();
void listDir();
void deleteFile(char*);
void writeFile(char*,char*,int);
void handleTimerInterrupt(int, int);

void main()
{

	makeInterrupt21();
	interrupt(0x21,8,"this is a test message","testmg",3);
	makeTimerInterrupt();
	interrupt(0x21,4,"shell",0,0);
	while(1);

}
void handleTimerInterrupt(int segment, int sp)
{
	
		printChar('T');
		printChar('i');
		printChar('c');
	
	returnFromTimer(segment,sp);
	
}
void printString(char* chars){
	int i = 0;
	while(chars[i]!=0x0){

	interrupt(0x10,0xe*256+chars[i],0,0,0);
	i++;
	}

}

void printChar(char c){
	interrupt(0x10,0xe*256+c,0,0,0);
}
void readString(char* line){

	int i =0;
	int exit =0;
	while(exit==0){

	char input = interrupt(0x16,0,0,0,0);
	if(input==0xd){
	printChar(0xd);
	printChar(0xa);
	line[i]=0xd;
	i++;
	line[i]=0xa;
	i++;
	line[i]=0x0;

	exit=1;

	}else if(input==0x8 && i!=0){
	printChar(0x8);
	printChar(' ');
	printChar(0x8);
	i--;
	}else if(input!=0x8){
	line[i]=input;
	printChar(input);
	i++;
	}
	}


}
void readSector(char* buffer,int sector){


	int ax = 2*256+1;
	int cx = 0*256+(sector+1);
	int dx = 0*256+0x80;
	interrupt(0x13,ax,buffer,cx,dx);


}
void writeSector(char* buffer, int sector){
	int ax = 3*256+1;
	int cx = 0*256+(sector + 1);
	int dx = 0*256+0x80;
	interrupt(0x13,ax,buffer,cx,dx);
}
void handleInterrupt21(int ax,int bx,int cx,int dx){

	if(ax==0){
	printString(bx);
	}else if(ax==1){

	readString(bx);

	}else if(ax==2){

	readSector(bx,cx);

	}else if(ax==3){

	readFile(bx,cx,dx);
	
	}else if(ax==4){

	executeProgram(bx);

	}else if(ax==5){
	
	terminate();

	}else if(ax==6){

	writeSector(bx,cx);

	}else if(ax==7) {

	deleteFile(bx);

	}else if(ax==8) {

	writeFile(bx,cx,dx);

	}else if(ax==9){

	listDir();

	}else{

	printString("ERROR no such function exists\n");

	}


}
//old readFile
/*void readFile(char* address,char* buffer,int* sectorsRead){
	int count=0;
	int i=0;
	int matches = 0;
	char dir[512];
	readSector(dir,2);
	for(count=0;count<512;count=count+32)
	{
	*sectorsRead=*sectorsRead+1;
		for(i=0;i<6;i++)
		{
		if(address[i]==dir[i+count]){
		matches++;
//		printChar(dir[i+count]);
		}else{
		matches=0;
		break;
		}
		}
	if(matches==6){
	readSector(buffer,*sectorsRead+4);
	break;
}
	}
	if(matches!=6){
	*sectorsRead=0;
	printString("/nError file not found.");
}
}
*/
//New readFile written by Joe Silveira
void readFile(char* fileName , char* fileBuffer,int* sectorsRead) {

//Variables
	char fileDirectory[512];
	char buffer[512];

	//for loop variables 

	int i = 0;	//Directory iteration
	int k = 0;	//File name iteration
	int j = 0;	//sector iteration
	int l = 0;	//Inner sector iteration (# sectors)

	int matches = 0;	

	/*
		*Function begin
	*/

	//1. Load file in to 512 char array
	readSector(buffer,2);

	//2. Loop through the directory 
	for(i = 0; i < 512; i++){

		//2.1 try to match the file name
		if(buffer[i] != fileName[matches]){

			//2.2 if it doesnt match go to the next sector
			i+=31-matches;
			matches = 0;
			*sectorsRead = 0;
		}else if (matches == 5){
			//printString("Matches \n");
			

			//Loop through the remaining 26 bytes (sectors)
			for(j = 0; j < 26; j++){
				*sectorsRead++;
				//If the next sector is empty (== '\0') set the filebuffer to \0 and break
				if(buffer[j + i + 1] == '\0'){

					fileBuffer[j * 512] = '\0';

					//printString("setting to 0\n");
					break;

				//If not read the sector 
				}else{

				//printString("Reading Sectors\n");

				//Read the sectors and set them equal to the given sector
					readSector(fileDirectory,buffer[j + i + 1]);

				//3C: This is where we eliminate sectors read and have the for loop perform 
					for(l = 0; l < 512; l++){

					//Set the passed sector equal to the directory sector
						fileBuffer[(j * 512) + l] = fileDirectory[l];
					}
				}
			}
			break;

		//buffer[i] does equal fileBuffer[macthes]	so ++;
		}else{
			matches++;
		}

	}
	if(matches==0)
	{

	*sectorsRead=0;

	}
	//return;
}






void listDir(){



	//local vars
	char directoryBuffer[512];
	char buffer[512];
	int i;
	int j;
	int k = 0;
	int l = 0;
	int size = 0;


	//Set both buffers at i to 0 to start
	for(i = 0; i < 512; i++){
		directoryBuffer[i] = 0x0;
		buffer[i] = 0x0;
	}

	//Read the given sector of the directory
	readSector(directoryBuffer,2);

	//loop through the sector 
	for(i = 0; i < 16; i++){

		//if file exists with in the given sector
		if(directoryBuffer[32 * i] != '\0'){

			//Read the file name 
			for(j = 0; j < 6; j++){

				buffer[k] = directoryBuffer[i * 32 + j];
				k++;		
				

			}

			//Add a new line and return for every file 
			//print nicely on its own line
			buffer[k] = '\r';
			k++;
			buffer[k] = '\n';
			k++;
			
		}
	}
	printString("\n\r");
	for(i = 0; i < 512; i++){

		if(buffer[i] != 0x0){
			printChar(buffer[i]);
		}	
	}

}
void executeProgram(char* name)
{
	int i;
	int sectorsRead;
	char buffer[13312];
	int segment = 0x2000;

	readFile(name, buffer, &sectorsRead);

	for(i = 0; i < 13312; i++)
	{
		putInMemory(segment, i, buffer[i]);
	}

	launchProgram(segment);


}

void terminate()
{

	char shell[6];
	shell[0] = 's';
	shell[1] = 'h';
	shell[2] = 'e';
	shell[3] = 'l';
	shell[4] = 'l';
	shell[5] = '\0';
	executeProgram(shell);

}
void deleteFile(char* name)
{
	char dir[512];
	char map[512];
	int matches;
	int i, j;
	int found, index;

	readSector(map, 1);
	readSector(dir, 2);
	for(i = 0;i<512;i+=32){

	for(j=0;j<6;j++){

	if(dir[j+i]=='l'&&name[j]=='l'&&dir[j+i+1]=='l'&&name[j+1]=='l'){

	matches=1;
	break;

	}else if(name[j]!=dir[j+i]){


	break;

	}else if(j==5){

	matches=1;
	break;

	}

	}

	if(matches==1){

	dir[i]=0x0;
	for(found=6;found<32;found++){

	if(dir[i+found]!=0){

	matches=dir[i+found];
	map[matches]=0;

	}else if(dir[i+found]==0&&dir[i+found+1]==0){

	break;

	}

	}
	writeSector(dir,2);
	writeSector(map,1);
	break;
	}

	}




}
void writeFile(char* buffer,char* file,int sectors){

	char map[512];
	char dir[512];
	int i,j,k,sector,address,sectorsWrote;


	readSector(map,1);
	readSector(dir,2);
	for(i=0;i<512;i+=32){

		if(dir[i]=='\0'){

			for(j=0;j<6;j++){

				if(file[j]=='\0'){


					for(k=0;k<(6-j);k++){

					dir[i+j+k]='\0';

					}
					break;

				}else{

					dir[i+j]=file[j];

				}

			}

			for(sector=3;sector<32;sector++){

				if(map[sector]==0){

					map[sector]=255;
					sectorsWrote++;
					if(sectorsWrote==sectors){
						sector++;
						break;
					}
				}


			}
			for(j=0;j<sectors;j++){

				for(k=6;k<26;k++){

					if(dir[i+k]==0){

					dir[i+k]=sector-(sectors-j);

					break;

					}

				}
				address=buffer+j*512;
				writeSector(address,(sector-(sectors-j)));


				if(map[sector]==255){

					for(k=sector;k<32;k++){

						if(map[k]==0){

							sector=k;
							break;

						}

					}


				}

				if(j==sectors){

					for(k=6+j;k<26-j;k++){

						dir[i+k]='\0';

					}

				}



			}


			writeSector(map,1);
			writeSector(dir,2);
			break;

		}
	}

}
