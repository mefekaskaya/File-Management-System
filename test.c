#include <fms.h>
#include <string.h>
typedef struct{
	char team[44];
	char shortname[10];
	int rank;
	int goalscored;
	int goalconceeded;
	int point;
} club;

void printClub(void*data){

	club*myclub=(club*)data;
	
	printf("Team:%s\nShortname:%s\nRank:%d\n",myclub->team,myclub->shortname,myclub->rank);
}

void scenario1(){
	myFileCreate("test","test.json");
	
	club c1;
	memset(&c1,0,sizeof(c1));
	strcpy(c1.team,"Giresun");
	strcpy(c1.shortname,"GRSN");
	
	c1.rank = 1;
	c1.goalscored = 2;
	c1.goalconceeded = 3;
	c1.point = 4;
	
	club c2;
	memset(&c2,0,sizeof(c2));
	strcpy(c2.team,"Hatayspor");
	strcpy(c2.shortname,"HTY");

	c2.rank = 5;
	c2.goalscored = 6;
	c2.goalconceeded = 7;
	c2.point = 8;
	
	myFileWrite(&c1);
	myFileWrite(&c2);
	
	printDataFile(printClub);
	
	myFileClose();
}

void scenario2(){
	
	myFileOpen("test");
	
	club c;
	
	myFileFind("GRSN",&c);

	printClub(&c);
			
	printIndexingFile();
	
	myFileClose();
}

int main(void){
	scenario2();
}