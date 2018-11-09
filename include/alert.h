#ifndef ALERT
#define ALERT
#include <curl/curl.h>
#include <string>
#include "include/base.h"


class Alert: virtual public Base
{
public:
    Alert();
    void startclock(std::string scheduledTime);
    void stopclock();
    void SetClockSycceeded(CURL *curl, string token, struct curl_slist *header, curl_httppost *postFirst, curl_httppost *postLast);
    void Speechstarted(CURL *curl, string token, struct curl_slist *header, curl_httppost *postFirst, curl_httppost *postLast);
    void Speechfinished(CURL *curl, string token, curl_slist *header, curl_httppost *postFirst, curl_httppost *postLast);
protected:
    void _setclock(std::string scheduledTime);
    bool clockActivity = true;
};

#endif // ALERT

