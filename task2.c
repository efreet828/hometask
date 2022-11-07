#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "uv.h"
#define DEFAULT_BACKLOG 128


int DEFAULT_PORT;
int suggested_size;
uv_loop_t *loop, *loop1;
struct sockaddr_in addr;
char buff[1600];
typedef struct {
 uv_write_t req;
 uv_buf_t buf;
} write_req_t;

typedef struct {
 uv_async_t async;
 uv_mutex_t mutex;
 uv_thread_t thread;
 uv_cond_t cond;
 } thread_comm_t;
thread_comm_t comm;

void invert(char *str)
{
 char c;
 int i,len,n;
 len=strlen(str);
 n=len/2;
 for(i=0;i<n;i++) {c=str[i]; str[i]=str[len-1-i]; str[len-1-i]=c;}
}

void printbuf() {
 printf("\n%s", buff);
}

void free_write_req(uv_write_t *req) {
 write_req_t *wr = (write_req_t*) req;
 //free(wr->buf.base);
 free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
 buf->base = (char*) malloc(suggested_size);
 buf->len = suggested_size;
 }

void echo_write(uv_write_t *req, int status) {
 if (status) {
 fprintf(stderr, "Write error %s\n", uv_strerror(status));
 }
 //free_write_req(req);
printbuf(&comm);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
 if (nread > 0) {
 strcpy(buff, buf->base);
 write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
 uv_async_send(&comm.async);
uv_cond_wait(&comm.cond, &comm.mutex);
  req->buf = uv_buf_init(buff, nread);
 uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
 return;
 }
 if (nread < 0) {
 if (nread != UV_EOF)
 fprintf(stderr, "Read error %s\n", uv_err_name(nread));
 uv_close((uv_handle_t*) client, NULL);
 }
 free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status) {
 if (status < 0) {
 fprintf(stderr, "New connection error %s\n", uv_strerror(status));
 return;
 }
 uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
 uv_tcp_init(loop1, client);
 if (uv_accept(server, (uv_stream_t*) client) == 0) {
 uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
 }
 else {
 uv_close((uv_handle_t*) client, NULL);
 }
 }

 void async_cb(void* arg) {
   printf("inverted \n" );
   uv_mutex_lock(&comm.mutex);
  invert(buff);
  uv_cond_signal(&comm.cond);
  uv_mutex_unlock(&comm.mutex);
  printf("%s\n", buff );
  }

void thread_cb(void* arg) {
 loop1 = uv_loop_new();
 uv_loop_init(loop1);
 uv_tcp_t server;
 uv_tcp_init(loop1, &server);
 uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);
 uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
 int r = uv_listen((uv_stream_t*) &server, DEFAULT_BACKLOG, on_new_connection);
 if (r) {
 fprintf(stderr, "Listen error %s\n", uv_strerror(r));
 return;
 }
 uv_run(loop1, UV_RUN_DEFAULT);
 }
 
void recieve_signal(void* arg){
 invert(buff);
 }

int main(int argc, char* argv[]) {
  int r;
 int a;
 char *p;
 long conv = strtol(argv[1], &p, 10);
 DEFAULT_PORT=conv;
 memset(&comm, 0, sizeof(comm));
 r = uv_thread_create(&comm.thread, thread_cb, (void*) &comm);
 assert(r == 0);
 
 r = uv_async_init(uv_default_loop(), &comm.async, async_cb);
 assert(r == 0);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);


}
