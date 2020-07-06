#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "board.h"
#include "audio_pipeline.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "raw_stream.h"
#include "esp_audio.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "mp3_decoder.h"
#include "filter_resample.h"
#include "rec_eng_helper.h"
#include "esp_http_client.h"
#include "baidu_access_token.h"
#include "play_mp3.h"

static const char *TAG = "example_asr_keywords";
static char *baidu_access_token = NULL;
#define baidu_asr "http://vop.baidu.com/server_api?dev_pid=1537&cuid=ESP32&token="
static char baidu_asr_url[256 ] = {0};
audio_pipeline_handle_t pipeline;
audio_element_handle_t i2s_stream_reader, filter, raw_read;
typedef enum {
    WAKE_UP = 1,
} asr_wakenet_event_t;
int get_access_token(void)
{
    if (baidu_access_token == NULL) {
        // Must freed `baidu_access_token` after used
        baidu_access_token = baidu_get_access_token(CONFIG_BAIDU_ACCESS_KEY, CONFIG_BAIDU_SECRET_KEY);
    }
    if (baidu_access_token == NULL) {
        ESP_LOGE(TAG, "Error issuing access token");
        return ESP_FAIL;
    }
    snprintf(baidu_asr_url, 256, "%s%s", baidu_asr, baidu_access_token);
    return ESP_OK;
}
void init_asr(void)
{
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[ 2.0 ] Create audio pipeline for recording");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[ 2.1 ] Create i2s stream to read audio data from codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.i2s_config.sample_rate = 48000;
    i2s_cfg.type = AUDIO_STREAM_READER;

#if defined CONFIG_ESP_LYRAT_MINI_V1_1_BOARD
    i2s_cfg.i2s_config.sample_rate = 16000;
    i2s_cfg.i2s_port = 1;
    i2s_cfg.i2s_config.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);
#else
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);
    ESP_LOGI(TAG, "[ 2.2 ] Create filter to resample audio data");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = 48000;
    rsp_cfg.src_ch = 2;
    rsp_cfg.dest_rate = 16000;
    rsp_cfg.dest_ch = 1;
    filter = rsp_filter_init(&rsp_cfg);
#endif

    ESP_LOGI(TAG, "[ 2.3 ] Create raw to receive data");
    raw_stream_cfg_t raw_cfg = {
        .out_rb_size = 8 * 1024,
        .type = AUDIO_STREAM_READER,
    };
    raw_read = raw_stream_init(&raw_cfg);

    ESP_LOGI(TAG, "[ 3 ] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, raw_read, "raw");

#if defined CONFIG_ESP_LYRAT_MINI_V1_1_BOARD
    ESP_LOGI(TAG, "[ 4 ] Link elements together [codec_chip]-->i2s_stream-->raw-->[SR]");
    audio_pipeline_link(pipeline, (const char *[]) {"i2s",  "raw"}, 2);
#else
    audio_pipeline_register(pipeline, filter, "filter");
    ESP_LOGI(TAG, "[ 4 ] Link elements together [codec_chip]-->i2s_stream-->filter-->raw-->[SR]");
    audio_pipeline_link(pipeline, (const char *[]) {"i2s", "filter", "raw"}, 3);
#endif
     
    ESP_LOGI(TAG, "[ 5 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);
    get_access_token();     //获取token
}

void start_asr(void)
{
    ESP_LOGI(TAG, "Initialize SR wn handle");
    esp_wn_iface_t *wakenet;
    model_coeff_getter_t *model_coeff_getter;
    model_iface_data_t *model_wn_data;
    get_wakenet_iface(&wakenet);
    get_wakenet_coeff(&model_coeff_getter);
    model_wn_data = wakenet->create(model_coeff_getter, DET_MODE_90);
    int wn_num = wakenet->get_word_num(model_wn_data);
    for (int i = 1; i <= wn_num; i++) {
        char *name = wakenet->get_word_name(model_wn_data, i);
        ESP_LOGI(TAG, "keywords: %s (index = %d)", name, i);
    }
    float wn_threshold = wakenet->get_det_threshold(model_wn_data, 1);
    int wn_sample_rate = wakenet->get_samp_rate(model_wn_data);
    int audio_wn_chunksize = wakenet->get_samp_chunksize(model_wn_data);
    ESP_LOGI(TAG, "keywords_num = %d, threshold = %f, sample_rate = %d, chunksize = %d, sizeof_uint16 = %d", wn_num, wn_threshold, wn_sample_rate, audio_wn_chunksize, sizeof(int16_t));
    int size = audio_wn_chunksize;
    int16_t *buffer = (int16_t *)malloc(size * sizeof(short));
    bool enable_wn = true;     
    while (1) {
        if (enable_wn) {
            raw_stream_read(raw_read, (char *)buffer, size * sizeof(short));
            if (wakenet->detect(model_wn_data, (int16_t *)buffer) ==  WAKE_UP) {
                start_play_mp3();
                ESP_LOGI(TAG, "wake up");
                enable_wn = false;
            }
        } else {
            if (baidu_asr_url[0] != '\0') {     //得到token
                char *buff = (char *)heap_caps_malloc(96 * 1024, MALLOC_CAP_SPIRAM);
                if (NULL == buff) {
                    ESP_LOGE(TAG, "Memory allocation failed!");
                    return;
                }
                memset(buff, 0, 96 * 1024);

                for(size_t i = 0; i < 12; i++) {
                    raw_stream_read(raw_read, (char *)buff + i * 8 * 1024, 8 * 1024);
                }
                esp_http_client_config_t config = {
                    .url = baidu_asr_url,
                    .method = HTTP_METHOD_POST,
                };       
                esp_http_client_handle_t client = esp_http_client_init(&config);
                esp_http_client_set_header(client, "Content-Type", "audio/pcm;rate=16000");  
                esp_http_client_set_post_field(client, (const char *)buff, 96 * 1024);
                esp_err_t err = esp_http_client_perform(client);
                if (err == ESP_OK) {
                    ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                            esp_http_client_get_status_code(client),
                            esp_http_client_get_content_length(client));
                    int max_len = 1 * 1024;
                    char *data = (char *)heap_caps_malloc(max_len, MALLOC_CAP_SPIRAM);
                    int read_index = 0, total_len = 0;
                    int read_len = 0;
                    //得到返回的数据
                    while (1) {
                        read_len = esp_http_client_read(client, data + read_index, max_len - read_index);
                        if (read_len <= 0) {
                            break;
                        }
                        read_index += read_len;
                        total_len += read_len;
                        data[read_index] = 0;
                    }
                    ESP_LOGI(TAG, "%d%s", read_len, data);
                    free(data);
                    data = NULL;
                } 
                else {
                    ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
                }
                esp_http_client_cleanup(client);

                free(buff);
                enable_wn = true;
                buff = NULL;    
            }
        }
    }
    release_play_mp3();
    ESP_LOGI(TAG, "[ 6 ] Stop audio_pipeline");
  
    audio_pipeline_terminate(pipeline);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    audio_pipeline_unregister(pipeline, raw_read);
    audio_pipeline_unregister(pipeline, i2s_stream_reader);
    audio_pipeline_unregister(pipeline, filter);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(raw_read);
    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(filter);

    ESP_LOGI(TAG, "[ 7 ] Destroy model");
    wakenet->destroy(model_wn_data);
    model_wn_data = NULL;
    free(buffer);
    buffer = NULL;
}

