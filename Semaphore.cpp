//compilar: g++ -std=c++11 -o Semaphore Semaphore.cpp -lpthread

#include <iostream>           
#include <pthread.h>             
#include <mutex>              
#include <condition_variable>
#include<time.h>          //utilizado para inicializar a "pilha" de numeros aleatorios
#include<stdlib.h>        //utilizado para geração de numeros aleatorios
#include<sstream>
#include<chrono>

using namespace std::chrono;

//Implementacao de um semaforo contador
class Semaphore
{
    private:
        std::mutex mtx;               
        std::condition_variable cv;
        int N;
    
    public:
        Semaphore(){N = 0;}
        Semaphore(int n) : N(n){}
        
        void Wait(){
            std::unique_lock<std::mutex> lck(mtx);
            while(N == 0) cv.wait(lck);
            N--;
        }
        
        void Signal(){
            std::unique_lock<std::mutex> lck(mtx);
            N++;
            cv.notify_one();
        }
    
};

bool termThread = false;    //flag para encerrar as threads
long int counter = 0;       //contador de numeros processados pelo consumidor
int N = 32;                  //tamanho do vetor correspondente a memoria compartilhada
int* sharedMemory;          //vetor compartilhado pelas threads
class Semaphore full;       
class Semaphore empty(N);   //inicialmente todas as posicoes da memoria compartilhada estao livres
std::mutex m;               //controle de acesso a regiao critica




//gera um numero inteiro aleatorio no intervalo [1,10000000]
int randNumber()
{
    return rand()%10000000 + 1;      
}



//Retorna true se o parametro num for primo. Caso contrario, retorna false
bool isPrime(int num)
{
	if(num == 0 || num == 1) return false;
	if(num == 2 || num == 3) return true;
	for(int i = 2; i < num/2 ; i++)
	{
		if( (num % i) == 0) return false;
	}

	return true;
}



//Retorna a primeira posicao livre da memoria compartilhada
//Caso nao haja posicao livre, retorna -1
int getFreePosition(int* sharedMem,int size)
{
    for(int i = 0; i < size; i++)
    {
        if(sharedMem[i] == 0) return i;
    }
    return -1;
}

//Retorna a primeira posicao ocupada da memoria compartilhada
//Caso nao haja posicao ocupada, retorna -1
int getBusyPosition(int* sharedMem,int size)
{
    for(int i = 0; i < size; i++)
    {
        if(sharedMem[i] != 0) return i;
    }
    return -1;
}


//Produtor
void* producer(void* param)
{
    int* vector = (int*)param;
    int freePos;
    while(!termThread)
    {
        freePos = getFreePosition(vector,N);
            empty.Wait();
                m.lock();
                //so coloca numero no vetor se houver posicao livre
                if(freePos != -1)
                        vector[freePos] = randNumber();                
                m.unlock();
            full.Signal();
 
        
    }
}

//Consumidor
void* consumer(void* param)
{
    int* vector = (int*)param;
    int busyPos;
    while(!termThread)
    {
            full.Wait();

                m.lock();
                busyPos = getBusyPosition(vector, N);

                if(busyPos != -1)
                    std::cout<<vector[busyPos]<<": "<<isPrime(vector[busyPos])<<"\n";
                vector[busyPos] = 0; //libera posicao do vetor

                counter++;           //incrementa contador de numeros processados pelo consumidor
                m.unlock();
            empty.Signal();
    }
}


using namespace std;
int main(int argc, char* argv[])
{
    int np;
    int nc;

    //Verifica se usuario passou numeros de threads produtoras e consumidoras como parametros
    if(argc < 3)
    {
        std::cout<<"Np e Nc devem ser passadas como parametro\n";
        return -1;
    }

    std::stringstream(argv[1])>>np;
    std::stringstream(argv[2])>>nc;
    

    srand(time(NULL));

    sharedMemory = new int[N];
    pthread_t producers[np];
    pthread_t consumers[nc];

    //comeca contagem do tempo
    high_resolution_clock::time_point start = high_resolution_clock::now(); 

    //Inicializa produtores 
    for (int order = 0; order < np; order++)
    {
        pthread_create(&producers[order], NULL, producer, sharedMemory);
    }

    //Inicializa consumidores          
    for(int order = 0; order < nc; order++)
    {
        pthread_create(&consumers[order], NULL, consumer, sharedMemory);
    }

    //programa termina quando counter > 10000
    while(counter < 1000);
    termThread = true;

    //termina contagem do tempo
    high_resolution_clock::time_point stop = high_resolution_clock::now();

    //calcula diferenca de tempo  
    duration<double> time_span = duration_cast<duration<double>>(stop - start);   
    
    std::cout<<"\n\nTempo decorrido: "<<time_span.count()<<"s\n\n"; 

    pthread_exit(NULL);
    return 0;
}

