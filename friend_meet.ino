#define DECODE_NEC 
#include <IRremote.h>
const int MAX_FRIEND = 20; //최대로 만날 수 있는 친구
int Send_pin = 3; //송신기 3번 핀
int Recv_pin = 10; // 수신기 10번 핀
int Mode_F = 1; //친구 만나기 모드, 0:신청 1:수락
int met_friend[MAX_FRIEND]; //만났던 친구 저장하는 배열
boolean overlap = false;
int f_number = 0; //지금까지 만난 친구

void setup()
{
  Serial.begin(9600);
  IrSender.begin(Send_pin, ENABLE_LED_FEEDBACK); // 송신기 작동 시작
  IrReceiver.begin(Recv_pin, ENABLE_LED_FEEDBACK); // 수신기 작동 시작
  Serial.println("Start");
}

//comand 및 address 정의
uint16_t sAddress = 0x0002;
uint8_t conCom = 0x01;
uint8_t succCom = 0x02;
uint8_t disconCom = 0x03;
uint8_t cackCom = 0x04;
uint8_t sackCom = 0x05;
uint8_t dackCom = 0x06;
uint8_t Nack = 0x07;


//적외선 센서 송신 함수
void send_ir_data(uint8_t Command, uint8_t Repeats) {


//    Serial.print(F("Sending: 0x"));
//    Serial.print(sAddress, HEX);
//    Serial.print(Command, HEX);
//    Serial.println(Repeats, HEX);

    // clip repeats at 4
    if (Repeats > 4) {
        Repeats = 4;
    }
    IrSender.sendNEC(sAddress, Command, Repeats);
}


void meet(int mode){
  //친구 요청, mode 0
  if (mode == 0){
    
    //1. Connect command 전송   
    send_ir_data(conCom,2);
    
    int i = 0;
    do{      
      if(IrReceiver.decode()){ //적외선 신호 수신
        
        //내가 받아야할 신호인지 확인(connect에 대한 ack)
        if(IrReceiver.decodedIRData.address != sAddress && IrReceiver.decodedIRData.command == cackCom){
          met_friend[f_number] = IrReceiver.decodedIRData.address;
          f_number++;          
          Serial.println("Connect!!");
          IrReceiver.resume(); //수신기 다시 시작
          break;
        }
        else if(IrReceiver.decodedIRData.address != sAddress && IrReceiver.decodedIRData.command == metCom){
          Serial.println("You already met this friend today");
          overlap = true;
          return;
        }
        //내가 받아야할 신호가 아니면 다시 수신
        else{
          IrReceiver.resume();
        }
      }
      //connect command 다시 전송 
      if(i>20){
        Serial.println("over");
        send_ir_data(conCom,2);
        i=0;
      }
      i++;
      delay(500);
    }while(true);

    //delay를 주어 다음 신호 전송까지 시간 간격을 둠
    //너무 빨리 통신이 이루어지는 것 방지
    delay(10000);


    //2. Success command 전송
    send_ir_data(succCom,2);
    
    i = 0;
    do{
      if(IrReceiver.decode()){    
        if(IrReceiver.decodedIRData.address == met_friend[f_number-1] && IrReceiver.decodedIRData.command == sackCom){
          Serial.println("success!!");
          IrReceiver.resume();
          break;
        }
        else{
          IrReceiver.resume();
        }
      }
      if(i>20){
        Serial.println("over");
        send_ir_data(succCom,2);
        i=0;
      }
      i++;
      delay(500);
    }while(true);

    delay(1000);

    //3. Disconnect command 전송
    send_ir_data(disconCom,2);

    i = 0;
    do{
      if(IrReceiver.decode()){      
        if(IrReceiver.decodedIRData.address == met_friend[f_number-1] && IrReceiver.decodedIRData.command == dackCom){
          Serial.println("Disconnect!!");
          IrReceiver.resume();
          break;
        }
        else{
          IrReceiver.resume();
        }
      }
      if(i>20){
        Serial.println("over");
        send_ir_data(disconCom,2);
        i=0;
      }
      i++;
      delay(500);
    }while(true);
  }

  

  //친구 수락, mode 1
  else{
    int i=0;

    //Connect command 기다림
    do{      
      if(IrReceiver.decode()){        
        //내가 받아야할 신호인지 확인(Connect command)
        if(IrReceiver.decodedIRData.address != sAddress && IrReceiver.decodedIRData.command == conCom){
          //이미 만났던 친구인지 확인       
          for (int j = 0; j < MAX_FRIEND; i++) {
            if(IrReceiver.decodedIRData.address == met_friend[j]){
              overlap = true;
              for(int a=0;a<4;a++){
                send_ir_data(Nack,2);
                delay(200);
              }
              Serial.println("You already met this friend today");
              break;
            }
          }
          if(overlap==false){
            met_friend[f_number] = IrReceiver.decodedIRData.address;
            f_number++;
            //Connect ack 전송 이후 delay(10000)을 기다리기 때문에 너무 오랜 시간 기다리는 것 방지
            //이후 Delay(10000)
            for(int a=0;a<4;a++){
              send_ir_data(conCom,2);
              delay(200);
            }
            Serial.println("connect!!");
            IrReceiver.resume();
            break;
          }
        }
        else{
          IrReceiver.resume();
        }
      }
      delay(500);
    }while(true);

    Serial.println("delay start");
    delay(9000);


    //2. Success command 기다림
    do{    
       
      if(IrReceiver.decode()){
        
        if(IrReceiver.decodedIRData.address == met_friend[f_number-1] && IrReceiver.decodedIRData.command == succCom){
          send_ir_data(sackCom,2);
          Serial.println("success!!");
          IrReceiver.resume();
          break;
        }
        else{
          IrReceiver.resume();
        }
      }
      if(i>20){
        Serial.println("over");
        send_ir_data(cackCom,2);
        i=0;
      }
      i++;
      delay(500);
    }while(true);

    delay(1000);

    //3. Disconnect command 기다림
    i=0;  
    do{  
        
      if(IrReceiver.decode()){
        
        if(IrReceiver.decodedIRData.address == met_friend[f_number-1] && IrReceiver.decodedIRData.command == disconCom){
          send_ir_data(dackCom,4);
          Serial.println("disconnect!!");
          IrReceiver.resume();
          break;
        }
        else{
          IrReceiver.resume();
        }
      }
      if(i>20){
        Serial.println("over");
        send_ir_data(sackCom,1);
        i=0;
      }
      i++;
      delay(500);
    }while(true);
    i=0;
    while(i<5){
      send_ir_data(dackCom,2);
      i++;
    }
  }

  //친구 만나기 종료
  Serial.println("finish");
}

  
void loop() {
  meet(Mode_F);
  return;
}
