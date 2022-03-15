#include "storage_mgr.h"
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/stat.h>
#include<stdbool.h>

// filepointer is pointed to the file that has been opened
FILE *filepointer;

int init = 0;

// introducing a function to store the value of return to avoid the function to terminate in the middle
RC return_val;
int checkinit(void){
    return_val = (init == 1)? RC_OK: -1;
    return return_val; // return_val key is referenced multiple times in the below code
}

/* manipulating page files */

// The initStorageManager initialized the process of executing all the below functions
void initStorageManager(void){
    if(checkinit() != RC_OK){
        init=1;
        printf("\n*** Initialized Storage Manager ***\n");
        //filepointer = NULL;
    }
    else{
        printf("\n*** Storage Manager already initialized ***\n");
    }
}

// In this function we are opening a file in write mode and checking if the same was executed or not. 
RC createPageFile(char *fileName){
    printf("\n*** Running createPageFile function ***\n");
    filepointer=fopen(fileName,"w");
    // RC return_val;
    int initial_file_size = 0; 

    if (filepointer == NULL){
        return RC_FILE_NOT_FOUND;
    }
    else{
        // here we are allocating memory size for an empty page to execute the file write function
        SM_PageHandle emptyPage = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));
        
        // we are introducting a check to ensure the required memory for the page is allocated
        if(fwrite(emptyPage,sizeof(char),PAGE_SIZE,filepointer)<PAGE_SIZE)
            printf("\nWrite Function Failed!\n");   
        
        else
            printf("\nWrite Function Executed Successfully!\n");

        // closing the file, it is necessary to close an opened file to avoid unwanted changes to the file
        fclose(filepointer);
        // once the task is done, we are deallocating the memory assigned for the page to ensure memory is restored
        free(emptyPage);

        return_val = RC_OK;
    }
    return return_val; // returns o/p as per the above included code
}

// In this function we open the created file and check the total pages, size, eof pointer, current page no.
RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    printf("\n*** Running openPageFile function ***\n");
    filepointer = fopen(fileName,"r");
    
    // Check if file was created
    if (filepointer == NULL)
	{
		return_val = RC_FILE_NOT_FOUND;
	}
	else
	{
		fseek(filepointer, 0, SEEK_END); // here we stream the file using the filepointer variable
		int EndByte = ftell(filepointer); // ftell returns the last byte of file
		int TotalLength = EndByte + 1;
		int totalNumPages = TotalLength / PAGE_SIZE; 
		
		// here we assign the variables such as filename, totalpages, pointer position with the file that was streamed
		fHandle -> fileName= fileName;
		fHandle -> totalNumPages = totalNumPages;
		fHandle -> curPagePos = 0;

        // in this we rewind or return back to the start of the stream
		rewind(filepointer); 
		return_val = RC_OK;
    }
    return return_val; // returns o/p as per the above included code
}

// In this function we close the file that was opened earlier. 
RC closePageFile (SM_FileHandle *fHandle){
    printf("\n*** Running closePageFile function ***\n");
    RC fileclosed;
    fileclosed = fclose(filepointer);

    if(fileclosed != 0){
        printf("\nFile not found!\n");
        return_val = RC_FILE_NOT_FOUND;
        }
	else{
        printf("\nFile was closed!\n");
		return_val = RC_OK;
        }
    return return_val; // returns o/p as per the above included code
}

// In this once the entire tasks are done, we destroy the file as it is not required. 
// This function will display OK message once the file is destroyed
RC destroyPageFile(char *fileName) {
    printf("\n*** Running destroyPageFile function ***\n");
	if (remove(fileName) !=0 ){
        printf("\nFile not found!\n");
        return_val = RC_FILE_NOT_FOUND; 
    }
    else {
        printf("\nFile Destroyed!\n");
        return_val = RC_OK;
    }
	return return_val; // returns o/p as per the above included code
	}

// In this function we read blocks from the created file in memory and update current page position
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n*** Running readBlock function ***\n");
    
    int read_ptr;

    if (pageNum < 0 || pageNum > fHandle->totalNumPages) // Here we raise exception if the page has less than the pageNum pages	
		return_val = RC_READ_NON_EXISTING_PAGE;
	else {
		fseek(filepointer, (pageNum * PAGE_SIZE), SEEK_SET); // We start the stream by reading the file
		read_ptr = fread(memPage, sizeof(char), PAGE_SIZE, filepointer); // We initiate a read pointer
		if (read_ptr < PAGE_SIZE || read_ptr > PAGE_SIZE) 
		{
			return_val = RC_READ_NON_EXISTING_PAGE;
            printf("\nTried to read a nonexistent page!\n");
		}
		fHandle->curPagePos = ftell(filepointer); // Here we set the current position of pointer to the eof

        fclose(filepointer); // close the file to avoid unwanted changes and to clear buffers
		return_val = RC_OK;
        printf("\nBlock read successfully!\n");
	}
	return return_val; // returns o/p as per the above included code
}

// In this function we return the current position of pointer in the open file
int getBlockPos (SM_FileHandle *fHandle){
    printf("\n*** Running getBlockPos function ***\n");
    if(fHandle==NULL) // Check if the file has any data
        {
          RC_message="\nNo data in file, file not initialized!\n";
          return_val = RC_FILE_NOT_FOUND;
        }
    else{       // returns the current position
          return_val = (fHandle->curPagePos);
        }
    return return_val; // returns o/p as per the above included code
}

// In this function we read the first page from the eof
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n*** Running readFirstBlock function ***\n");
    // if the file contains data, we set the read block pointer to the start of the file
    if(fHandle!=NULL){
        int i=readBlock (0,fHandle,memPage);
        return_val = RC_OK;
        printf("\nSuccessfull read first block of file!\n");
        }
    else{
        RC_message="\nThere is no data in the first block to read\n";
        return_val = RC_FILE_HANDLE_NOT_INIT;
    }
    return return_val; // returns o/p as per the above included code
}

// In this function we read the previous page
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n*** Running readPreviousBlock function ***\n");
    // here we are setting the read pointer to the previous page to read the file 
    if (RC_OK == readBlock(getBlockPos(fHandle)-1, fHandle, memPage)){
        return_val = RC_OK;
        printf("\nSuccessfully executed readpreviousblock function!\n");
        }
    else{
        return_val = RC_READ_NON_EXISTING_PAGE;
        printf("\nFunction failed\n");
    }
    return return_val; // returns o/p as per the above included code
}

// In this function we read the current page
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n*** Running readCurrentBlock function ***\n");
    // here we are setting the read pointer to the current page and reading the file
    if (RC_OK == readBlock(getBlockPos(fHandle),fHandle,memPage)){
        return_val = RC_OK;
        printf("\nSuccessfully read the current page\n");
    }
    else {
        return_val = RC_READ_NON_EXISTING_PAGE;
        printf("\nFunction failed!\n");
    }
    return return_val; // returns o/p as per the above included code
}

// In this function we get the read pointer to next page to read the next block
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n*** Running readNextBlock function ***\n");
    // setting the read pointer to next page
    if (RC_OK == readBlock(getBlockPos(fHandle)+1, fHandle, memPage)){
        return_val = RC_OK;
        printf("\nSuccessfully read the next page on file!\n");
    }
    else{
        return_val = RC_READ_NON_EXISTING_PAGE;
        printf("\nFunction failed!\n");
    }                             
    return return_val; // returns o/p as per the above included code
}

// In this function we read the eof page
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n*** Running readLastBlock function ***\n");
    // set the read pointer to the end of file
    if (RC_OK == readBlock((fHandle->totalNumPages-1), fHandle, memPage)){
        return_val = RC_OK; 
        printf("\nSuccessfully read the eof!\n");
    }
    else {
        return_val = RC_READ_NON_EXISTING_PAGE;
        printf("\function failed!\n");
    } 
    return return_val; // returns o/p as per the above included code
}

/* writing blocks to a page file */

// This function commits data to file if the file handle conditions are satisfied
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n*** Running writeBlock function ***\n");
    if(fHandle==NULL){
        RC_message="\nNo data in file, file not initialized\n"; 
        return_val = RC_FILE_NOT_FOUND;
     }
    else
    {
        if(fHandle->totalNumPages>pageNum && pageNum>=0)
        {
            if(memPage!=NULL)
            {
                //start the stream from the start of file
                fseek(filepointer,(pageNum+1)*PAGE_SIZE,SEEK_SET);
                fwrite(memPage,1,PAGE_SIZE,filepointer);

                // update the current page position with the current page number 
                fHandle->curPagePos=pageNum;
                RC_message="\nData written to file!\n";
                return_val = RC_OK;
            }
        }
        else
        {
            RC_message="\nNo pages in file!\n"; 
            return_val = RC_WRITE_FAILED;
        }
    }
    return return_val; // returns o/p as per the above included code
}

// In this function we write to the current page of file
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    printf("\n*** Running writeCurrentBlock function ***\n");

    // Check if file exists
    if(fHandle==NULL){
        RC_message="No data in file, file not initialized"; 
        return RC_FILE_NOT_FOUND;
    }
    else{
        // We are using writeblock to write to the current page and getblockpos to get the current position in the file
        return writeBlock (getBlockPos(fHandle),fHandle,memPage);
        printf("\nWrite current block function executed successfully!\n");
    }
}

// In this function we are appending an empty page at the eof
RC appendEmptyBlock (SM_FileHandle *fHandle){

    // We initialize an emptyblock pointer to write an empty page at the eof
    SM_PageHandle emptyBlk = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));

    if(fseek(filepointer, 0, SEEK_END) == 0){
        fwrite(emptyBlk,sizeof(char), PAGE_SIZE, filepointer);
        printf("\nSuccessfully appended an empty block at the eof!\n");
    }
    else {
        free(emptyBlk);
        return_val = RC_WRITE_FAILED;
    }
    // We free the memory allocated to ensure memory management
    free(emptyBlk);

    fHandle->totalNumPages++; // Here we increase the total number of pages as a new empty page has been added to the block
    return_val = RC_OK;

    return return_val; // returns o/p as per the above included code

}

// In this function we find the value of total number of pages of the file
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    filepointer = fopen(fHandle->fileName, "a"); // Open file in append mode

    if(filepointer == NULL){
        RC_message="No data in file, file not initialized"; 
        return RC_FILE_NOT_FOUND;
    }
    // here we check if the new capacity of block is greater than the current capacity of block
    while(numberOfPages > fHandle->totalNumPages){
        appendEmptyBlock(fHandle);
    }
    // Close the file to avoid any unwanted changes to the main file
    fclose(filepointer);
    return RC_OK;
}