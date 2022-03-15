# *********** SPRING 2022 - 2ST ASSIGNMENT - BUFFER MANAGER *********** #

#  GROUP MEMBER INFO #


  >  1. KOMAL BODHANKAR - A20492705 - kbodhankar@hawk.iit.edu
  >  2. MANAN SAGAR - A20496941 - msagar@hawk.iit.edu
  > 3. RAVISHANKAR SIVAGNANAM - A20503091 - rsivagnanam@hawk.iit.edu

# ********************************************************************* #

# Files in the buffer manager #

###  1. C files:
 > - test_assign2_1.c
 > - dberror.c
 > - buffer_mgr_stat.c
 > - storage_mgr.c
 > - buffer_mgr.c
###  2. Header files
 > - test_helper.h
 > - dt.h
 > - buffer_mgr.h
 > - storage_mgr.h
 > - dberror.h
 > - buffer_mgr_stat.h
###   3. Makefile
###   4. README

# ********************************************************************* #

# STEPS TO START THE SCRIPT #

0. Start the Linux system
1. Open terminal and go to the project root
2. We can run the `ls` command to view the files to ensure if we are in the right directory
3. Then we run the command `make clean` to clean the preexisting output files
4. To execute the file, we then run the `make` or the `makeall` command.

After we execute the 4th step, a new file named `test_assign2_1.c` and `test_assign2_2` will be generated.

5. Run the `./test_assign2_1` and `test_assign2_2` command to run the newly generated file. 


# ********************************************************************* #

# FUNCTION DESCRIPTIONS: #

### 1.  initBufferPool():
- The buffer pool will be set up by this method which will be of the size numPages and initially all the pagesframes will be empty. 

### 2. shutdownBufferPool():
- This function will shut down the buffer pool and all the pages will be removed from the memory. The resources used will also become free. 

### 3. forceFlushPool():
- The pool will be flushed with all the dirty pages written from page frame to page file.

### 4. markDirty():
- This function will mark the page requested as dirty. It will also change the value of the variable assigned. 

### 5. unpinPage():
- This method will unpin the page

### 6. forcepage():
- The current contents will be written from page back to the page file on the disk.

### 7. pinpage():
- This will pin the page with the page number. 
- FIFO - The page will be pinned with the page number in the pool using the  First in first out stratergy. 
- LRU - the page will be pinned with the page number in the pool and load from bufferpool if it is not there. 
- CLOCK - It will check if the page still exists or not in the buffer pool. 

### 8. getFrameContents():
- This method will return an array of page numbers. The values in the array is the number of page in the ith page. The empty page frame will be denoted by a contant like NO PAGE

### 9. getDirtyFlags():
-  THis is a boolean function and return an array of boolean valiues of the size of number of pages. If the value in the array is empty, it is clean and if it is true, it is considered as dirty. 

### 10. getFixCounts():
- This is a function which will return an array of the type integer and will be of size of number of pages. The values in the array are the count of page stored in that page frame. For empty frames, 0 will be returned. 

### 11. getNumReadIO(): 
- This will return the total number of pages that have already been read since the initialization of the buffer pool. 

### 12. getNumWriteIO():
- This will return the total number of pages that have already been written to the page file since the initialization of the buffer pool. 

# ********************************************************************* #

*An error message will be provided whenever we encounter an error which will be envoked from the dberror.c file*
