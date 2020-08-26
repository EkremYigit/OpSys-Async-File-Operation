//2015510075 ekrem yigit
#include <fcntl.h> /* open, O_RDONLY */
#include <unistd.h> /* close */
#include <stdio.h> /* fprintf, perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h> /* memset */
#include <aio.h> /* aio_read, aio_error, aio_return */
#include <errno.h> /* EINPROGRESS */
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#define SIZE 80 
//  /

struct arg_struct { 
		  
    char* fp_read;
    char* fp_write;
    int thread_bytesize;
    int thread_no;
    int thread_offset;
    
};

pthread_mutex_t lock; 




void *thread_runner(void*argg){
	int T_fd,T_fd2,T_err,T_ret;
	struct aiocb aio_tr,aio_tw;
	struct arg_struct *machine;
	machine=(struct arg_struct*)malloc(sizeof(struct arg_struct));
	
	machine=(struct arg_struct*)argg;
	
	
	char read_buf[machine->thread_bytesize];

	T_fd=open(machine->fp_read, O_RDONLY);
	if (T_fd == -1) {
        perror("open");
        exit(2);
    }
	sleep(2);
	memset(&aio_tr,0,sizeof(aio_tr));
	memset(&read_buf,0,sizeof(read_buf));
	aio_tr.aio_fildes=T_fd;
	aio_tr.aio_offset=(int)machine->thread_offset;//offset using in here.
	
	printf ("Thread %d:Progress %22 Loading... \n",machine->thread_no);
	aio_tr.aio_buf=read_buf;//data are read in here according to byte for each thread
	aio_tr.aio_nbytes=sizeof(read_buf);
	
	aio_read(&aio_tr);
	while (aio_error(&aio_tr) == EINPROGRESS) { }
	T_err = aio_error(&aio_tr);
	T_ret = aio_return(&aio_tr);

    if (T_err != 0) {
        printf ("Error at aio_error() : %s\n", strerror (T_err));
        close (T_fd);
        exit(2);
    }

    if (T_ret != sizeof(read_buf)) {
        printf("Error at aio_return()\n");
        close(T_fd);
        exit(2);
    }
	fprintf(stdout,"Thread %d:Read Value :: %s\n",machine->thread_no,read_buf);

    printf ("Thread %d:Progress %55 \n",machine->thread_no); //progress step for thread


	
	
	
	
	pthread_mutex_lock(&lock);//writing progress can't be divided
	T_fd2 = open (machine->fp_write, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (T_fd2 == -1) {
        perror("open");
        return 0;
    }
	
    memset(&aio_tw, 0, sizeof(aio_tw));
    aio_tw.aio_fildes = T_fd2;
    aio_tw.aio_offset=(int)machine->thread_offset;
    aio_tw.aio_buf = read_buf;
	sleep(1/2);
    aio_tw.aio_nbytes = sizeof(read_buf);
    if (aio_write(&aio_tw) == -1) {
        printf(" Error at aio_write(): %s\n", strerror(T_err));
        close(T_fd2);
        exit(2);
    }
    while (aio_error(&aio_tw) == EINPROGRESS) { }
    T_err = aio_error(&aio_tw);
    T_ret = aio_return(&aio_tw);

    if (T_err != 0) {
        printf ("Error at aio_error() : %s\n", strerror (T_err));
        close (T_fd2);
        exit(2);
    }

    if (T_ret != sizeof(read_buf)) {
        printf("Error at aio_return()\n");
        close(T_fd2);
        exit(2);
    }
	fprintf(stdout,"Thread %d Write  Value :: %s\n",machine->thread_no,read_buf);
    printf ("Thread %d:Progress %100 \n",machine->thread_no);//read and write progresses are done. %100
	pthread_mutex_unlock(&lock);
    	
	
	
}


int main(int argc, char * argv[]){ 
        struct aiocb aio_r,aio_w;
        int fd,err,ret;
	int r,k,s ;
	int byte_for_thread;
	srand(time(NULL));
	r = rand(); 
	int number_of_threads=atoi(argv[3]); //number of threads prepare with argv 3 input.
	pthread_t machines[number_of_threads];
 	struct arg_struct *T_struct=(struct arg_struct*)malloc(sizeof(struct arg_struct)*number_of_threads);//
	
	
	
	char data[SIZE];
	int temp=0;
	srand(time(NULL));
	s=rand();
	for(k=0;k<SIZE;k++){
	if((k+s)%6==0){
	
	data[k]='a';
}
	if((k+s)%6==1){
	data[k]='b';
}
	if((k+s)%6==2){
	data[k]='c';
}
	if((k+s)%6==3){
	
	data[k]='d';
}
	if((k+s)%6==4){
	data[k]='e';
}
	if((k+s)%6==5){
	data[k]='X';
}
	s=rand();
}
	//printf("Data %s\n",data);
	int len=strlen(data);
	//printf("Length=%d\n size in bytes =%d\n",len,sizeof(data));


	fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

        if (fd == -1) {
            perror("open");
            return EXIT_FAILURE;
        }
	memset(&aio_w, 0, sizeof(aio_w));
        aio_w.aio_fildes = fd;
        aio_w.aio_buf = data; //Data of source file are added in here.
        aio_w.aio_nbytes = sizeof(data);

	if (aio_write(&aio_w) == -1) {
            printf(" Error at aio_write(): %s\n", strerror(errno));
            close(fd);
            exit(2);
        }
        while (aio_error(&aio_w) == EINPROGRESS) { }
        err = aio_error(&aio_w);
        ret = aio_return(&aio_w);

        if (err != 0) {
            printf ("Error at aio_error() : %s\n", strerror (err));
            close (fd);
            exit(2);
        }

        if (ret != sizeof(data)) {
            printf("Error at aio_return()\n");
            close(fd);
            exit(2);
        }

   
        printf ("Source File Is Done Successfully!\n");
	printf("Total Byte of Source File : %d\n",sizeof(data));
	printf("Number of Thread : %d\n",number_of_threads);
       
      
 	
	byte_for_thread=sizeof(data)/number_of_threads;
	printf("Byte For Each Thread :  %d\n",byte_for_thread);
	
	int b=0;
	for(k=0;k<number_of_threads;k++){
	
	T_struct[k].fp_read=argv[1];
	T_struct[k].fp_write=argv[2];
	T_struct[k].thread_bytesize=byte_for_thread;
	T_struct[k].thread_no=(k+1);
	T_struct[k].thread_offset=(k)*byte_for_thread;

	//For debug:
	//printf("Thread_offset :%d\n",T_struct[k].thread_offset);      
	//printf("fpread : %s\n",T_struct[k].fp_read);
	//printf("fpwrite : %s\n",T_struct[k].fp_write);
	//printf("ThreadNo :%d\n",T_struct[k].thread_no);
	//printf("Bytesizeofthread :%d\n",T_struct[k].thread_bytesize);
}


	for(k=0;k<number_of_threads;k++){
	
 pthread_create(&machines[k],NULL,thread_runner,(void*)&T_struct[k]);
}


 for(k=0;k<number_of_threads;k++){
	pthread_join(machines[k],NULL);
}

 
}
