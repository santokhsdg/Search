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
// SEARCH MIDDLE MAN, TO BE EXECUTED BY PHP WITH ARGUMENTS


#include<iostream>
using namespace std;
#include<semaphore.h>
#include<errno.h>
#include<fcntl.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdarg.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/stat.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<fstream>



void CoreSignal(int signo)
{
	if(signo==SIGALRM){ cout<<"1";  exit(EXIT_SUCCESS);}
    if(signo==SIGUSR1){ cout<<"0";  exit(EXIT_FAILURE);}
}


int main(int argc, char* searchString[]) {

  signal(SIGALRM,CoreSignal);
  signal(SIGUSR1,CoreSignal);

  ofstream mmlogw; // writer
  ifstream mmlogr;  // reader
  sem_t * sema;
  int val,pid,logpid,coreppid;
  char path[50];

// get pid of core search process
mmlogr.open ("SearchPPID.txt");
mmlogr>>coreppid;
mmlogr.close();
////////////////////////////
// check reconstruction semaphore
sema = sem_open("/reconSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
sem_getvalue(sema,&val);
if(val==0){ cout<<"0";  exit(EXIT_FAILURE);} // if reconstruction working, no request will proceed
sem_close(sema);
////////////////////////////
// open semaphore
sema = sem_open("/searchSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
sem_getvalue(sema,&val);
//cout<<"sema:"<<val<<"\n";
// if previous process has crashed
mmlogr.open ("MiddleManLog.txt");
mmlogr>>logpid;
mmlogr.close();
sprintf(path,"%s%d","/proc/",logpid);
    struct stat sts;
   if (stat(path,&sts) == -1 && errno == ENOENT && val==0)
      {
         // previous process has died or crashed
         // write pid before entering the input file
        pid=getpid();
        mmlogw.open ("MiddleManLog.txt");
        mmlogw.clear();
        mmlogw<<pid;
        mmlogw.close();
        // write search input to the input file
        mmlogw.open("SearchInput.txt");
        mmlogw.clear();
      //  mmlogw<<argc<<"\n";
        mmlogw<<pid<<"\n";
        mmlogw<<searchString[1]<<"\n"; //event
        mmlogw<<searchString[2]<<"\n"; //reqid
        for(int i=3;i<argc;i++)
       { mmlogw<<searchString[i]<<" "; }
       mmlogw<<"\n";
        mmlogw.close();
        kill(coreppid,SIGALRM);
        //cout<<"signal1\n";
      }
    else
      {
        sem_wait(sema);// wait until file is free to write
        //sem_getvalue(sema,&val);
        //cout<<"sema:"<<val<<"\n";
        // write pid before entering the input file
        pid=getpid();
        mmlogw.open ("MiddleManLog.txt");
        mmlogw.clear();
        mmlogw<<pid;
        mmlogw.close();
        // write search input to the input file
        mmlogw.open ("SearchInput.txt");
        mmlogw.clear();
       // mmlogw<<argc<<"\n"; // no of arguments in the file
        mmlogw<<pid<<"\n";
        mmlogw<<searchString[1]<<"\n"; //event
        mmlogw<<searchString[2]<<"\n"; //reqid
        for(int i=3;i<argc;i++)
        {
        mmlogw<<searchString[i]<<" ";// write input file
        }
        mmlogw<<"\n";
        mmlogw.close();
        kill(coreppid,SIGALRM);
      }
//sem_getvalue(sema,&val);
//cout<<"sema:"<<val<<"\n";
sem_close(sema);
sleep(2); // pause the process until result arrives
cout<<"0";// process fails
sema = sem_open("/searchSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
sem_getvalue(sema,&val);
if(val==0){sem_post(sema);}
sem_close(sema);
return 0;

}






















