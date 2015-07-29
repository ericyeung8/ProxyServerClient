int Cache(int csd, char *url);
int WriteCache(char *filename, char *newbuf);

char *build_get(char *host, char *page){
  char *query;
  char *getpage = page;
//  char *temp = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
  char *temp = "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n";
  if(getpage[0] == '/'){
    getpage = getpage + 1;
    fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
  }
  // -5 is to consider the %s %s %s in temp and the ending \0
//  query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(temp)-5);
  query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(temp)-4);
//  sprintf(query, temp, getpage, host, USERAGENT);
  sprintf(query, temp, getpage, host);
  return query;
}

int send_get(int csd, char *get){
  int temp = 0;
  int sent = 0;
  int ssd = 0;
  int port = 0;
  int n_bytes = 0;
  char buf[4096];
  struct sockaddr_in addr;
  struct hostent *hret;
  
  char host[256];
  char page[256];
  char *temp1;
  char *temp2;
  
  temp1 = strstr(get, "Host: ");
  temp2 = strstr(get, "\r\n\r\n");
  temp1 += 6;
  memset(&host, 0, sizeof(host));
  memcpy(&host, temp1, (int)(temp2-temp1));
  
  temp1 = strstr(get, "GET /");
  temp2 = strstr(get, " HTTP/");
  temp1 += 4;
  memset(page, 0, strlen(page));
  //page[0]='/';
  memcpy(&page, temp1, temp2-temp1);
  page[temp2-temp1] = '\0';
  
  char url[256];
  char *tran = &url[0];
  tran += strlen(host);
  
  memset(&url, 0, sizeof(url));
  memcpy(&url, &host, strlen(host));
  
  memcpy(tran, &page, strlen(page));
  url[strlen(host)] = '_';
  
  if ((ssd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("Cannot create server socket");
    exit(EXIT_FAILURE);
  }

  port = PORT;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  
  if ((hret = gethostbyname((const char*)host)) == NULL){
    perror("Cannot get server's address");
    exit(EXIT_FAILURE);
  }

  memcpy(&addr.sin_addr.s_addr, hret->h_addr, hret->h_length);
  
  if (connect(ssd,(struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1){
    perror("Cannot connect to Server");
    exit(EXIT_FAILURE);
  }
  
  printf("connect to web server!\n");
  while(sent < strlen(get)){
    temp = send(ssd, get + sent, strlen(get) - sent, 0);
    if (temp == -1){
      perror("Send ERROR!\n");
      exit(EXIT_FAILURE);
    }
    sent += temp;
  }
  
  printf("Receive data from web server!\n");
  while ((n_bytes = recv(ssd, buf, sizeof(buf), 0)) > 0){
  
    if (n_bytes < 0) {
      perror("Receive ERROR!\n");
      return 0;
    }
    
    if (WriteCache(url, buf) != 1){
      perror("WriteCache() error!\n");
      return 0;
    }
    
    if (send(csd, buf, sizeof(buf), 0) == -1){
      perror("Send ERROR(proxy to client)!\n");
      return 0;
    }
    
    memset(buf, 0, sizeof(buf));
  }
  printf("Receiving completed and sent to client!\n");
  
  close(ssd);
  return 1;
}

int HandleMsg(int csd, char *buf){
  if (buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T') {
    printf("GET received from client!\n");
  
    char host[256];
    char page[256];
    char *temp1;
    char *temp2;
    char url[256];
    char *tran = &url[0];
  
    temp1 = strstr(buf, "Host: ");
    temp2 = strstr(buf, "\r\n\r\n");
    temp1 += 6;
    memset(&host, 0, sizeof(host));
    memcpy(&host, temp1, (int)(temp2-temp1));
  
    temp1 = strstr(buf, "GET /");
    temp2 = strstr(buf, " HTTP/");
    temp1 += 4;
    memset(page, 0, strlen(page));
    memcpy(&page, temp1, temp2-temp1);
    page[temp2-temp1] = '\0';
    
    tran += strlen(host);
    
    memset(&url, 0, strlen(url));
    memcpy(&url, &host, strlen(host));
    memcpy(tran, &page, strlen(page)+1);
    url[strlen(host)] = '_';
    url[strlen(host)+strlen(page)] = '\0';
    
    if (Cache(csd, url) != 1) {
      perror("Cache error!\n");
      return 0;
    }
  } else return 0;
  return 1;
}
