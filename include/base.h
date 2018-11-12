#ifndef BASE_H
#define BASE_H
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <arpa/inet.h>
#define HANDLECOUNT 5
#define DOWN_HANDLE 0
#define PING_HANDLE 1
#define EVENT_HANDLE 2
#define TOKEN_HANDLE 3
#define SPEECH_HANDLE 4

#define ENDPOINT_DIRE 10
#define SETALERT_DIRE 11

using namespace rapidjson;
using namespace std;

struct ALEXA_READY{
    bool FIRSTPING;
    bool SYNCHRONIZE;
    ALEXA_READY(){
        FIRSTPING=false;
        SYNCHRONIZE=false;
    }
};

typedef struct ALEXA_CONFIG{
    std::string access_token;
    std::string refresh_token;
    std::string client_id;
    std::string client_sceret;
    std::string s_audio_path;
    std::string r_audio_path;
    std::string expires;
    std::string sys_audio_path;
}alexa_config;

typedef struct ALEXA_DEVICE{
    int useDeviceNum;
    int recTruckSize;
}alexa_device;

struct Header{
    std::string namesp;
    std::string name;
    std::string msgid;
    std::string dialogRequestid;
};

struct Payload{
    std::string profile;
    std::string format;
    std::string token;
    std::string initiator;
};

struct Event{
    Header header;
    Payload payload;
};

struct DownParams{
    int mark;
    string msg_token;
    string others;
};

typedef enum NET_STATE_T{
    NET_STATE_IDLE,
    NET_STATE_PING,
    NET_STATE_SEND_EVENT,
    NET_STATE_SEND_STATE,
    NET_STATE_SPEECH_ON,
    NET_STATE_SPEECH_OFF
}net_state_t;


class Base
{

public:
    Base();
    void getWorkdir();
    void getTime();
    void ReadDeviceinfo();
    void Readtoken();
    void Speechinfo(std::string json);
    void playback(std::string path);
    void playalert(std::string path);
    std::string message_id;
    std::string speak_token;

protected:
    std::string workdir;
    std::string nowtime;
    ALEXA_CONFIG config; 
    ALEXA_DEVICE device;
    fd_set fds;

private:


};


void IsEndpoint(const char *json, DownParams *params);
void MultiMsgPt1(std::string &Pt1,struct Event EventState);
void SpeechSynthesizer(string &Pt1, struct Event EventState);


#endif 
