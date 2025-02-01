#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<chrono>
#include <unistd.h>
#include <random>
#include<cTime>

using namespace std;

int N;
int grpSize,grpCnt,w,x,y;
int psStatus[4];
int *grpCompletedCnt;
int readerCnt=0;
int subCnt=0;
pthread_mutex_t consoleMtx;
pthread_mutex_t psMtx[4];
pthread_mutex_t *grpCompletedCntMtx;
sem_t *bindStart;
sem_t bindingStations;
sem_t entryBook;
pthread_mutex_t *students;
pthread_mutex_t rcMtx;

pthread_mutex_t seedMtx;
time_t startTime;
unsigned seed =777;

bool verbose=false;
unsigned randomDelay(int d)
{
    // Use a Mersenne Twister engine for better randomness
    pthread_mutex_lock(&seedMtx);

    unsigned seed = seed*1664525+1013904223;
    seed=seed>>24;
    seed+=std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::poisson_distribution<int> distribution (d);

    pthread_mutex_unlock(&seedMtx);

    unsigned t= distribution(generator);
    // cout<<t<<endl;
    return (t%37);

}
void * printingThreadFnc(void * arg)
{
    int sId = *((int*)arg);
    int grpId = (sId-1) / grpSize + 1;
    int psId = sId%4+1;

    sleep(randomDelay(7));

    int gotAccess=0;
    auto now = std::chrono::system_clock::now();
    time_t currTime= std::chrono::system_clock::to_time_t(now);

    pthread_mutex_lock(&consoleMtx);
    cout<<"Student "<<sId<<" has arrived at the print station at time "<<(currTime-startTime)<<endl;
    pthread_mutex_unlock(&consoleMtx);

    while(gotAccess==0)
    {

        pthread_mutex_lock(&students[sId-1]);

        pthread_mutex_lock(&psMtx[psId-1]);
        if(psStatus[psId-1]==1)
        {
            psStatus[psId-1]=2;
            gotAccess=1;

            if(verbose)
            {
                auto now = std::chrono::system_clock::now();
                time_t currTime= std::chrono::system_clock::to_time_t(now);
                pthread_mutex_lock(&consoleMtx);
                cout<<"Student "<<sId<<" got access to "<<"ps "<<psId<<" at time "<<(currTime-startTime)<<endl;
                pthread_mutex_unlock(&consoleMtx);
            }

        }
        else
        {
            if(verbose)
            {
                auto now = std::chrono::system_clock::now();
                time_t currTime= std::chrono::system_clock::to_time_t(now);
                pthread_mutex_lock(&consoleMtx);
                cout<<"Student "<<sId<<" back to wait for ps: "<<psId<<" at time "<<(currTime-startTime)<<endl;
                pthread_mutex_unlock(&consoleMtx);
            }



        }
        pthread_mutex_unlock(&psMtx[psId-1]);
        if(gotAccess)
        {
            // printing time
            sleep(w);
            auto now = std::chrono::system_clock::now();
            time_t currTime= std::chrono::system_clock::to_time_t(now);

            pthread_mutex_lock(&consoleMtx);
            cout<<"Student "<<sId<<" has finished printing at time "<<(currTime-startTime)<<endl;
            pthread_mutex_unlock(&consoleMtx);

            pthread_mutex_lock(&grpCompletedCntMtx[grpId-1]);
            grpCompletedCnt[grpId-1]++;
            if(grpCompletedCnt[grpId-1]==grpSize)
            {
                sem_post(&bindStart[grpId-1]);
            }
            pthread_mutex_unlock(&grpCompletedCntMtx[grpId-1]);

            pthread_mutex_lock(&psMtx[psId-1]);
            psStatus[psId-1]=1;
            pthread_mutex_unlock(&psMtx[psId-1]);
            int i = (grpId-1)*grpSize+1;
            int l = (grpId)*grpSize;

            int j=N;
            while(j--)
            {
                if(i>l&&l!=-1)
                {
                    // Inform other group members after a delay to prioritize self
                    sleep(randomDelay(2));
                    l=-1;
                }
                if(i>N)
                    i=i%N;


                if((i)%4+1==psId)
                {
                    pthread_mutex_unlock(&students[i-1]);
                }

                i++;
            }


        }


    }

    return NULL;

}

void * bindingThreadFnc(void * arg)
{
    int sId = *((int*)arg);
    int grpId = (sId-1) / grpSize + 1;
    if(verbose)
    {
        pthread_mutex_lock(&consoleMtx);
        auto now = std::chrono::system_clock::now();
        time_t currTime= std::chrono::system_clock::to_time_t(now);
        cout<<"Group "<<grpId<<" waiting for their printing to be done "<<(currTime-startTime)<<endl;
        pthread_mutex_unlock(&consoleMtx);
    }

    sem_wait(&bindStart[grpId-1]);

    auto now = std::chrono::system_clock::now();
    time_t currTime= std::chrono::system_clock::to_time_t(now);
    pthread_mutex_lock(&consoleMtx);
    cout<<"Group "<<grpId<<" has finished printing at time "<<(currTime-startTime)<<endl;
    pthread_mutex_unlock(&consoleMtx);
    sem_wait(&bindingStations);
    now = std::chrono::system_clock::now();
    currTime= std::chrono::system_clock::to_time_t(now);

    pthread_mutex_lock(&consoleMtx);
    cout<<"Group "<<grpId<<" has started binding at time "<<(currTime-startTime)<<endl;
    pthread_mutex_unlock(&consoleMtx);
    //Binding going On ...
    sleep(x);
    now = std::chrono::system_clock::now();
    currTime= std::chrono::system_clock::to_time_t(now);

    pthread_mutex_lock(&consoleMtx);
    cout<<"Group "<<grpId<<" has finished binding at time "<<(currTime-startTime)<<endl;
    pthread_mutex_unlock(&consoleMtx);
    sem_post(&bindingStations);


    //go to Library for submit...
    sleep(randomDelay(3));

    now = std::chrono::system_clock::now();
    currTime= std::chrono::system_clock::to_time_t(now);

    pthread_mutex_lock(&consoleMtx);
    cout<<"Group "<<grpId<<" reached library to submit at time "<<(currTime-startTime)<<endl;
    pthread_mutex_unlock(&consoleMtx);

    sem_wait(&entryBook);
    now = std::chrono::system_clock::now();
    currTime= std::chrono::system_clock::to_time_t(now);
    if(verbose)
    {
        pthread_mutex_lock(&consoleMtx);
        cout<<"Group "<<grpId<<" has started submitting at time "<<(currTime-startTime)<<endl;
        pthread_mutex_unlock(&consoleMtx);
    }

    //writting on ...
    sleep(y);
    now = std::chrono::system_clock::now();
    currTime= std::chrono::system_clock::to_time_t(now);
    subCnt++;
    pthread_mutex_lock(&consoleMtx);
    cout<<"Group "<<grpId<<" has submitted at time "<<(currTime-startTime)<<endl;
    pthread_mutex_unlock(&consoleMtx);
    sem_post(&entryBook);

    return NULL;
}

void * readerThreadFnc(void * arg)
{
    int rId = *((int*)arg);
    bool readAgain=true;
    //Sleep before starting reading

    while(readAgain)
    {
        //Wait for an interval before try to read
        sleep(randomDelay(7));
        pthread_mutex_lock(&rcMtx);
        readerCnt+=1;
        if(readerCnt==1) sem_wait(&entryBook);
        pthread_mutex_unlock(&rcMtx);

        //Reading ...
        auto now = std::chrono::system_clock::now();
        time_t currTime= std::chrono::system_clock::to_time_t(now);
        sleep(y);

        pthread_mutex_lock(&consoleMtx);
        cout<<"Staff "<<rId<<" has started reading the entry book at time "<<(currTime-startTime)<<". No. of submission = "<<subCnt<<endl;
        pthread_mutex_unlock(&consoleMtx);

        if(subCnt==grpCnt) readAgain=false;

        pthread_mutex_lock(&rcMtx);
        readerCnt--;
        if(readerCnt==0)
            sem_post(&entryBook);
        pthread_mutex_unlock(&rcMtx);

    }

    return NULL;
}
void init()
{
    auto now = std::chrono::system_clock::now();
    startTime= std::chrono::system_clock::to_time_t(now);
    pthread_mutex_init(&consoleMtx,NULL);
    pthread_mutex_init(&seedMtx,NULL);

    pthread_mutex_lock(&consoleMtx);

    cin>>N>>grpSize;
    cin>>w>>x>>y;
//   N=24;
    // grpSize=8;
    //w=x=y=2;
    pthread_mutex_unlock(&consoleMtx);
    grpCnt=N/grpSize;

    students = new pthread_mutex_t[N];
    grpCompletedCnt= new int[grpCnt];
    grpCompletedCntMtx=new  pthread_mutex_t[grpCnt];
    bindStart = new sem_t [grpCnt];


    for(int i=0; i<N; i++)
    {
        pthread_mutex_init(&students[i],NULL);
    }
    for(int i=0; i<grpCnt; i++)
    {
        grpCompletedCnt[i]=0;
        pthread_mutex_init(&grpCompletedCntMtx[i],NULL);
        sem_init(&bindStart[i],0,0);
    }
    pthread_mutex_init(&rcMtx,NULL);
    sem_init(&bindingStations,0,2);
    sem_init(&entryBook,0,1);
    for(int i=0; i<4; i++)
    {
        pthread_mutex_init(&psMtx[i],NULL);
    }

    for(int i=0; i<4; i++)
    {
        psStatus[i]=1;
    }
}
int main()
{
    //for(int i=0;i<5;i++)
    //  cout<<randomDelay(71)<<endl;;
    init();
    // Verbose
    verbose=false;


    vector<int>v;
    for(int id=1; id<=N; id++)
    {
        v.push_back(id);

    }

    for (int i = N - 1; i > 0; --i)
    {
        int j = std::rand() % (i + 1);
        swap(v[i], v[j]);

    }

    vector<pthread_t> Threads;
    vector<int*> ptrs;
    for(int i=0; i<N; ++i)
    {
        pthread_t thread;
        int* stId = new int(v[i]);
        pthread_create(&thread,NULL,printingThreadFnc,stId);
        Threads.push_back(thread);
        ptrs.push_back(stId);

    }

    for(int i= grpSize; i<=N; i+=grpSize)
    {
        pthread_t thread;
        int* stId = new int(i);
        pthread_create(&thread,NULL,bindingThreadFnc,stId);
        Threads.push_back(thread);
    }
    int staff1=1;
    pthread_t thread;
    pthread_create(&thread,NULL,readerThreadFnc,&staff1);
    Threads.push_back(thread);
    int staff2=2;
    pthread_t thread2;
    pthread_create(&thread2,NULL,readerThreadFnc,&staff2);
    Threads.push_back(thread);

    for (auto& thread : Threads)
    {
        pthread_join(thread, NULL);
    }


    //delete allocated mem

    for(int i=0; i<N; i++)
    {
        delete ptrs[i];
    }

}
