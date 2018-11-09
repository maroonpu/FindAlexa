#include "include/base.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <errno.h>

Base::Base()
{
    // Base::Socketinit();
    // close socket mode
    // songs_num = 0;
}


void Base::getWorkdir()
{
    workdir = std::string(getenv("HOME"))+"/.Alexa";
}

void Base::getTime()//获取系统本地时间
{
    static char Time[32];
    time_t t = time(NULL);
    strftime(Time,32,"%Y%m%d_%H%M%S",localtime(&t));
    nowtime = Time;
    cout<<nowtime<<endl;
}

void Base::ReadDeviceinfo()//读取deviceinfo.json中内容
{
    Document doc;
    ifstream fin((workdir+"/deviceinfo.json").c_str());
    stringstream buffer;
    buffer<<fin.rdbuf();
    string devices(buffer.str());

    doc.Parse<0>(devices.c_str());

    Value &deviceNum = doc["InputDeviceNum"];
    Value &truckSize = doc["RecTruckSize"];


    device.useDeviceNum = deviceNum.GetInt();
    device.recTruckSize = truckSize.GetInt();

    fin.close();

}

void Base::Readtoken()//读取config.json中内容
{
    Document doc;

    ifstream fin((workdir+"/config.json").c_str());
    stringstream buffer;
    buffer<<fin.rdbuf();
    string contents(buffer.str());

    doc.Parse<0>(contents.c_str());

    Value &ref_token = doc["refresh_token"];
    Value &client_id = doc["client_id"];
    Value &client_secret = doc["client_secret"];
    Value &acc_token = doc["access_token"];
    Value &expires = doc["expires_in"];
    Value &s_audio_path = doc["s_audio_path"];
    Value &r_audio_path = doc["r_audio_path"];
    Value &sys_audio_path = doc["sys_audio_path"];

    config.access_token = acc_token.GetString();
    config.refresh_token = ref_token.GetString();
    config.client_id = client_id.GetString();
    config.client_sceret = client_secret.GetString();
    config.expires = expires.GetInt();
    config.s_audio_path = s_audio_path.GetString();
    config.r_audio_path = r_audio_path.GetString();
    config.sys_audio_path = sys_audio_path.GetString();
    fin.close();

}

void Base::Speechinfo(std::string json)
{
    Document doc;
    doc.Parse<0>(json.c_str());

    Value &msg_id = doc["directive"]["header"]["messageId"];
    Value &token = doc["directive"]["payload"]["token"];

    message_id = msg_id.GetString();
    speak_token = token.GetString();
}

/********************socket mode**********************************/
// void Base::playback(std::string path,std::string orderFlag)
// {
//     std::string msg = "{\"orderFlag\" : " + orderFlag +" ,\"action\" : \"playback\",\"file\":\"" + path +"\"}";
//     send(cli_sockdf, msg.c_str(), msg.size(), 0);
// }

// void Base::playalert(std::string path,std::string orderFlag)
// {
//     std::string msg = "{\"orderFlag\" : " + orderFlag +" ,\"action\" : \"playalert\",\"file\":\"" + path +"\"}";
//     send(cli_sockdf, msg.c_str(), msg.size(), 0);
// }


// void Base::Socketinit()
// {
//     cli_sockdf = socket(AF_INET, SOCK_STREAM, 0);
//     socklen_t addrlen = sizeof(struct sockaddr_in);
//     bzero(&ser_addr, addrlen);

//     ser_addr.sin_family=AF_INET;
//     ser_addr.sin_addr.s_addr=inet_addr("0.0.0.0");
//     ser_addr.sin_port=htons(8002);
//     int error = connect(cli_sockdf, (const sockaddr*)&ser_addr, addrlen);
//     if(error < 0){
//         cout << "Connect Error: "<<strerror(errno)<<endl;
//         exit(1);
//     }
// }
/******************************************************************/

void Base::playback(std::string path)//,std::string orderFlag)
{
    // std::string msg = "{\"orderFlag\" : " + orderFlag +" ,\"action\" : \"playback\",\"file\":\"" + path +"\"}";
    // send(cli_sockdf, msg.c_str(), msg.size(), 0);
    string del_command = "cd "+config.r_audio_path+" && ls | grep -v "+path+" | grep -v welcome.mp3 | grep -v start.mp3 | grep -v stop.mp3 | xargs rm";
    // string del_command = "cd "+config.r_audio_path+" && sudo rm !( "+path+" )";

    // cout<<"del_command::::"<<del_command<<endl;
    system(del_command.c_str());

    
    // system("mpc stop");
    system("mpc clear");
    system("mpc update");
    /*Add the audio to playlist*/       
    string add_command = "mpc add " + path;
    system(add_command.c_str());

    system("mpc play 1");
    // /*Play the audio*/
    // string song_num = "";
    // GetNumOfAudio();
    // stringstream ss;
    // ss << songs_num;
    // ss >> song_num;
    // string song_play = "mpc play " + song_num;
    // system(song_play.c_str());
    cout << "-----------------------------------开始播放-----------------------------------" << endl;

    // cout<<"xiaohuihui:::"<<"cd "+config.r_audio_path+" && rm !("+path+")"<<endl;


}

/* Get the audio num by using "mpc playlist"
 */
// int Base::GetNumOfAudio() {
//     FILE *fp = NULL;
//     char buffer[100];
//     unsigned int song_num = 0;
//     fp = popen("mpc playlist", "r");
//     if (fp == NULL) {
//         song_num = 0;
//     }
//     while(fgets(buffer, sizeof(buffer), fp)) {
//             ++song_num;
//     }
//     songs_num = song_num;
//     return song_num;
// }


void Base::playalert(std::string path)//,std::string orderFlag)
{
    // std::string msg = "{\"orderFlag\" : " + orderFlag +" ,\"action\" : \"playalert\",\"file\":\"" + path +"\"}";
    // send(cli_sockdf, msg.c_str(), msg.size(), 0);
}

// int8_t Base::FDselect()
// {
//     FD_ZERO(&fds);
//     FD_SET(cli_sockdf, &fds);
//     struct timeval timeout={0,0};
//     int state = select(cli_sockdf+1, &fds, NULL, NULL ,&timeout);
//     if(state>0){
//         if(FD_ISSET(cli_sockdf, &fds)){
//             int8_t recmsg[10];
//             size_t recvBytes = recv(cli_sockdf, recmsg, sizeof(recmsg),0);
//             recmsg[recvBytes] = '\0';
//             return *recmsg;
//         }
//     }
//     else{
//         return 0;
//     }
// }

void IsEndpoint(const char* json, DownParams* params)//解析StopCapture
{
//    struct DownParams params;
    Document doc;
    doc.SetObject();

    cout<<"\n\r"<<json<<"\n\r"<<endl;

    doc.Parse<0>(json);
    Value &name = doc["directive"]["header"]["name"];
    string temp(name.GetString());
    if(temp.compare("StopCapture") == 0){

        params->mark = ENDPOINT_DIRE;
        params->msg_token = "";
        params->others = "";
//        return params;
    }
    else if(temp.compare("SetAlert") == 0){
        Value &theTime = doc["directive"]["payload"]["scheduledTime"];
        Value &token = doc["directive"]["payload"]["token"];
        params->mark = SETALERT_DIRE;
        params->msg_token = token.GetString();
        params->others = theTime.GetString();
        cout<<"XXXXXXXX"<<endl;
//        return params;
    }
}

void SpeechSynthesizer(string &Pt1, struct Event EventState)
{
    Document json;
    json.SetObject();
    Document::AllocatorType &allocator = json.GetAllocator();

    Value Event(kArrayType);
    Event.SetObject();

    Value jHeader(kArrayType);
    jHeader.SetObject();
    Value jPayload(kArrayType);
    jPayload.SetObject();

    Value namesp_tmp(EventState.header.namesp.c_str(), allocator);
    Value name_tmp(EventState.header.name.c_str(), allocator);
    Value msgid_tmp(EventState.header.msgid.c_str(), allocator);
    Value token_tmp(EventState.payload.token.c_str(), allocator);

    jHeader.AddMember("namespace",namesp_tmp,allocator);
    jHeader.AddMember("name",name_tmp,allocator);
    jHeader.AddMember("messageId",msgid_tmp,allocator);

    jPayload.AddMember("token", token_tmp, allocator);

    Event.AddMember("header",jHeader,allocator);
    Event.AddMember("payload",jPayload,allocator);

    json.AddMember("event",Event,allocator);

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    json.Accept(writer);

    Pt1 += buffer.GetString();
}

void MultiMsgPt1(string &Pt1, struct Event EventState)//构建状态信息json
{

    Document json;
    json.SetObject();
    Document::AllocatorType &allocator = json.GetAllocator();

    Value Context(kArrayType);

//**************Player Context*********************************
    Value PlayerHeader(kArrayType);
    PlayerHeader.SetObject();
    PlayerHeader.AddMember("namespace","AudioPlayer",allocator);
    PlayerHeader.AddMember("name","PlaybackState",allocator);

    Value PlayerPayload(kArrayType);
    PlayerPayload.SetObject();
    PlayerPayload.AddMember("token","",allocator);
    PlayerPayload.AddMember("offsetInMilliseconds",7000,allocator);
    PlayerPayload.AddMember("playerActivity","IDLE",allocator);

    Value PlayerContext(kArrayType);
    PlayerContext.SetObject();
    PlayerContext.AddMember("header",PlayerHeader,allocator);
    PlayerContext.AddMember("payload",PlayerPayload,allocator);

    Context.PushBack(PlayerContext,allocator);

//**************Alerts Context*********************************
    Value AlertsHeader(kArrayType);
    AlertsHeader.SetObject();
    AlertsHeader.AddMember("namespace","Alerts",allocator);
    AlertsHeader.AddMember("name","AlertsState",allocator);

    Value allAlerts(kArrayType);
    allAlerts.SetObject();
    Value allAlerts_array(kArrayType);
//    allAlerts_array.PushBack(allAlerts,allocator);

    Value activeAlerts(kArrayType);
    activeAlerts.SetObject();
    Value activeAlerts_array(kArrayType);
//    activeAlerts_array.PushBack(activeAlerts,allocator);

    Value AlertsPayload(kArrayType);
    AlertsPayload.SetObject();
    AlertsPayload.AddMember("allAlerts",allAlerts_array,allocator);
    AlertsPayload.AddMember("activeAlerts",activeAlerts_array,allocator);

    Value AlertsContext(kArrayType);
    AlertsContext.SetObject();
    AlertsContext.AddMember("header",AlertsHeader,allocator);
    AlertsContext.AddMember("payload",AlertsPayload,allocator);

    Context.PushBack(AlertsContext,allocator);

//***************Volume Context********************************
    Value VolumeHeader(kArrayType);
    VolumeHeader.SetObject();
    VolumeHeader.AddMember("namespace","Speaker",allocator);
    VolumeHeader.AddMember("name","VolumeState",allocator);

    Value VolumePayload(kArrayType);
    VolumePayload.SetObject();
    VolumePayload.AddMember("volume",50,allocator);
    VolumePayload.AddMember("muted",false,allocator);

    Value VolumeContext(kArrayType);
    VolumeContext.SetObject();
    VolumeContext.AddMember("header",VolumeHeader,allocator);
    VolumeContext.AddMember("payload",VolumePayload,allocator);

    Context.PushBack(VolumeContext,allocator);

//***************SpeechState Context***************************
    Value SpeechStateHeader(kArrayType);
    SpeechStateHeader.SetObject();
    SpeechStateHeader.AddMember("namespace","SpeechSynthesizer",allocator);
    SpeechStateHeader.AddMember("name","SpeechState",allocator);

    Value SpeechStatePayload(kArrayType);
    SpeechStatePayload.SetObject();
    SpeechStatePayload.AddMember("token","",allocator);
    SpeechStatePayload.AddMember("offsetInMilliseconds",0,allocator);
    SpeechStatePayload.AddMember("playerActivity","FINISHED",allocator);

    Value SpeechStateContext(kArrayType);
    SpeechStateContext.SetObject();
    SpeechStateContext.AddMember("header",SpeechStateHeader,allocator);
    SpeechStateContext.AddMember("payload",SpeechStatePayload,allocator);

    Context.PushBack(SpeechStateContext,allocator);

//****************IndicatorState Context************************
    Value IndicatorHeader(kArrayType);
    IndicatorHeader.SetObject();
    IndicatorHeader.AddMember("namespace","Notifications",allocator);
    IndicatorHeader.AddMember("name","IndicatorState",allocator);

    Value IndicatorPayload(kArrayType);
    IndicatorPayload.SetObject();
    IndicatorPayload.AddMember("isEnabled",false,allocator);
    IndicatorPayload.AddMember("isVisualIndicatorPersisted",false,allocator);

    Value IndicatorContext(kArrayType);
    IndicatorContext.SetObject();
    IndicatorContext.AddMember("header",IndicatorHeader,allocator);
    IndicatorContext.AddMember("payload",IndicatorPayload,allocator);

    Context.PushBack(IndicatorContext,allocator);
//***************************************************************//
    json.AddMember("context",Context,allocator);

    Value Event(kArrayType);
    Event.SetObject();

    Value jHeader(kArrayType);
    jHeader.SetObject();
    Value jPayload(kArrayType);
    jPayload.SetObject();

    if(EventState.header.name == "SynchronizeState")
    {
        Value namesp_tmp(EventState.header.namesp.c_str(), allocator);
        Value name_tmp(EventState.header.name.c_str(), allocator);
        Value msgid_tmp(EventState.header.msgid.c_str(), allocator);

        jHeader.AddMember("namespace",namesp_tmp,allocator);
        jHeader.AddMember("name",name_tmp,allocator);
        jHeader.AddMember("messageId",msgid_tmp,allocator);
    }
    else if(EventState.header.namesp == "SpeechSynthesizer")
    {
        Value namesp_tmp(EventState.header.namesp.c_str(), allocator);
        Value name_tmp(EventState.header.name.c_str(), allocator);
        Value msgid_tmp(EventState.header.msgid.c_str(), allocator);
        Value token_tmp(EventState.payload.token.c_str(), allocator);

        jHeader.AddMember("namespace",namesp_tmp,allocator);
        jHeader.AddMember("name",name_tmp,allocator);
        jHeader.AddMember("messageId",msgid_tmp,allocator);

        jPayload.AddMember("token", token_tmp, allocator);

    }

    else if(EventState.header.name == "Recognize")
    {
        Value namesp_tmp(EventState.header.namesp.c_str(), allocator);
        Value name_tmp(EventState.header.name.c_str(), allocator);
        Value msgid_tmp(EventState.header.msgid.c_str(), allocator);
        Value diaId_tmp(EventState.header.dialogRequestid.c_str(), allocator);
        Value initiator_tmp(EventState.payload.initiator.c_str(), allocator);
        Value pfile_tmp(EventState.payload.profile.c_str(), allocator);
        Value format_tmp(EventState.payload.format.c_str(), allocator);

        jHeader.AddMember("namespace",namesp_tmp,allocator);
        jHeader.AddMember("name",name_tmp,allocator);
        jHeader.AddMember("messageId",msgid_tmp,allocator);
        jHeader.AddMember("dialogRequestId",diaId_tmp,allocator);

        Value Initiator(kArrayType);
        Initiator.SetObject();
        Initiator.AddMember("type",initiator_tmp,allocator);

        jPayload.AddMember("profile",pfile_tmp,allocator);
        jPayload.AddMember("format",format_tmp,allocator);
        jPayload.AddMember("initiator",Initiator,allocator);
    }

    Event.AddMember("header",jHeader,allocator);
    Event.AddMember("payload",jPayload,allocator);

    json.AddMember("event",Event,allocator);

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    json.Accept(writer);

    Pt1 += buffer.GetString();
}



