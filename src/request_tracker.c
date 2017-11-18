#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <math.h>
#include "buffer.h"
#include "bencode.h"
#include "dico_finder.h"
#include "request_tracker.h"
#include "debug.h"
#include "decode_binary_peers.h"
#include "socket_init.h"
#include "client.h"

static char *
build_tracker_uri(struct be_node *dico, CURL *curl)
{
  char *urn = dico_find_str(dico, "announce");
  char *e_info_hash = curl_easy_escape(curl, g_bt.info_hash, 20);

  char *port = calloc(6, sizeof(char));
  sprintf(port, "%u", get_port());

  debug("listening on port %s", port);

  char *format = "%s?peer_id=%s&info_hash=%s&port=%s&left=0&downloaded=0&"
                 "uploaded=0&compact=1";

  long long len = strlen(format) + strlen(urn) + strlen(port) + 36;
  char *uri = calloc(len, sizeof(char));
  if (!uri)
    return NULL;

  sprintf(uri, format, urn, g_bt.peer_id, e_info_hash, port);

  free(port);
  curl_free(e_info_hash);
  return uri;
}

static size_t
write_callback(char *ptr, size_t size, size_t nmemb, s_buf **userdata)
{
  char *data = calloc(nmemb + 1, size);
  memcpy(data, ptr, size * nmemb);
  *userdata = buffer_init(data, size * nmemb);
  return size * nmemb;
}

static CURL *
build_curl_request(struct be_node *dico, s_buf **data)
{
  debug("curl initialization");
  CURL *curl = curl_easy_init();
  if (!curl || init_socket() < 0)
    return NULL;

  char *uri = build_tracker_uri(dico, curl);
  if (!uri)
    return NULL;

  debug("curl prepared uri: '%s'", uri);
  curl_easy_setopt(curl, CURLOPT_URL, uri);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);

  free(uri);

  return curl;
}

struct be_node *
get_peer_list(struct be_node *dico)
{
  s_buf *data = NULL;
  struct be_node *peer_list = NULL;

  CURL *curl = build_curl_request(dico, &data);
  if (!curl)
    return NULL;

  debug("performming curl request");

  CURLcode res = curl_easy_perform(curl);
  debug("curl request is resolved with code %d", res);

  if (res == CURLE_OK)
  {
    debug("raw answer form server: '%s'", data->str);
    peer_list = bencode_decode(data->str, data->len);
    buffer_free(data);
  }
  curl_easy_cleanup(curl);

  decode_binary_peers(dico_find(peer_list, "peers"));
  return peer_list;
}
