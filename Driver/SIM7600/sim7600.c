#include "sim7600.h"

//�жϻ��洮������
#define     UART_BUFF_SIZE      255
volatile    uint8_t uart_p = 0;
uint8_t     uart_buff[UART_BUFF_SIZE];

const char num[]=   "13232963434";          							//  �绰����
const char ip[]=    "207.246.97.145";									//  IP��ַ 207.246.97.145
const char port[]=  "33333";											//  �˿� 33333


void SIM7600_Init(void)
{   
    SIM7600_Reset_Init();
    
	sim900a_init();	                        //��ʼ�������ģ����Ӧ�Ƿ�����
	SIM900A_DELAY(2000);

    // ����
// 	sim900a_call((char *)num);
	
	server_connect((char *)ip,(char *)port);
	sim900a_gprs_send("TMThis_HMis_SMSa_LSstarting_WEus_SSOtest_SST!_HTtest_ARby_PMJM.Lin");

    // ���Ͷ���
// 	sim900a_sms((char *)num,"Hello World JM.Lin");
// 	PC_Usart((unsigned char*)"\r\n�����Ѿ���������%s\r\n",num);
    
}

void SIM7600_IRQHandler(void)
{
    if(uart_p<UART_BUFF_SIZE)       // �ж��Ƿ񻺴��Ƿ����
    {
        char ch;
        if(USART_GetITStatus(SIM7600_Usart_Port, USART_IT_RXNE) != RESET)
        {
            uart_buff[uart_p] = USART_ReceiveData(SIM7600_Usart_Port);
            ch = uart_buff[uart_p];
//            USART_SendData(PC_Usart_Port, ch);      // ת���� PC����
            uart_p++;     
        }
    }
}

static void SIM7600_Reset_Init(void)											// ������
{
	GPIO_InitTypeDef GPIO_Initstructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
	
	GPIO_Initstructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Initstructure.GPIO_Pin   = GPIO_Pin_8;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD,&GPIO_Initstructure);
		
    SIM7600_RST_HIGH_LEVEL();
    SIM900A_DELAY(50);
    SIM7600_RST_LOW_LEVEL();    
}

//��ȡ���յ������ݺͳ���
char *get_rebuff(uint8_t *len)
{
    *len = uart_p;
    return (char *)&uart_buff;
}

void clean_rebuff(void)
{
    uint16_t i=UART_BUFF_SIZE+1;
    uart_p = 0;
	while(i)
		uart_buff[--i]=0;
}

//0��ʾ�ɹ���1��ʾʧ��
uint8_t sim900a_cmd(char *cmd, char *reply,uint32_t waittime )
{    
    SIM900A_CLEAN_RX();                 //����˽��ջ���������

    SIM900A_TX(cmd);                    //��������

    if(reply == 0)                      //����Ҫ��������
    {
        return SIM900A_TRUE;
    }
    
    SIM900A_DELAY(waittime);                 //��ʱ

    return sim900a_cmd_check(reply);    //�Խ������ݽ��д���
}


//0��ʾ�ɹ���1��ʾʧ��
uint8_t sim900a_cmd_check(char *reply)
{
    uint8_t len;
    uint8_t n;
    uint8_t off;
    char *redata;
    
    redata = SIM900A_RX(len);   //��������
    n = 0;
    off = 0;
    while((n + off)<len)
    {
        if(reply[n] == 0)                 //����Ϊ�ջ��߱Ƚ����
        {
            return SIM900A_TRUE;
        }
        
        if(redata[ n + off]== reply[n])
        {
            n++;                //�ƶ�����һ����������
        }
        else
        {
            off++;              //������һ��ƥ��
            n=0;                //����
        }
    }

    if(reply[n]==0)   //�պ�ƥ�����
    {
        return SIM900A_TRUE;
    }
    
    return SIM900A_FALSE;       //����ѭ����ʾ�Ƚ���Ϻ�û����ͬ�����ݣ��������
}

char * sim900a_waitask(uint8_t waitask_hook(void))      //�ȴ�������Ӧ��
{
    uint8_t len=0;
    char *redata;
    do{
        redata = SIM900A_RX(len);   //��������
        if(waitask_hook!=0)
        {
            if(waitask_hook()==SIM900A_TRUE)           //���� SIM900A_TRUE ��ʾ��⵽�¼�����Ҫ�˳�
            {
                redata = 0;
                return redata;               
            }
        }
    }while(len==0);                 //��������Ϊ0ʱһֱ�ȴ�
    
    
    SIM900A_DELAY(20);              //��ʱ��ȷ���ܽ��յ�ȫ�����ݣ�115200�������£�ÿms�ܽ���11.52���ֽڣ�
    return redata;
}


//���ģ���Ƿ�����
void sim900a_init(void)
{
    PC_Usart((unsigned char*)"fuck \r\n");
	while(sim900a_cmd("AT\r","OK",100) != SIM900A_TRUE)
	{
		PC_Usart((unsigned char*)"AT_ERROR\r\n");
	}
	PC_Usart((unsigned char*)"AT_SUC\r\n");
	
	while(sim900a_cmd("AT+CPIN?\r","READY",400) != SIM900A_TRUE)
	{
		PC_Usart((unsigned char*)"CPIN_ERROR\r\n");
	}
	PC_Usart((unsigned char*)"CPIN_SUC\r\n");
	
	while(sim900a_cmd("AT+CREG?\r","0,1",400) != SIM900A_TRUE)
	{
		PC_Usart((unsigned char*)"CREG_ERROR\r\n");
	}
	PC_Usart((unsigned char*)"CREG_SUC\r\nSIM_OK\r\n\r\n");
}

/*---------------------------------------------------------------------*/

//���𲦴�绰
void sim900a_call(char *num)
{
    SIM900A_CLEAN_RX();                 //����˽��ջ���������
    SIM7600_Usart( (unsigned char*)"ATD%s;\r",num);
}

//���Ͷ���
void sim900a_sms(char *num,char *smstext)
{
	SIM900A_CLEAN_RX();                 //����˽��ջ���������
	
	SIM7600_Usart( (unsigned char*)"AT+CMGF=1\r");     //�ı�ģʽ
	SIM900A_DELAY(100);
	
	SIM7600_Usart( (unsigned char*)"AT+CSCS=\"GSM\"\r");           
	SIM900A_DELAY(100);
	
	SIM7600_Usart( (unsigned char*)"AT+CSCA?\r");           
	SIM900A_DELAY(100);
	
	SIM7600_Usart( (unsigned char*)"AT+CSMP=17,167,0,241\r");           
	SIM900A_DELAY(100);
	
	SIM7600_Usart( (unsigned char*)"AT+CMGS=\"%s\"\r",num);  //�绰����
	SIM900A_DELAY(100);

	SIM7600_Usart( (unsigned char*)"%s",smstext);            //��������        

    SIM900A_DELAY(1);   
	SIM7600_Usart_Port->DR=(u32)0x1A;		//����ʮ����������0X1A,��Ϣ��������
}

void sim900a_gprs_send(char * str)
{
	char *redata;
	SIM900A_CLEAN_RX();
	SIM7600_Usart( (unsigned char*)"AT+CIPSEND=0,\r");
  
    redata=sim900a_waitask(0);
	PC_Usart((unsigned char*)"%s\r\n",redata);
	
	SIM900A_CLEAN_RX();
    SIM7600_Usart( (unsigned char*)"%s",str);
	while((SIM7600_Usart_Port->SR&0X40)==0);//�ȴ���һ�����ݷ������  
	SIM7600_Usart_Port->DR=(u32)0x1A;		//����ʮ����������0X1A,��Ϣ��������
	
	redata=sim900a_waitask(0);	
	SIM900A_DELAY(1000);
	bufcheck();
	return;
}

void server_connect(char *ipaddr,char *port)
{
	char i;
	i=server_connect_tcp(ipaddr,port);
	while(i)
	{
		i=server_connect_tcp(ipaddr,port);
	}
	PC_Usart((unsigned char*)"connect_suc\r\n");
}

void bufcheck()
{
	char *redata;
    uint8_t len;  
    redata = SIM900A_RX(len);   //��������
 	PC_Usart((unsigned char*)"%s\r\n",redata);
}

char server_connect_tcp(char *ipaddr,char *port)
{
	SIM900A_CLEAN_RX();
	SIM7600_Usart( (unsigned char*)"AT+CGREG?\r");    
	SIM900A_DELAY(200);
	if(sim900a_cmd_check("0,1"))
	{
		PC_Usart((unsigned char*)"CGREG_ERROR\r\n");
		return 1;
	}
	
	SIM900A_CLEAN_RX();
	SIM7600_Usart( (unsigned char*)"AT+CGSOCKCONT=1,\"IP\",\"cmnet\"\r");    
	SIM900A_DELAY(200);
	if(sim900a_cmd_check("OK"))
	{
		PC_Usart((unsigned char*)"CGSOCKCONT_ERROR\r\n");
		return 2;
	}
	
	SIM900A_CLEAN_RX();
	SIM7600_Usart( (unsigned char*)"AT+CSOCKSETPN=1\r");    
	SIM900A_DELAY(600);
	if(sim900a_cmd_check("OK"))
	{
		PC_Usart((unsigned char*)"CSOCKSETPN_ERROR\r\n");
		return 3;
	}
	
	SIM900A_CLEAN_RX();
	SIM7600_Usart( (unsigned char*)"AT+NETOPEN\r");    
	SIM900A_DELAY(1000);
		bufcheck();
	if(sim900a_cmd_check("+NETOPEN: 0"))
	{
		bufcheck();
		SIM900A_CLEAN_RX();
		SIM7600_Usart( (unsigned char*)"AT+NETCLOSE\r"); 
		SIM900A_DELAY(1000);
		bufcheck();
		if(sim900a_cmd_check("+NETCLOSE: 0"))
		{
			bufcheck();
		}
		return 4;
	}
	
	SIM900A_CLEAN_RX();
	SIM7600_Usart( (unsigned char*)"AT+IPADDR\r");    
	SIM900A_DELAY(500);
	if(sim900a_cmd_check("OK"))
	{
		bufcheck();
		return 5;
	}
	
	SIM900A_CLEAN_RX();
	SIM7600_Usart( (unsigned char*)"AT+CIPOPEN=0,\"TCP\",\"%s\",%s\r",ipaddr,port);  
	SIM900A_DELAY(1000);
	if(sim900a_cmd_check("+CIPOPEN: 0,0"))
	{
		PC_Usart((unsigned char*)"CIPOPEN_ERROR\r\n");
		return 6;
	}
	return 0;
}

/*---------------------------------------------------------------------*/