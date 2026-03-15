#ifdef WAV_FILE_DEBUG
#include "debug.h"
#else

#include <Arduino.h>
#include <Audio.h>
#include "WakeWord.h"
#include "NoiseSuppressor.h"

// オーディオ
AudioClass *theAudio;

// オーディオの警告コールバック
static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
    printf("Attention! code = %d\n", atprm->error_code);
}

// マイクからの音声データの読み出し
bool mic_record(int16_t* rec_data, size_t array_len)
{
    const uint32_t byte_size = array_len * sizeof(int16_t);
    uint32_t filled = 0;

    while (filled < byte_size) {
        uint32_t to_read = byte_size - filled;
        uint32_t read_size = 0;

        // マイクから読み出す
        int err = theAudio->readFrames((char*)rec_data + filled, to_read, &read_size);

        if (err != AUDIOLIB_ECODE_OK && err != AUDIOLIB_ECODE_INSUFFICIENT_BUFFER_AREA) {
            Serial.printf("Error err = %d\n", err);
            sleep(1);
            theAudio->stopRecorder();
            return false;
        }
        filled += read_size;
    }
    return true; 
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Hello!");

    // オーディオ初期化
    theAudio = AudioClass::getInstance();
    theAudio->begin(audio_attention_cb);
    theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 210); // gain +21.0dB (max)
    theAudio->initRecorder(AS_CODECTYPE_PCM, "/mnt/sd0/BIN", AS_SAMPLINGRATE_16000, AS_BITLENGTH_16, AS_CHANNEL_MONO);
    theAudio->startRecorder();

    // ウェイクワード初期化
    wakeword_init();
}

// 0:コマンド待ち 1:ウェイクワード登録 2:ウェイクワード比較
int state = 0;

void loop(void)
{
    if(state == 0){
        printf("Please Input 1 - 2 or Q.\n");
        printf("1:Regist Wake Word / 2:Compare Wake Word\n");
        char c = 0;
        while (1) {
            if(Serial.available() > 0){
            c = Serial.read();
            while(Serial.available() > 0){}
            break;
            }
        }
        c = c - '0';

        // 1 : ウェイクワード登録
        if (c == 1) {
            printf("Regist Wake Word\n");
            state = 1;
        }
        // 2 : ウェイクワード比較
        else if (c == 2) {
            printf("Compare Wake Word\n");
            state = 2;
        }
        else {
            printf("Wrong Command!\n");
        }
        printf("\n");
    }else{
        // Q : 中断
        if(Serial.available() > 0){
            char c = Serial.read();
            if(c == 'q' || c == 'Q')
            {
                printf("Quit!\n");
                state = 0;
            }
        }
        // 1 : ウェイクワード登録
        if (state == 1) {
            bool ret = wakeword_regist();
            if (ret == true) {
                printf("Regist Completed!\n\n");
                state = 0;
            }
        }
        // 2 : ウェイクワード比較
        else if (state == 2) {
            bool ret = wakeword_compare();
            if (ret == true) {
                printf("Compare Completed!\n\n");
                state = 0;
            }
        }
    }
}
#endif