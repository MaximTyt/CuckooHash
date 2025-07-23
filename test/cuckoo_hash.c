#include <rte_hash.h>
#include <rte_jhash.h>
#include <rte_random.h>
#include <rte_eal.h>
#include <rte_errno.h>
#include <rte_debug.h>
#include <inttypes.h>

#define MAX_ENTRIES 1000000
#define NUM_ENTRIES 16000000
#define REPORT_ITER 1000000
#define SPECIAL_KEY 42
#define SPECIAL_ITER 123456

int main(int argc, char **argv)
{  
	int ret, index;
	uint32_t key, data;
	uint32_t special_data, special_key = SPECIAL_KEY;	
	uint64_t elapsed_ns, lookuptime;
	struct timespec start_time, start_lookuptime, current_time, end_lookuptime;
	struct rte_hash_parameters params;
	struct rte_hash *cuckoo_hash;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");
	
	params.name = "cuckoo_hash";
	params.entries = MAX_ENTRIES;
	params.key_len = sizeof(uint32_t);
	params.hash_func = rte_jhash;
	params.hash_func_init_val = 0;
	params.socket_id = rte_socket_id();
    
	cuckoo_hash = rte_hash_create(&params);
	if (cuckoo_hash == NULL)
		rte_panic("Failed to create cuckoo hash, errno = %d\n", rte_errno);
  	
    clock_gettime(CLOCK_MONOTONIC, &start_time);
  
	for (uint32_t i = 1; i <= NUM_ENTRIES; ++i)
		{
			key = (i == SPECIAL_ITER) ? special_key : rte_rand();
			data = (i == SPECIAL_ITER) ? SPECIAL_ITER : rte_rand();
			rte_hash_add_key_data(cuckoo_hash, &key, &data);
			if (i % REPORT_ITER == 0)
			{				
				clock_gettime(CLOCK_MONOTONIC, &current_time);
				elapsed_ns = (current_time.tv_sec - start_time.tv_sec) * 1.0e9 + (current_time.tv_nsec - start_time.tv_nsec);
				printf("%u k / %u k, time: %"PRIu64" ns\n", i/1000, NUM_ENTRIES/1000, elapsed_ns);
			}				
		}
	 	
	clock_gettime(CLOCK_MONOTONIC, &start_lookuptime);
	
	index = rte_hash_lookup_data(cuckoo_hash, &special_key, (void**)&special_data);
		
    clock_gettime(CLOCK_MONOTONIC, &end_lookuptime);
	if (index >= 0)
	{
		lookuptime = (start_lookuptime.tv_sec - end_lookuptime.tv_sec) * 1.0e9 + (start_lookuptime.tv_nsec - end_lookuptime.tv_nsec);
		printf("Hash size: %u, key: %u, data: %u, lookuptime: %"PRIu64" ns\n", rte_hash_count(cuckoo_hash), SPECIAL_KEY, special_data, lookuptime); 
	}
  
	rte_hash_free(cuckoo_hash);
	rte_eal_cleanup();
	return 0;
}
