#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
char* extract(char *s)
{
    // extracts body from string
    // char *result = strstr(s, "<div>");
    // int position = result - s;
    // printf("%d \n",position);
    // for(int i=position;i<position+10;i++)
    // {
    //     printf("%c",s[i]);
    // }
    char ans[1000];
    int anslen=0;
    memset(ans,0,sizeof(ans));
    bool start=0;
    int len=strlen(s);
    for(int i=0;i<len;i++)
    {
        if(i+4<len && s[i]=='<' && s[i+1]=='d' && s[i+2]=='i' && s[i+3]=='v' && s[i+4]=='>')
        {
            start=1;
            i=i+4;
            continue;
        }
        if(i+5<len && s[i]=='<' && s[i+1]=='/' && s[i+2]=='d' && s[i+3]=='i' && s[i+4]=='v' && s[i+5]=='>')
        {
            start=0;
            i=i+5;
        }
        if(i+4<len && s[i]=='<' && s[i+1]=='b' && s[i+2]=='r' && s[i+3]=='>')
        {
            start=0;
            i+=4;
        }
        if(i+5<len && s[i]=='<' && s[i+1]=='b' && s[i+2]=='r' && s[i+3]=='/' && s[i+4]=='>')
        {
            start=0;
            i+=5;
        }
        if(start==1)
        {
            ans[anslen++]=s[i];
        }
        
        
    }
    char * result = (char*)malloc(sizeof(char)*anslen);
    strcpy(result,ans);
    return result;
}
int main()
{
    char *s = "MIME-Version: 1.0Date: Fri, 19 Nov 2021 10:37:21 +0530Message-ID: <CAAUh_a=ZGsKwY4M4mPN-U7TWLKMe-WyP1WRGEYRsbr28CAUDdw@mail.gmail.com>Subject: IMP From: Brown Munde <testhailaude@gmail.com> To: Brown Munde <testhailaude@gmail.com>Content-Type: multipart/alternative; boundary=\"00000000000013409905d11d401c\"X-Antivirus: Avast (VPS 211118-10, 11/18/2021), Inbound messageX-Antivirus-Status: Clean--00000000000013409905d11d401cContent-Type: text/plain; charset=\"UTF-8\"brown mundeborn gunde--00000000000013409905d11d401cContent-Type: text/html; charset=\"UTF-8\"<div dir=\"ltr\"><div>brown munde</div><div>born gunde<br></div></div>--00000000000013409905d11d401c--";
    char * ans=extract(s);
    printf("%s",ans);
    return 0;
}