void open_server_mac(char * port){
  char * string;
  asprintf(&string, "osascript -e \'tell application \"Terminal\" to do script \"nc -lk %s\"\'", port );
  system(string);
}
