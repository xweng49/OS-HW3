#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>
#include <sstream>
#include <math.h>
#include <list>
#include <algorithm>
using namespace std;




int totalpageframe;
int maxsegment;
int pagesize;
int pframeprocess;
int totalprocess;
int lookahead;
int minlookahead;
int maxlookahead;



typedef struct frame
{
	bool allocated;
	int processid;
	int pageid;
	int segmentid;
	int frameid;
	frame()
	{
    allocated = false;
	}
}Frame;




vector<frame>mainframe;
vector<frame>diskframe;
list<string>instruction;
vector<int>order;		//vector used for algorithm that needs to use specific orders



typedef struct page
{
	bool allocated;
	int frameid;
	int pageid;
	page(int id)
	{
		pageid = id;
		allocated = false;
	}
}page;

typedef struct pagetable
{
  vector <page> pages;
}pagetable;

typedef struct segment
{
  pagetable pageTable;
}segment;

typedef struct segmenttable
{
  vector <segment> segments;
}segmenttable;


typedef struct process
{
  int processid;
  int processframe;
  segmenttable segmentTable;
}process;
process *Process;



void ReadFile(string filename);
void buildmainframe();
void builddisk();
void FIFO();
void LIFO();
void LRU();
void LDF();
void OPT();


int main(int argc,char* argv[])
{
	if (!argv[1])
	{
  cout<<">>>>>>>>>>>>Missing argv[1] input for input file<<<<<<<<<<<<<<"<<endl<<endl<<endl;
	}

	ReadFile(argv[1]);

  cout <<"----------------------Initialization-------------------------"<< endl;
  cout <<"Physical Memory: \t\t\t" << totalpageframe << endl;
  cout <<"Max Segment Length: \t\t\t" << maxsegment << endl;
  cout <<"Page Size: \t\t\t\t" << pagesize << endl;
  cout <<"Page Frame Per Process: \t\t" << pframeprocess << endl;
  cout <<"X Lookahead: \t\t\t\t" << lookahead << endl;
  cout <<"Minimum lookahead: \t\t\t" << minlookahead << endl;
  cout <<"Max Lookahead: \t\t\t\t" << maxlookahead << endl;
  cout <<"Total Process: \t\t\t\t" << totalprocess << endl;
  cout <<"_____________________________________________________________"<< endl;


  for(int i=0; i<totalprocess; i++)
  {
  //Process[i].processid = stoi (line);
  cout << "Process "<<i+1<< " ID is: \t"<<Process[i].processid << "\t";
  //Process[i].processframe = stoi (line);
  cout << "Process Frame is: \t" <<Process[i].processframe << endl;
  }
//----------------------------FIFO----------------------------------------

  buildmainframe();
  builddisk();
	cout << "Size of Main Frame: \t" << mainframe.size() << endl;
	FIFO();
	mainframe.clear();
	delete[] Process;
	diskframe.clear();
	instruction.clear();



//----------------------------LIFO----------------------------------------
	ReadFile(argv[1]);
	buildmainframe();
  builddisk();
	cout << "Size of Main Frame: \t" << mainframe.size() << endl;
	LIFO();
	mainframe.clear();
	delete[] Process;
	diskframe.clear();
	instruction.clear();




//----------------------------LRU------------------------------------------
	ReadFile(argv[1]);
	buildmainframe();
	builddisk();
	cout << "Size of Main Frame: \t" << mainframe.size() << endl;
	LRU();
	mainframe.clear();
	delete[] Process;
	diskframe.clear();
	instruction.clear();
	order.clear();




//----------------------------LDF------------------------------------------
	ReadFile(argv[1]);
	buildmainframe();
	builddisk();
	cout << "Size of Main Frame: \t" << mainframe.size() << endl;
	LDF();
	mainframe.clear();
	delete[] Process;
	diskframe.clear();
	instruction.clear();
	order.clear();



//_----------------------------OPT-X----------------------------------------
	ReadFile(argv[1]);
	buildmainframe();
	builddisk();
	cout << "Size of Main Frame: \t" << mainframe.size() << endl;
	OPT();
	mainframe.clear();
	delete[] Process;
	diskframe.clear();
	instruction.clear();
	order.clear();





}



void ReadFile(string filename)
{
  ifstream file(filename);
  if(file.is_open())
  {
      string line;
			file >> totalpageframe;
			file >> maxsegment;
			file >> pagesize;
			file >> pframeprocess;
			file >> lookahead;
			file >> minlookahead;
			file >> maxlookahead;
			file >> totalprocess;

  Process = new process[totalprocess];
  for(int i=0;i<totalprocess;i++)
    {
      file >> Process[i].processid;
      file >> Process[i].processframe;
      getline(file, line);
    }
  while(getline(file,line))
    {
      instruction.push_back(line);
    }
  }
}

void buildmainframe()
{
  for (int i=0; i<totalpageframe; i++)
  {
    mainframe.push_back(*(new frame));
		mainframe[i].frameid=i;
  }
}

void builddisk()
{
	int framenumber=0;

	//create frames in disk based on based on total process, number of pages per process = maxsegment*pframeprocess == 16 in this case
  for (int i=0; i<totalprocess; i++)
  {
		Process[i].segmentTable = * (new segmenttable);
    for (int j=0; j<maxsegment; j++)
    {
      Process[i].segmentTable.segments.push_back(*(new segment));
			Process[i].segmentTable.segments[j].pageTable= *(new pagetable);
			for (int k=0; k<pframeprocess; k++)
			{
				Process[i].segmentTable.segments[j].pageTable.pages.push_back(*(new page(k)));
			}
			frame *Frame = new frame();
			Frame->allocated=true;
			Frame->segmentid=j;
			Frame->processid=Process[i].processid;
			Frame->frameid=framenumber;
			//cout <<Frame->allocated<<"Segment "<<Frame->segmentid<<"Process id "<<Frame->processid<<"Frame id"<<Frame->frameid<<endl;
			diskframe.push_back(*Frame);
			framenumber++;
    }
  }
}


void FIFO()
{
	int mainframeloc = 0;		//the location of frame in mainframe to use for insertion and replacement
	int faults=0;
	long int hextodec;
	int loopcount=instruction.size();
	for(int i=0; i<loopcount; i++)
	{
		string instruct = instruction.front();
		//cout << "Instruction: \t"<<instruct<<"\t" << endl;
		for(int j=0; j<totalprocess;j++)
		{
			string processid = to_string(Process[j].processid);
			if (instruct.find(processid) != string::npos)
			{
				//conversion for segment, page, and process id from string of list in instruction
				size_t pos=instruct.find(processid)+4;
				string address = instruct.substr(pos+2, 2);
				char *cstr = new char[2];
				strcpy(cstr, address.c_str());
				hextodec = strtol(cstr, NULL, 16);
				int segment = hextodec/(pagesize * maxsegment);
				int page = ((hextodec/(pagesize)%maxsegment));
				//cout << "Segment is \t" << segment << "\t Page is \t" << page << endl;
				//cout << "Accessing process: " << processid << " segment: " << segment << " page: " << page << endl;
				bool pfault=false;
				bool found=false;
				// cout for process exit
				if(strlen(cstr)==0 || strlen(cstr)==1)
				{
					cout<<"Process "<<Process[j].processid<<" has exited."<<endl;
					instruction.pop_front();
				}
				if(strlen(cstr)!=0 && strlen(cstr)!=1)
				{
				// check page table if process is allocated
				if(Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated==false)
				{
					cout << "Page Fault: Accessing Hard Disk "<<endl;
					pfault = true;
				}
				// check if page was replaced in main memory
				else if (mainframe[Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid].processid != Process[j].processid &&
								 mainframe[Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid].pageid == page &&
								 mainframe[Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid].segmentid == segment)
				{
					cout<<"Page was replaced in main memory"<<endl;
					pfault = true;
				}
				else
				{
					cout << "Page Found in main memory" <<endl;
				}

				if(pfault)
				{

					faults+=1;
					frame *frame=NULL;
					//mainframe has empty frame
					for(int i=0;i<mainframe.size();i++)
					{
						if(mainframe[i].allocated==false)
						{
							frame = &mainframe[i];
							break;
						}
					}
					//put page into empty mainframe
					if(frame!=NULL)
					{
					frame->allocated=true;
					frame->processid=Process[j].processid;
					frame->pageid=page;
					frame->segmentid=segment;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid=frame->frameid;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated=true;
					//cout <<"Allocated "<<frame->allocated<<" Process "<<frame->processid<< " Page Number"<<frame->pageid<<" Segment "<<frame->segmentid<<endl;
					cout <<"Unallocated main frame at: "<<mainframe[frame->frameid].frameid<<endl;
					}
					//page replacement
					else
					{
						mainframe[mainframeloc].processid=Process[j].processid;
						mainframe[mainframeloc].pageid=page;
						mainframe[mainframeloc].segmentid=segment;
						//cout << "Frame " << mainframe[mainframeloc].frameid << " process " << mainframe[mainframeloc].processid << " segment "<< mainframe[mainframeloc].segmentid<<endl;
						cout<<"Reallocating frame: "<< mainframe[mainframeloc].frameid<<" for process " << mainframe[mainframeloc].processid<<endl;

					}
					mainframeloc+=1;
					//reset mainframeloc to 0 if mainframeloc is at the end of mainframe
					if(mainframeloc==totalpageframe)
					{
						mainframeloc=0;
					}
				}

				instruction.pop_front();
			}
			}
		}
	}
	cout<<"------------------FIFO:: Total Page Faults = " << faults <<"-----------------------------"<< endl;
}


void LIFO()
{
	int mainframeloc = totalpageframe-1; //mainframeloc for LIFO will be at the end location since it'll be the last one filled in mainframe
	int faults=0;
	long int hextodec;
	int loopcount=instruction.size();
	for(int i=0; i<loopcount; i++)
	{
		string instruct = instruction.front();
		//cout << "Instruction: \t"<<instruct<<"\t" << endl;
		for(int j=0; j<totalprocess;j++)
		{
			string processid = to_string(Process[j].processid);
			if (instruct.find(processid) != string::npos)
			{
				size_t pos=instruct.find(processid)+4;
				string address = instruct.substr(pos+2, 2);
				char *cstr = new char[2];
				strcpy(cstr, address.c_str());
				hextodec = strtol(cstr, NULL, 16);
				int segment = hextodec/(pagesize * maxsegment);
				int page = ((hextodec/(pagesize)%maxsegment));
				bool pfault=false;
				bool found=false;
				if(strlen(cstr)==0 || strlen(cstr)==1)
				{
					cout<<"Process "<<Process[j].processid<<" has exited."<<endl;
					instruction.pop_front();
				}
				if(strlen(cstr)!=0 && strlen(cstr)!=1)
				{
				// check page table if page is allocated
				if(Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated==false)
				{
					cout << "Page Fault: Accessing Hard Disk "<<endl;
					pfault = true;
				}
				// check if page was replaced in main memory
				else if (!mainframe[totalpageframe-1].processid == Process[j].processid &&
								 mainframe[totalpageframe-1].pageid == page &&
								 mainframe[totalpageframe-1].segmentid == segment)
				{
					cout<<"_____________________________________"<<endl;
					pfault = true;
				}
				else
				{
					cout << "Page Found in main memory" <<endl;
				}

				if(pfault)
				{

					faults+=1;
					frame *frame=NULL;
					//mainframe has empty frame
					for(int i=0;i<mainframe.size();i++)
					{
						if(mainframe[i].allocated==false)
						{
							frame = &mainframe[i];
							break;
						}
					}
					//put page into empty mainframe
					if(frame!=NULL)
					{
					frame->allocated=true;
					frame->processid=Process[j].processid;
					frame->pageid=page;
					frame->segmentid=segment;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid=frame->frameid;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated=true;
					//cout <<"Allocated "<<frame->allocated<<" Process "<<frame->processid<< " Page Number"<<frame->pageid<<" Segment "<<frame->segmentid<<endl;
					cout <<"Unallocated main frame at: "<<mainframe[frame->frameid].frameid<<endl;
					}
					//page replacement
					else
					{
						mainframe[mainframeloc].processid=Process[j].processid;
						mainframe[mainframeloc].pageid=page;
						mainframe[mainframeloc].segmentid=segment;
						//cout << "Frame " << mainframe[mainframeloc].frameid << " process " << mainframe[mainframeloc].processid << " segment "<< mainframe[mainframeloc].segmentid<<endl;
						cout<<"Reallocating frame: "<< mainframe[mainframeloc].frameid<<" for process " << mainframe[mainframeloc].processid<<endl;
					}
					/*mainframeloc+=11;
					if(mainframeloc==totalpageframe+1)
					{
						mainframeloc=0;
					}*/
				}

				instruction.pop_front();
			}
			}
		}
	}
	cout<<"------------------LIFO:: Total Page Faults = " << faults <<"-----------------------------"<< endl;
}

void LRU()
{
	int faults=0;
	long int hextodec;
	int loopcount=instruction.size();
	for(int i=0; i<loopcount; i++)
	{
		string instruct = instruction.front();
		//cout << "Instruction: \t"<<instruct<<"\t" << endl;
		for(int j=0; j<totalprocess;j++)
		{
			string processid = to_string(Process[j].processid);
			if (instruct.find(processid) != string::npos)
			{
				size_t pos=instruct.find(processid)+4;
				string address = instruct.substr(pos+2, 2);
				char *cstr = new char[2];
				strcpy(cstr, address.c_str());
				hextodec = strtol(cstr, NULL, 16);
				int segment = hextodec/(pagesize * maxsegment);
				int page = ((hextodec/(pagesize)%maxsegment));
				//cout << "Segment is \t" << segment << "\t Page is \t" << page << endl;
				//cout << "Accessing process: " << processid << " segment: " << segment << " page: " << page << endl;
				bool pfault=false;
				bool found=false;
				if(strlen(cstr)==0 || strlen(cstr)==1)
				{
					cout<<"Process "<<Process[j].processid<<" has exited."<<endl;
					instruction.pop_front();
				}
				if(strlen(cstr)!=0 && strlen(cstr)!=1)
				{
				// check page table if process is allocated
				if(Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated==false)
				{
					cout << "Page Fault: Accessing Hard Disk "<<endl;
					pfault = true;
				}
				// check if page was replaced in main memory
				else if (mainframe[Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid].processid != Process[j].processid &&
								 mainframe[Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid].pageid == page &&
								 mainframe[Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid].segmentid == segment)
				{
					cout<<"Page was replaced in main memory"<<endl;
					pfault = true;
				}
				else
				{
					cout << "Page Found in main memory" <<endl;
					int f=Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid;
					//order will be used to store recently used frames
					order.push_back(f);
					//if a frame is used recently, then erase the same frame from order and put the frame at the end
					if(order.size()!=0)
					{

						for(int j=0;j<order.size()-1;j++)
						{
							if(order[j]==f)
							{
								order.erase(order.begin()+j);
							}
						}
					}
				}

				if(pfault)
				{

					faults+=1;
					frame *frame=NULL;
					//mainframe has empty frame
					for(int i=0;i<mainframe.size();i++)
					{
						if(mainframe[i].allocated==false)
						{
							frame = &mainframe[i];
							break;
						}
					}
					//put page into empty mainframe
					if(frame!=NULL)
					{
					frame->allocated=true;
					frame->processid=Process[j].processid;
					frame->pageid=page;
					frame->segmentid=segment;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid=frame->frameid;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated=true;

					//cout <<"Allocated "<<frame->allocated<<" Process "<<frame->processid<< " Page Number"<<frame->pageid<<" Segment "<<frame->segmentid<<endl;
					cout <<"Unallocated main frame at: "<<mainframe[frame->frameid].frameid<<endl;
					order.push_back(frame->frameid);
					if(order.size()!=0)
					{
						for(int i=0;i<order.size()-1;i++)
						{
							if(order[i]==frame->frameid)
							{
								order.erase(order.begin()+1);
							}
						}
					}
					}
					//page replacement
					else
					{
						//first frame is the oldest and least recently used
						mainframe[order[0]].processid=Process[j].processid;
						mainframe[order[0]].pageid=page;
						mainframe[order[0]].segmentid=segment;
						//cout << "Frame " << mainframe[mainframeloc].frameid << " process " << mainframe[mainframeloc].processid << " segment "<< mainframe[mainframeloc].segmentid<<endl;
						cout<<"Reallocating frame: "<< mainframe[order[0]].frameid<<" for process " << mainframe[order[0]].processid<<endl;
						int f=mainframe[order[0]].frameid;
						order.push_back(f);
						if(order.size()!=0)
						{

							for(int j=0;j<order.size()-1;j++)
							{
								if(order[j]==f)
								{
									order.erase(order.begin()+j);
								}
							}
						}
					}

				}

				instruction.pop_front();
			}
			}
		}
	}
	cout<<"-------------------LRU:: Total Page Faults = " << faults <<"-----------------------------"<< endl;
}

void LDF()
{
	int mainframeloc=0;
	int faults=0;
	long int hextodec;
	int loopcount=instruction.size();
	for(int i=0; i<loopcount; i++)
	{
		string instruct = instruction.front();
		//cout << "Instruction: \t"<<instruct<<"\t" << endl;
		for(int j=0; j<totalprocess;j++)
		{
			string processid = to_string(Process[j].processid);
			if (instruct.find(processid) != string::npos)
			{
				size_t pos=instruct.find(processid)+4;
				string address = instruct.substr(pos+2, 2);
				char *cstr = new char[2];
				strcpy(cstr, address.c_str());
				hextodec = strtol(cstr, NULL, 16);
				int segment = hextodec/(pagesize * maxsegment);
				int page = ((hextodec/(pagesize)%maxsegment));
				//cout << "Segment is \t" << segment << "\t Page is \t" << page << endl;
				//cout << "Accessing process: " << processid << " segment: " << segment << " page: " << page << endl;
				bool pfault=false;
				bool found=false;
				if(strlen(cstr)==0 || strlen(cstr)==1)
				{
					cout<<"Process "<<Process[j].processid<<" has exited."<<endl;
					instruction.pop_front();
				}
				if(strlen(cstr)!=0 && strlen(cstr)!=1)
				{
				// check page table if process is allocated
				if(Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated==false)
				{
					cout << "Page Fault: Accessing Hard Disk "<<endl;
					pfault = true;
				}
				// check if page was replaced in main memory
				else if (mainframe[Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid].processid != Process[j].processid &&
								 mainframe[Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid].pageid == page &&
								 mainframe[Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid].segmentid == segment)
				{
					cout<<"Page was replaced in main memory"<<endl;
					pfault = true;
				}
				else
				{
					cout << "Page Found in main memory" <<endl;
				}

				if(pfault)
				{

					faults+=1;
					frame *frame=NULL;
					//mainframe has empty frame
					for(int i=0;i<mainframe.size();i++)
					{
						if(mainframe[i].allocated==false)
						{

							//switch between i and mainframe.size - i in alternating order to get the longest distance of each alternating frames
							//for initialization that is not already empty
							if(order.size()%2==0)
							{
								frame = &mainframe[i];
								order.push_back(i);
							}
							else if(order.size()%2!=0)
							{
								i=mainframe.size()-i;
								frame = &mainframe[i];
								order.push_back(i);
							}
							break;
						}
					}
					//put page into empty mainframe
					if(frame!=NULL)
					{
					frame->allocated=true;
					frame->processid=Process[j].processid;
					frame->pageid=page;
					frame->segmentid=segment;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid=frame->frameid;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated=true;

					//cout <<"Allocated "<<frame->allocated<<" Process "<<frame->processid<< " Page Number"<<frame->pageid<<" Segment "<<frame->segmentid<<endl;
					cout <<"Unallocated main frame at: "<<mainframe[frame->frameid].frameid<<endl;
					}
					//page replacement
					else
					{
						//use mainframeloc to get the furthest distance from mainframe.size/2
						//number < (mainframe.size/2) furthest location is mainframe
						//number > (mainframe.size/2) furthest location is 0
						if (Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid<mainframe.size()/2)
						{
							mainframeloc=mainframe.size()-1;
						}
						if (Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid>mainframe.size()/2)
						{
							mainframeloc=0;
						}
						mainframe[mainframeloc].processid=Process[j].processid;
						mainframe[mainframeloc].pageid=page;
						mainframe[mainframeloc].segmentid=segment;
						//cout << "Frame " << mainframe[mainframeloc].frameid << " process " << mainframe[mainframeloc].processid << " segment "<< mainframe[mainframeloc].segmentid<<endl;
						cout<<"Reallocating frame: "<< mainframe[mainframeloc].frameid<<" for process " << mainframe[mainframeloc].processid<<endl;

					}

				}

				instruction.pop_front();
			}
			}
		}
	}
	cout<<"------------------LDF:: Total Page Faults = " << faults <<"-----------------------------"<< endl;
}

void OPT()
{
	vector<int>optpage;		//stores lookahead values in vector to not be replaced
	vector<int>optsegment;
	vector<int>optprocessid;
	vector<int>optframe;
	int mainframeloc = totalpageframe-1;
	int faults=0;
	long int hextodec;
	int loopcount=instruction.size();
	for(int i=0; i<loopcount; i++)
	{

		string instruct = instruction.front();
		//cout << "Instruction: \t"<<instruct<<"\t" << endl;
		for(int j=0; j<totalprocess;j++)
		{


			//stores lookahead processid, segment, and page in vector to not be replaced in page replacement
			string oprocessid = to_string(Process[j].processid);
			for(int i=0;i<lookahead;i++)
			{
				if (instruct.find(oprocessid) != string::npos)
				{
					size_t pos=instruct.find(oprocessid)+4;
					string address = instruct.substr(pos+2, 2);
					char *cstr = new char[2];
					strcpy(cstr, address.c_str());
					hextodec = strtol(cstr, NULL, 16);
					int segment = hextodec/(pagesize * maxsegment);
					int page = ((hextodec/(pagesize)%maxsegment));

					optpage.push_back(page);
					optsegment.push_back(segment);
					optprocessid.push_back(stoi(oprocessid));
				}
			}


			string processid = to_string(Process[j].processid);
			if (instruct.find(processid) != string::npos)
			{
				size_t pos=instruct.find(processid)+4;
				string address = instruct.substr(pos+2, 2);
				char *cstr = new char[2];
				strcpy(cstr, address.c_str());
				hextodec = strtol(cstr, NULL, 16);
				int segment = hextodec/(pagesize * maxsegment);
				int page = ((hextodec/(pagesize)%maxsegment));
				//cout << "Segment is \t" << segment << "\t Page is \t" << page << endl;
				//cout << "Accessing process: " << processid << " segment: " << segment << " page: " << page << endl;
				bool pfault=false;
				bool found=false;
				if(strlen(cstr)==0 || strlen(cstr)==1)
				{
					cout<<"Process "<<Process[j].processid<<" has exited."<<endl;
					instruction.pop_front();
				}
				if(strlen(cstr)!=0 && strlen(cstr)!=1)
				{
				// check page table if page is allocated
				if(Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated==false)
				{
					cout << "Page Fault: Accessing Hard Disk "<<endl;
					pfault = true;
				}
				// check if page was replaced in main memory
				else if (!mainframe[totalpageframe-1].processid == Process[j].processid &&
								 mainframe[totalpageframe-1].pageid == page &&
								 mainframe[totalpageframe-1].segmentid == segment)
				{
					cout<<"_____________________________________"<<endl;
					pfault = true;
				}
				else
				{
					cout << "Page Found in main memory" <<endl;
				}

				if(pfault)
				{

					faults+=1;
					frame *frame=NULL;
					//mainframe has empty frame
					for(int i=0;i<mainframe.size();i++)
					{
						if(mainframe[i].allocated==false)
						{
							frame = &mainframe[i];
							break;
						}
					}
					//put page into empty mainframe
					if(frame!=NULL)
					{
					frame->allocated=true;
					frame->processid=Process[j].processid;
					frame->pageid=page;
					frame->segmentid=segment;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].frameid=frame->frameid;
					Process[j].segmentTable.segments[segment].pageTable.pages[page].allocated=true;
					//cout <<"Allocated "<<frame->allocated<<" Process "<<frame->processid<< " Page Number"<<frame->pageid<<" Segment "<<frame->segmentid<<endl;
					cout <<"Unallocated main frame at: "<<mainframe[frame->frameid].frameid<<endl;
					}
					//page replacement
					else
					{
						for(int n=0;n<optprocessid.size();n++)
						{
							for(int m=0;m<mainframe.size();m++)
							{
								if(mainframe[m].processid==optprocessid[n] &&
									mainframe[m].segmentid==optsegment[n] &&
									mainframe[m].pageid==optpage[n])
									{
										optframe.push_back(mainframe[m].frameid);
									}
							}
						}
						sort(optframe.begin(), optframe.end());	//sort opt frames
						for(int n=0;n<optframe.size(); n++)
						{
							if(optframe[n]==mainframeloc)
							{
								mainframeloc++;
							}
						}

						optpage.clear();
						optsegment.clear();
						optprocessid.clear();
						optframe.clear();



						mainframe[mainframeloc].processid=Process[j].processid;
						mainframe[mainframeloc].pageid=page;
						mainframe[mainframeloc].segmentid=segment;
						//cout << "Frame " << mainframe[mainframeloc].frameid << " process " << mainframe[mainframeloc].processid << " segment "<< mainframe[mainframeloc].segmentid<<endl;
						cout<<"Reallocating frame: "<< mainframe[mainframeloc].frameid<<" for process " << mainframe[mainframeloc].processid<<endl;
					}
					mainframeloc+=1;
					if(mainframeloc==totalpageframe+1)
					{
						mainframeloc=0;
					}
				}

				instruction.pop_front();
			}
			}
		}
	}
	cout<<"------------------OPT:: Total Page Faults = " << faults <<"-----------------------------"<< endl;
}
