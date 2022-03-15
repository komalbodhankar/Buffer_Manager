#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RC_QUEUE_IS_EMPTY 5;
#define RC_NO_FREE_BUFFER_ERROR 6;

SM_FileHandle *file_handle;
Queue *queue;
int read_IO;
int write_IO;

 //LRU Replacement Strategy
RC PAGE_LRU(BM_BufferPool * const bm, BM_PageHandle * const page,const PageNumber pageNum);

// FIFO Replacement Strategy 
RC PAGE_FIFO(BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum); 


/**************************************************************************************
 * Function Name: emptyQueue
 *
 * Description: 
 *		Empty the queue
 *	
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		
 *
 * Return:
 *		queue->filledframes==0
 *
 **************************************************************************************/
RC emptyQueue() 
{
	return ((*queue).filledframes == 0);
}

/**************************************************************************************
 * Function Name: createQueue
 *
 * Description: 
 *		Creating queue
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		
 *
 * Return:
 *		queue
 *
 **************************************************************************************/
 
void createQueue(BM_BufferPool *const bm)
{
	pageInfo *newPage[(*bm).numPages];
	int lastPage = ((*bm).numPages) - 1; 
	int i = 0;
	while (i <= lastPage)
	{
		newPage[i] = (pageInfo*)malloc(sizeof(pageInfo));
		(*newPage[i]).frameNum = i;
		(*newPage[i]).isDirty = 0;
		(*newPage[i]).fixCount = 0;
		(*newPage[i]).pageNum = -1;
		(*newPage[i]).bufferData = (char*) calloc(PAGE_SIZE, sizeof(char));
		i++;
	}
	int j = 0;
	while (j <= lastPage)
	{
		int i = j;
		(j == 0) ? (*newPage[i]).prevPageInfo = NULL, (*newPage[i]).nextPageInfo = newPage[i + 1] : ((i == lastPage) ? (*newPage[i]).nextPageInfo = NULL, (*newPage[i]).prevPageInfo = newPage[i - 1] : ((*newPage[i]).nextPageInfo = newPage[i + 1], (*newPage[i]).prevPageInfo = newPage[i - 1]));
		j++;
	}
	(*queue).head = newPage[0];
	(*queue).tail = newPage[lastPage];
	(*queue).filledframes = 0;
	(*queue).totalNumOfFrames = (*bm).numPages;
}


/**************************************************************************************
 * Function Name: pageInfo* CreateNewList
 *
 * Description: 
 *		Creating new list for page
 *		
 *
 * Parameters:
 *		PageNumber pageNum:  the page number of the requested page
 *		
 *
 * Return:
 *		new_page_info
 *
 **************************************************************************************/

pageInfo* createNewList(const PageNumber pageNum) 
{
	    pageInfo* new_page_info = (pageInfo*) malloc(sizeof(pageInfo));
	    char *c = (char*) calloc(PAGE_SIZE, sizeof(char));
	    (*new_page_info).pageNum = pageNum;
	    (*new_page_info).isDirty = 0;
	    (*new_page_info).frameNum = 0;
	    (*new_page_info).fixCount = 1;
	    (*new_page_info).bufferData = c;
	    (*new_page_info).prevPageInfo=NULL;
	    (*new_page_info).nextPageInfo = NULL;
	    return new_page_info;
}

/**************************************************************************************
 * Function Name: RC dequeue
 *
 * Description: 
 *		Page Frame Enqueue
 *		
 *
 * Parameters:
 *		none
 *		
 *
 * Return:
 *		pageDelete
 *
 **************************************************************************************/
RC deQueue() 
{
	int i=0;
	if (emptyQueue())
   	{
        	return RC_QUEUE_IS_EMPTY;
    	}
    	pageInfo *p = (*queue).head;
    	while (i < (*queue).filledframes)
    	{
        	(i == ((*queue).filledframes-1)) ? ((*queue).tail = p) : (p = p->nextPageInfo);  
        	i++; 
    	}
	int tail_pnum; 
	int pageDelete=0;
	pageInfo *pinfo = (*queue).tail;
	for (int i = 0; i < (*queue).totalNumOfFrames; i++)
	{

		if (((*pinfo).fixCount) != 0)
        {
            tail_pnum=(*pinfo).pageNum;
            pinfo = (*pinfo).prevPageInfo;
        }
        else 
        {
            
            if (pinfo->pageNum == queue->tail->pageNum)
			{
				pageDelete=pinfo->pageNum;
				queue->tail = (queue->tail->prevPageInfo);
				queue->tail->nextPageInfo = NULL;
 
			}
			else
			{
				pageDelete=pinfo->pageNum;
				pinfo->prevPageInfo->nextPageInfo = pinfo->nextPageInfo;
				pinfo->nextPageInfo->prevPageInfo = pinfo->prevPageInfo;
			}

        }
	}

	if (tail_pnum == (*queue).tail->pageNum)
	{	
		return 0;		//Add error	
	}

	if ((*pinfo).isDirty == 1) 
	{
		writeBlock((*pinfo).pageNum, file_handle, (*pinfo).bufferData);	
		write_IO++;
	}

	(*queue).filledframes--;
	return pageDelete;
}


/**************************************************************************************
 * Function Name: RC Enqueue
 *
 * Description: 
 *		Page Frame Enqueue
 *		
 *
 * Parameters:
 *      	BM_PageHandle *const page: Buffer Page Handler
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		const pageNum: the page number of the requested page
 *		
 *
 * Return:
 *		RC: Return Code
 *
 **************************************************************************************/

RC Enqueue(BM_PageHandle * const page, const PageNumber pageNum,BM_BufferPool * const bm) 
{

	int pageDelete=-1;
	while ((*queue).filledframes == (*queue).totalNumOfFrames ) { //If frames are full remove a page
		pageDelete=deQueue();
	}

	pageInfo* pinfo = createNewList(pageNum);
	//(*pinfo).bufferData=(*bm).mgmtData;

	while (emptyQueue()) {

		readBlock((*pinfo).pageNum,file_handle,(*pinfo).bufferData);
		(*page).data = (*pinfo).bufferData;
		read_IO++;

		(*pinfo).frameNum = (*queue).head->frameNum;
		(*pinfo).nextPageInfo = (*queue).head;
		(*queue).head->prevPageInfo = pinfo;
		(*pinfo).pageNum = pageNum;
		(*page).pageNum= pageNum;
		(*queue).head = pinfo;
		
		break;

	} while(!emptyQueue()) {  
		readBlock(pageNum, file_handle, (*pinfo).bufferData);
		pageDelete==-1 ? (*pinfo).frameNum = (*queue).head->frameNum+1 : ((*pinfo).frameNum = pageDelete);
		
		(*page).data = (*pinfo).bufferData;
		read_IO++;
		(*pinfo).nextPageInfo = (*queue).head;
		(*queue).head->prevPageInfo = pinfo;
		(*queue).head = pinfo;
		(*page).pageNum= pageNum;
		
		break;

	}
	(*queue).filledframes++;

	return RC_OK; 
}



//------------Buffer Manager Interface Pool Handling-----------------
/**************************************************************************************
 * Function Name: initBufferPool
 *
 * Description: 
 *		Initiate the BufferPool with page number
 *		using the Page Replacement Strategy
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		const char *const pageFileName: the name of requested page file
 *		const int numPages: capacity of Buffer Pool
 *		ReplacementStrategy strategy: Replacement strategy
 *		void *stratData: 
 *
 * Return:
 *		RC: Return Code
 *
 **************************************************************************************/


RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{
          read_IO = 0;
	   write_IO = 0;
	   char* buffersize = (char *)calloc(numPages,sizeof(char)*PAGE_SIZE);
	   file_handle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));
	   queue = (Queue *)malloc(sizeof(Queue));


	   (*bm).pageFile = (char *)pageFileName;
	   (*bm).numPages = numPages;
	   (*bm).strategy = strategy;
	   (*bm).mgmtData = buffersize;
	   openPageFile((*bm).pageFile,file_handle);

	   createQueue(bm);

	   return RC_OK;

}

/**************************************************************************************
 * Function Name: shutdownBufferPool
 *
 * Description: 
 *		Shut down the BUffer Pool
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		
 *
 * Return:
 *		RC: Return Code
 *
 **************************************************************************************/
 
RC shutdownBufferPool(BM_BufferPool *const bm)
{
	int i = 0;
    	pageInfo *page_info = NULL;
    	page_info = (*queue).head;
	while(i < (*queue). filledframes)
	    {
		if((*page_info).fixCount == 0 && (*page_info).isDirty == 1)
		{
		    writeBlock((*page_info).pageNum,file_handle, (*page_info). bufferData);
		    write_IO++;
		    (*page_info).isDirty = 0;    
		}
		page_info = (*page_info).nextPageInfo;
		i++;      
	    }
	  closePageFile(file_handle);
	  return RC_OK;
}



/**************************************************************************************
 * Function Name: ForceFlushPool
 *
 * Description: 
 *		Causes the buffer pool dirty pages (with fix count 0) from
 *		to be written to disk
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		
 *
 * Return:
 *		RC: Return Code
 *
 **************************************************************************************/

RC forceFlushPool(BM_BufferPool *const bm)
{
	int i;
	pageInfo *temp_flush;
	temp_flush = (*queue).head;
	for(i=0; i< (*queue).totalNumOfFrames; i++)
	{
		if(((*temp_flush).isDirty==1) && ((*temp_flush).fixCount==0))
		{
			writeBlock((*temp_flush).pageNum,file_handle,(*temp_flush).bufferData);
			write_IO++;
			(*temp_flush).isDirty=0;
		}
		temp_flush=(*temp_flush).nextPageInfo;
	}
	return RC_OK;
}




//------------Buffer Manager Interface Access Page-----------------
/**************************************************************************************
 * Function Name: markDirty
 *
 * Description: 
 *		Mark the requested page as Dirty
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		BM_PageHandle *const page: Buffer Page Handler
 *
 * Return:
 *		RC: Return Code
 *
 **************************************************************************************/

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    pageInfo *page_info;
    int cnt=0;
    page_info = (*queue).head;
    if(cnt<(*bm).numPages){
    do{
 
    	if((*page_info).pageNum==(*page).pageNum){
            break;
            }
    	if((*page_info).nextPageInfo!=NULL){
        	page_info=(*page_info).nextPageInfo; 
        	}
        cnt++;
    } while(cnt<(*bm).numPages);
 	}
    if(cnt == (*bm).numPages)
    {
        return RC_READ_NON_EXISTING_PAGE;
    }
    (*page_info).isDirty=1;
    return RC_OK;
}

/**************************************************************************************
 * Function Name: unpinPage
 *
 * Description: 
 *		Unpins the page
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		BM_PageHandle *const page: Buffer Page Handler
 *
 * Return:
 *		RC: Return Code

 **************************************************************************************/

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{	
	pageInfo *info;
	int count=0;
	info = (*queue).head;
	if(count < (*bm).numPages){
	do{

		if((*info).pageNum==(*page).pageNum)
			break;
		info=(*info).nextPageInfo;
		count++;
	}while(count < (*bm).numPages);
	}
	if(count == (*bm).numPages)
		return RC_READ_NON_EXISTING_PAGE;		
	else
		(*info).fixCount=(*info).fixCount-1;
return RC_OK;
	
}

/**************************************************************************************
 * Function Name: forcePage
 *
 * Description: 
 *		Write the current content of the oage back to the
 *		page file on disk
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		BM_PageHandle *const page: Buffer Page Handler
 *
 * Return:
 *		RC: Return Code
 *
 **************************************************************************************/

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	pageInfo *info;
	int count=0;
	info = (*queue).head;
	if(count < (*bm).numPages){
	do{
		
		if((*info).pageNum==(*page).pageNum)
			break;
		info=(*info).nextPageInfo;
		count++;
	}while(count < (*bm).numPages);
	}
	
	int flag;

	if(count == (*bm).numPages)
		return 1;          //give error code
	if((flag=writeBlock((*info).pageNum,file_handle,(*info).bufferData))==0)
		write_IO++;
	else
		return RC_WRITE_FAILED;

	return RC_OK;
}

/**************************************************************************************
 * Function Name: pinPage
 *
 * Description: 
 *		Pin the page with requested page number in the BufferPool
 *		Load the page from file to BufferPool if page is not there
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		BM_PageHandle *const page: Buffer Page Handler
 *		PageNumber pageNum: the page number of the requested page
 *
 * Return:
 *		RC: Return Code
 *
 **************************************************************************************/


RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page,const PageNumber pageNum)
{
	int res;
	
	res = ((*bm).strategy == RS_FIFO) ? PAGE_FIFO(bm,page,pageNum) : PAGE_LRU(bm,page,pageNum);	
	return res;
}


/**************************************************************************************
 * Function Name: pinpageLRU
 *
 * Description: 
 *		Pin the page with requested page number in the BufferPool
 *		Load the page from file to BufferPool if page is not there
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		BM_PageHandle *const page: Buffer Page Handler
 *		PageNumber pageNum: the page numberof the requested page
 *
 * Return:
 *		RC: Return Code
 *
 **************************************************************************************/

//pins the page with page number with LRU replacement strategy
RC PAGE_LRU(BM_BufferPool * const bm, BM_PageHandle * const page,const PageNumber pageNum)
{

	int pageFound = 0;
	pageInfo *pinfo = (*queue).head;
	int i=0;

	while(i<(*bm).numPages)
	{
		if (pageFound == 0) 
		{
			if ((*pinfo).pageNum != pageNum)
			{
				pinfo = (*pinfo).nextPageInfo;
				
			}
			else
			{
				pageFound=1;
				break;
			}
				
		}
		i++;
	}
	if (pageFound == 0)
		Enqueue(page,pageNum,bm);

	//provide the client with the data and details of page and if pinned then increase the fix count
	int ch = 2;
	
	switch(ch){
		case 1:
			printf("not valid page");
			break;
		
		case 2:
		
			if (pageFound == 1)
					{
						(*pinfo).fixCount++;
						(*page).data = (*pinfo).bufferData;
						(*page).pageNum=pageNum;
						
						if (pinfo == (*queue).head)
						{
							(*pinfo).nextPageInfo = (*queue).head;
							(*queue).head->prevPageInfo = pinfo;
							(*queue).head = pinfo;
						}
						// To make the given node the head of the list
						if (pinfo != (*queue).head)
						{
							(*pinfo).prevPageInfo->nextPageInfo = (*pinfo).nextPageInfo;
							if ((*pinfo).nextPageInfo)
							{
								(*pinfo).nextPageInfo->prevPageInfo = (*pinfo).prevPageInfo;

								if (pinfo == (*queue).tail)
								{
									(*queue).tail = (*pinfo).prevPageInfo;
									(*queue).tail->nextPageInfo = NULL;
								}
								
								(*pinfo).nextPageInfo = (*queue).head;
								(*pinfo).prevPageInfo = NULL;
								(*pinfo).nextPageInfo->prevPageInfo = pinfo;
								(*queue).head = pinfo;
							}
						}
					}
		
			
				
			break;
						
		
		case 3:
			
			break;
	
	}

	
	return RC_OK;

}



/**************************************************************************************
 * Function Name: PAGE_FIFO
 *
 * Description: 
 *		Pin the page with requested page number in the BufferPool
 *		using FIFO replacement strategy
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *		BM_PageHandle *const page: Buffer Page Handler
 *		PageNumber pageNum: the page numberof the requested page
 *
 * Return:
 *		RC: Return Code
 *
 **************************************************************************************/



RC PAGE_FIFO(BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
    int pageFound = 0,numPages;
    RC returnvalue;
    numPages = (*bm).numPages;
    pageInfo *list=NULL,*temp=NULL,*temp_flush=NULL;
    list = (*queue).head;
    int j=0;
    while(j < numPages)
    {
    	if (pageFound == 0) 
	{
		if (list->pageNum != pageNum)
		{
			list = list->nextPageInfo;
			
		}
		else
		{
			pageFound = 1;
			break;
		}
	}
	j++;
	}
    if (pageFound == 1 )
    {
        list->fixCount++;
        (*page).data = list->bufferData;
        (*page).pageNum = pageNum;
        return RC_OK;
    }
    temp = (*queue).head;
    int returncode = -1;
    while ((*queue).filledframes < (*queue).totalNumOfFrames)
    {
        if((*temp).pageNum != -1)
        {
            temp = (*temp).nextPageInfo;
            
        }
        else
        {
            (*temp).fixCount = 1;
            (*temp).isDirty = 0;
            (*temp).pageNum = pageNum;
            (*page).pageNum= pageNum;     
            (*queue).filledframes = (*queue).filledframes + 1 ;
            readBlock((*temp).pageNum,file_handle,(*temp).bufferData);      
            (*page).data = (*temp).bufferData;
            read_IO++;
            returncode = 0;
            break;
        }
    }       
    if(returncode == 0)
        return RC_OK;
        pageInfo *addnode = (pageInfo *) malloc (sizeof(pageInfo));
        (*addnode).fixCount = 1;
        (*addnode).isDirty = 0;
        (*addnode).pageNum = pageNum;
        (*addnode).bufferData = NULL;
        (*addnode).nextPageInfo = NULL;
        (*page).pageNum= pageNum;
        (*addnode).prevPageInfo = (*queue).tail;
        temp = (*queue).head;
        int i=0;
        while(i<numPages)
        {
            if((*temp).fixCount != 0) 
            {
            	temp = (*temp).nextPageInfo;
            	
            }
            else
            {
            	break;            	
            }
            i++;
        }
        if(i==numPages)
        {
            return RC_NO_FREE_BUFFER_ERROR;
        }
        temp_flush=temp;
        if(temp == (*queue).head)
        {
            (*queue).head = (*queue).head->nextPageInfo;
            (*queue).head->prevPageInfo = NULL;
        }
        else if(temp == (*queue).tail)
        {
            (*queue).tail = (*temp).prevPageInfo;
            (*addnode).prevPageInfo=(*queue).tail;
        }
        else
        {
            (*temp).prevPageInfo->nextPageInfo = (*temp).nextPageInfo;
            (*temp).nextPageInfo->prevPageInfo=(*temp).prevPageInfo;
        }
        if(temp_flush->isDirty == 1)
        {
            writeBlock(temp_flush->pageNum,file_handle,temp_flush->bufferData);
            write_IO++;
        }
        (*addnode).bufferData = temp_flush->bufferData;
        (*addnode).frameNum = temp_flush->frameNum;    
        readBlock(pageNum,file_handle,(*addnode).bufferData);
        (*page).data = (*addnode).bufferData;
        read_IO++;   
        (*queue).tail->nextPageInfo = addnode;
        (*queue).tail=addnode;    
        return RC_OK;
}


//------------Statistics Interfaces-----------------

/**************************************************************************************
 * Function Name: getFrameContents
 *
 * Description: 
 *		Return an array of PageNumbers (of size numPages)
 *		where the ith element is the number of the page stored
 *		in the ith page frame
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		PageNumber *: an array of PageNumber of size numPages
 *
 **************************************************************************************/

PageNumber *getFrameContents (BM_BufferPool *const bm)
{	
	char buffer = 2;
	int cnt_1= 1;
	int cnt_max=20;

	PageNumber (*pages)[(*bm).numPages];
	int cnt=0;
	pages = calloc((*bm).numPages,sizeof(PageNumber));
	pageInfo *info;
	
	
	for(int f=0; f<1; f++){
		switch(buffer){
		case 1:
				while(cnt_max>5){
				for (int cnt = 0; cnt < (*bm).numPages; cnt++) 
				{
					cnt_max= 0;	
				}
				}
		case 2:	
				while(cnt < (*bm).numPages)
				{	
       				for(info=(*queue).head ; info!=NULL; info=(*info).nextPageInfo)
       			{
	           			if((*info).frameNum ==cnt)
	           		{
			       	(*pages)[cnt] = (*info).pageNum;
			   	break;
				}
			}
		cnt++;
		}
	return *pages;
}
}	
}
	

/**************************************************************************************
 * Function Name: getDirtyFlags
 *
 * Description: 
 *		Return an array of bools (of size numPages)
 *		Empty page frames are considered as clean
 *		
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		bool *: an array of bools indicating each page is dirty or not
 *
 **************************************************************************************/


bool *getDirtyFlags (BM_BufferPool *const bm)
{
	char buf = 2;
	int cnt_1= 1;
	int cnt_max=20;
	int cnt;
	bool *isDirty=(bool *)calloc((*bm).numPages,sizeof(PageNumber));
	pageInfo *info;
	
	
	for(int f=0; f<1; f++){
		switch(buf){
		case 1:
				while(cnt_max>5){
				for (int cnt = 0; cnt < (*bm).numPages; cnt++) 
				{
					(*isDirty)= 0;	
				}
				}
		case 2:	
				if(cnt_1=1){
				for(cnt=0; cnt< (*bm).numPages ;cnt++)
				{
					for(info=(*queue).head ; info!=NULL; info=(*info).nextPageInfo)
					{
						   if((*info).frameNum ==cnt)
						   {
						   	((*info).isDirty==1) ? isDirty[cnt]=TRUE : (isDirty[cnt]=FALSE);
						   	break; 
							
						}
					}
				}
			}
	return isDirty;
}
}
}




/**************************************************************************************
 * Function Name: getFixCounts
 *
 * Description: 
 *		Return an array of ints (of size numPages)
 *		where the fix count of page stored is the 
 *		nth page frame is the nth element
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		int*: an array of fix count of each page in the BufferPool
 *
 **************************************************************************************/
int *getFixCounts (BM_BufferPool *const bm)
{


	int pool = 2;
	int count_1= 1;
	int count_max=20;
	int (*fixCounts)[(*bm).numPages];
	fixCounts=calloc((*bm).numPages,sizeof(PageNumber));
	pageInfo *temp;

	for(int j=0;j<1;j++)
	{
		switch(pool){
			case 1:
				while(count_max>5){
				for (int cnt = 0; cnt < (*bm).numPages; cnt++) 
				{
					(*fixCounts)[cnt] = 0;	
				}
				}
			case 2: 
				if (count_1 = 1){
				
				for(int n=0; n< (*bm).numPages ;n++)
				{	
			       	for(temp=(*queue).head ; temp!=NULL; temp=(*temp).nextPageInfo)
			       	{
					   if((*temp).frameNum == n)
					   {
					       (*fixCounts)[n] = (*temp).fixCount;
						   break;
							}
						}
					}
				}
					
return *fixCounts;
}

}
}

/**************************************************************************************
 * Function Name: getNumReadIO
 *
 * Description:
 *		Since the Buffer Pool has been initialized, 
 *		Return the number of pages that have been read from disk
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		int: the number of pages that have been read from disk
 *
 **************************************************************************************/
 
int getNumReadIO (BM_BufferPool *const bm)
{
	return read_IO;
}

/**************************************************************************************
 * Function Name: getNumWriteIO
 *
 * Description:
 *		Since the Buffer Pool has been initialized, 
 *		Return the number of pages written to the page file
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		int: the number of pages written to the page file
 *
 **************************************************************************************/
 
int getNumWriteIO (BM_BufferPool *const bm)
{
	return write_IO;
}
