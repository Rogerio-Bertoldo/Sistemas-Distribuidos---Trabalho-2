//compilar: g++ -std=c++11 -o spinLock spinLock.cpp -lpthread

#include<iostream>
#include<atomic>          //std::atomic_flag
#include<pthread.h>
#include<time.h>          //utilizado para inicializar a "pilha" de numeros aleatorios
#include<stdlib.h>        //utilizado para geração de numeros aleatorios
#include<sstream>
#include<chrono>          //utilizado para marcar tempo de execucao

using namespace std;
using namespace std::chrono;

std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;

//numero de threads
long int numThreads; 

//armazena o resultado da soma      
long int n = 0;   

//quantidade de numeros aleatorios a serem gerados         
long int num = 10000000;  

//armazena os numeros aleatorios gerados   
int* array;                

//gera numeros (inteiros) aleatorios no intervalo [-100,100]
int* randNumbers(int totalNumeros)
{
    int* numArray = new int[totalNumeros];
    srand(time(NULL));
    
    for(int i = 0; i < totalNumeros; i++)
    {
        //srand(time(NULL)); nao deixar aqui
        numArray[i] = rand()%200 - 100;
        
    }

    return numArray;
}

//--------funcoes do semaforo-----------//

void aquire()
{
    while(lock_stream.test_and_set());
}

void release()
{
    lock_stream.clear();
}

//--------funcoes do semaforo-----------//



//armazena os indices(posicoes inicial e final) da parcela (do array de numeros aleatorios) pertencente a uma thread
struct IndexInfo
{
    int index;
    int lastIndex;
};

//Adiciona numero
void* addNumber(void* x)
{

    struct IndexInfo* ind = (struct IndexInfo*) x;

    for(int i = ind->index; i < ind->lastIndex; i++)
    {
        aquire();
        n+= *(array + i);
        release();
    }

    
}



int main(int argc, char* argv[])
{
    //Verifica se o usuario passou o numero de threads como parametro
    if(argc < 2)
    {
        cout<<"Numero de threads deve ser passado como parametro\n";
        return -1;
    }

    //armazena valor do parametro passado em numThreads
    stringstream(argv[1])>>numThreads;
    
    //armazena numero de numeros
    stringstream(argv[2])>>num;

    //armazena numeros aleatorios na variavel array
    array = randNumbers(num);   

    struct IndexInfo data[numThreads];    
    pthread_t threads[numThreads];

    //comeca contagem do tempo
    high_resolution_clock::time_point start = high_resolution_clock::now();    

    for(int i = 0; i < numThreads; i++)
    {
        //cada thread fica responsavel por uma parcela do array de numeros aleatorios
        data[i].index = i*num/numThreads;
        data[i].lastIndex = data[i].index + num/numThreads;

        if(i == numThreads - 1)
        { 
            /*caso a divisao "quantidade de numeros aleatorios / quantidade de threads" tenha resto diferente de zero, 
              a utltima thread criada fica responsavel por uma parcela maior do array de numeros aleatorios
            */
            data[i].lastIndex += (num % numThreads); 
        }

        pthread_create(&threads[i], NULL, addNumber,(void*)&data[i]);
        
    }


    for(int i = 0; i < numThreads; i++)
    {
        //thread principal espera finalizacao das demais threads antes de prosseguir
        pthread_join(threads[i], NULL); 
    }

    //termina contagem do tempo
    high_resolution_clock::time_point stop = high_resolution_clock::now();

    //calcula diferenca de tempo  
    duration<double> time_span = duration_cast<duration<double>>(stop - start);   
    
    cout<<"\nTempo decorrido: "<<time_span.count()<<"s\n"; 
    cout<<"Resultado soma: "<<n<<"\n\n";

    pthread_exit(NULL);
    return 0;
    
}
