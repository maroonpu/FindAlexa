#include "include/alert.h"
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <thread>
Alert::Alert()
{

}

//void Alert::IsAlert(char* json)
//{

//}

void Alert::startclock(std::string scheduledTime)
{

    cout<<"Set time at "<<scheduledTime<<endl;
    clockActivity = true;

    thread p1(&Alert::_setclock, this, scheduledTime);
    p1.detach();

}

void Alert::_setclock(string scheduledTime)
{
    static char Time[32];
    do{
        time_t t = time(NULL);
        strftime(Time,32,"%FT%T%z",gmtime(&t));
        string nowtime(Time);
        cout<<nowtime<<endl;
        sleep(1);

        if((nowtime==string(scheduledTime))){
            cout<<"time up "<<endl;
            // playback(config.sys_audio_path + "alert.mp3","0");
            playback("alert.wav");//,"0");

            break;
        }

        if(!clockActivity){
            cout<<"clock cancel "<<endl;
        }
    }while(clockActivity);
}


void Alert::stopclock()
{
    clockActivity = false;
}


void Alert::Speechstarted(CURL* curl, string token, struct curl_slist *header, curl_httppost *postFirst, curl_httppost *postLast)
{


    string auth = "Authorization:Bearer " + config.access_token;

    struct Event eventState;
    eventState.header.namesp = "SpeechSynthesizer";
    eventState.header.name = "SpeechStarted";
    eventState.header.msgid = "abvvseee33565";
    eventState.payload.token = token.c_str();

    string json;
    SpeechSynthesizer(json, eventState);

    header = curl_slist_append(header,auth.c_str());
    header = curl_slist_append(header,"content-type:multipart/form-data");

    curl_formadd(&postFirst,&postLast,
                 CURLFORM_COPYNAME,"metadata",
                 CURLFORM_COPYCONTENTS,json.c_str(),
                 CURLFORM_CONTENTTYPE,"application/json; charset=UTF-8",
                 CURLFORM_END);//构建multipart数据

    curl_easy_setopt(curl,CURLOPT_FORBID_REUSE,1L);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_2_0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);

    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,header);
    curl_easy_setopt(curl,CURLOPT_URL,"https://avs-alexa-na.amazon.com/v20160207/events");
    curl_easy_setopt(curl,CURLOPT_HTTPPOST,postFirst);

}

void Alert::SetClockSycceeded(CURL *curl, string token, struct curl_slist *header, curl_httppost *postFirst, curl_httppost *postLast)
{
    string auth = "Authorization:Bearer " + config.access_token;

    struct Event eventState;
    eventState.header.namesp = "Alerts";
    eventState.header.name = "SetAlertSucceeded";
    eventState.header.msgid = "abvvseee335ddasf";
    eventState.payload.token = token.c_str();

    string json;
    SpeechSynthesizer(json, eventState);

    header = curl_slist_append(header,auth.c_str());
    header = curl_slist_append(header,"content-type:multipart/form-data");

    curl_formadd(&postFirst,&postLast,
                 CURLFORM_COPYNAME,"metadata",
                 CURLFORM_COPYCONTENTS,json.c_str(),
                 CURLFORM_CONTENTTYPE,"application/json; charset=UTF-8",
                 CURLFORM_END);//构建multipart数据

    curl_easy_setopt(curl,CURLOPT_FORBID_REUSE,1L);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_2_0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);

    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,header);
    curl_easy_setopt(curl,CURLOPT_URL,"https://avs-alexa-na.amazon.com/v20160207/events");
    curl_easy_setopt(curl,CURLOPT_HTTPPOST,postFirst);
}

void Alert::Speechfinished(CURL *curl, string token, struct curl_slist *header, curl_httppost *postFirst, curl_httppost *postLast)
{
    string auth = "Authorization:Bearer " + config.access_token;

    struct Event eventState;
    eventState.header.namesp = "SpeechSynthesizer";
    eventState.header.name = "SpeechFinished";
    eventState.header.msgid = "abvvseee335ddd6";
    eventState.payload.token = token.c_str();

    string json;
    SpeechSynthesizer(json, eventState);

    header = curl_slist_append(header,auth.c_str());
    header = curl_slist_append(header,"content-type:multipart/form-data");

    curl_formadd(&postFirst,&postLast,
                 CURLFORM_COPYNAME,"metadata",
                 CURLFORM_COPYCONTENTS,json.c_str(),
                 CURLFORM_CONTENTTYPE,"application/json; charset=UTF-8",
                 CURLFORM_END);//构建multipart数据

    curl_easy_setopt(curl,CURLOPT_FORBID_REUSE,1L);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    curl_easy_setopt(curl,CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_2_0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);

    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,header);
    curl_easy_setopt(curl,CURLOPT_URL,"https://avs-alexa-na.amazon.com/v20160207/events");
    curl_easy_setopt(curl,CURLOPT_HTTPPOST,postFirst);
}
