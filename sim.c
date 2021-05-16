/*
 *  Author: Bryce Souers
 *	File: sim.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define COST_PER_KB 0.09

// Internal enum used to make code a bit more readable
enum {
	 RR = 1,
	RND = 2,
	LRU = 3
};

// Structure that contains the parsed arguments as well as the calculated cache values
typedef struct _args {
	// Input values parsed from the command line arguments
	char trace_file[64];
	int  cache_size;
	int  block_size;
	int  associativity;
	int  replacement_policy;

	// Cache values calculated using calculate_cache();
	int num_blocks;
	int tag_size;
	int index_size;
	int offset_size;
	int num_rows;
	int overhead_size;
	int mem_size_bytes;
	float mem_size_kb;
	float cost;

	// Cache simulation results
	int total_cache_accesses;
	int total_addresses;
	int cache_hits;
	int cache_misses;
	int compulsory_misses;
	int conflict_misses;

	// Cache hit & miss rate results
	double hit_rate;
	double miss_rate;
	double cpi;
	unsigned int cpi_cycles;
	unsigned int num_instructions;
	double unused_cache_space;
	double unused_cache_percentage;
	double waste;
	int unused_cache_blocks;
	int cpu_cycle;
} _args;

// Structure for cache block
typedef struct block {
	int valid;
	char* tag;
	int timestamp;
	int rr;
} block;

// Forward declarations for functions
void parse_args(_args* args, int argc, char* argv[]);
void argument_error();
char* pretty_replacement_policy();
void calculate_cache(_args* args);
void trace_cache(_args* args);

int Log2(int n) {
	return log(n)/log(2);  
}

int main(int argc, char *argv[]) {
	// Seed rand()
	srand(time(0));

	// Allocate memory for the struct
	_args* args = (_args* ) malloc(sizeof(_args));
	if(args == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate cache arguments!\n");
		exit(EXIT_FAILURE);
	}

	// Delegate argument parsing to seperate function which writes values to the struct
	parse_args(args, argc, argv);

	// Delegate cache calculations to seperate function which also writes values to the struct
	calculate_cache(args);

	// Print out the output as per the PDF
	printf("Cache Simulator\n\n");
	printf("Trace File: %s\n\n", args->trace_file);
	printf("***** Cache Input Parameters *****\n");
	printf("%-32s%d KB\n", "Cache Size:", args->cache_size);
	printf("%-32s%d bytes\n", "Block Size:", args->block_size);
	printf("%-32s%d\n", "Associativity:", args->associativity);
	printf("%-32s%s\n\n", "Replacement Policy:", pretty_replacement_policy(args->replacement_policy));
	printf("***** Cache Calculated Values *****\n\n");
	printf("%-32s%d\n", "Total # Blocks:", args->num_blocks);
	printf("%-32s%d bits\n", "Tag Size:", args->tag_size);
	printf("%-32s%d bits\n", "Index Size:", args->index_size);
	printf("%-32s%d\n", "Total # Rows:", args->num_rows);
	printf("%-32s%d bytes\n", "Overhead Size:", args->overhead_size);
	printf("%-32s%.2lf KB (%d bytes)\n", "Implementation Memory Size:", args->mem_size_kb, args->mem_size_bytes);
	printf("%-32s$%.2lf\n\n", "Cost:", args->cost);

	// Delegate the parsing of the trace file to seperate function
	trace_cache(args);

	printf("***** CACHE SIMULATION RESULTS *****\n\n");
	printf("%-24s%-7d(%d addresses)\n", "Total Cache Accesses:", args->total_cache_accesses, args->total_addresses);
	printf("%-24s%d\n", "Cache Hits:", args->cache_hits);
	printf("%-24s%d\n", "Cache Misses:", args->cache_misses);
	printf("%-27s%d\n", "--- Compulsory Misses:", args->compulsory_misses);
	printf("%-27s%d\n\n\n", "--- Conflict Misses:", args->conflict_misses);
	printf("***** ***** CACHE HIT & MISS RATE: ***** *****\n\n");
	printf("%-5s%-18s%.4lf%%\n", "Hit", "Rate:", args->hit_rate);
	printf("%-5s%-18s%.4lf%%\n", "Miss", "Rate:", args->miss_rate);
	printf("%-23s%.2lf Cycles/Instruction  (%d)\n", "CPI:", args->cpi, args->num_instructions);
	printf("%-23s%.2lf KB / %.2lf KB = %.2lf%%  Waste: $%.2lf\n", "Unused Cache Space:", args->unused_cache_space, args->mem_size_kb, args->unused_cache_percentage, args->waste);
	printf("%-23s%d / %d\n\n", "Unused Cache Blocks:", args->unused_cache_blocks, args->num_blocks);

	// Append output to csv file
	FILE *fp;
	char filename[50] = "Trace_Results.csv";
	fp = fopen(filename, "a+");
	fprintf(fp, "%s,%d,%d,%d,%s,%d,%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
		args->trace_file,
		args->cache_size,
		args->block_size,
		args->associativity,
		pretty_replacement_policy(args->replacement_policy),
		args->num_blocks,
		args->num_rows,
		args->overhead_size,
		args->mem_size_kb,
		args->cost,
		args->hit_rate,
		args->miss_rate,
		args->cpi,
		args->unused_cache_space,
		args->unused_cache_percentage,
		args->waste
	);
	fclose(fp);

	// Clean up :)
	free(args);
	return 0;
}

void hex2bin(char* hex, char* bin) {
	int page = 0;
	int i = 0;
	for(i = 0; i < 8; i++) {
		switch(hex[i]) {
			case '0': memcpy(bin + (page * 4), "0000", 4); break;
			case '1': memcpy(bin + (page * 4), "0001", 4); break;
			case '2': memcpy(bin + (page * 4), "0010", 4); break;
			case '3': memcpy(bin + (page * 4), "0011", 4); break;
			case '4': memcpy(bin + (page * 4), "0100", 4); break;
			case '5': memcpy(bin + (page * 4), "0101", 4); break;
			case '6': memcpy(bin + (page * 4), "0110", 4); break;
			case '7': memcpy(bin + (page * 4), "0111", 4); break;
			case '8': memcpy(bin + (page * 4), "1000", 4); break;
			case '9': memcpy(bin + (page * 4), "1001", 4); break;
			case 'a': memcpy(bin + (page * 4), "1010", 4); break;
			case 'b': memcpy(bin + (page * 4), "1011", 4); break;
			case 'c': memcpy(bin + (page * 4), "1100", 4); break;
			case 'd': memcpy(bin + (page * 4), "1101", 4); break;
			case 'e': memcpy(bin + (page * 4), "1110", 4); break;
			case 'f': memcpy(bin + (page * 4), "1111", 4); break;
			default: {
				fprintf(stderr, "[ERROR] Unknown hex value found!\n");
				exit(EXIT_FAILURE);
			}
		}
		page++;
	}
	bin[32] = '\0';
}

// Function to handle each address to process
void handle_address(_args* args, block* cache, char* address, int bytes_read, int is_eip) {
	args->cpu_cycle++;

	char bin[33];
	hex2bin(address, bin);
	char offset[args->offset_size + 1];
	char index[args->index_size + 1];
	char tag[args->tag_size + 1];
	int i;

	// Split out address
	int tag_i = 0;
	for(i = 0; i < (args->tag_size); i++) {
		tag[tag_i] = bin[i];
		tag_i++;
	}
	tag[tag_i] = '\0';
	int index_i = 0;
	for(i = (args->tag_size); i < (args->tag_size + args->index_size); i++) {
		index[index_i] = bin[i];
		index_i++;
	}
	index[index_i] = '\0';
	int offset_i = 0;
	for(i = (args->tag_size + args->index_size); i < 32; i++) {
		offset[offset_i] = bin[i];
		offset_i++;
	}
	offset[offset_i] = '\0';

	// Convert to decimal
	int exp_index;
	int tag_value = 0;
	exp_index = 0;
	for(i = (tag_i - 1); i >= 0; i--) {
		if(tag[i] == '1') tag_value += pow(2, exp_index);
		exp_index++;
	}
	int index_value = 0;
	exp_index = 0;
	for(i = (index_i - 1); i >= 0; i--) {
		if(index[i] == '1') index_value += pow(2, exp_index);
		exp_index++;
	}
	int offset_value = 0;
	exp_index = 0;
	for(i = (offset_i - 1); i >= 0; i--) {
		if(offset[i] == '1') offset_value += pow(2, exp_index);
		exp_index++;
	}

	int num_overruns = ceil((offset_value + bytes_read) / ((float) args->block_size));

	int j;
	int z;
	int q;
	// Loop over each overrun required
	for(i = 0; i < num_overruns; i++) {
		args->total_cache_accesses++;
		int idx = (index_value + i);
		int row_start_index = (idx * args->associativity) % args->num_blocks;
		
		// Loop over each block in current row
		int found_tag = 0;
		for(j = 0; j < args->associativity; j++) {
			int row_block_index = row_start_index + j;
			if(strcmp(cache[row_block_index].tag, tag) == 0 && cache[row_block_index].valid == 1) {
				args->cache_hits++;
				args->cpi_cycles++;
				found_tag = 1;
				break;
			}
		}

		// If we don't find a row with a matching tag, replace one
		if(found_tag == 0) {
			args->cpi_cycles += (4 * (args->block_size / (32/8)));
			// Look for an empty block in the row
			int found_empty = 0;
			for(j = 0; j < args->associativity; j++) {
				int row_block_index = row_start_index + j;
				if(cache[row_block_index].valid == 0) {
					found_empty = 1;
					args->cache_misses++;
					args->compulsory_misses++;
					cache[row_block_index].valid = 1;
					cache[row_block_index].timestamp = args->cpu_cycle;
					strcpy(cache[row_block_index].tag, tag);
					break;
				}
			}
			if(found_empty == 0) {
				args->cache_misses++;
				args->conflict_misses++;
				int replacement_offset = 0;
				int max_ts = -1;
				int max_ts_i = -1;
				int min_ts = -1;
				int min_ts_i = -1;
				for(j = 0; j < args->associativity; j++) {
					int row_block_index = row_start_index + j;
					if(cache[row_block_index].timestamp > max_ts || max_ts == -1) {
						max_ts = cache[row_block_index].timestamp;
						max_ts_i = j;
					}
					if(cache[row_block_index].timestamp < min_ts || min_ts == -1) {
						min_ts = cache[row_block_index].timestamp;
						min_ts_i = j;
					}
				}
				switch(args->replacement_policy) {
					case RR:
						replacement_offset = cache[row_start_index].rr;
						break;
					case RND:
						replacement_offset = rand() % args->associativity;
						break;
					case LRU:
						fprintf(stderr, "[ERROR] >> LRU not implemented, try RR or RND instead!\n");
						exit(EXIT_FAILURE);
						break;
					default:
						fprintf(stderr, "[ERROR] >> Invalid replacement policy!\n");
						exit(EXIT_FAILURE);
						break;
				}
				int replacement_index = row_start_index + replacement_offset;
				cache[replacement_index].valid = 1;
				cache[replacement_index].timestamp = args->cpu_cycle;
				strcpy(cache[replacement_index].tag, tag);
				int new_rr = (cache[row_start_index].rr + 1) % args->associativity;
				for(q = 0; q < args->associativity; q++) {
					int row_block_index = row_start_index + q;
					cache[row_block_index].rr = new_rr;
					//printf("set row_block_index.rr = %d\n", new_rr);
				}
			}
		}
	}
	if(is_eip) {
		args->cpi_cycles += 2;
		args->num_instructions++;
	}
	else args->cpi_cycles++;
}

/*	void trace_cache(_args* args)
 *		_args* args: pass struct by reference so we can change the values
 *		returns: void
 *		This function parses the trace file specified in the struct
 */
void trace_cache(_args* args) {
	// Try to open the file, gracefully error out on fail
	FILE* trace_file = fopen(args->trace_file, "r");
	if(trace_file == NULL) {
		fprintf(stderr, "[ERROR] Unable to open trace file!\n");
		exit(1);
	}

	// Initialize cache for simulation
	block* cache = (block* ) malloc(args->num_blocks * sizeof(block));
	int i;
	for(i = 0; i < args->num_blocks; i++) {
		cache[i].valid = 0;
		cache[i].rr = 0;
		cache[i].tag = (char*) malloc(sizeof(char) * (args->tag_size + 1));
		if(cache[i].tag == NULL) {
			fprintf(stderr, "ERROR: Unable to cache tag!\n");
			exit(EXIT_FAILURE);
		}
		int j;
		for(j = 0; j < args->tag_size; j++) cache[i].tag[j] = '0';
	}

	// Buffer used to store each read line
	char buffer[256];
	int num = 0;
	// Loop through each line of the trace file and parse
	while(fgets(buffer, 256, trace_file)) {
		// Mostly empty line, pass on it
		if(strlen(buffer) <= 5) continue;

		// Check if this is an EIP line or not
		if(strncmp(buffer, "EIP", 3) == 0) {
			// Parse out the number of bytes accessed
			char bytes_read_string[3] = {buffer[5], buffer[6], '\0'};
			int bytes_read = atoi(bytes_read_string);
			// Parse out the one address
			char address[9];
			for(i = 0; i < 8; i++) address[i] = buffer[i + 10];
			address[8] = '\0';

			handle_address(args, cache, address, bytes_read, 1);
			args->total_addresses++;
		} else {
			// Parse out the dstM address
			char dst_address[9];
			for(i = 0; i < 8; i++) dst_address[i] = buffer[i + 6];
			dst_address[8] = '\0';
			// Check if we need to ignore it
			if(strcmp(dst_address, "00000000") != 0) {
				handle_address(args, cache, dst_address, 4, 0);
				args->total_addresses++;
			}

			// Parse out the srcM address
			char src_address[9];
			int i;
			for(i = 0; i < 8; i++) src_address[i] = buffer[i + 33];
			src_address[8] = '\0';
			// Check if we need to ignore it
			if(strcmp(src_address, "00000000") != 0) {
				handle_address(args, cache, src_address, 4, 0);
				args->total_addresses++;
			}
		}
	}
	// Free and clean up
	for(i = 0; i < args->num_blocks; i++) {
		free(cache[i].tag);
	}
	fclose(trace_file);

	args->hit_rate = args->cache_hits / (float)args->total_cache_accesses * 100;
	args->miss_rate = 100 - args->hit_rate;
	int num_blocks_wasted = 0;
	for(i = 0; i < args->num_blocks; i++) {
		if(cache[i].valid == 0) num_blocks_wasted++;
	}
	args->unused_cache_blocks = num_blocks_wasted;
	args->unused_cache_space = num_blocks_wasted * (args->block_size + ((args->tag_size + 1) / (float)8)) / (float)1024;
	args->unused_cache_percentage = args->unused_cache_space / (float)args->mem_size_kb * 100;
	args->waste = args->unused_cache_space * (float)COST_PER_KB;
	args->cpi = args->cpi_cycles / (float)args->num_instructions;
}

/*	void calculate_cache(_args* args)
 *		_args* args: pass struct by reference so we can change the values
 *		returns: void
 *		This function does all the calculations for the cache values
 */
void calculate_cache(_args* args) {
	args->num_blocks = (args->cache_size * 1024) / args->block_size;
	args->num_rows = args->num_blocks / args->associativity;
	args->index_size = Log2(args->num_rows);
	args->offset_size = Log2(args->block_size);
	args->tag_size = 32 - args->index_size - args->offset_size;
	args->overhead_size = (args->num_blocks * (args->tag_size + 1)) / 8;
	args->mem_size_bytes = (args->cache_size * 1024) + (args->overhead_size);
	args->mem_size_kb = args->mem_size_bytes / 1024.0;
	args->cost = args->mem_size_kb * COST_PER_KB;
	args->cpu_cycle = 0;
}

/*	void pretty_replacement_policy(int p)
 *		int p: Integer that corresponds to the enum declared above
 *		returns: char* that represents the "pretty" name of the 
 *				 shorthanded replacement policy value
 *		This function returns the pretty string version of the user-input
 */
char* pretty_replacement_policy(int p) {
	if(p == RR ) return "Round Robin";
	if(p == RND) return "Random";
	if(p == LRU) return "Least Recently Used";
	return NULL;
}

/*	void parse_args(_args* args, int argc, char* argv[])
 *		_args* args: struct passed by reference
 *		int argc: pass number of argument
 *		char* argv[]: pass arguments
 *		returns: void
 *		This function parses the arguments into a pretty struct :)
 */
void parse_args(_args* args, int argc, char* argv[]) {
	// Keep track of which flag we hit
	int num_filled = 0;
	int hit_f = 0, hit_s = 0, hit_b = 0, hit_a = 0, hit_r = 0, hit_any = 0;
	// Loop through all arguments (except first)
	int i;
	for(i = 1; i < argc; i++) {
		char* arg = argv[i];
		// Trigger flag bools
		if(strcmp(arg, "-f") == 0) {
			if(hit_any) argument_error();
			hit_f = 1, hit_any = 1;
			continue;
		} else if(strcmp(arg, "-s") == 0) {
			if(hit_any) argument_error();
			hit_s = 1, hit_any = 1;
			continue;
		} else if(strcmp(arg, "-b") == 0) {
			if(hit_any) argument_error();
			hit_b = 1, hit_any = 1;
			continue;
		} else if(strcmp(arg, "-a") == 0) {
			if(hit_any) argument_error();
			hit_a = 1, hit_any = 1;
			continue;
		} else if(strcmp(arg, "-r") == 0) {
			if(hit_any) argument_error();
			hit_r = 1, hit_any = 1;
			continue;
		}
		// Assign value to triggered flag
		if(hit_any == 1) {
			num_filled++;
			if(hit_f) {
				strcpy(args->trace_file, arg);
			} else if(hit_s) {
				int n = atoi(arg);
				if(n < 1 || n > 8192) argument_error();
				args->cache_size = n;
			} else if(hit_b) {
				int n = atoi(arg);
				if(n < 4 || n > 64) argument_error();
				args->block_size = n;
			} else if(hit_a) {
				int n = atoi(arg);
				if(n != 1 && n != 2 && n != 4 && n != 8 && n != 16) argument_error();
				args->associativity = n;
			} else if(hit_r) {
				if(strcmp(arg, "RR") == 0) args->replacement_policy = RR;
				else if(strcmp(arg, "RND") == 0) args->replacement_policy = RND;
				else if(strcmp(arg, "LRU") == 0) args->replacement_policy = LRU;
				else argument_error();
			} else argument_error();
			hit_f = 0, hit_s = 0, hit_b = 0, hit_a = 0, hit_r = 0, hit_any = 0;
		} else argument_error();
	}
	if(num_filled != 5) argument_error();
}

/*	argument_error()
 *		returns: void
 *		This function is a shorthand to the formal usage error message
 */
void argument_error() {
	fprintf(stderr, "[ERROR] Invalid arguments. Usage: ./sim2.c -f <trace file name> -s <cache size in KB>[1 KB to 8 MB] -b <block size>[4 to 64 bytes] -a <associativity>[1,2,4,8,16] -r <replacement policy>[RR,RND,LRU]\n");
	exit(1);
}
