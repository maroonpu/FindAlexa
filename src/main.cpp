#include <iostream>
#include "include/alexa.h"

#define MAX_WAIT_MSECS 1000
using namespace std;

int main(int argc,char *argv[])//使用libcurl的multi interface实现和亚马逊服务器的HTTP通信
{

    //说明：
    //libcurl中每一个HTTP链接由一个easy interface来管理，对应一个easy handle
    //多个链接同时运行则由multi interface来管理，需要将各个easy handle加入multi handle中

    /*********WIDORA DEBUG************/
    // system("export PA_ALSA_PLUGHW=1");
    // system("mpc volume 95");
    // system("mpc update");
    /*********************************/

    curl_global_init(CURL_GLOBAL_ALL);
    CURLM *multi_handles = curl_multi_init();//初始化    

    CURL *handles[5];
    long response_code;
    curl_off_t DLspeed;
    net_state_t netState;
    DownParams *params = new DownParams;
    
    /*********WIDORA DEBUG************/
    // bool welcome_flag = true;
    /*********************************/

    char cmd[10] = {0};


    ALEXA_READY ready;
    bool global_ready = false;
    bool time2ping = false;
    int still_running = 0,msg_left = 0;
    struct curl_slist *DownHead=NULL;
    struct curl_slist *SynHead=NULL;
    struct curl_slist *PingHead=NULL;
    struct curl_slist *AudioHead=NULL;
    struct curl_slist *TokenHead=NULL;
    struct curl_httppost *postFirst = NULL,*postLast = NULL;

    Alexa alexa;

    alexa.getWorkdir();
    alexa.ReadDeviceinfo();
    alexa.Readtoken();
    alexa.init();


    curl_multi_setopt(multi_handles,CURLMOPT_PIPELINING,CURLPIPE_MULTIPLEX);//链接复用

    curl_multi_perform(multi_handles,&still_running);
    for(int i=0;i<HANDLECOUNT;i++)
    {
       handles[i] = curl_easy_init();//初始化所有的easy interface的easy handle
    }

    alexa.FlashToken(handles[TOKEN_HANDLE],TokenHead);//刷新token
    curl_multi_add_handle(multi_handles,handles[TOKEN_HANDLE]);
    curl_multi_perform(multi_handles,&still_running);

    do{

        CURLMsg *msg=NULL;
        int numfds = 0;
        int res = curl_multi_wait(multi_handles,NULL,0,MAX_WAIT_MSECS,&numfds);

        if(res != CURLM_OK)
        {
            std::cout<<"error: curl_multi_wait return"<<std::endl;
            return EXIT_FAILURE;
        }
        else
        {
            curl_multi_perform(multi_handles,&still_running);
        }


        if(cmd[0] != 0)
        {//判断输入的字符
            cout<<cmd<<endl;

            if(cmd[0]=='b')
            {
                alexa.getTime();
                alexa.capture();
                netState = NET_STATE_SEND_EVENT;
            }

            else if(cmd[0]=='s')
            {
                alexa.stopCapture();
                cout<<"Record finished..."<<endl;
            }
            else if(cmd[0]=='q' )
            {
                exit(1);
            }
            else
            {
                cout<<"invalid command..."<<endl;
            }

            memset(cmd, '\0', sizeof(cmd));
        }





        switch(netState)//netState状态，初始为NET_STATE_IDLE
        {
            case NET_STATE_IDLE:
            {
                if(time2ping){
                    netState = NET_STATE_PING;
                }


                if(params->mark==SETALERT_DIRE){

                    alexa.startclock(params->others);
                    alexa.SetClockSycceeded(handles[EVENT_HANDLE], params->msg_token, SynHead, postFirst, postLast);
                    curl_multi_add_handle(multi_handles,handles[EVENT_HANDLE]);

                    params->mark = 0;
                }

            }
                break;
            case NET_STATE_SEND_EVENT://发送音频数据
            {

                alexa.SendAudioFile(handles[EVENT_HANDLE],AudioHead,postFirst,postLast);
 
                curl_multi_add_handle(multi_handles,handles[EVENT_HANDLE]);
                cout<<"sending event..."<<endl;
                netState=NET_STATE_IDLE;
            }
                break;
            case NET_STATE_PING://发送PING
            {
                alexa.Ping(handles[PING_HANDLE],PingHead);
                curl_multi_add_handle(multi_handles,handles[PING_HANDLE]);
                netState=NET_STATE_IDLE;
            }
                break;
            case NET_STATE_SEND_STATE:
            {

            }
                break;
            case NET_STATE_SPEECH_ON:
            {
                alexa.Speechstarted(handles[SPEECH_HANDLE], alexa.speak_token, SynHead, postFirst, postLast);

                curl_multi_add_handle(multi_handles, handles[SPEECH_HANDLE]);
                netState=NET_STATE_IDLE;
            }
                break;
            case NET_STATE_SPEECH_OFF:
            {
                alexa.Speechfinished(handles[SPEECH_HANDLE], alexa.speak_token, SynHead, postFirst, postLast);
                curl_multi_add_handle(multi_handles, handles[SPEECH_HANDLE]);
                netState=NET_STATE_IDLE;
            }break;
            default:
            {

            }
                break;
        }

        while((msg=curl_multi_info_read(multi_handles,&msg_left)))//根据服务器返回状态进行下一步
        {

            if(msg->msg == CURLMSG_DONE)//检测哪一个easy handle完成
            {
                int idx, found =0;

                for(idx=0;idx<HANDLECOUNT;idx++){
                    found = (msg->easy_handle == handles[idx]);
                    if(found)
                        break;
                }

                switch(idx){
                    case DOWN_HANDLE://DownChannel的handle完成

//                        curl_multi_add_handle(multi_handles,handles[DOWN_HANDLE]);
                        break;
                    case PING_HANDLE://PING的handle完成
                        curl_easy_getinfo(handles[idx],CURLINFO_RESPONSE_CODE,&response_code);
                        if(response_code==200 || response_code==204){
                            cout<<"\nping completed: "<<msg->data.result<<"\n"<<endl;
                            ready.FIRSTPING=true;
                        }
                        curl_multi_remove_handle(multi_handles,handles[PING_HANDLE]);
                        break;
                    case EVENT_HANDLE://EVENT的handle完成
                        cout<<"\n\n\nevent completed: "<<msg->data.result<<"\r\n";
                        
                        curl_easy_getinfo(handles[idx],CURLINFO_RESPONSE_CODE,&response_code);
                        curl_easy_getinfo(handles[idx],CURLINFO_SPEED_DOWNLOAD,&DLspeed);
                        cout<<"Response Code is : "<<response_code<<"\r\n";
                        cout<<"Dowload Speed is : "<<DLspeed<<endl;
                        if(response_code==200 || response_code==204){
                            ready.SYNCHRONIZE=true;

                        }
                        if(response_code==200){
                            string json;
                            alexa.getTime();
                            string mp3file = alexa.Parser(json);//解析返回数据
                            alexa.Speechinfo(json);

                            cout<<mp3file<<endl;
                            curl_multi_remove_handle(multi_handles,handles[EVENT_HANDLE]);

                            alexa.playback(mp3file.c_str());

                            netState=NET_STATE_SPEECH_ON;


                            break;
                        }
                        else{
                            curl_multi_remove_handle(multi_handles,handles[EVENT_HANDLE]);
                            netState=NET_STATE_IDLE;
                            break;
                        }

                    case SPEECH_HANDLE:
                        curl_multi_remove_handle(multi_handles, handles[SPEECH_HANDLE]);
                        break;
                    case TOKEN_HANDLE://flash的hanlde完成
                        alexa.Readtoken();
                        cout<<"token refresh completed: "<<msg->data.result<<"\n"<<endl;
                        curl_multi_remove_handle(multi_handles,handles[TOKEN_HANDLE]);

                        alexa.SetDownChannel(handles[DOWN_HANDLE],DownHead, params);
                        curl_multi_add_handle(multi_handles,handles[DOWN_HANDLE]);

                        alexa.Synchronize(handles[EVENT_HANDLE],SynHead,postFirst,postLast);
                        curl_multi_add_handle(multi_handles,handles[EVENT_HANDLE]);

                        alexa.Ping(handles[PING_HANDLE],PingHead);
                        curl_multi_add_handle(multi_handles,handles[PING_HANDLE]);
                        break;
                    default:
                        break;
                }


            }

        }

        if(!global_ready)
            if(ready.FIRSTPING&&ready.SYNCHRONIZE)
            {
                /*********WIDORA DEBUG************/
                // if(welcome_flag == true)
                // {
                //     system("mpc clear");
                //     system("mpc add welcome.mp3");
                //     system("mpc play 1");//welcome sound

                //     welcome_flag = false;
                // }
                /*********************************/

                global_ready = true;
                cout<<"#############################################"<<"\n";
                cout<<"#############################################"<<"\n";
                cout<<"#############################################"<<"\n";
                cout<<"############## ALEXA IS READY ###############"<<"\n";
                cout<<"USAGE:"<<"\n";

                /*********WIDORA DEBUG************/
                // cout<<"press button to start record"<<endl;
                /*********************************/

                /***********PC DEBUG**************/
                cout<<"\"b\":start record"<<"\n";
                cout<<"\"q\":quit"<<endl;
                /*********************************/

                alexa.KeyboardInput(cmd);

            }


    }while(true);

    return 0;
}


