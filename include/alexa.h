#ifndef ALEXA_H
#define ALEXA_H
#include <curl/curl.h>
#include <portaudio.h>
#include <netinet/in.h>
#include <string>
#include <sstream>
#include "include/base.h"
#include "include/alert.h"
#include <arpa/inet.h>
#include <iostream>


class Alexa: virtual public Base, public Alert
{
public:
    Alexa();
    ~Alexa();
    void KeyboardInput(char* cmd);
    // string Skill(string directiveJson);
    string Parser(string &json);
    void FlashToken(CURL *curl, struct curl_slist *head);
    void Synchronize(CURL *curl, struct curl_slist *sheader, curl_httppost *postFirst, curl_httppost *postLast);
    void SetDownChannel(CURL *curl, struct curl_slist *header, DownParams *params);
    void Ping(CURL *curl, struct curl_slist *header);
    void SendAudioFile(CURL *curl, struct curl_slist *header, curl_httppost *postFirst, curl_httppost *postLast);
    void capture();
    void stopCapture();
    void devicesInfo();
    void init();


private:
    static int writeAudio;

    void _KeyboardInput(char *cmd);
    void _playback(string path);
    static size_t trunkUpload(void *ptr);
    static size_t ReWriteConfig(void *ptr);
    static size_t DownChannelParser(void *ptr, size_t size, size_t nmemb, void *pointer);
    static size_t write_audiodata(void *ptr, size_t size, size_t nmemb, void *stream);
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);

};

int CaptureCallback(const void *input,
                    void *output,
                    unsigned long frameCount,
                    const PaStreamCallbackTimeInfo * timeInfo,
                    PaStreamCallbackFlags statusFlags,
                    void *userData);



#endif 
