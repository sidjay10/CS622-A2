#include<stdio.h>
#include<map>
#include<vector>
#include<iostream>
#include<fstream>

using namespace std;
typedef unsigned long long ull;

map<ull,ull> currTime;
vector<ull> accessDist;
map<ull,ull> distCount;

/* 
 * The access distances are calculated from 
 * the previously generated traces, and the CDF
 * is computed from it, and stored in a csv
 * file for plotting.
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
	ull tid;
	ull addr;
	ull blkAddr;
	ull ctime=0;
	ull maxDist=0;

	//Access distances are calculated from the trace data below.
	while(!feof(fp))
	{
		ctime++;
		fread(&tid,sizeof(ull),1,fp);
		fread(&addr,sizeof(ull),1,fp);
		blkAddr=(addr>>6);
		if(currTime.find(blkAddr)==currTime.end())
			currTime[blkAddr]=ctime;
		else
		{
			ull dist=ctime-currTime[blkAddr];
			accessDist.push_back(dist);
			currTime[blkAddr]=ctime;
			if(dist>maxDist)
				maxDist=dist;
			if(distCount.find(dist)==distCount.end())
				distCount[dist]=1;
			else
				distCount[dist]++;
		}
	}


	/*
	 * The CDF is calculated below from the access distance counts.
	 * The cumulative probability and the corresponding access 
	 * distance is saved into a csv file, to be plotted by a separate
	 * plotting script.
	 */
	ull N=accessDist.size();
	ull cumDist=0;
	double cumProb=0.0;
	ofstream ofs;
	ofs.open("plotData.csv",ofstream::out);
	for(ull d=1;d<=maxDist;d++)
	{
		if(distCount.find(d)!=distCount.end())
		{
			cumDist=cumDist+distCount[d];
			cumProb=(double)(cumDist)/N;
			ofs<<d<<','<<cumProb<<endl;
		}
	}
	ofs.close();
	fclose(fp);
	return 0;
}