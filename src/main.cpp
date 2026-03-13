#include <Arduino.h>
#include "WakeWord.h"
#include "NoiseSuppressor.h"

// 音声バッファ (10msecぶん)
int16_t rxbuff[160];
// フレーム番号 (デバッグ用)
int frameNo;

// 音声データのコピー
bool mic_record(int16_t* rec_data, size_t array_len)
{
    if (array_len != 160) {
        printf("Invalid array length. Expected 160, got %zu\n", array_len);
        return false;
    }
    memcpy(rec_data, rxbuff, 160 * sizeof(int16_t));
    return true; 
}

// ウェイクワード登録
int regist()
{
    // WAVファイル
    FILE* wavefile = fopen("/mnt/sd0/test1.wav", "rb");
    if (!wavefile) {
        perror("fopen in regist() failed");
        return -1;
    }

    // 先頭 44 バイトを読み飛ばす (WAV ヘッダ)
    size_t n = fread(rxbuff, 1, 44, wavefile);
    if (n < 44) {
        fprintf(stderr, "source file too small\n");
        fclose(wavefile);
        return -2;
    }

    frameNo = 0;
    while (1) {
        // 160サンプル (10msec) ずつ読み込む
        size_t n = fread(rxbuff, 2, 160, wavefile);
        frameNo++;

        if (n < 160) {
            printf("end of file!");
            break;
        } else {
            // ウェイクワード登録処理
            wakeword_regist();
        }
    }
    fclose(wavefile);

    return 0;
}

// ウェイクワード比較
int compare()
{
    // WAVファイル
    FILE* wavefile = fopen("/mnt/sd0/test2.wav", "rb");
    if (!wavefile) {
        perror("fopen in compare() failed");
        return -1;
    }

    // 先頭 44 バイトを読み飛ばす (WAV ヘッダ)
    size_t n = fread(rxbuff, 1, 44, wavefile);
    if (n < 44) {
        fprintf(stderr, "source file too small\n");
        fclose(wavefile);
        return -2;
    }

    frameNo = 0;
    while (1) {
        // 160サンプル (10msec) ずつ読み込む
        size_t n = fread(rxbuff, 2, 160, wavefile);
        frameNo++;

        if (n < 160) {
            printf("end of file!");
            break;
        } else {
            // ウェイクワード比較処理
            wakeword_compare();
        }
    }
    fclose(wavefile);

    return 0;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Hello!");

    // ウェイクワード初期化
    wakeword_init();
}

void loop(void)
{
    printf("Please Input 1 or 2\n");
    printf("1:Regist Wake Word / 2:Compare Wake Word\n");
    char c = 0;
    while (1) {
        if(Serial.available() > 0){
          c = Serial.read();
          while(Serial.available() > 0){}
          break;
        }
    }

    // 1 : ウェイクワード登録
    if (c == '1') {
        printf("Regist Wake Word\n");
        int ret = regist();
        if(ret != 0) {
            printf("Failed to regist wake word. (%d)\n", ret);
        }
    }
    // 2 : ウェイクワード比較
    else if (c == '2') {
        printf("Compare Wake Word\n");
        int ret = compare();
        if (ret != 0) {
            printf("Failed to compare wake word.(%d)\n", ret);
        }
    }
    else {
        printf("Wrong Command!\n");
    }
}
