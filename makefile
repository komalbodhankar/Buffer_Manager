.PHONY: all
all: test_assign2_1 test_assign2_2

test_assign2_1: test_assign2_1.c storage_mgr.c buffer_mgr.c buffer_mgr_stat.c dberror.c 
	gcc -o test_assign2_1 test_assign2_1.c storage_mgr.c buffer_mgr.c buffer_mgr_stat.c dberror.c

test_assign2_2: test_assign2_2.c storage_mgr.c buffer_mgr.c buffer_mgr_stat.c dberror.c 
	gcc -o test_assign2_2 test_assign2_2.c storage_mgr.c buffer_mgr.c buffer_mgr_stat.c dberror.c


.PHONY: clean
clean2_1:
	rm test_assign2_1
clean2_2:
	rm test_assign2_2	
