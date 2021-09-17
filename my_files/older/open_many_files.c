void opening_more_files(int open_file_limit)
{
    char * filename="open  ";
    int opened_files_count=0;
    for(char a='1';a<='9';a++){
        for(char b='1';b<='9';b++){
            filename[4]=a;
            filename[5]=b;
            FILE * f1=fopen(filename,"r");
            if(f1==NULL){
                printf("%d",opened_files_count);
                return;
            }
            opened_files_count++;
        }
    }
}
