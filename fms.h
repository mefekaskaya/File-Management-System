
#ifndef FMS_H_
#define FMS_H_

#include <stdio.h>

FILE *p; // File Pointer (for data file)
void *ndx; //key-sort array (To hold data of indexing file)

int myFileCreate(char * myfilename, char *jsonfilename);
/**************
This function creates an empty file (data file) with the name given in the first parameter myfilename and then the header of the respective file is taken from the second parameter jsonfilename. A variable called open should be initialized in the header part. It returns integer value 1 if creation is successful, otherwise 0.
It also should create an empty file for indexing.
*****************/
int myFileOpen(char * myfilename);
/**************
This function requires the name of the created file (data file). After opening the file, open parameter of the header will be changed to true. There must be all the necessary controls such as file not found, in case file is not created. It returns integer value 1 if the file is successfully opened, otherwise 0.
It should also open the indexing file and write all data into key-sort array (ndx pointer). Then it closes indexing file.
*****************/
int myFileClose();
/**************
This function doesnt require any parameter. It closes data file. Before closing the data file, open parameter of the header will be changed to false and other necessary changes must be done such as re-indexing if needed. It returns integer value 1 if the file is successfully closed, otherwise 0.
*****************/
int myFileWrite(void *data);
/**************
Insert the data into the file. It returns integer value 1 if the data is successfully written, otherwise 0.
Note 1: The data will be added to data file but not to indexing file in this part. It should update key-sort array (ndx pointer).
*****************/
int myFileDelete(void * key);
/**************
Delete the data with the given key from the file. It returns integer value 1 if deletion is successful, otherwise 0.
*****************/
int myWriteIndex();
/**************
This function writes all data from key-sort array (ndx pointer) into indexing file. It returns integer value 1 if the data is successfully written, otherwise 0.
*****************/
void printIndexingFile();
/** this function prints all elements in Indexing file **/
void printDataFile();
/** this function prints all records in Data file **/

int myFileFind(void * key, void * data);
/**************
Find the data with the given key in the file. If the key is found in key-sort array (ndx pointer), the parameter data should be loaded with the found data corresponding to the given key. It returns integer value 1 if key is found, otherwise 0.
Note 2: Search operation should be done in key-sort array (ndx pointer) to find the place of actual data in data file.
Note 3: This function should control if key-sort array (ndx pointer) is updated.
*****************/
int myFileFindNext(void * key, void * data);
/**************
Find the next key of the given key in the key-sort array (ndx pointer). If key is found, the parameter data should be loaded with the data corresponding to the key. It returns integer value 1 if data is found, otherwise 0.
Consider the same notes (2,3) for this function, too.
E.g.: if the input key is GRSN, it should return the data of GS
*****************/
int myFileReindex();
/**************
This function will be used in MyFileClose function, if any insertion/deletion is done and key-sort array (ndx pointer) is not written to indexing file. It returns integer value 1 if re-indexing is successful, otherwise 0.
*****************/
/**************
Note 4: You are free to add other global variables and other parameters as input to the functions. However, return type will not change.
Note 5: This library will not be aware of your structures. Therefore, you must use void pointers in this library. You can use casting in your .c file. ******/

#endif