int ModifyCache(char *url);


int ReadCache(int csd, char *filename){

  FILE *infile;
  int MAX_LENGTH;

  infile = fopen(filename, "rb");
  if (infile == NULL){
    perror("Could not read cache!\n");
    return 0;
  }
  else {
    fseek(infile, 0, SEEK_END);
    MAX_LENGTH = ftell(infile);
    rewind(infile);

    char buf[MAX_LENGTH];
    
    if (fread(buf, 1, MAX_LENGTH, infile) != MAX_LENGTH){
      perror("Could not copy data from cache!\n");
      fclose(infile);
    }
    else {
      fclose(infile);
      if (send(csd, buf, sizeof(buf), 0) == -1){
        perror("Send ERROR(proxy to client)!\n");
        return 0;
      }
    }
  }
  
  return 1;
}

int WriteCache(char *filename, char *newbuf){
  FILE *outfile;

  outfile = fopen(filename, "ab+");
  if (outfile == NULL){
    perror("Could not read cache!\n");
    return 0;
  }
  else {
    char buf[strlen(newbuf)];
    memset(&buf, 0, sizeof(buf));
    memcpy(&buf, newbuf, strlen(newbuf));
    
    int temp = strlen(newbuf);
    buf[temp] = '\0';
    
    if (fputs(buf, outfile) == -1){
      return 0;
    }
    
    fclose(outfile);
  }
  
  return 1;
}

int Cache(int csd, char *url){

  FILE *infile;
  int MAX_LENGTH;

  infile = fopen("website", "ab+");
  if (infile == NULL){
    perror("Could not read cache!\n");
    return 0;
    
/*    char new[strlen(url)+1];*/
/*    memset(&new, 0, sizeof(new));*/
/*    memcpy(&new, url, strlen(url));*/
/*    new[strlen(url)]='\n';*/
/*    */
/*    if (fputs(new, infile) == -1) {*/
/*      return 0;*/
/*    }*/
  }
  else {
    fseek(infile, 0, SEEK_END);
    MAX_LENGTH = ftell(infile);
    rewind(infile);

    char buf[MAX_LENGTH];
    
    if (fread(buf, 1, MAX_LENGTH, infile) != MAX_LENGTH){
      perror("Could not copy data from cache!\n");
      fclose(infile);
    }
    else {
      fclose(infile);
      if (strstr(buf, url) != NULL) {//read from cache
        if (ReadCache(csd, url) != 1) {
          perror("send cache error!\n");
          return 0;
        }
      } else {// connect to web server
          char host[256];
          char page[256];
          char *get;
          char *tran = url;
          int k = 0;
          
          while (url[k] != '_'){
            k++;
            if (url[k] == '\0')
              break;
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
        }
    }
  }
  
  if (ModifyCache(url) != 1) {
    perror("ModifyCache() error!\n");
    return 0;
  }
  
  return 1;
}

int ModifyCache(char *url){

  FILE *cachefile;
  int MAX_LENGTH;
  int i = 0;
  char *temp;
  char newbuf[256];
  char *tran = &newbuf[0];

  cachefile = fopen("website", "ab+");
  if (cachefile == NULL){
    perror("Could not read cache!\n");
    return 0;
  }
  else {
    fseek(cachefile, 0, SEEK_END);
    MAX_LENGTH = ftell(cachefile);
    rewind(cachefile);

    char buf[MAX_LENGTH];
    
    if (fread(buf, 1, MAX_LENGTH, cachefile) != MAX_LENGTH){
      perror("Could not copy data from cache!\n");
      fclose(cachefile);
    }
    else {
      if ((temp = strstr(buf, url)) == NULL) {
        memset(&newbuf, 0, sizeof(newbuf));
        memcpy(&newbuf, url, strlen(url));
        newbuf[strlen(url)] = '\n';
        
        for (i=MAX_LENGTH; i >= 0; i--){
          if (buf[i] == '\n')
            break;
        }
        i++;
        
        tran = &newbuf[0];
        tran += strlen(url)+1;
        memcpy(tran, buf, i);
        fclose(cachefile);
        cachefile = fopen("website", "wb+");
        if (fputs(newbuf, cachefile) == -1) {
          return 0;
        }
        
        char *filename[256];
        memset(filename, 0, sizeof(filename));
        memcpy(filename, &buf[i+1], MAX_LENGTH-i);
        unlink(*filename);
      } else {
          memset(&newbuf, 0, sizeof(newbuf));
          memcpy(&newbuf, url, strlen(url));
          newbuf[strlen(url)] = '\n';
          
          tran = &newbuf[0];
          tran += strlen(url)+1;
          memcpy(tran, buf, (temp-&buf[0]));
          
          for (i=(temp-&buf[0]); i <= MAX_LENGTH; i++){
            if (buf[i] == '\n')
              break;
          }
          //i--;
          
          tran = &newbuf[0];
          tran += strlen(url)+1+(temp-&buf[0]);
          memcpy(tran, buf+i+1, MAX_LENGTH-i-1);
          
          fclose(cachefile);
          cachefile = fopen("website", "wb+");
          if (fputs(newbuf, cachefile) == -1) {
            return 0;
          }
        }
    }
    fclose(cachefile);
  }
  
  return 1;
}
