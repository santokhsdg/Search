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
#include<string.h>
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
#include<stdlib.h>
#include<fstream>

void CoreSignal(int signo)
{
	if(signo==SIGALRM){ cout<<"1";  exit(EXIT_SUCCESS);}
   else if(signo==SIGUSR1){ cout<<"0";  exit(EXIT_FAILURE);}
}


int main(int argc, char* updateString[]) {

  signal(SIGALRM,CoreSignal);
  signal(SIGUSR1,CoreSignal);

  ofstream mmlogw; // writer
  ifstream mmlogr;  // reader
  sem_t * sema;
  int val,pid,logpid,coreppid;
  char path[50];

// get pid of core update parent process
mmlogr.open ("SearchPPID.txt");
mmlogr>>coreppid;
mmlogr.close();


// open semaphore
sema = sem_open("/rproSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
sem_getvalue(sema,&val);
// if previous process has crashed
mmlogr.open("TrieReconstructLog.txt");
mmlogr>>logpid;
mmlogr.close();
sprintf(path,"%s%d","/proc/",logpid);
    struct stat sts;

    if (stat(path, &sts) == -1 && errno == ENOENT && val==0)
      {
         //for:: previous process has died or crashed
///////////////////////////////////////////////////////
         // write pid before entering the input file
        pid=getpid();
        mmlogw.open("TrieReconstructLog.txt");
        mmlogw.clear();
        mmlogw<<pid;
        mmlogw.close();
        // write content in file
        mmlogw.open ("TrieReconstruct.txt");
        mmlogw.clear();
        for(int i=1;i<argc;i++)
       {
        // write input file format of reading php arguments: DONE  WORD  FLAG0 FLAG1 FLAG2 FLAG3 FLAG4 PREF 
        mmlogw<<updateString[i]<<"\n";
       }

        mmlogw.close();
        //cout<<"signal2";
        kill(coreppid,SIGUSR2);
      }
    else
      {
        sem_wait(sema);// wait until file is free to write
        // write pid before entering the input file
        pid=getpid();
        mmlogw.open("TrieReconstructLog.txt");
        mmlogw.clear();
        mmlogw<<pid;
        mmlogw.close();
        // write content in file
        mmlogw.open ("TrieReconstruct.txt");
        mmlogw.clear();
        for(int i=1;i<argc;i++)
        {
        // write input file format of reading php arguments:  DONE  WORD  FLAG0 FLAG1 FLAG2 FLAG3 FLAG4 PREF ::   ; | means enter(line ends \n)
         mmlogw<<updateString[i]<<"\n";
        }
         mmlogw.close();
         //cout<<"signal1";
         kill(coreppid,SIGUSR2);
      }
sem_close(sema);
sleep(2);// pause the process until result arrives
cout<<"0"; // process fails
// open semaphore
sema = sem_open("/rproSema",O_CREAT, S_IRUSR | S_IWUSR, 1);
sem_getvalue(sema,&val);
if(val==0){sem_post(sema);}
sem_close(sema);
return 0;

}






















