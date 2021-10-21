

/*
Exemplo de implementação para guiar o trabalho da M2
São implementados os protótipos de exemplo de cada função
A implementação de cada algoritmo é de resposabilidade de cada grupo/aluno
São usados conceitos básicos de implementação vistos em qualquer disciplina de algoritmos
A estrutura do código pode ser modificada como for conveniente
*/


#include <iostream>

using namespace std;
#include <vector>
#include <time.h>
#include <Windows.h>

enum estado_sender_fsm_t {send_and_wait0,send_and_wait1} estado_sender;
enum estado_receiver_fsm_t {wait0,wait1} estado_receiver;
clock_t start_timer;
clock_t end_timer;
int delay = 0;
bool counting_timer;
int packet_loss_sender = 0;
int packet_loss_receiver = 0;

void init_app_layer_A()
{
    estado_sender = send_and_wait0;
}
void init_app_layer_B()
{
    estado_receiver = wait0;
}
//Define um exemplo de mensagem gerada pela aplicacaoo

char app_ack[5] = {'i','s','a','c','k'};

//Estrutura basica para um pacote
typedef struct rtp_packet_t
{
    int seqnum;
    int acknum;
    char checksum;
    char payload[5]; //pode implementar como "char payload[20]"
} rtp_packet_t;


//Camada de aplicacao B
void app_layer_B(char *payload)
{
    int i;
    char *data = payload;
    printf("\npayload:\n");
    for(i=0; i<5; i++)
    {
        printf("%c ",payload[i]);
    }
    printf("\nmensagem enviada ao receiver com sucesso!");
}

char checksum(char* payload)
{

    vector<int> resultado = { 0,0,0,0,0,0,0,0 };
    vector<int> complemento = { 0,0,0,0,0,0,0,0 };
    int carry = 0;

    for (int index = 0; index < 5; index++)
    {
        char c = payload[index];
        int i;
        vector<int> valores;

        for (i = 7; i >= 0; i--)
        {

            valores.push_back((unsigned int)((c & (1 << i)) ? 1 : 0));

        }


        for (int i = 7; i >= 0; i--)
        {
            resultado[i] = resultado[i] + valores[i] + carry;
            if (resultado[i] == 2)
            {
                carry = 1;
                resultado[i] = 0;
            }
            else if (resultado[i] == 3)
            {
                carry = 1;
                resultado[i] = 1;
            }
            else
            {
                carry = 0;
            }

            if (i == 0 && carry == 1)
            {
                carry = 0;
                vector<int> auxiliar = { 0,0,0,0,0,0,0,1 };

                for (int index = 7; index >= 0; index--)
                {
                    resultado[index] = resultado[index] + auxiliar[index] + carry;

                    if (resultado[index] == 2)
                    {
                        carry = 1;
                        resultado[index] = 0;

                    }
                    else if (resultado[index] == 3)
                    {
                        carry = 1;
                        resultado[index] = 1;
                    }
                    else
                    {
                        carry = 0;
                    }
                }
            }
        }
    }

    char checksum;

    checksum = resultado[0] << 7 | resultado[1] << 6 | resultado[2] << 5 | resultado[3] << 4 |
               resultado[4] << 3 | resultado[5] << 2 | resultado[6] << 1 | resultado[7];

    return checksum;

}

char complemento_checksum(char* payload)
{

    vector<int> resultado = { 0,0,0,0,0,0,0,0 };
    vector<int> complemento = { 0,0,0,0,0,0,0,0 };
    int carry = 0;

    for (int index = 0; index < 5; index++)
    {
        char c = payload[index];
        int i;
        vector<int> valores;

        for (i = 7; i >= 0; i--)
        {

            valores.push_back((unsigned int)((c & (1 << i)) ? 1 : 0));

        }

        for (int i = 7; i >= 0; i--)
        {
            resultado[i] = resultado[i] + valores[i] + carry;
            if (resultado[i] == 2)
            {
                carry = 1;
                resultado[i] = 0;
            }
            else if (resultado[i] == 3)
            {
                carry = 1;
                resultado[i] = 1;
            }
            else
            {
                carry = 0;
            }

            if (i == 0 && carry == 1)
            {
                carry = 0;
                vector<int> auxiliar = { 0,0,0,0,0,0,0,1 };

                for (int index = 7; index >= 0; index--)
                {
                    resultado[index] = resultado[index] + auxiliar[index] + carry;

                    if (resultado[index] == 2)
                    {
                        carry = 1;
                        resultado[index] = 0;

                    }
                    else if (resultado[index] == 3)
                    {
                        carry = 1;
                        resultado[index] = 1;
                    }
                    else
                    {
                        carry = 0;
                    }
                }
            }
        }
    }

    //complemento
    for (int i = 0; i < resultado.size(); i++)
    {
        if (resultado[i] == 1)
        {
            complemento[i] = 0;
        }
        else
        {
            complemento[i] = 1;
        }
    }

    char checksum;

    checksum = complemento[0] << 7 | complemento[1] << 6 | complemento[2] << 5 | complemento [3] << 4 |
               complemento [4] << 3 | complemento [5] << 2 | complemento [6] << 1 | complemento[7];

    return checksum;

}

rtp_packet_t make_pkt(int seqnum,int acknum,char *data,char checksum)
{
    char payload[5];
    int i;
    for(i=0; i<5; i++)
    {
        payload[i] = data[i];
    }

    rtp_packet_t packet;

    packet.acknum = acknum;
    packet.seqnum = seqnum;
    for(i=0; i<5; i++)
    {
        packet.payload[i] = payload[i];
    }
    packet.checksum = checksum;
    return packet;
}
int notcorrupt(rtp_packet_t packet)
{
    char complemento_checksum = packet.checksum;
    char check = checksum(packet.payload);
    char checksum_xor = check^complemento_checksum;
    int i;
    vector<int> valores;

    for (i = 7; i >= 0; i--)
    {
        if(((checksum_xor & (1 << i)) ? 1 : 0)==0)
        {
            return 0;
        }
    }
    return 1;
}
bool timeout()
{
    //TIMEOUT DE 1 SEGUNDOS
    end_timer = clock();
    if((double)(end_timer - start_timer) / CLOCKS_PER_SEC>1)
    {
        printf("timeout!");
        counting_timer=false;
        return 1;
    }
    else
    {
        return 0;
    }
}
rtp_packet_t rdt_rcv(rtp_packet_t packet)
{
    printf("\nO pacote chegou na camada de transporte do destinatario!\n");
    switch(estado_receiver)
    {
    case wait0:
        printf("\nestado receiver:wait0\n");
        if(notcorrupt(packet)&&packet.seqnum==0)
        {
            app_layer_B(packet.payload);//deliver_data to receiver application_layer.
            rtp_packet_t sndpkt = make_pkt(0,0,app_ack,complemento_checksum(app_ack));
            estado_receiver = wait1;
            return sndpkt;
        }
        else if(notcorrupt(packet)&&packet.seqnum==1)
        {
            printf("reenviando ack");
            rtp_packet_t sndpkt = make_pkt(0,1,app_ack,complemento_checksum(app_ack));
            return sndpkt;
        }
        else
        {
            printf("\n ACK errado...\n");
            rtp_packet_t sndpkt = make_pkt(0,1,app_ack,complemento_checksum(app_ack));
            estado_receiver = wait0;
            return sndpkt;
        }
        break;
    case wait1:
        printf("\nestado receiver:wait0\n");
        if(notcorrupt(packet) && packet.seqnum==1)
        {
            app_layer_B(packet.payload);//deliver_data to receiver application_layer.
            rtp_packet_t sndpkt = make_pkt(1,1,app_ack,complemento_checksum(app_ack));
            estado_receiver = wait0;
            return sndpkt;
        }
        else if(notcorrupt(packet)&&packet.seqnum==0)
        {
            printf("reenviando ack");
            rtp_packet_t sndpkt = make_pkt(1,0,app_ack,complemento_checksum(app_ack));
            return sndpkt;
        }
        else
        {
            printf("\n ACK errado...\n");
            rtp_packet_t sndpkt = make_pkt(1,0,app_ack,complemento_checksum(app_ack));
            estado_receiver = wait1;
            return sndpkt;
        }
        break;
    }
}

//Camada de transporte
rtp_packet_t udt_send(rtp_packet_t packet)
{
    printf("\nPassando pela camada de rede...\n");
    int meio = rand()%4;

    rtp_packet_t rcvpkt;
    if(meio==0)
    {
        printf("\nO pacote passou normalmente pela camada de rede\n");
        //normal
        //nada ocorre
        packet_loss_sender = 0;
        rcvpkt = rdt_rcv(packet);
    }
    else if(meio==1)
    {
        printf("\nO pacote teve o dado corrompido\n");
        //com erro
        //corrompimento do dado
        packet.payload[3] = 'h';
        packet_loss_sender = 0;
        rcvpkt = rdt_rcv(packet);

    }
    else
    {
        //nao envio
        //rcvpkt = rdt_rcv(packet);
        printf("\nO pacote foi perdido na camada de rede\n");
        packet_loss_sender = 1;
    }

    if(packet_loss_sender==0)
    {
        printf("\nPassando pela camada de rede novamente...\n");
        meio = rand()%4;

        if(meio==0)
        {
            printf("\nO pacote(ack/nack) passou normalmente pela camada de rede\n");
            //normal
            //nada ocorre
            packet_loss_receiver = 0;
            delay = 0;
        }
        else if(meio==1)
        {
            //com erro
            //corrompimento do dado
            printf("\nO pacote teve o dado(ack/nack) corrompido\n");
            rcvpkt.payload[3]= 'h';
            packet_loss_receiver = 0;
            delay = 0;
        }
        else if(meio==2)
        {
            printf("\nO pacote(ack/nack) foi enviado com atraso\n");
            //com atraso
            packet_loss_receiver = 0;
            delay = 1;
        }
        else
        {
            printf("\nO pacote(ack/nack) foi perdido na camada de rede\n");
            //nao envio
            packet_loss_receiver = 1;
            delay = 0;
        }
    }
    return rcvpkt;
}

int isAck(rtp_packet_t packet)
{
    if(packet.payload[0]=='i'&&packet.payload[1]=='s'&&packet.payload[2]=='a'&&packet.payload[3]=='c'&&packet.payload[4]=='k')
    {
        return 1;
    }
    return 0;
}
void rdt_send(char *data)
{
    rtp_packet_t sndpkt;
    rtp_packet_t rcvpkt;
    do
    {
        switch(estado_sender)
        {
        case send_and_wait0:
            if(packet_loss_sender==0&&packet_loss_receiver==0||timeout())
            {
                printf("\nSender estado send:0\n");
                sndpkt = make_pkt(0,0,data,complemento_checksum(data));
                start_timer = clock();
                counting_timer = true;
                printf("\nSender enviando pacote...\n");
                printf("\nSender estado wait:0\n");
                rcvpkt = udt_send(sndpkt);
                if(packet_loss_sender==0&&packet_loss_receiver==0&&delay==0)
                {
                    //ACK0, OK
                    if(notcorrupt(rcvpkt) && isAck(rcvpkt)==1 && rcvpkt.acknum==0)
                    {
                        estado_sender = send_and_wait1;
                    }
                    else
                    {
                        printf("\nreenviando\n");
                        estado_sender = send_and_wait0;
                        rdt_send(data);
                    }
                }
                else if(delay==1)
                {
                    printf("\nOcorreu delay\n");
                    Sleep(1000);
                    rdt_send(data);
                    //ignora o recebimento do rcvpkt e prepara para reenviar esse pacote "atrasado"
                }
                else
                {

                    //perda de pacote...
                    printf("\nOcorreu perda de pacote\n");
                    //exit(0);
                }
            }
            else
            {
                //perda de pacote...
                printf("\nOcorreu perda de pacote\n");
                //exit(0);
            }
            break;
        case send_and_wait1:
            if(packet_loss_sender==0&&packet_loss_receiver==0||timeout())
            {
                printf("\nSender estado send:1\n");
                sndpkt = make_pkt(1,1,data,complemento_checksum(data));
                start_timer = clock();
                counting_timer = true;
                printf("\nSender estado wait:1\n");
                rcvpkt = udt_send(sndpkt);
                if(packet_loss_sender==0&&packet_loss_receiver==0&&delay==0)
                {
                    printf("\nPacote(ack/nack) chegou normalmente ao sender\n");
                    //ACK1, OK
                    if(notcorrupt(rcvpkt) && isAck(rcvpkt)==1 && rcvpkt.acknum==1)
                    {
                        estado_sender = send_and_wait0;
                    }
                    //ACK0 || CORRUPT, NAO OK
                    else
                    {
                        printf("\nreenviando\n");
                        estado_sender = send_and_wait1;
                        rdt_send(data);
                    }
                }
                else if(delay==1)
                {
                    printf("\nOcorreu delay\n");
                    Sleep(1000);
                    rdt_send(data);
                    //ignora o recebimento do rcvpkt e prepara para reenviar esse pacote "atrasado"
                }
                else
                {
                    //perda de pacote...
                    printf("\nOcorreu perda de pacote ou delay\n");
                    //exit(0);
                }
            }
            else
            {
                //perda de pacote...
                printf("\nOcorreu perda de pacote\n");
                //exit(0);
            }
            break;
        }
    }
    while(packet_loss_sender==1||packet_loss_receiver==1||delay==1);
}


//Camada de aplicacao A
void app_layer_A()
{
    char data[5] = {'a','b','c','d','e'};
    printf("\nCamada de aplicacao chamou rdt_send();\n");
    rdt_send(data);
}

int main()
{
    srand(time(NULL));
    init_app_layer_A();
    init_app_layer_B();
  //  for(int i=0; i<10; i++)
    //{
        //pseudo-fila de pacotes
        app_layer_A();
        printf("\n\n\n\n");
        Sleep(1000);
    //}
    return 0;
}
