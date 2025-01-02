#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "protocol_examples_utils.h"
#include "esp_tls.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#if !CONFIG_IDF_TARGET_LINUX
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#endif

#include "esp_http_client.h"
#include "http_client.h"

#include "cJSON.h"

#include "lvgl.h"
#include "gui_guider.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
static const char *TAG = "HTTP_CLIENT";

static int rx_data_len = 0;
static int prog_progress = 0;
static int prog_status = PROG_STATUS_IDLE;

int get_rx_data_len(void)
{
    return rx_data_len;
}

int get_prog_progress(void)
{
    return prog_progress;
}

prog_status_t get_prog_status(void)
{
    return prog_status;
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            rx_data_len = evt->data_len;
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;
                if (evt->user_data) {
                    copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                    if (copy_len) {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }
                } else {
                    const int buffer_len = esp_http_client_get_content_length(evt->client);
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(buffer_len);
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    copy_len = MIN(evt->data_len, (buffer_len - output_len));
                    if (copy_len) {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                }
                output_len += copy_len;
            }
            else {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;
                if (evt->user_data) {
                    copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                    if (copy_len) {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }
                } else {
                    const int buffer_len = esp_http_client_get_content_length(evt->client);
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(buffer_len);
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    copy_len = MIN(evt->data_len, (buffer_len - output_len));
                    if (copy_len) {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                }
                output_len += copy_len;                
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            esp_http_client_set_redirection(evt->client);
            break;
    }
    return ESP_OK;
}

void update_prog_progress_and_status(void)
{
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    /**
     * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
     * If host and path parameters are not set, query parameter will be ignored. In such cases,
     * query parameter should be specified in URL.
     *
     * If URL as well as host and path parameters are specified, values of host and path will be considered.
     */
    esp_http_client_config_t config = {
        .host = "127.0.0.1",
        .path = "/api/query",
        .query = "type=program-status",
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,        // Pass address of local buffer to get response
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client), rx_data_len);
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    ESP_LOGI(TAG, "return:%s", local_response_buffer);
    cJSON *root = NULL;
    root = cJSON_Parse(local_response_buffer);
    if (root) {
        cJSON *p = cJSON_GetObjectItem(root, "progress");
        if (p) {
            ESP_LOGI(TAG, "progress:%d", p->valueint);
            prog_progress = p->valueint;
        }
        cJSON *s = cJSON_GetObjectItem(root, "status");
        if (s) {
            ESP_LOGI(TAG, "status:%s", s->valuestring);
            if (strcmp(s->valuestring, "idle") == 0) {
                prog_status = PROG_STATUS_IDLE;
            }
            else if (strcmp(s->valuestring, "busy") == 0) {
                prog_status = PROG_STATUS_BUSY;
            }
        }
    }
    // ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));


    esp_http_client_cleanup(client);
}

void start_swd_flash(void)
{
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   // Buffer to store response of http request
    int content_length = 0;
    esp_http_client_config_t config = {
        .url = "http://192.168.4.1/program",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    char temp[100] = {0};
    char hex_or_bin[50] = {0};
    char alg[100] = {0};
    char post_data[1024] = {0};

    lv_dropdown_get_selected_str(guider_ui.screen_ddlist_2, temp, 100);
    lv_dropdown_get_selected_str(guider_ui.screen_ddlist_1, alg, 100);

    size_t len = strlen(temp);
    if (strncmp(&temp[len-3], "hex", 3) == 0) {
        snprintf(hex_or_bin, 50, "hex");
    }
    else if (strncmp(&temp[len-3], "bin", 3) == 0) {
        snprintf(hex_or_bin, 50, "bin");
    }
    else {
        snprintf(hex_or_bin, 50, "hex");
    }

    snprintf(post_data, 1024, 
    "{\"program_mode\":\"offline\",\"format\":\"%s\",\"total_size\":0,\"flash_addr\":134217728,\"ram_addr\":536870912,\"algorithm\":\"%s\",\"program\":\"%s\"}",
    hex_or_bin, alg, temp);    

    ESP_LOGI(TAG, "%s", post_data);

    esp_http_client_set_url(client, "http://192.168.4.1/program");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");    
    esp_err_t err = esp_http_client_open(client, strlen(post_data));

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) {
                ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %"PRIu64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
                ESP_LOG_BUFFER_HEX(TAG, output_buffer, strlen(output_buffer));
            } else {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }
    esp_http_client_cleanup(client);
}