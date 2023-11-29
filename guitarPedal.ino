class SoundModifier{
  private:
    struct WaveNode{
      int freq, amplitude;
      char comb_mode;
      float shift;
      char sine; // 1 - sine, 0 - cosine
      struct WaveNode* next;
    };

    struct WaveNode* head, last_node;
    unsigned int waves_count = 0, max_voltage, bottom_treshold;
    unsigned long start_time;

    unsigned int use_combine_method(char comb_mode, unsigned int voltage, unsigned int delta){
      switch(comb_mode){
        case 0:
          if(voltage + delta >= this->max_voltage){
            voltage = this->max_voltage - 1;
          } else {
            voltage += delta; // sum
          }
        break;
        case 1:
          if(voltage < delta){
            voltage = 0;
          } else {
            voltage -= delta; // substruct
          }
        break;
        case 2:
          if(voltage * delta >= this->max_voltage){
            voltage = this->max_voltage - 1;
          } else {
            voltage *= delta; // multiply
          }
        break;
        case 3:
          voltage = (voltage > delta) ? voltage: delta; // overlap
          if(voltage >= this->max_voltage) voltage = this->max_voltage;
        break;
      }

      return voltage;
    }
  public:
    void SoundModifier(unsigned char resolution_bits, unsigned int bottom_treshold = 10){
      if(resolution_bits > 32) resolution_bits = 32;
      analogReadResolution(resolution_bits);
      this->max_voltage = (unsigned int) pow(2, resolution_bits);

      this->bottom_treshold = bottom_treshold;

      *head = {0, 0, -1, 0, 0, NULL};
      this->lastnode = head;

      this->start_time = micros();
    }

    unsigned int combine(unsigned int voltage){
      struct WaveNode* buff = head;
      float delta;

      if(voltage < this->bottom_treshold){
        this->start_time = micros();
      } else{
        for(unsigned int i = 0; i < *(this->waves_count); i++){
          buff = head->next;
          if(buff->sine){
            delta = buff->amplitude * sin(2 * M_PI * buff->freq * (micros() - this->start_time) + buff->shift);
          } else{
            delta = buff->amplitude * cos(2 * M_PI * buff->freq * (micros() - this->start_time) + buff->shift);
          }
          voltage = use_combine_method(buff->comb_mode, voltage, (unsigned int) delta);
        }
      }

      return voltage;
    }

    unsigned int addWave(unsigned int freq, unsigned int amplitude, unsigned char comb_mode, float shift, char sine){ // -> id (position in waves array)
      struct WaveNode new_node = {freq, amplitude, comb_mode, shift, sine, NULL};
      this->last_node->next = &new_node;
      this->last_node = &new_node;

      this->waves_count ++;
      return this->waves_count - 1;
    }

    char removeWave(unsigned int id){
      struct WaveNode* buff = this->head;
      for(unsigned int i = 0; i < id - 1; i++){
        if(buff == NULL) return -1;
        buff = buff->next;
      }  

      struct WaveNode* node2del = buff->next;
      buff->next = buff->next->next;
      delete node2del;

      this->waves_count --;

      return 0;
    }
}

int analog_inp = A1, output = A2;
unsigned int vol;

void setup() {
  SoundModifier* sm = new SoundModifier(12, 10);
  
  pinMode(analog_pin, INPUT);
  pinMode(output, OUTPUT);

  //Here we need to add waves to cimbine into sm
}

void loop() {
  vol = analogRead(analog_inp);
  vol = sm->combine(vol);

  analogWrite(output, vol);
}
