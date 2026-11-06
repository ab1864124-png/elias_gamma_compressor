#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int binary(int n) {
    int binary = 0;
    int place = 1;
    while (n>0) {
        int rem=n% 2;
        binary0=binary+(rem*place);
        num=num/2;
        place=place*10;
    }
    return binary;
}
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
                    //now frequency table is of no use                
                }
            }
        }
        //we write the words in compressed.dat
        FILE *fp2=fopen("compressed.dat","wb");
        fwrite(&word_count, sizeof(int), 1, fp2);
        for(i=0;i<word_count;i++){
            int len = strlen(words[i]) + 1;
            fwrite(words[i], sizeof(char), len, fp2);
            fwrite(words[i], sizeof(char), len, fp2);
        }
        //elias-gama_table
        int elias[word_count];
        for(i=0;i<word_count;i++){
            elias[i]=binary[i];
        }
    }
int decompress(){}
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
