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
  while(sent < strlen(get)){
    temp = send(csd, get + sent, strlen(get) - sent, 0);
    if (temp == -1){
      perror("Send ERROR!\n");
      exit(EXIT_FAILURE);
    }
    sent += temp;
  }
  return 1;
}

int initialize(int csd, char *buf){
  char host[256];
  char page[256];
  char *get;

  
  int k = 0;
  char *tran = &buf[0];
  
  if (strstr(buf, "http://") == NULL) {
    while(buf[k] != '/'){
      k++;
      if (buf[k] == '\0')
        break;
    }
  } else {
      while(buf[k+7] != '/'){
        k++;
        if (buf[k+7] == '\0')
          break;
      }
      tran += 7;
    }

  memset(host, 0, strlen(host));
  memcpy(host, tran, strlen(tran));
  host[k]='\0';

  memset(page, 0, strlen(page));
  page[0]='/';
  memcpy(page+1, tran+k+1, strlen(tran)-k);
  page[strlen(tran)-k] = '\0';

  get = build_get(host, page);
  fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);
  
  if (send_get(csd, get) != 1) {
    perror("Send ERROR!\n");
    exit(EXIT_FAILURE);
  }
  
  return 1;
}
