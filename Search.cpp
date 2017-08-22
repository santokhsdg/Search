/*MIT License

Copyright (c) [2017] [Santokh Singh, Gurjot Singh]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

     |b*_*b|
ROLLSIDE SEARCH (RS)
*/

#include<iostream>
using namespace std;
#include <semaphore.h>
#include<fcntl.h>
#include<pthread.h>
#include<signal.h>
#include <sched.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<string.h>
#include<sys/sem.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<fstream>
#include<stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctime>


#define LIMIT 36  //limit on type of character alphabet small caps and nums
#define FLAG 5    //type of word
#define CHARS 26  // available character a-z
#define NUM 10   // number 0-9
#define INR 0.00001 // increment on search triggered by user preference on trieRecon
#define WORDLEN 150  // maximum search able keyword length
#define ROOTPATH "/var/www/html/HTML/CORE/Search/"  // path from root    ->     
#define OUTPUTDIR "/var/www/html/HTML/CORE/SearchOutput/"     // search output dir  ->     
#define SNGIDLEN 30  // length of songID
#define HASHLIMIT 5000  // max hash value OR max data that can be read
#define MAXDLEVEL 4  // max downward level search can move
#define BASEPREF -1  // BASE PREF CHOSEN FOR SEARCH
#define RESULTCOUNT 5 // max recommendations
#define REPCHARS 5 // no character for replacements 
#define REPCOUNT 100 //  no replacements available
#define REPRELATION 2 // binary relation one to one only
#define REPLACEMENTLEN 4 // upto what level replacements are valid
#define LEVELINCR 2 // level traversed increment after replacement policy


// struct to get keyword in trie
struct TrieWord{
 char keyword[WORDLEN];
 char result[RESULTCOUNT][WORDLEN];
 int flag[RESULTCOUNT][FLAG];
 int rcount;
 float pref[RESULTCOUNT];
 int found;
};

//for getting results
struct File{
 char sngid[SNGIDLEN];
 int count;
};

//UNIT TESTING
void Test(){
ofstream of;
of.open("test.txt");
of<<"here it is";
of.close();
}


//forward declaration
class Trie;
class Search;
void* PhysicalSearch(void *arg);
void* PhysicalUpdation(void *arg);

// for getting highest preference word
struct PrefNode{
   Trie *node; // max pref node
   float pref; // max pref
   int indx;  // indx in the node
   int len; // initial valid len of string  
};

// GLOBAL VARIABLES
Trie *root=NULL; // root to Trie
int start=0,PARENTID,CHILDID,replacements; // start of core changes after first creation of trie and never again until its running
char ReplacementVector[REPCOUNT][REPRELATION][REPCHARS]; // replacement strings 
//////

class Trie {

public:
float Pref[LIMIT];  // searched preference 
Trie *Next[LIMIT];  // refrence to next trie link
int Valid[LIMIT];   // checks validity of alphabet
int Wexist[LIMIT];  // checks existance of the word
int WordCount[LIMIT]; // count the same word updations
int Flag[LIMIT][FLAG]; // decides song, artist, album, band, type
Trie *ParentNode;      // gives parent node of a node
int ParentIndex;      //index of parent node
char PrefWord[LIMIT][WORDLEN];  //most preferred word
float MaxPrefWord[LIMIT];  //  preferred words pref
int PrefFlag[LIMIT][FLAG]; // flags of max preferred word



   Trie()
    { // initialize data in object
            ParentNode=NULL;
            ParentIndex=-1;
             for(int i=0;i<LIMIT;i++)
                 {
                  Next[i]=NULL;
                  Valid[i]=0;
                  Wexist[i]=0;
                  Pref[i]=0;
                  MaxPrefWord[i]=0;
                  strcpy(PrefWord[i],"");
                  for(int j=0;j<FLAG;j++){ Flag[i][j]=0; PrefFlag[i][j]=0;}
                  WordCount[i]=0;
                 }

    }

  static void createTrie()
   { // create root noded of trie
     Trie *rt= new Trie();
     root =rt;
   }
static int ReverseIndex(int ch)
   { // return character from index
    if(ch<CHARS)
      { return ch+97;}
    else { return ch+48-CHARS;}
   }

 static int Index(char ch)
   {  // return index of the character in Trie
    if(ch>=97  && ch<=122)
      {return ch-97;}
    else
      { return LIMIT-NUM+ch-48; }

   }

//TRIE PREF BUILD

static void BackTrack(struct Trie *node,PrefNode *prefnode,int node_indx)
       {   // backtrack from the node to the root node
          int st_indx=prefnode->indx;
          Trie *pnode=prefnode->node;
          node->MaxPrefWord[node_indx]=prefnode->pref;
	  int len=prefnode->len;

          for(int f=0;f<FLAG;f++){ node->PrefFlag[node_indx][f]=pnode->Flag[st_indx][f];}

          while(len!=-1)
          {
             node->PrefWord[node_indx][len]=Trie::ReverseIndex(st_indx);
             if(node==root){ break;}
             st_indx=pnode->ParentIndex;
             pnode=pnode->ParentNode;             
             len--;
          }
          
       }

static void ExtensiveRecommend(Trie *node,int level,PrefNode *prefnode,int indx)
                {   // looks at all possible words down and look the max pref words
                 if(node==NULL){ return;}    // move down until this level and node existence          
                 for(int i=0;i<LIMIT;i++)
                   {
                    if(node->Valid[i]==1)
                      {                    
                       if(node->Wexist[i]==1 && prefnode->pref<node->Pref[i])
                          {  
                           prefnode->node=node;
			  			   prefnode->pref=node->Pref[i];
		          		   prefnode->indx=i;
			   			   prefnode->len=level+indx;                             
                          }
                        if(node->Next[i]!=NULL)
                        {                                             
                        ExtensiveRecommend(node->Next[i],level+1,prefnode,indx); 
                        } 
                                            
                      }
                   }    
                  return;
                }

// build the pref in each node to limit search at subsequent levels
static void BuildPrefTrie(Trie *node,PrefNode *prefnode,int level)
	{ 
               if(node==NULL){ return; }    // move down until node existence          
                 for(int i=0;i<LIMIT;i++)
                   {
                    if(node->Valid[i]==1)
                      {    
                         if(node->Wexist[i]==1)
                          {
                           prefnode->node=node;
			               prefnode->pref=node->Pref[i];
		                   prefnode->indx=i;
			               prefnode->len=level;  
                          }
                         else
                             {	
                              prefnode->pref=BASEPREF;  //-1 for getting intial 0 pref for all words
                             }
                        Trie::ExtensiveRecommend(node->Next[i],0,prefnode,level);   
                        Trie::BackTrack(node,prefnode,i);          
                        if(node->Next[i]!=NULL)
                        {                        
                        Trie::BuildPrefTrie(node->Next[i],prefnode,level+1); 
                        }                     
                      }
                   }
              return;
       	}



// UPDATE TRIE WITH NEW ENTRY OR UPDATE PREFERENCE
  static void updateTrie(char word[],int flag,float pref=0)
   {  // update the Trie with new enteries
     int l=strlen(word);               // length of word
     Trie *node=root,*prev_node=root;  // pointer to root, pointer prev to previous node
     int i=0,indx,prev_indx=0;
     while(i<l)
     {
      indx=Index(word[i]);
      if(node!=NULL)
             {
              node->Valid[indx]=1;
              if(i==l-1)
                  {
                    node->Wexist[indx]=1;     // word is possible after the last character as it ends here, make wordexist flag 1
                    node->Flag[indx][flag]=1; // set flag value
                    node->WordCount[indx]=node->WordCount[indx]+1;
                    node->Pref[indx]=pref; // set preference **** sti-ll to deci-de **** now decided to update only by php.....
                     
                  }
                  prev_indx=indx;         // store prev index and node
                  prev_node=node;
                  node=node->Next[indx];
             }
      else
          {
            Trie *nd=new Trie();
            node=nd;
            prev_node->Next[prev_indx]=node;// assign new node ,connection to trie
            node->Valid[indx]=1;
              if(i==l-1)
                  {
                    node->Flag[indx][flag]=1;
                    node->WordCount[indx]=node->WordCount[indx]+1;
                    node->Wexist[indx]=1;// word is possible after the last character as it ends here, make wordexist flag 1
                    node->Pref[indx]=pref; // set preference **** still to decide ****
                  }
                  node->ParentNode=prev_node;  // setting parents only occurs when new nodes are created
                  node->ParentIndex=prev_indx;
                  prev_indx=indx;         // store prev index and node
                  prev_node=node;
                  node=node->Next[indx];
          }
            i++;
     }
   }

//SYNCHRONIZATION FUNCTIONS
static void ReleaseUpdateSemaphore()
    { // release update  semaphore
      int val;
      sem_t * sema;
      sema = sem_open("/updateSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
      sem_getvalue(sema,&val);
      if(val==0){sem_post(sema);}
    }
 static void ReleaseSearchSemaphore()
    { // release search semaphore
      int val;
      sem_t * sema;
      sema = sem_open("/searchSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
      sem_getvalue(sema,&val);
      if(val==0){sem_post(sema);}

    }
    static void LockUpdateSemaphore()
    { // lock update  semaphore
      int val;
      sem_t * sema;
      sema = sem_open("/updateSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
      sem_getvalue(sema,&val);
      if(val==1){sem_wait(sema);}
    }
 static void LockSearchSemaphore()
    { // lock update  semaphore
      int val;
      sem_t * sema;
      sema = sem_open("/searchSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
      sem_getvalue(sema,&val);
      if(val==1){sem_wait(sema);}
    }
 static void ReleaseReconstructSemaphore()
    { // release recon semaphore :: its opposite to all other semaphores
      int val;
      sem_t * sema;
      sema = sem_open("/reconSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
      sem_getvalue(sema,&val);
      if(val==0){sem_post(sema);}
    }
 static void LockReconstructSemaphore()
    { // lock recon semaphore :: its opposite to all other semaphores
      int val;
      sem_t * sema;
      sema = sem_open("/reconSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
      sem_getvalue(sema,&val); Test();
      if(val==1){sem_wait(sema);} Test();
    }
static void ReleaseReconstructProcessSemaphore()
    { // release recon process semaphore
      int val;
      sem_t * sema;
      sema = sem_open("/rproSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
      sem_getvalue(sema,&val);
      if(val==0){sem_post(sema);}
    }
 static void LockPrefSemaphore()
    { // lock pref semaphore :: its opposite to all other semaphores
      int val;
      sem_t * sema;
      sema = sem_open("/prefSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
      sem_getvalue(sema,&val);
      if(val==1){sem_wait(sema);}
    }
static void ReleasePrefSemaphore()
    { // release pref semaphore
      int val;
      sem_t * sema;
      sema = sem_open("/prefSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
      sem_getvalue(sema,&val);
      if(val==0){sem_post(sema);}
    }
static void ConstructSemaphore()
   { //make and initialise all the semaphore
      sem_t * sema;
      sema = sem_open("/rproSema",O_CREAT, S_IRUSR | S_IWUSR, 1); sem_close(sema);
      sema = sem_open("/reconSema",O_CREAT, S_IRUSR | S_IWUSR, 1); sem_close(sema); // search needs to unlocked before for release
      sema = sem_open("/searchSema",O_CREAT, S_IRUSR | S_IWUSR, 1); sem_close(sema);
      sema = sem_open("/updateSema",O_CREAT, S_IRUSR | S_IWUSR, 1); sem_close(sema);
      sema = sem_open("/prefSema",O_CREAT, S_IRUSR | S_IWUSR, 1); sem_close(sema);
   }


//TRIE RECONSTRUCTION USING REPETETIVE LOGICAL UPDATION
static void TrieReconstruct()
   {
    int callpid=getpid();
    int done;
    float pref;
    char word[WORDLEN];
    int flag[FLAG];
    ifstream fwr;
    fwr.open("TrieReconstruct.txt");
    // DONE , WORD , FLAG, PREF  ::FILE READING
    fwr>>done; // reconstruction is complete , flag done is set 1
    fwr>>word; // word to be updated in trie ; only set 1 for the last word to be updated
    for(int i=0;i<FLAG; i++)
    {
      fwr>>flag[i];  // read flag
    }
    fwr>>pref;
    fwr.clear(); // empty the file
    fwr.close();
    for(int i=0;i<FLAG; i++)
    {
      if(flag[i]==1)
      {
          Trie::updateTrie(word,i,pref); // update trie for each flag==1           
      }
    }

   if(callpid==CHILDID && done==1)
   { // all updates are done , now system is ready for searches and updates
     Trie::ReleaseSearchSemaphore();
     Trie::ReleaseUpdateSemaphore();
     Trie::ReleaseReconstructSemaphore();
   }
   // release semaphore for process of recontruction
   if(callpid==CHILDID){
   Trie::ReleaseReconstructProcessSemaphore();}

   }

// get pref of a word
static float GetPref(char word[])
{
    int l=strlen(word);               // length of word
     Trie *node=root;  // pointer to root, pointer prev to previous node
     int i=0,indx;
     float pref;
     while(i<l)
      {     indx=Index(word[i]);
             node=node->Next[indx];
             if(i==l-1 && node!=NULL);
             {
                 pref=node->Pref[indx];
                 return pref;
              }
              i++;
      }
}

};


//THE SEARCH

class Search
{
public:
//KEYWORD MATCHING SECTION
static int ReverseIndex(int ch)
   { //convert index to character of the node
    if(ch<CHARS)
      { return ch+97;}
    else { return ch+48-CHARS;}
   }

static int Index(char ch)
   {  // return index of the character in Trie
    if(ch>=97  && ch<=122)
      {return ch-97;}
    else
      { return LIMIT-NUM+ch-48; }

   }

static void SortPref(float pref[],int nodes[],int low , int high)
	{  // sorting the preferences to generate output
            float temp1;
	    int temp2,i,j;
            for(i=low;i<high;++i)
    		{
        	for(j=0;j<(high-i);++j)
                 {
         	   if(pref[j]<pref[j+1])
           	    {
                    temp1=pref[j];
                    pref[j]=pref[j+1];
                    pref[j+1]=temp1; 		    
		            temp2=nodes[j];
                    nodes[j]=nodes[j+1];
                    nodes[j+1]=temp2;		    
                    }
                  }
                }
	}


static void GetWord(TrieWord *data,Trie *node)
  { // get the maximum pref words from the next level and if more than req then sort and get maximum
   
   float pref[LIMIT];
   int count=0,nodes[LIMIT];
   for(int i=0;i<LIMIT;i++)
   { 
    if(node->Valid[i]==1)
      {
        nodes[count]=i;
        pref[count]=node->MaxPrefWord[i];
        count++;
      }
   }

  if(count<=RESULTCOUNT)
    {   data->rcount=count;
        data->found=1;
      for(int i=0;i<count;i++)
        {
          strcpy(data->result[i],node->PrefWord[nodes[i]]);
          data->pref[i]=node->MaxPrefWord[nodes[i]];
          for(int j=0;j<FLAG;j++)
            { data->flag[i][j]=node->PrefFlag[nodes[i]][j];}
        }
       
    }
  else
    { // if results are more, then get max pref only after sorting
        SortPref(pref,nodes,0,count);
        for(int i=0;i<RESULTCOUNT;i++)
        {
          strcpy(data->result[i],node->PrefWord[nodes[i]]);
          data->pref[i]=node->MaxPrefWord[nodes[i]];
          for(int j=0;j<FLAG;j++)
          { data->flag[i][j]=node->PrefFlag[nodes[i]][j];}
        }
       
    }

  }
  
//Replacement Policy 
// There are one to one replacementS and then get the longest match
static int GetTraversalLength(char word[])
	{ // check validity until the level
              Trie *node=root;
             int len=strlen(word),indx,i;
             for(i=0;i<len;i++)
                {
                 indx=Search::Index(word[i]);
		 if((node->Valid[indx])==1){
		    node=node->Next[indx];    // move to next level
                    if(node==NULL){return i;}
                  }
                 else{ return i; }         		

	        	}
            return i;
	}
static void CopyString(char target[],char source[], int start,int end)
        {        int indx=0; // copy the string
                for(int i=start;i<end;i++){
                 target[indx]=source[i];
	         	}
         }
static int SearchReplacementVector(char word[])
	{ // search word from the replacement vector
          for(int i=0;i<replacements;i++)
             {
               if(strcmp(word,ReplacementVector[i][0])==0 || strcmp(word,ReplacementVector[i][1])==0){
		         return i;
				}                 
             }
          return -1;
	}

static int GetAvailableReplacement(TrieWord *data)
	{  // get all replacements           
           int max_traversed_level, rep_index,rep_len,val,len,init_maxval;
           char temp[WORDLEN],maxmatch[len],keyword[WORDLEN]; 

   	   max_traversed_level=GetTraversalLength(data->keyword);
           len=strlen(data->keyword);
           init_maxval=max_traversed_level;
           strcpy(keyword,data->keyword);  

	   for(int i=0;i<len;i++){ // get every possible combination for search
            for(int j=i+1;j<len;j++){
                 strcpy(temp,"");
                 Search::CopyString(temp,keyword,i,j);
                 rep_index=Search::SearchReplacementVector(temp);

                  if(rep_index!=-1){
 		     if(strcmp(temp,ReplacementVector[rep_index][0])==0){
			  rep_len=strlen(ReplacementVector[rep_index][1]);
 			  Search::CopyString(temp,keyword,0,i-1);
			  Search::CopyString(temp,ReplacementVector[rep_index][1],i,j);
			  Search::CopyString(temp,keyword,i+1,j);
                          val=Search::GetTraversalLength(temp);
                          if(val>max_traversed_level){
                             max_traversed_level=val;
                             Search::CopyString(maxmatch,temp,0,len);
    			    		}
			}
 		     else{
			  rep_len=strlen(ReplacementVector[rep_index][1]);
 			  Search::CopyString(temp,keyword,0,i-1);
			  Search::CopyString(temp,ReplacementVector[rep_index][0],i,j);
			  Search::CopyString(temp,keyword,i+1,j);
                          val=Search::GetTraversalLength(temp);
                          if(val>max_traversed_level){
                             max_traversed_level=val;
                             Search::CopyString(maxmatch,temp,0,len);
    			   			 }
		 		}
		    }
	      }
	   }
		if(max_traversed_level>init_maxval+LEVELINCR){
              	 len=strlen(maxmatch);
               	 strcpy(data->keyword,maxmatch);
		         return 1;
		 }
                 return 0;
	}

/*// give no of replacements that are need to be performed
static int CalculateReplacement(TrieWord *data){
            int len;	    len=strlen(data->keyword);
            if(len>REPLACEMENTLEN)            { len=REPLACEMENTLEN;}
            
}
static void RunReplacementPolicy(TrieWord *data){
   // includes the NLP replacement policy for fixed characters and sounds
   		int reps; reps=CalculateReplacement(data);   }

*/


static void FindKeyword(TrieWord *data)
     {  // find the appropriate keyword for search
      float max_pref=0;
      int len,indx,level=-1,windx=-1,validity=1,last_vindx,i,k,No_result;
      len=strlen(data->keyword); // get keywrod length
      Trie *node=root,*wnode; // set root of trie

      for( i=0;i<len && i<WORDLEN ;i++)
          {
           indx=Search::Index(data->keyword[i]);  // get index of character
           if((node->Valid[indx])==1)  // read only if index is valid
               { cout<<indx<<"\n";
                if(node->Wexist[indx]==1) // set data if its a word
                   {
                     level=i; //set the level of word
                     for(int f=0;f<FLAG;f++){ data->flag[0][f]=node->Flag[indx][f];}// set flags if its a word
                     for( k=0;k<=level;k++)  //write word to result
                     {data->result[0][k]=data->keyword[k];}
                     data->pref[0]=node->Pref[indx]; // set preference
                     data->found=1; //word like that exist
                     windx=indx; wnode=node; // index and node of word
                   }
                    node=node->Next[indx];    // move to next level
                    if(node==NULL){break;} // if next node does not exist, break Search::Recommend(data,rnode,k);
               }
                 else{ validity=0; last_vindx=i-1; break;}
            }
                
	 if(validity==1 && i==len && level!=-1){ data->rcount=1;return;} // return after exact match   
              

         if(validity==0 && last_vindx==-1){
		No_result=Search::GetAvailableReplacement(data);
		if(No_result==1){ Search::FindKeyword(data);}
                else{data->found==0;}
                return;
              } // no result ... here we need to make replacement policies to work
 	     
         
		 if((validity==0 && i<len) || level<len-1){ GetWord(data,node);}  // get max pref words                       
               
          
    }

//RESULT WRITING AND SORTING

static void MergeSort(struct File a[], int low, int high)
{   // sort the song ids as per the count value or the most common songid order
    int mid;
    if (low < high)
    {
        mid=(low+high)/2;
        MergeSort(a,low,mid);
        MergeSort(a,mid+1,high);
        Merge(a,low,high,mid);
    }
    return;
}
static void Merge(struct File a[], int low, int high, int mid)
{  // merge process in sorting
    int i, j, k;
    char temp[SNGIDLEN];
    struct File c[HASHLIMIT];
    i = low;
    k = low;
    j = mid + 1;
    while (i <= mid && j <= high)
    {
        if (a[i].count < a[j].count)
        {
            c[k].count = a[i].count;
            strcpy(c[k].sngid,a[i].sngid);
            k++;  i++;
        }

        else
        {  c[k].count = a[j].count;
           strcpy(c[k].sngid,a[j].sngid);
            k++;  j++;
        }
    }

    while (i <= mid)
    {
        c[k].count = a[i].count;
        strcpy(c[k].sngid,a[i].sngid);
        k++;  i++;
    }

    while (j <= high)
    {   c[k].count = a[j].count;
        strcpy(c[k].sngid,a[j].sngid);
        k++;  j++;
    }

    for (i = low; i < k; i++)
    {   a[i].count = c[i].count;
        strcpy(a[i].sngid,c[i].sngid);
    }

}


static void WriteResult(char reqID[],int count, struct File result[])
     {
        int max=30; // max results to be shown
        char path[150];
        sprintf(path,"%s%s%s",OUTPUTDIR,reqID,".txt"); // make the path
        ofstream outw;
        outw.open(path);
        for(int i=count-1;i>=0 && max>0 ;i--,max--)
        { outw<<result[i].sngid<<"\n";}   // write the song ids as output
     }

//GTS SECTION 
static char* strlwr1(char* s)
{
    char* tmp = s;

    for (;*tmp;++tmp) {
        *tmp = tolower((unsigned char) *tmp);
    }

    return s;
}
static void createPath(struct File (&hasharray)[HASHLIMIT],char result[] ,int i)
{

	int f=0;
	char titleflag[FLAG][WORDLEN] = {"song/","artist/","album/","band/","type/"} ;
	char path[1000] = ROOTPATH;
	if(i==0)
	sprintf(path,"%s%s%s%s",path,titleflag[i],result,".txt");		//COMPARE for same keyw//initialize//PID

	if(i>0)
	{
	sprintf(path,"%s%s%s%s%s%s",path,titleflag[i],result,"/",result,".txt");
	}
	Search :: readKeywordFiles(path,hasharray);

}
static void readKeywordFiles(char path[],struct File (&hasharray)[HASHLIMIT])
{
	char songid[30];
	int pos,num,hashid;
	int x=access(path,F_OK);
	//cout<<"x : "<<x<<endl;
	cout<<path<<" "<<endl;
	if(x==0)
	{
	FILE *fp;
	fp = fopen(path,"r");
  cout<<path<<" "<<fp<<endl;
while(!feof(fp))
{
fscanf(fp,"%s %d %d %d",songid,&pos,&num,&hashid);
strcpy(hasharray[hashid].sngid,songid);
hasharray[hashid].count++;
}
fclose(fp);
}
}

};



//SEARCH EXECUTION

 void LogicalSearch()
{
	struct TrieWord data[100]; //taking here max 200 keywords and STRLIMIT is length of keyword
   	char value;//get char from file one by one
   	ifstream file;
   	ofstream wfile;
   	file.open("SearchInput.txt");
   	int i=0,s=0,j=0,k=0; //s will increment when space is encountered and j index will give str char by char from data variable
        for(int r=0;r<100;r++){  data[r].found=0; strcpy(data[r].keyword,"");}

  	while(file.get(value))
   	{

   		if(value==' ') //if space then start new keyword
   		{
   			strcpy(data[s].keyword,Search::strlwr1(data[s].keyword));
   			s++;
   			j=0;
   			 cout<<data[s].keyword<<endl;
   		}
   		else if(value=='\n')//also flag will change with s
   		{
   			strcpy(data[s].keyword,Search::strlwr1(data[s].keyword));
   			//cout<<data[s].keyword<<endl;
   			s++;
   			j=0;

   		}
   		else
   		{	if((value>=48&&value<=58)||(value>=65&&value<=90)||(value>=97&&value<=122))
   			data[s].keyword[j] = value ;
   			j++;
   		}
   	}

    file.close();



   	i=2;
   	while(i<s)
   	{

   		int stopflag = 0;
   		int a=i;
   		if(i>2)
   		{
   			a--;
   			while(a>1)
   			{
   				if(strcmp(data[a].keyword,data[i].keyword)==0)
   				{
   					stopflag=1;
   				}
   				a--;
   			}
   		}

   		if(stopflag==0)
   		{
  		Search::FindKeyword(&data[i]);
                 cout<<"Logical Found:"<<data[i].found<<"\n";
                }
   		i++;
   	}
}


 void* PhysicalSearch(void *arg)
{

   std::clock_t start = std::clock();
        int callpid=getpid();
	int result_count=0;
	struct File hasharray[HASHLIMIT] ;
	struct TrieWord data[100]; //taking here max 200 keywords and STRLIMIT is length of keyword



   	for(int r=0;r<HASHLIMIT;r++){  hasharray[r].count=0; strcpy(hasharray[r].sngid,""); }
   	for(int r=0;r<100;r++){  data[r].found=0; strcpy(data[r].keyword,"");}

   	char value;//get char from file one by one
   	ifstream file;
   	ofstream wfile;
   	file.open("SearchInput.txt");
   	int i=0,s=0,j=0,k=0; //s will increment when space is encountered and j index will give str char by char from data variable

  	while(file.get(value))
   	{

   		if(value==' ') //if space then start new keyword
   		{
   			strcpy(data[s].keyword,Search::strlwr1(data[s].keyword));
   			//cout<<data[s].keyword<<endl;
   			s++;
   			j=0;
   			 //cout<<data[s].keyword<<"HERE\n";
   		}
   		else if(value=='\n')//also flag will change with s
   		{
   			strcpy(data[s].keyword,Search::strlwr1(data[s].keyword));
   			//cout<<data[s].keyword<<"HERE\n";
   			s++;
   			j=0;

   		}
   		else
   		{	if((value>=48&&value<=58)||(value>=65&&value<=90)||(value>=97&&value<=122))
   			data[s].keyword[j] = value ;
   			j++;
   		}
   	}
    file.clear();
    file.close();

	int middlemanpid = atoi(data[0].keyword);

// Release buffer for next request

Trie::ReleaseSearchSemaphore();

   	strcpy(data[s].keyword,Search::strlwr1(data[s].keyword));

   	i=2;
   	while(i<s)
   	{

   		int stopflag = 0;
   		int a=i;
   		if(i>2)
   		{
   			a--;
   			while(a>1)
   			{
   				if(strcmp(data[a].keyword,data[i].keyword)==0)
   				{
   					stopflag=1;
   				}
   				a--;
   			}
   		}

   		if(stopflag==0)
   		{
  		Search::FindKeyword(&data[i]);
            cout<<"Physical Found:"<<data[i].found<<"\n";
                }
   		i++;
   	}


int u=0;
   char path[] = ROOTPATH ;
   i=2;
    k=0;
    int z=0;
   char titleflag[FLAG][WORDLEN] = {"song/","artist/","album/","band/","type/"} ;
   while(i<s)
   {

   		if(data[i].found==1)
   		{
   			k=0;
        u=0;
        while(u<data[i].rcount)
        {
	while(k<FLAG)
	{
	if(data[i].flag[u][k]>0)
	{
		Search::createPath(hasharray,data[i].result[u],k);
	}
		k++;
	}
	if(i>2)
	{
			int v=1;
			while(v<FLAG) //This loop is for first keyword other than song(artist,etc) and check for that in their folders
			{

				if(data[i-1].flag[u][v]>0)
				{


			char path[300]=ROOTPATH;
			sprintf(path,"%s%s%s%s%s%s",path,titleflag[v],data[i-1].result[0],"/",data[i].result[u],".txt");

			Search :: readKeywordFiles(path,hasharray);
				}

				v++;
			}
	}
	u++;
  }
  }
   		i++;
   }

   for(int rc=0;rc<HASHLIMIT;rc++)
    { // get result with count greater than 0,,, all valid results
     if(hasharray[rc].count>0){ result_count++;}
    }
  struct File result_final[result_count];
  // copy final results in the array
   int indx=0;
   for(int rc=0;rc<HASHLIMIT;rc++)
     {
        if(hasharray[rc].count>0){
        strcpy(result_final[indx].sngid,hasharray[rc].sngid); // copy sng ids
        result_final[indx].count=hasharray[rc].count;  // copy count
        indx++;
        //cout<<"sngid:"<<hasharray[rc].sngid<<"\n";
        }
     }


  cout<<"sngid:"<<result_final[0].sngid<<"\n";

   Search::MergeSort(result_final,0,result_count-1);
   cout<<"sngid:"<<result_final[0].sngid<<"\n";
   Search::WriteResult(data[1].keyword,result_count,result_final);
   // inform middleman .. result has come
   kill(middlemanpid,SIGALRM);


        double st = (std::clock() - start) / (double) (CLOCKS_PER_SEC/1000000);
 fstream jj;
    jj.open("executionsearch.txt",ios::app);
    jj<<st<<endl;
    jj.close();
    std::cout << "Finished in " << st << "microsec" << std::endl;
}




 int hashfunction(string sid) //hash function algorithm
{
	const char* str = sid.c_str();
	int hashid=0,i=3,j=1,k=1;
	while(str[i]!='\0')
	{
		if(i<=8)
		{
			hashid += (j*str[i]);
			j++;
		}
		else
		{
			hashid += (str[i]-k);
			k++;
		}
		i++;
	}
	return hashid ;
}
void uploadfilewrite(string path,string sid,int pos,int num) //to write content to corresponding file given in path
{
    int id	=  hashfunction(sid);
	ofstream wfile;
	wfile.open(path.c_str(),ios::app);
	wfile<<sid<<" "<<pos<<" "<<num<<" "<<id<<endl;
	wfile.close();
}

void checkdirectory(string path,string file_path,string str,int pos,int num)
{
	struct stat sb;
	const char* folderr = path.c_str();
	int x=0;

    if (stat(folderr, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
    	file_path+=string(".txt") ;
    	path+= string("/") + string(file_path) ;
        uploadfilewrite(path,str,pos,num);
    }
    else
    {
        mkdir(folderr,0777);
		file_path+=string(".txt") ;
    	path+= string("/") + string(file_path) ;
        uploadfilewrite(path,str,pos,num);
    }
}


void LogicalUpdation(){
	char str[200][WORDLEN]; //taking here max 200 keywords and STRLIMIT is length of keyword
   	char value;//get char from file one by one
        int callpid=getpid();// get pid of the process
   	ifstream file;
   	ofstream wfile;
   	file.open("TrieUpdate.txt");
   	int i=0,s=0,j=0,k=0,num=1,pos=1;
        //s will increment when space is encountered and j index will give str char by char from data variable
   	int sflag[FLAG+2];
   	int numkeywords[6],it=0; //to calculate number of keywords in particular field
   	while(i<6){numkeywords[i]=1;i++;} //initialized numkeywords with 1 coz there would be atleast 1 keyword in every field
    i=0;
   	while(file.get(value))
   	{


   		if(value==' ') //if space then start new keyword
   		{
   			numkeywords[it]++;//increase count for particular field
   			s++;
   			j=0;
   		}
   		else if(value=='\n')//also flag will change with s
   		{
   			sflag[k]=s;

   			s++;
   			k++;
   			j=0;
   			it++;//to iterate to next field
   		}
   		else
   		{
   			str[s][j] = value ;
   			j++;
   		}
   	}
   	file.close();
int flag=0 , m=0;
std::string path (ROOTPATH) ; //main path
std :: string file_path ;
char copy[60][100] ;
int x=0,index=0;
i=0;
while(i<=s){
              char* strings=Search::strlwr1(str[i]);
              strcpy(str[i],strings);
              i++;
          }
i=0;

while(i<=s)
{
  x=0;
	path = ROOTPATH ;
	file_path = str[i] ;
if(i>sflag[index]&&index<6)
	{
		flag++;
		pos=1; //to initialize position for next field keyword
                index++;
	}

	 //updating trie with flag as chosen by upto sflag[i] value index
	if(flag==1)
	{
	Trie :: updateTrie(str[i],flag-1,0);
	}
	else if(flag==2)
	{
        Trie :: updateTrie(str[i],flag-1,0);
	}
	else if(flag==3)
	{
	Trie :: updateTrie(str[i],flag-1,0);
	}
	else if(flag==4)
	{
	Trie :: updateTrie(str[i],flag-1,0);
	}
	else if(flag==5)
	{
	Trie :: updateTrie(str[i],flag-1,0);
	}

i++;
}


}


void* PhysicalUpdation(void *arg){
 std::clock_t start = std::clock();
         // todo the updateTrie call here
	char str[200][WORDLEN]; //taking here max 200 keywords and STRLIMIT is length of keyword
   	char value;//get char from file one by one
    int callpid=getpid();// get pid of the process
   	ifstream file;
   	ofstream wfile;
   	file.open("TrieUpdate.txt");
   	int i=0,s=0,j=0,k=0,num=1,pos=1;
        //s will increment when space is encountered and j index will give str char by char from data variable
   	int sflag[FLAG+2];
   	int numkeywords[6],it=0; //to calculate number of keywords in particular field
   	while(i<6){numkeywords[i]=1;i++;} //initialized numkeywords with 1 coz there would be atleast 1 keyword in every field
    i=0;
   	while(file.get(value))
   	{


   		if(value==' ') //if space then start new keyword
   		{
   			numkeywords[it]++;//increase count for particular field
   			s++;
   			j=0;
   		}
   		else if(value=='\n')//also flag will change with s
   		{
   			sflag[k]=s;

   			s++;
   			k++;
   			j=0;
   			it++;//to iterate to next field
   		}
   		else
   		{
   			str[s][j] = value ;
   			j++;
   		}
   	}
        file.clear();
   	file.close();
int flag=0 , m=0;
std::string path (ROOTPATH) ; //main path
std :: string file_path ;
char copy[60][100] ;
int x=0,index=0;
i=0;
while(i<=s){
              char* strings=Search::strlwr1(str[i]);
              strcpy(str[i],strings);
              i++;
          }
i=0;

char check[5]="x??";
while(i<=s)
{
  x=0;
	path = ROOTPATH ;
	file_path = str[i] ;

if(i>sflag[index]&&index<6)
	{
		flag++;
		pos=1; //to initialize position for next field keyword
    index++;
	}
int z=strcmp(check,str[i]);
if(z==0){i++; continue;}


	 //updating trie with flag as chosen by upto sflag[i] value index
    if(flag==1)
	{

		Trie :: updateTrie(str[i],flag-1,0);

		path.append("song/") ;
    	file_path+=".txt" ;
    	path+= file_path ;
	cout<<path<<endl;


        uploadfilewrite(path,string(str[0]),pos,numkeywords[flag]);
       pos++;
        strcpy(copy[m],str[i]);  //copy flag =2 i.e. song keywords to copy[] to create song files in other fields like artist,album,etc
        m++;

	}


	else if(flag==2)
	{

    Trie :: updateTrie(str[i],flag-1,0);

		std :: string temp ;
				path += string("artist/")  +string(str[i]); //converting string to const char* with help of string()

         checkdirectory(path,file_path,string(str[0]),pos,numkeywords[flag]);
				pos++;

    int pos2 =1; // for song keyword position in paricular artist folder song keyword file

    while(x<m) //this loop creates song keywords files in corresponding current artist folder
    {

    	temp = string(ROOTPATH) + string("artist/") + string(str[i]) + string("/")+string(copy[x])+string(".txt"); //concatenation
    	 uploadfilewrite(temp,string(str[0]),pos2,numkeywords[flag]);
    	pos2++;
    	x++;
    }
	}
	else if(flag==3)
	{

		Trie :: updateTrie(str[i],flag-1,0);

		std :: string temp ;
				path += string("album/")  +string(str[i]);
   checkdirectory(path,file_path,string(str[0]),pos,numkeywords[flag]);
				pos++;
    int pos2 =1;
    while(x<m)
    {
    	temp = string(ROOTPATH) + string("album/") + string(str[i]) + string("/")+string(copy[x])+string(".txt");
    	 uploadfilewrite(temp,string(str[0]),pos2,numkeywords[flag]);
    	pos2++;
    	x++;
    }



	}
	else if(flag==4)
	{
			Trie :: updateTrie(str[i],flag-1,0);

				std :: string temp ;
				path += string("band/")  +string(str[i]);
    		 checkdirectory(path,file_path,string(str[0]),pos,numkeywords[flag]);
				pos++;
    int pos2=1;
    while(x<m)
    {
    	temp = string(ROOTPATH) + string("band/") + string(str[i]) + string("/")+string(copy[x])+string(".txt");
    	 uploadfilewrite(temp,string(str[0]),pos2,numkeywords[flag]);
    	x++;
    }



	}
	else if(flag==5)
	{
			Trie :: updateTrie(str[i],flag-1,0);

		std :: string temp ;
				path +=  string("type/")  +string(str[i]);
   				 checkdirectory(path,file_path,string(str[0]),pos,numkeywords[flag]);
				pos++;
         int pos2=1;
    while(x<m)
    {
    	temp = string(ROOTPATH) + string("type/") + string(str[i]) + string("/")+string(copy[x])+string(".txt");
    	 uploadfilewrite(temp,string(str[0]),pos2,numkeywords[flag]);
    	x++;
    }

	}

i++;
}

         int logpid;
         ifstream mmlogr;
           // read update log to get pid of update process
        mmlogr.open ("TrieUpdateLog.txt");
        mmlogr>>logpid;
        mmlogr.close();
       kill(logpid,SIGALRM);
       Trie::ReleaseUpdateSemaphore();




        double st = (std::clock() - start) / (double) (CLOCKS_PER_SEC/1000);
    fstream jj;
    jj.open("executionupdate.txt",ios::app);
    jj<<st<<endl;
    jj.close();
    std::cout << "Finished in " << st << "ms" << std::endl;


}
// GTS end

// NLP replacement vector initialization
 void InitReplaceVector(){

        int items;  // enteries in the replace vector
        ifstream read;
     	read.open("ReplacementPolicy.txt");
        read>>items;
        replacements=items; // save available replacements as global data
        for(int i=0;(i<items)&& !(read.eof());i++)
        { // read all items into the array
         read>>ReplacementVector[i][0];
         read>>ReplacementVector[i][1];
	} 
	read.close();
   }
   
   
   
//INTERFACING FUNCTION : LINKING FOR COMMUNICATION VIA SIGNALS 
void CoreSearchSignal(int signo)
  {
           
    // PARENT PROCESS
   int callpid=getpid();
  if(signo==SIGCHLD && callpid==PARENTID){
           int status;
  // create new CoreSearch process as child has died
              int d=wait(&status); // wait for process to get removed from kernel data structure
               int k=fork();
                 if(k==0)
                   {
                    //signals handeled by child
                    signal(SIGCHLD,CoreSearchSignal);
                    signal(SIGALRM,CoreSearchSignal);
                    signal(SIGUSR1,CoreSearchSignal);
                    signal(SIGUSR2,CoreSearchSignal);
                    signal(SIGPROF,CoreSearchSignal);

                    //child id in the log (for Search)
                   int childpid=getpid();
                   CHILDID=childpid;
                   ofstream logpid;
                   logpid.open("SearchPID.txt");
                   logpid.clear();
                   logpid<<childpid;
                   logpid.close();                  
                    // release semaphores
                   Trie::ReleaseUpdateSemaphore();
                   Trie::ReleaseSearchSemaphore();                   
                   system("nohup sleep infinity");
                  }


               }
   else if(signo==SIGUSR1){
           // update trie deployment
         int err;
        if(callpid==PARENTID)
        {
         LogicalUpdation();
         // UPDATE CHILD PROCESS
         kill(CHILDID,SIGUSR1);

        }
        else if(callpid==CHILDID)
        {
         pthread_t tid;
         int err;
         err = pthread_create(&(tid), NULL, &PhysicalUpdation, NULL);
           if(err!=0)
            {
             int pid;
             ifstream mmlogr;
             // read update log to get pid of update process
             mmlogr.open ("TrieUpdateLog.txt");
             mmlogr>>pid;
             mmlogr.close();
             Trie::ReleaseUpdateSemaphore();
             kill(pid,SIGUSR1);
            }

        }

      }


        else if(signo==SIGALRM){
         // search execution
         int err;
        if(callpid==PARENTID)
        {   cout<<"Logical Search by Parent\n";
            LogicalSearch();
            // Search CHILD PROCESS
            kill(CHILDID,SIGALRM);

        }
        else if(callpid==CHILDID)
        { cout<<"Physical Search by Child\n";
         pthread_t tid;
         int err;
         err = pthread_create(&(tid), NULL, &PhysicalSearch, NULL);
           if(err!=0)
            {
             int pid;
             ifstream mmlogr;
             // read update log to get pid of update process
             mmlogr.open ("MiddleManLog.txt");
             mmlogr>>pid;
             mmlogr.close();
             Trie::ReleaseSearchSemaphore();
             kill(pid,SIGUSR1);
            }

         }
        }

   else if(signo==SIGUSR2){
           // trie reconstruction    
    
         cout<<"Reconstruction working\n";
        if(callpid==PARENTID)
        {
            // lock everything
            Trie::LockSearchSemaphore(); 
            Trie::LockUpdateSemaphore(); //Test();
            Trie::LockReconstructSemaphore();
            Test();
            // construct trie
            Trie::TrieReconstruct();
         // UPDATE CHILD PROCESS
         kill(CHILDID,SIGUSR2);
        }
        else if(callpid==CHILDID)
        {
         Trie::TrieReconstruct();
         ifstream fwr;
         int mmid;
         fwr.open("TrieReconstructLog.txt");
         fwr>>mmid;
         fwr.close();
         kill(mmid,SIGALRM);
        }

      }
  else if(signo==SIGPROF)
     {
       cout<<"updating preference\n";
       // preference
        char word[WORDLEN];
        int mmid,flag;
        float pref;
        ifstream fr;
        // read mmid and word
        fr.open("PrefWord.txt"); // mmid, word, flag(any flag which is 1 , pref
        fr>>mmid;  fr>>word;   fr>>flag;    fr>>pref;
        fr.close();
        Trie::updateTrie(word,flag,pref);        
        // send signal
        if(callpid==PARENTID)
        { kill(CHILDID,SIGPROF);}
         // terminate middle man successfully updated
        if(callpid==CHILDID)
        {
         Trie::ReleasePrefSemaphore();
         kill(mmid,SIGALRM);       
        }
     }

   else if(signo==SIGURG)
     {
        cout<<"Build Preference Tree\n";  
        PrefNode prefnode; 
        if(callpid==PARENTID) // send signal to child
        {  kill(CHILDID,SIGURG);}   
        Trie::BuildPrefTrie(root,&prefnode,0);
        cout<<"Build Finished..";
        }
}


int main()
{
 
     // signals handled by parent
     signal(SIGCHLD,CoreSearchSignal);
     signal(SIGALRM,CoreSearchSignal);
     signal(SIGUSR1,CoreSearchSignal);
     signal(SIGUSR2,CoreSearchSignal);
     signal(SIGPROF,CoreSearchSignal);
     signal(SIGURG,CoreSearchSignal);

// initialize Replacement vector for NLP
     InitReplaceVector();
     // parent id in the search parent log
     int parentid=getpid();
     PARENTID=parentid;
// write parent id in file
     ofstream logppid;
      logppid.open("SearchPPID.txt");
      logppid.clear();
      logppid<<parentid;
      logppid.close();

       if(start==0)
       {// create trie first time
         Trie::createTrie();
         start=1;
       }

// write root node location   (extra not of practical use unitl now)
        ofstream tr;
        tr.open("TrieAddress.txt");
        tr.clear();
        tr<<root;
        tr.close();
     int k=fork();
      if(k==0)
       {
         // construct semaphore and initialize
       Trie::ConstructSemaphore();
       Trie::LockReconstructSemaphore() ;// search remains locked initially
         struct rlimit rlim;//to check resources of process
         //child id in the log (for Search)
        signal(SIGCHLD,CoreSearchSignal);
        signal(SIGALRM,CoreSearchSignal);
        signal(SIGUSR1,CoreSearchSignal);
        signal(SIGUSR2,CoreSearchSignal);
        signal(SIGPROF,CoreSearchSignal);
	signal(SIGURG,CoreSearchSignal);
        int childpid=getpid();
        CHILDID=childpid;

        // write search pid
        ofstream logpid;
        logpid.open("SearchPID.txt");
        logpid.clear();
        logpid<<childpid;
        logpid.close();
        system("nohup sleep infinity");// pause the process until request arrives
       }
      else
      {
        CHILDID=k;
        system("nohup sleep infinity"); // pause the process until request arrives
      }
  return 0;
}










