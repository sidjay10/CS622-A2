#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#define L2R 2048
#define L2A 16
#define L2B 64

typedef unsigned long long ull;

typedef struct Cache_Block{
    char valid;
    unsigned long long tag;
    unsigned long long time;
    
}cache;

void Cache_Filter(char ** name);
void free_cache(cache** C, int B, int R, int A);
int Hit_Miss(cache **C,int R, int A, int B, ull Addr, ull Time);
ull Miss_Handler(cache ** C,int B,int R,int A,ull Addr, ull Time);
int Invalidate(cache ** C,int B,int R,int A,ull Addr);
cache ** init(int B,int R,int A);
int LRU_Block(cache * set, int A);
int Cache(cache **,int S, int A, int B, ull Addr, ull Time);

int main(int argc, char ** argv)
{
    Cache_Filter(argv);
    return 0;
}

/*	Function to simulate Cache Filter*/
void Cache_Filter(char** argv)
{
	cache **L2;
	L2 = init(L2B,L2R,L2A);

	unsigned long long Addr, tid;
	
	ull Time = 0;
	ull L2_Miss = 0;
	ull L2_Hit = 0;

	FILE* fp;
	fp = fopen(argv[1], "rb");
    
    char output_name[100];
    sprintf(output_name, "%s_filter", argv[1]);
    FILE* fp1;
    fp1 = fopen(output_name,"wb");
	assert(fp1 != NULL);


	while (!feof(fp)) {
        unsigned long long tid;
        fread(&tid, sizeof(unsigned long long), 1, fp);
		fread(&Addr, sizeof(unsigned long long), 1, fp);
		Time++;
		if(Hit_Miss(L2,L2R,L2A,L2B,Addr,Time))
		{
			L2_Hit++;
		}
		else
		{
			L2_Miss++;
            fwrite(&tid, sizeof(unsigned long long), 1, fp1);
            fwrite(&Addr, sizeof(unsigned long long), 1, fp1);
            Miss_Handler(L2,L2B,L2R,L2A,Addr,Time);
		}
	}
	fclose(fp);
    fclose(fp1);

    printf("Section     \tCount     \tPKA\n");
	printf("Accesses \t%-10llu\t%-10.3f\n",Time,1000.0);
	printf("Hit      \t%-10llu\t%-10.3f\n",L2_Hit,1000.0*L2_Hit/Time);
	printf("Miss     \t%-10llu\t%-10.3f\n",L2_Miss,1000.0*L2_Miss/Time);
	
	free_cache(L2,L2B,L2R,L2A);
}

//function to free memory occupied by caches
void free_cache(cache** C, int B, int R, int A)
{
	for(int i=0;i<R;i++)
	{
		free(C[i]);
	}
	free(C);
}

//function to initialise caches
cache ** init(int B,int R,int A)
{
	cache ** C;
	C = (cache **)malloc(R*sizeof(cache *));
	for(int i=0;i<R;i++)
	{
		C[i] = (cache *)malloc(A*sizeof(cache ));
		memset(C[i],0,A*sizeof(cache));
	}
	return C;
}

/*	function to invalidate the cache block corresponding
	to an address (word address) in a cache
	it returns 1 if the block was valid and is invalidated;
	returns 0 if the block was not present or already invalid */
int Invalidate(cache ** C,int B,int R,int A,ull Addr)
{
	int Idx = (Addr / (B))%(R);
	ull Tag = (Addr / (B*R));

	for(int i=0;i<A;i++)
	{
		if(C[Idx][i].valid && C[Idx][i].tag == Tag)
		{
			C[Idx][i].valid = 0;
			return 1;
		}
	}	
	return 0;
}

/*	Function to handle a miss. It fills up the cache block
	with the block corresponding to the address and if it evicts a valid
	block, returns its address; otherwise returns -1 */ 
ull Miss_Handler(cache ** C,int B,int R,int A,ull Addr, ull Time)
{
	int Idx = (Addr / (B))%(R);
	ull Tag = (Addr / (B*R));
	ull Add_Evi;

	int R_Way = LRU_Block(C[Idx],A);
	ull Tag_Evi;
	if(C[Idx][R_Way].valid)
	{
		Tag_Evi = C[Idx][R_Way].tag;
		Add_Evi = (Tag_Evi*R + Idx)*B;
	}
	else
	{
		Add_Evi = -1;
	}

	C[Idx][R_Way].tag = Tag;
	C[Idx][R_Way].valid = 1;
	C[Idx][R_Way].time = Time;
	
	return Add_Evi;
}

/*	Function to check whether a given block hits in a cache or not
	updates time and returns 1 if it is a hit
	otherwise returns 0 */
int Hit_Miss(cache **C,int R, int A, int B, ull Addr, ull Time)
{
	int Idx = (Addr / (B))%(R);	//Block Index
	ull Tag = (Addr / (B*R));	//Block Tag

	for(int i=0;i<A;i++)
	{
		if(C[Idx][i].valid && C[Idx][i].tag == Tag)
		{
			C[Idx][i].time = Time;
			return 1;
		}
	}	
	return 0;
}

/*	Returns the way number of the LRU block in a given set 
	Returns the way number of an invalid block in the set, if any */
int LRU_Block(cache * Set, int A){
	int min = 0;
	for(int i=0;i<A;i++){
		if(Set[i].valid == 0)
		{
			min = i;
			break;
		}
		else if(Set[i].time < Set[min].time)
		{
			min = i;
		}
	}
	return min;
}
