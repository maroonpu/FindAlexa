#ifndef CPPMPCPLAY_H_INCLUDE_
#define CPPMPCPLAY_H_INCLUDE_

#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

class MPCPlayer {
public:
    MPCPlayer() {
      songs_num = 0;
    }
    /* Get the audio num by using "mpc playlist"
     */
    int GetNumOfAudio() {
        FILE *fp = NULL;
        char buffer[100];
        unsigned int song_num = 0;
        fp = popen("mpc playlist", "r");
        if (fp == NULL) {
            song_num = 0;
        }
        while(fgets(buffer, sizeof(buffer), fp)) {
                ++song_num;
        }
        songs_num = song_num;
	return song_num;
    }
    /* Play songs
     */
    void SongPlay(const string AudioURL) {
        /*Add the audio to playlist*/       
        string add_command = "mpc add " + AudioURL;
        system(add_command.c_str());
        /*Play the audio*/
        string song_num = "";
        GetNumOfAudio();
        stringstream ss;
        ss << songs_num;
        ss >> song_num;
        string song_play = "mpc play " + song_num;
        system(song_play.c_str());
        cout << "-----------------------------------开始播放-----------------------------------" << endl;
    }
    /* Stop play
     */
    void StopPlay() {
        system("mpc stop");
    }
    /* Set Volume 
     */
    void SetVolume(string volume) {
        string setvolume = "amixer set Headphone " + volume;
        system(setvolume.c_str());
    }
    
    /* Speed up
     */
    void PlaySpeed() {
        system("mpc seek +2%");
    }
    /* Slow down
     */
    void PlayBack() {
        system("mpc seek  -2%");
    }
    
    /* Show img of Audio
     */
    void ShowImgOfAudio(string img_path) {
        audio_img_path = img_path;
//         show_bpm(audio_img_path.c_str(),20,60,1);
        printf("The img path is %s\n", img_path.c_str());
    }
    static string audio_img_path;
private:
    string song_url;
    int songs_num;
};
/*img path of audio*/
string MPCPlayer::audio_img_path = "./Cover/民谣/赵雷的小屋/鼓楼";
#endif
