FILE * f1=fopen("output.txt","w");
        for(int i=0;i<7;i++){
            char * p = str[i];
            fprintf(f1,p);
            fprintf(f1,"\n");
        }
