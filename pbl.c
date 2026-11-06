#include<stdio.h>
#include<string.h>
#include<stdlib.h>
compress(){
    FILE *fp=fopen("input.txt","rb");
    if(fp==NULL){
        printf("file opening error");
        return 1;
    }
    FILE *fp2=fopen("compressed.dat","wb");
    int word_count=0;
    int token=2;
    char temp[20];
    char **words=malloc(sizeof(*words));
    int* freq=(int*)malloc(token*sizeof(int));
    //creating the rank
        while((fscanf(fp,"%s",temp))==1){//fscanf will read till next space or enter
            int i;
            for(i=0;i<word_count;i++){
                if(strcmp(temp,*(words+i))==0){
                    freq[i]++;
                    break;
                }
            }
            if(i==word_count){
                //extending capacity of our dictionary
                if(word_count>token){
                    words=realloc(words,sizeof(char*)*2*token);
                    freq=realloc(freq,2*token);
                    token*=2;
                }
                words[word_count]=malloc(strlen(temp)+1);
                strcpy(*(words+word_count),temp);
                freq[word_count]=1;
                word_count++;
            }
        }
        //sorting dictionary
        for(i=0;i<word_count;i++){
            for(j=i+1;j<word_count;j++){
                if(freq[i]<freq[j]){
                    int t=freq[i];
                    freq[i]=freq[j];
                    freq[j]=t;
                    temp = words[i];
                    words[i] = words[j];
                    words[j] = temp;                
                }
            }
        }//now frequency table is of no use
        //elias-gama_table
        int elias[word_count];
        for(i=0;i<word_count;i++){
            
        }
    }
int main(){
    if(compress()){
        printf("compression failed");
        return 1;
    }
    printf("compression successful");
    if((decompress())){
        printf("decompression failed");
        return 1;
    }
    printf("decompression successful");
}
