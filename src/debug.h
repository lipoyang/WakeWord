/****************************************
    WAVファイルを用いたデバッグ用コード
 ****************************************/

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
            printf("end of file!\n");
            break;
        } else {
            // ウェイクワード登録処理
            bool ret = wakeword_regist();
            if (ret == true) {
                printf("Completed! frameNo = %d\n", frameNo);
                break;
            }
        }
    }
    printf("\n");
    fclose(wavefile);

    return 0;
}

// ウェイクワード比較
int compare(int c)
{
    // WAVファイル
    char filename[16];
    sprintf(filename, "/mnt/sd0/test%d.wav", c);
    FILE* wavefile = fopen(filename, "rb");
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
            printf("end of file!\n");
            break;
        } else {
            // ウェイクワード比較処理
            bool ret = wakeword_compare();
            if (ret == true) {
                printf("Completed! frameNo = %d\n", frameNo);
                break;
            }
        }
    }
    printf("\n");
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
    printf("Please Input 0 - 3 or Q.\n");
    printf("0:Regist Wake Word / 1-3:Compare Wake Word\n");
    char c = 0;
    while (1) {
        if(Serial.available() > 0){
          c = Serial.read();
          while(Serial.available() > 0){}
          break;
        }
    }
    c = c - '0';

    // 0 : ウェイクワード登録
    if (c == 0) {
        printf("Regist Wake Word\n");
        int ret = regist();
        if(ret != 0) {
            printf("Failed to regist wake word. (%d)\n", ret);
        }
    }
    // 1-3 : ウェイクワード比較
    else if (c == 1 || c == 2 || c == 3) {
        printf("Compare Wake Word (%d)\n", c);
        int ret = compare(c);
        if (ret != 0) {
            printf("Failed to compare wake word.(%d)\n", ret);
        }
    }
    else {
        printf("Wrong Command!\n");
    }
}
