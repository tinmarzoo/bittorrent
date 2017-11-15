#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include "peer_id.h"
#include "request_tracker.h"

/**
 * 117 beeing the length of peer_id + info_hash + port added to all the
 * nescessary keywords related to the GET post
*/
#define C_SIZE 118

static char *
request_create(struct be_node *dico)
{
  char *peer_id = peer_id_generate();
  char *announce = dico_get_str(dico, "announce");
  char *info_hash = dico_get_str(dico, "info_hash");
  char *port = dico_get_str(dico, "port");
  char *bytes_left = bytes_left_get();
  char *bytes_dwl = bytes_dwl_get();
  char *bytes_upl = bytes_upl_get();
  size_t req_len = C_SIZE + strlen(bytes_left) + strlen(bytes_dwl)
                          + strlen(bytes_upl) + strlen(announce);

  char *request = calloc(req_len, sizeof(char));

  sprintf(request, "%s?peer_id=%s&info_hash=%s&port=%s&left=%s&downloaded=%s&"
                   "uploaded=%s&compact=1",
                   announce, peer_id, info_hash, port, bytes_left, bytes_dwl,
                   bytes_upl);
  return request;
}

static size_t
write_callback(char *ptr, size_t size, size_t nmemb, char **userdata)
{
  *userdata = calloc(nmemb, size);
  memcpy(*userdata, ptr, 6);
  return size * nmemb;
}

struct be_dico *
peer_list_get(void)
{
  CURL *curl;
  CURLcode res;

  char *request = request_create(dico); //don't know were to get that
  char *answer = NULL;

  curl = curl_easy_init();

  if (!curl)
    return NULL;

  curl_easy_setopt(curl, CURLOPT_URL, request);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &answer);

  curl_easy_perform(curl);
  free(request);

  struct be_dico *peer_list = bencode_decode(&answer, strlen(answer));
  free(answer);

  return peer_list;
}
