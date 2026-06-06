
int buffer[4096];

void init_buffer(){
    for(int i=0;i<4096;i++){
        buffer[i] = i * 31;
    }
}
