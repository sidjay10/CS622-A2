#include<stdio.h>
#include<map>
#include<vector>
#include<iostream>
#include<fstream>
#include<algorithm>
#include<bitset>

using namespace std;
typedef unsigned long long ull;
typedef struct node
{
	ull tid;
	ull blkAddr;
	struct node* next;
}block;

map<int,ull> shareCount;
map<ull,bitset<8>> blockShare;

/*
 * The sharing profile is computed from the previously
 * generated traces. The blocks are read in one-by-one,
 * and the distinct Thread IDs that access that block are stored.
 * Then, for each Thread ID, the number of blocks that have been
 * used by it are calculated in order to compute the sharing profile.
 */
int main(int argc,char** argv)
{
	FILE* fp;
	if(argc!=2)
	{
		printf("Supply trace name!");
		return -1;
	}
	fp=fopen(argv[1],"rb");
	block* head=(block*)malloc(sizeof(block));
	ull tid;
	ull addr;
	ull blkAddr;
	block* it=head;

	//A linked-list of machine accesses(blocks) is created.
	while(!feof(fp))
	{
		fread(&tid,sizeof(ull),1,fp);
		fread(&addr,sizeof(ull),1,fp);
		blkAddr=(addr>>6);
		it->next=(block*)malloc(sizeof(block));
		it=it->next;
		it->tid=tid;
		it->blkAddr=blkAddr;
		it->next=NULL;
		blockShare[blkAddr]=0;
	}
	it=head->next;
	ull ort=1;

	/*
	 * A bitset of 8 bits is associated with every unique block address.
	 * Every bit of the bitset corresponds to a thread id, which is set
	 * in the following loop for a particular block when that block is
	 * used by a thread. The number of set bits in a particular block
	 * denotes the number of threads sharing the block.
	 */
	while(it!=NULL)
	{
		ull idx=it->blkAddr;
		blockShare[idx][it->tid]=1;
		it=it->next;
	}	

	//The set bits are counted, and the sharing profile is computed below.
	for(int i=1;i<=8;i++)
		shareCount[i]=0;
	for(map<ull,bitset<8>>::iterator it=blockShare.begin();it!=blockShare.end();it++)
	{
		int shrIdx=(it->second).count();
		shareCount[shrIdx]++;
	}
	cout<<"--------------Sharing Profile-------------------------"<<endl;
	cout<<"Private Blocks : \t\t"<<shareCount[1]<<endl;
	for(int i=2;i<=8;i++)
		cout<<"Shared by "<<i<<" Threads : \t\t"<<shareCount[i]<<endl;
	return 0;
}
